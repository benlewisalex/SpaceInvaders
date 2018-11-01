#include <stdbool.h>

#include "text.h"
#include "video.h"

void text_drawLetter(const uint8_t * letter, uint32_t x, uint32_t y, uint32_t color) {
	int row, col, scalar;							// counter
	bool pixels[LETTER_WIDTH*LETTER_SCALE];					// current row
	for (row = 0; row < LETTER_HEIGHT; ++row) {				// for each row
		for (col = 0; col < LETTER_WIDTH; ++col) {			// for each col
			for (scalar = 0; scalar < LETTER_SCALE; ++scalar) {	// multiply col
				pixels[LETTER_SCALE*col+scalar] = *letter;	// same pixel
			}
			++letter;						// next pixel
		}
		for (scalar = 0; scalar < LETTER_SCALE; ++scalar) {		// muiltiply row
			video_writeLine(					// draw line
				x,						// same x
				y+(LETTER_SCALE*row)+scalar,			// calculate y
				color,						// color
				LETTER_WIDTH*LETTER_SCALE,			// row width
				pixels						// current row
			);
		}
	}
}

void text_drawLargeLetter(const uint8_t * letter, uint32_t x, uint32_t y, uint32_t color) {
	int row, col, scalar;							// counter
	bool pixels[LETTER_WIDTH*LARGE_LETTER_SCALE];				// current row
	for (row = 0; row < LETTER_HEIGHT; ++row) {				// for each row
		for (col = 0; col < LETTER_WIDTH; ++col) {			// for each col
			for (scalar = 0; scalar < LARGE_LETTER_SCALE; ++scalar) {// multiply col
				pixels[LARGE_LETTER_SCALE*col+scalar] = *letter;// same pixel
			}
			++letter;						// next pixel
		}
		for (scalar = 0; scalar < LARGE_LETTER_SCALE; ++scalar) {	// muiltiply row
			video_writeLine(					// draw line
				x,						// same x
				y+(LARGE_LETTER_SCALE*row)+scalar,		// calculate y
				color,						// color
				LETTER_WIDTH*LARGE_LETTER_SCALE,		// row width
				pixels						// current row
			);
		}
	}
}
