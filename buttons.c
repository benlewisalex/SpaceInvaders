#include <stdint.h>			// contains declaration for integer types
#include <stdlib.h>

#include "buttons.h"			// contains function declarations for this file
#include "gpio.h"			// contains functions to manipulate gpio memory space
#include "utils.h"			// contains some helper functions/macros

#define DEBOUNCE_COUNT	5		// number of clock ticks to wait on debounce

static int fd;				// file descriptor for device file
static uint8_t * btns_gpio;		// pointer to mmap'd memory

static uint32_t debounced_values;	// a bit set to 1 means that button is pressed
static uint32_t prev_read;		// tells if a button's value changes between reads
static uint8_t ticks[BUTTON_COUNT];	// keeps track of debounce counts for each button
static void (*callback)(int button);	// function to call when a button has been debounced

/* must be called before any other functions may be used */
int buttons_init(const char * filename)
{
	int i; /* counter variable */

	debounced_values = 0;		// no buttons pressed
	prev_read = 0;			// no previous read
	callback = NULL;		// no function to call

	/* initialize 'ticks' array */
	for (i = 0; i < BUTTON_COUNT; ++i) {
		/* debounce all button values immediately */
		ticks[i] = DEBOUNCE_COUNT;
	}

	/* initialize gpio device */
	if (FAIL_PTR(btns_gpio = gpio_init(filename, &fd))) {
		/* return failure status */
		return EXIT_FAILURE;
	}

	/* set the four buttons to input mode */
	gpio_set_mode(btns_gpio, ALL_BUTTONS, GPIO_MODE_INPUT);

	/* initialization successful */
	return EXIT_SUCCESS;
}

void buttons_exit()
{
	gpio_exit(btns_gpio, fd);
}

/* acknowledge interrupt */
void buttons_ack_interrupt()
{
	/* pass it on */
	gpio_ack_interrupt(btns_gpio);
}

/* set callback for button debounce */
void buttons_callback(void (*func)(int button))
{
	/* set function pointer */
	callback = func;
}

/* enable interrupts */
void buttons_enable()
{
	/* pass it on */
	gpio_int_enable(btns_gpio);
}

/* disable interrupts */
void buttons_disable()
{
	/* pass it on */
	gpio_int_disable(btns_gpio);
}

/* interrupt handler */
void buttons_isr()
{
	int i;				// counter variable
	uint32_t value;			// stores value read from GPIO data register

	value = gpio_read(btns_gpio);	// read the register

	/* do the following checks for each button */
	for (i = 0; i < BUTTON_COUNT; ++i) {
		/* just so it doesn't have to re-calculate the mask multiple times */
		uint32_t mask = BUTTON_MASK(i);

		/* value has changed since last read */
		if ( (mask & value) != (mask & prev_read) ) {
			/* reset debounce counter */
			ticks[i] = DEBOUNCE_COUNT;
		}
	}

	prev_read = value;		// store last read
	buttons_ack_interrupt();	// acknowledge the interrupt
}

/* called on a regular interval by a hardware timer */
void buttons_tick()
{
	int i;	// counter variable

	/* do the following checks for each button */
	for (i = 0; i < BUTTON_COUNT; ++i) {
		/* precalculate the mask */
		uint32_t mask = BUTTON_MASK(i);

		/* if counter > 0; decrement it
		   when it hits zero, it is considered debounced */
		if (ticks[i] && !(--ticks[i]) ) {
			/* it only counts if the new value is different from the old one */
			if ( (prev_read & mask) != (debounced_values & mask) ) {
				if (prev_read & mask) {
					/* if button pressed, set corresponding bit */
					debounced_values |= mask;
				} else {
					/* if not pressed, unset that bit */
					debounced_values &= ~mask;
				}

				/* call callback function */
				if ( callback != NULL ) {
					callback(i);
				}
			}
		}
	}
}

/* gets the DEBOUNCED button value */
int buttons_read(uint32_t button)
{
	return debounced_values & BUTTON_MASK(button);
} 
