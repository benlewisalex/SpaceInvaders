#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "buttons.h"			// functions to access buttons
#include "fit.h"			// declares MS_PER_TICK
#include "intc.h"			// functions to access interrupt controller
#include "switches.h"			// functions to access switches
#include "utils.h"			// helper functions and macros

#define INTC_FIT_MASK	0x00000001
#define INTC_BTNS_MASK	0x00000002
#define INTC_SW_MASK	0x00000004

#define MAX_HOURS	23
#define MAX_MINUTES	59
#define MAX_SECONDS	59

/*                    1000 ms/sec */
/* ticks/sec given as ----------- */
/*                     10 ms/tick */
#define TICKS_PER_SEC	(1000 / MS_PER_TICK)

/* defines auto-increment speed */
#define TICKS_PER_INCREMENT	(TICKS_PER_SEC/2)

/* what interrupts should be enabled */
#define INTC_MASK		(INTC_FIT_MASK|INTC_BTNS_MASK|INTC_SW_MASK)

/* index for switch 0 */
#define SW0	0

#define SECONDS		0		// index of seconds field in clock array
#define MINUTES		1		// index of minutes field
#define HOURS		2		// index of hours field
#define CLOCK_BYTES	(HOURS+1)	// length of clock index

uint8_t clock[CLOCK_BYTES];		// 3 bytes to hold current time
const uint8_t max[CLOCK_BYTES] = {MAX_SECONDS, MAX_MINUTES, MAX_HOURS};	// max size of each field
uint32_t ticks = TICKS_PER_SEC;		// when this hits zero, the clock will advance a second

uint8_t buttons_pressed;		// keeps track of how many buttons are down
uint8_t current_button;			// if buttons_pressed == 1, which is it?

/* perform cleanup */
void sigint_handler(int signum)
{
	putchar('\n');			// print newline

	switches_disable();		// disable switches
	buttons_disable();		// disable buttons
	intc_irq_disable(INTC_MASK);	// disable interrupt controller

	switches_exit();		// shut down switches
	buttons_exit();			// shut down buttons
	intc_exit();			// shut down interrupt controller

	exit(EXIT_SUCCESS);		// terminate process
}

void print_clock()
{
	/* print clock */
	printf("\r%02d:%02d:%02d", clock[HOURS], clock[MINUTES], clock[SECONDS]);

	/* REALLY print clock */
	fflush(stdout);
}

/* increment a field in the clock */
bool increment(uint8_t index)
{
	/* see if a field is at its max value */
	if (clock[index] == max[index]) {
		clock[index] = 0;	// if so, reset to 0
		return true;		// true = overflow
	} else {
		++clock[index];		// increment normally
		return false;		// no overflow
	}
}

/* decrement a field in the clock */
bool decrement(uint8_t index)
{
	/* see if the field has hit zero */
	if (clock[index]) {
		--(clock[index]);	// if not, decrement normally
		return false;		// no overflow
	} else {
		clock[index] = max[index];	// reset to max value
		return true;		// overflow
	}
}

/* ISR for FIT hardware timer */
void fit_isr()
{
	int i;	// counter variable
	buttons_tick();			// call buttons tick function
	switches_tick();		// call switches tick function

	/* if no buttons are pressed */
	if ( !buttons_pressed ) {
		/* decrement */
		if ( !(--ticks) ) {
			/* when the tick count reaches 0 */
			/* increment seconds; if it overflows, increment minutes, etc */
			for (i = 0; i < CLOCK_BYTES; ++i) {
				if (!increment(i))
					break;		// break out if no overflow
			}

			print_clock();			// re-print clock
			ticks = TICKS_PER_SEC;		// reset ticks value
		}
	/* if exactly 1 button is pressed */
	} else if (buttons_pressed == 1) {
		/* ticks used here for auto-increment */
		/* ignore button 3 */
		if ( !(--ticks) && current_button < CLOCK_BYTES) {
			if (switches_read(SW0)) {
				/* switch 0 is in the up position */
				increment(current_button);
			} else {
				/* switch 0 is in the down position */
				decrement(current_button);
			}

			print_clock();			// re-print clock
			ticks = TICKS_PER_INCREMENT;	// set ticks to half-second
		}
	}
}

/* callback for button debounce */
void button_press(int button)
{
	int i;			// counter variable
	buttons_pressed = 0;	// reset so we can count how many are pressed

	/* for each button i */
	for (i = 0; i < BUTTON_COUNT; ++i) {
		/* don't count 'button' for now */
		if (button != i && buttons_read(i)) {
			current_button = i;		// keep track of which button is pressed
			++buttons_pressed;		// increment count
		}
	}

	/* on a rising edge */
	if (buttons_read(button)) {
		/* if no other buttons are pressed, and it's not button 3 */
		if (!buttons_pressed && button < CLOCK_BYTES) {
			if (switches_read(SW0)) {
				/* switch 0 is in the up position */
				increment(button);
			} else {
				/* switch 0 is in the down position */
				decrement(button);
			}

			print_clock();			// re-print clock
		}

		current_button = button;		// keep track of which button is pressed
		++buttons_pressed;			// increment count
	}
	ticks = TICKS_PER_SEC;				// set the count to run one second
}

/* runs the clock */
int main()
{
	uint32_t interrupts;	// used to tell which interrupts were raised

	signal(SIGINT, sigint_handler);			// register signal handler

	/* initialize interrupt controller */
	if ( FAIL_INT(intc_init("/dev/uio4")) ) {
		return EXIT_FAILURE;
	}

	/* initialize buttons */
	if ( FAIL_INT(buttons_init("/dev/uio1")) ) {
		return EXIT_FAILURE;
	}

	/* initialize switches */
	if ( FAIL_INT(switches_init("/dev/uio2")) ) {
		return EXIT_FAILURE;
	}

	/* set callback for debounced buttons */
	/* note that there is no callback for switches */
	/* we will simply read switch value each time the clock increments */
	buttons_callback(button_press);

	/* print the initial clock */
	print_clock();

	intc_irq_enable(INTC_MASK);			// enable interrupts
	buttons_enable();				// enable button interrupts
	switches_enable();				// enable switch interrupts
	while (1) {
		intc_enable_uio_interrupts();		// re-enable UIO interrupt
		interrupts = intc_wait_for_interrupt();	// block until something happens

		/* if the interrupt came from the FIT */
		if (interrupts & INTC_FIT_MASK) {
			fit_isr();			// call FIT interrupt handler
		}

		/* if the interrupt came from the buttons */
		if (interrupts & INTC_BTNS_MASK) {
			buttons_isr();			// defined in buttons.h
		}

		/* if the interrupt came from the switches */
		if (interrupts & INTC_SW_MASK) {
			switches_isr();			// defined in switches.h
		}

		/* acknowledge with interrupt controller */
		intc_ack_interrupt(interrupts);
	}
}
