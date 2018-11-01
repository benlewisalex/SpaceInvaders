#include <stdint.h>
#include <stdio.h>

#include "gpio.h"
#include "switches.h"
#include "utils.h"

#define SW_FAILED	-1			// return value for switches_init
#define DEBOUNCE_COUNT	5			// how many ticks to wait for debounce

static int fd;					// device file descriptor
static uint8_t * sw_gpio;			// mmap'd address

static uint32_t debounced_values;		// holds values after they are debounced
static uint32_t prev_read;			// keeps track of the previous read
static uint8_t ticks[SWITCH_COUNT];		// keeps track of debounce count for each switch
static void (*callback)(int sw);		// called every time a switch is debounced

/* initialize switches */
int switches_init(const char * filename)
{
	int i;					// counter variable

	debounced_values = 0;			// clear debounced values
	prev_read = 0;				// clear previous read
	callback = NULL;			// no callback function defined

	/* set initial debounce count for each switch */
	for (i = 0; i < SWITCH_COUNT; ++i) {
		ticks[i] = DEBOUNCE_COUNT;	// this will cause the values to update
	}

	/* try initializing GPIO device */
	if (FAIL_PTR(sw_gpio = gpio_init(filename, &fd))) {
		return SW_FAILED;		// return -1
	}

	/* set switches to input mode */
	gpio_set_mode(sw_gpio, ALL_SWITCHES, GPIO_MODE_INPUT);
	return SW_SUCCESS;			// return 0
}

/* perform clean-up */
void switches_exit()
{
	/* pass it along */
	gpio_exit(sw_gpio, fd);
}

/* acknowledge an interrupt */
void switches_ack_interrupt()
{
	/* pass it along */
	gpio_ack_interrupt(sw_gpio);
}

/* register callback function */
void switches_callback(void (*func)(int sw))
{
	/* set function pointer */
	callback = func;
}

/* enable all interrupts */
void switches_enable()
{
	/* pass it along */
	gpio_int_enable(sw_gpio);
}

/* disable all interrupts */
void switches_disable()
{
	gpio_int_disable(sw_gpio);
}

/* call this function at each interrupt */
void switches_isr()
{
	int i;					// counter variable
	uint32_t value = gpio_read(sw_gpio);	// holds the value from the data register

	/* do the following checks for each switch i */
	for (i = 0; i < SWITCH_COUNT; ++i) {
		/* pre-compute mask */
		uint32_t mask = SWITCH_MASK(i);

		/* if the value has changed, set the debounce counter */
		if ( (mask & value) != (mask & prev_read) ) {
			ticks[i] = DEBOUNCE_COUNT;
		}
	}

	prev_read = value;			// store the value read
	switches_ack_interrupt();		// acknowledge the interrupt
}

/* call on a regular interval */
void switches_tick()
{
	int i;					// counter variable

	/* do th efollowing for each switch */
	for (i = 0; i < SWITCH_COUNT; ++i) {
		/* pre-compute mask */
		uint32_t mask = SWITCH_MASK(i);

		/* decrement any outstanding debounce counters */
		if (ticks[i] && !(--ticks[i]) ) {
			/* when it hits zero */
			/* make sure the value has actually changed */
			if ( (prev_read & mask) != (debounced_values & mask) ) {
				if (prev_read & mask) {
					/* if switch is up, set bit */
					debounced_values |= mask;
				} else {
					/* if not, unset bit */
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

/* gets the DEBOUNCED switch value */
int switches_read(uint32_t sw)
{
	return debounced_values & SWITCH_MASK(sw);
}
