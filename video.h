#include <stdbool.h>

#define RED	0x00ff0000
#define GREEN	0x0000ff00
#define BLUE	0x000000ff
#define BLACK	0x00000000
#define WHITE	(RED|GREEN|BLUE)

#define COLUMNS			640	// screen width
#define ROWS			480	// screen height

/* initialize hdmi stuff */
int video_init(const char * filename);

/* draw a single pixel */
void video_writePixel(uint32_t x, uint32_t y, uint32_t rgbValue);

/* draw a line of pixels */
void video_writeLine(uint32_t x, uint32_t y, uint32_t rgbValue, uint32_t width, bool pixels[]);

/* writes a line of a single color */
void video_writeHLine(uint32_t x, uint32_t y, uint32_t rgbValue, uint32_t width);

/* fill screen with a single color */
void video_writeScreen(uint32_t rgbValue);
