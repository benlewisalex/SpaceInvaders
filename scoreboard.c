#include <stdint.h>
#include <string.h>

#include "game_over.h"
#include "player.h"
#include "scoreboard.h"
#include "text.h"
#include "video.h"

#define TEXT_Y			10	// y position for all text
#define LIVES_TEXT_OFFSET	360	// x position of "LIVES"
#define SCORE_TEXT_OFFSET	20	// x position of "SCORE"
#define SCORE_NUM_OFFSET	100	// x position of score
#define TANK_X			(LIVES_TEXT_OFFSET+(5*PADDED_WIDTH)+8)	// x position for tanks
#define TANK_Y			6					// y position for tanks

/* shortcut to calculate x position for a tank image */
#define TANK_OFFSET(n)		(TANK_X+((TANK_WIDTH+1)*TANK_SCALE*(n)))

#define SCORE_DIGITS		5	// how many digits of score to display
#define STARTING_LIVES		3	// start with this many lives
#define MAX_LIVES		5	// max lives

static uint32_t score;			// current player score
static uint32_t lives;			// how many lives

void draw_score() {							// draw score
	int i;								// counter
	uint32_t s = score;						// working copy
	uint8_t digit;							// current digit
	uint32_t x = SCORE_NUM_OFFSET + (SCORE_DIGITS*PADDED_WIDTH);	// x position

	for (i = SCORE_DIGITS; i > 0; --i) {				// print right to left
		x -= PADDED_WIDTH;					// decrement x position
		digit = s % NUMBER_SYSTEM;				// get last digit
		s /= NUMBER_SYSTEM;					// divide

		text_drawLetter(DIGITS[digit], x, TEXT_Y, GREEN);	// draw digit
	}
}

void scoreboard_add(uint32_t points) {					// score points
	score += points;						// increment score
	draw_score();							// draw
}

void scoreboard_add_life() {						// add a life
	if (lives < MAX_LIVES) {					// check bounds
		player_draw_tank(TANK_OFFSET(lives++), TANK_Y, DRAW);	// increment and draw
	}
}

void scoreboard_remove_life() {						// lose a life
	if (lives) {							// check bounds
		if ( !(--lives) ) {					// decrement
			game_over_end_game(score);			// game over
		}
		player_draw_tank(TANK_OFFSET(lives), TANK_Y, ERASE);	// erase
	}
}

void scoreboard_init(){							// initialize
	int i, j;							// counters
	uint32_t offset;						// temporary variable
	uint32_t offsets[] = {SCORE_TEXT_OFFSET, LIVES_TEXT_OFFSET};	// x offsets
	char * strings[] = {"SCORE", "LIVES"};				// strings

	for (i = 0; i < sizeof(offsets)/sizeof(uint32_t); ++i) {	// for each string
		offset = offsets[i];					// get x position
		for (j = 0; j < strlen(strings[i]); ++j) {		// for each letter
			text_drawLetter(ALPHA(strings[i][j]), offset, TEXT_Y, WHITE);	// draw
			offset += PADDED_WIDTH;				// change offset
		}
	}

	score = 0;							// initialize score
	draw_score();							// draw score

	lives = 0;							// start with zero
	for (i = 0; i < STARTING_LIVES; ++i) {				// add a bunch
		scoreboard_add_life();					// increment and draw
	}
}
