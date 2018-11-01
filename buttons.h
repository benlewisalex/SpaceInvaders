#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>

#define BUTTON_COUNT	4				// number of buttons on the board
#define BUTTON_MASK(i)	(1 << i)			// calculate mask for button i
#define ALL_BUTTONS	((1 << BUTTON_COUNT) - 1)	// calculate mask for all buttons

/* initializes the buttons */
int buttons_init(const char * filename);

/* cleans up */
void buttons_exit();

/* acknowledges an interrupt */
void buttons_ack_interrupt();

/* register a callback */
void buttons_callback(void (*func)(int button));

/* enable interrupts for buttons */
void buttons_enable();

/* disable interrupts for buttons */
void buttons_disable();

/* should be called on every button interrupt */
void buttons_isr();

/* should be called on a regular interval */
void buttons_tick();

/* returns nonzero if button is pressed */
/* this function reads only the debounced button values */
int buttons_read(uint32_t button);

#endif
