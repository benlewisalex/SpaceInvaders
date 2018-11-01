#ifndef SWITCHES_H
#define SWITCHES_H

#include <stdint.h>

#define SW_SUCCESS	0

#define SWITCH_COUNT	2				// how many switches on the board
#define SWITCH_MASK(i)	(1 << i)			// computes bit mask
#define ALL_SWITCHES	((1 << SWITCH_COUNT) - 1)	// bit mask for all switches

/* initialize GPIO device */
int switches_init(const char * filename);

/* perform clean-up */
void switches_exit();

/* acknowledge interrupt */
void switches_ack_interrupt();

/* register callback function */
void switches_callback(void (*func)(int sw));

/* enable all interrupts */
void switches_enable();

/* disable all interrupts */
void switches_disable();

/* call me on each interrupt */
void switches_isr();

/* call me every time the FIT ticks */
void switches_tick();

/* get debounced switch values */
int switches_read(uint32_t sw);

#endif
