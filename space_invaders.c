#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "army.h"
#include "bunker.h"
#include "buttons.h"
#include "game_over.h"
#include "intc.h"
#include "player.h"
#include "red_alien.h"
#include "scoreboard.h"
#include "utils.h"
#include "video.h"

/* this function is called on every tick of the FIT */
void fit_isr() {
	if(!game_over_get_state()){		// active gameplay
		army_tick();			// move aliens, fire bullets
		player_tick();			// move tank, fire bullets
		red_alien_tick();		// activate flying saucer
		buttons_tick();			// handle button presses
	}
	else if (!game_over_get_name_state()){	// set initials
		game_over_tick();		// pass tick along
		buttons_tick();			// handle button presses
	}
}

#define RIGHT_BUTTON	0	// button 0
#define RMIDDLE_BUTTON	1	// button 1
#define LMIDDLE_BUTTON	2	// button 2
#define LEFT_BUTTON	3	// button 3

#define LINE_OFFSET 1 // the offset to draw the green line at the bottom

/* called every time a button is debounced */
/* button parameter tells you which button was debounced */
void button_press(int button) {
	bool pressed = buttons_read(button) ? true : false;	// check button state
	if(!game_over_get_state()){				// active gameplay
		switch (button) {				// choose action
		case LEFT_BUTTON:				// move tank left
			player_left_button(pressed);		// call player function
			break;					// break
		case LMIDDLE_BUTTON:				// fall through
		case RMIDDLE_BUTTON:				// both middle buttons fire
			player_fire_button(pressed);		// call player function
			break;					// break
		case RIGHT_BUTTON:				// move tank right
			player_right_button(pressed);		// call player function
			break;					// break
		default:					// shouldn't happen
			break;					// ignore it
		}
	} else {						// game over screen
		switch (button) {				// choose action
		case LEFT_BUTTON:				// move A->Z
			game_over_increment_letter(pressed);	// call function
			break;					// break
		case LMIDDLE_BUTTON:				// move Z->A
			game_over_decrement_letter(pressed);	// call function
			break;					// break
		case RMIDDLE_BUTTON:				// move to next letter
			game_over_advance_letter(pressed);	// call function
			break;					// break
		case RIGHT_BUTTON:				// do nothing
		default:					// also do nothing
			break;					// break
		}
	}
}

void screen_init() {
	video_writeScreen(BLACK);			// erase the entire screen
	video_writeHLine(0, ROWS - LINE_OFFSET, GREEN, COLUMNS);	// draw the green line at the bottom
	army_init();					// initialize army
	bunker_init();					// initialize bunkers
	player_init();					// initialize tank
	scoreboard_init();				// initialize score/lives
	red_alien_init();				// initialize saucer
	game_over_init();				// initialize game over state
}

#define INTC_MASK	(INTC_FIT_MASK|INTC_BTNS_MASK)	// which interrupts to listen to

/* perform cleanup on a SIGINT */
void sigint_handler(int signum)
{
	putchar('\n');			// print newline

	buttons_disable();		// disable buttons
	intc_irq_disable(INTC_MASK);	// disable interrupt controller

	buttons_exit();			// shut down buttons
	intc_exit();			// shut down interrupt controller

	exit(EXIT_SUCCESS);		// terminate process
}

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

	/* initialize video driver */
	if ( FAIL_INT(video_init("/dev/ecen427_hdmi")) ) {
		return EXIT_FAILURE;
	}

	/* erase the screen and initialize all components */
	screen_init();

	/* set callback for debounced buttons */
	/* we will simply read switch value each time the clock increments */
	buttons_callback(button_press);

	intc_irq_enable(INTC_MASK);			// enable interrupts
	buttons_enable();				// enable button interrupts
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

		/* acknowledge with interrupt controller */
		intc_ack_interrupt(interrupts);
	}
}
