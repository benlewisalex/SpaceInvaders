#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils.h"
#include "video.h"

#define TOP_LEFT_OFFSET		0				// first pixel
#define OFFSET(x, y)		( (COLUMNS * y + x) * 3 )	// get HDMI offset from coords

typedef struct {	// struct to represent a pixel
	uint8_t red;	// red value
	uint8_t green;	// green value
	uint8_t blue;	// blue value
} color_t;

/* converts a uint32_t to a color_t */
#define UNPACK(n, c)		{(c).red=(n>>16)&0xff;(c).green=(n>>8)&0xff;(c).blue=n&0xff;}

static int fd;

int video_init(const char * filename) {
	if ( FAIL_INT(fd = open(filename, O_RDWR)) ) {	// open /dev file
		print_error("main", "open");		// notify of error
		return EXIT_FAILURE;			// return failure code
	}
	return EXIT_SUCCESS;				// return success code
}

void video_writePixel(uint32_t x, uint32_t y, uint32_t rgbValue)
{
	int i;					// counter
	color_t color;				// current color

	UNPACK(rgbValue, color);		// decode color
	lseek(fd, OFFSET(x, y), SEEK_SET);	// set offset
	write(fd, &color, sizeof(color_t));	// write pixel
}

void video_writeLine(uint32_t x, uint32_t y, uint32_t rgbValue, uint32_t width, bool pixels[]) {
	int i;					// counter
	color_t temp;				// holds current color
	color_t black;				// holds the color black
	color_t line[COLUMNS];			// array of pixels

	if (width > COLUMNS) {			// if row too big
		width = COLUMNS;		// can't draw more than one row
	}

	UNPACK(BLACK, black);			// decode BLACK
	UNPACK(rgbValue, temp);			// decode given color
	for (i = 0; i < width; ++i) {		// for each pixel
		if (pixels[i]) {		// if pixel is on
			line[i] = temp;		// set pixel to given color
		} else {			// pixel is not on
			line[i] = black;	// set pixel to black
		}
	}

	lseek(fd, OFFSET(x, y), SEEK_SET);	// set offset
	write(fd, line, sizeof(color_t)*width);	// write to screen
}

void video_writeHLine(uint32_t x, uint32_t y, uint32_t rgbValue, uint32_t width)
{
	int i;					// counter
	color_t temp;				// holds current color
	color_t line[COLUMNS];			// array of pixels

	if (width > COLUMNS) {			// if row too big
		width = COLUMNS;		// can't draw more than one row
	}

	UNPACK(rgbValue, temp);			// decode given color
	for (i = 0; i < width; ++i) {		// for each pixel
		line[i] = temp;			// set pixel to given color
	}

	lseek(fd, OFFSET(x, y), SEEK_SET);	// set offset
	write(fd, line, sizeof(color_t)*width);	// write to screen
}

void video_writeScreen(uint32_t rgbValue)
{
	int i, j;					// counters
	color_t temp;					// temporary variable
	color_t screen[ROWS][COLUMNS];			// array of pixels

	UNPACK(rgbValue, temp);				// decode uint32_t to color_t
	for (i = 0; i < ROWS; ++i) {			// for each row
		for (j = 0; j < COLUMNS; ++j) {		// for each col
			screen[i][j] = temp;		// set contents of array
		}
	}

	lseek(fd, TOP_LEFT_OFFSET, SEEK_SET);		// top left corner
	write(fd, screen, sizeof(color_t)*COLUMNS*ROWS);// write to screen
}
