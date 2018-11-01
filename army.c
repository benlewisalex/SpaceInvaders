#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "army.h"
#include "player.h"
#include "scoreboard.h"
#include "video.h"
#include "bunker.h"

#define ARMY_LENGTH	11			// number of columns
#define ARMY_HEIGHT	5			// number of rows

#define ALIEN_WIDTH	16			// width of alien image
#define ALIEN_HEIGHT	14			// height of alien image
#define ALIEN_SCALE	2			// real pixels per image pixel
#define DROP_DISTANCE	(4*ALIEN_SCALE)		// how far aliens drop when they hit the edge
#define BULLET_OFFSET	((ALIEN_WIDTH/2)-1)	// left/right bullet position relative to alien

#define BULLET_HEIGHT	5			// height of bullet image
#define BULLET_WIDTH	3			// width of bullet image

#define TICKS_PER_MARCH	25			// ticks between each movement
#define BULLET_SPEED	1			// pixels moved per tick

#define ALIEN_TYPE(row)	((row+1)/2)		// determines an alien's type based on row
#define TOP		0			// top row type
#define MIDDLE		1			// middle type
#define BOTTOM		2			// bottom type
#define NUM_TYPES	(BOTTOM+1)		// number of alien types

#define AIR_SPACE	DROP_DISTANCE		// padding above alien
#define PADDING(type)	((type) == TOP ? 4 : 3)	// padding left/right

#define TOP_POINTS	40			// point value for top row
#define MIDDLE_POINTS	20			// point value for middle row
#define BOTTOM_POINTS	10			// point value for bottom row

#define IN		0			// alien stance
#define OUT		1			// alien stance
#define NUM_STANCES	(OUT+1)			// number of possible stances

#define LEFT		0			// alien moving left
#define RIGHT		1			// alien moving right

#define DOWN		0			// bullet orientation down
#define UP		1			// bullet orientation up
#define ORIENTATIONS	2			// number of orientations
#define NUM_BULLETS	2			// number of bullet types
#define RANDOM_BULLET_TYPE	(random() % NUM_BULLETS)	// pick a type
#define BULLETS_ALLOWED	4			// number of bullets allowed on the screen
#define MAX_BULLET_Y	(ROWS-4)		// don't overwrite green line at the bottom

#define DRAW		false			// used for 'bool erase'
#define ERASE		true			// used for 'bool erase'

#define LEFT_EDGE	16			// padding on the edge of the screen
#define RIGHT_EDGE	(COLUMNS-LEFT_EDGE)	// padding on the edge of the screen
#define Y		50			// initial y position of army

#define X_OFFSET(col)	(col*ALIEN_WIDTH*ALIEN_SCALE)	// calculate x position of a column
#define Y_OFFSET(row)	(row*ALIEN_HEIGHT*ALIEN_SCALE)	// calculate y position of a row

#define DRAW_OFF 1 // offset for drawing the videos

/* erases an alien after it explodes */
static const uint8_t DEAD[ALIEN_HEIGHT][ALIEN_WIDTH] = {
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

/* draws explosion */
static const uint8_t EXPLOSION[ALIEN_HEIGHT][ALIEN_WIDTH] = {
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0},
	{0,0,0,0,0,1,0,0,1,0,0,0,1,0,0,0},
	{0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0},
	{0,0,0,1,0,0,1,0,0,0,1,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0},
	{0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,0},
	{0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0},
	{0,0,0,1,0,0,0,1,0,0,1,0,0,0,0,0},
	{0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0}
};

/* top row, in stance */
static const uint8_t TOP_IN[ALIEN_HEIGHT][ALIEN_WIDTH] = {
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0},
	{0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
	{0,0,0,0,1,1,0,1,1,0,1,1,0,0,0,0},
	{0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},
	{0,0,0,0,0,1,0,1,1,0,1,0,0,0,0,0},
	{0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0},
	{0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

/* top row, out stance */
static const uint8_t TOP_OUT[ALIEN_HEIGHT][ALIEN_WIDTH] = {
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0},
	{0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
	{0,0,0,0,1,1,0,1,1,0,1,1,0,0,0,0},
	{0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},
	{0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0},
	{0,0,0,0,0,1,0,1,1,0,1,0,0,0,0,0},
	{0,0,0,0,1,0,1,0,0,1,0,1,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

/* middle section, in stance */
static const uint8_t MIDDLE_IN[ALIEN_HEIGHT][ALIEN_WIDTH] = {
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0},
	{0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0},
	{0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0},
	{0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,0},
	{0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0},
	{0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0},
	{0,0,0,1,0,1,0,0,0,0,0,1,0,1,0,0},
	{0,0,0,0,0,0,1,1,0,1,1,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

/* middle section, out stance */
static const uint8_t MIDDLE_OUT[ALIEN_HEIGHT][ALIEN_WIDTH] = {
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0},
	{0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0},
	{0,0,0,1,0,1,1,1,1,1,1,1,0,1,0,0},
	{0,0,0,1,1,1,0,1,1,1,0,1,1,1,0,0},
	{0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0},
	{0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0},
	{0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0},
	{0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

/* bottom section, in stance */
static const uint8_t BOTTOM_IN[ALIEN_HEIGHT][ALIEN_WIDTH] = {
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0},
	{0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
	{0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
	{0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0},
	{0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
	{0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0},
	{0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0},
	{0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

/* bottom section, out stance */
static const uint8_t BOTTOM_OUT[ALIEN_HEIGHT][ALIEN_WIDTH] = {
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0},
	{0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
	{0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
	{0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0},
	{0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
	{0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0},
	{0,0,0,0,1,1,0,1,1,0,1,1,0,0,0,0},
	{0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

/* bullet type 1, down orientation */
static uint8_t BULLET_1_DOWN[BULLET_HEIGHT][BULLET_WIDTH] = {
	{0,1,0},
	{0,1,0},
	{0,1,0},
	{1,1,1},
	{0,1,0}
};

/* bullet type 1, up orientation */
static uint8_t BULLET_1_UP[BULLET_HEIGHT][BULLET_WIDTH] = {
	{0,1,0},
	{1,1,1},
	{0,1,0},
	{0,1,0},
	{0,1,0}
};

/* bullet type 2, down orientation */
static uint8_t BULLET_2_DOWN[BULLET_HEIGHT][BULLET_WIDTH] = {
	{0,1,0},
	{0,0,1},
	{0,1,0},
	{1,0,0},
	{0,1,0}
};

/* bullet type 2, up orientation */
static uint8_t BULLET_2_UP[BULLET_HEIGHT][BULLET_WIDTH] = {
	{0,1,0},
	{1,0,0},
	{0,1,0},
	{0,0,1},
	{0,1,0}
};

/* erase a bullet */
static uint8_t BULLET_ERASE[BULLET_HEIGHT][BULLET_WIDTH] = {
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
};

/* shortcut array of alien images */
/* indexed as [stance][type] */
static const uint8_t * sprites[NUM_STANCES][NUM_TYPES] = {
	{(const uint8_t *) TOP_IN, (const uint8_t *) MIDDLE_IN, (const uint8_t *) BOTTOM_IN},
	{(const uint8_t *) TOP_OUT, (const uint8_t *) MIDDLE_OUT, (const uint8_t *) BOTTOM_OUT}
};

/* shortcut array of bullet images */
/* indexed as [orientation][type] */
static const uint8_t * bullet_images[ORIENTATIONS][NUM_BULLETS] = {
	{(const uint8_t *) BULLET_1_DOWN, (const uint8_t *) BULLET_2_DOWN},
	{(const uint8_t *) BULLET_1_UP, (const uint8_t *) BULLET_2_UP}
};

/* contains point values; indexed by alien type */
static const uint32_t POINTS[NUM_TYPES] = {TOP_POINTS, MIDDLE_POINTS, BOTTOM_POINTS};

/* struct used to represent an alien */
typedef struct {
	bool alive;		// indicates if alien is alive or dead (appears or not)
	bool dying;		// tells you an alien is dead, but needs to be erased
	uint8_t type;		// TOP, MIDDLE, or BOTTOM alien
} alien_t;

/* assigns a random time to wait before firing bullet */
#define RANDOM_WAIT_TIME	(random() % (100 * BULLETS_ALLOWED) + 1)

/* struct representing alien bullet */
typedef struct {
	bool alive;		// indicates that the bullet is active
	bool orientation;	// indicates how bullet is oriented
	uint8_t type;		// which type of bullet
	uint32_t wait;		// time to wait before becoming active
	uint32_t x;		// x position
	uint32_t y;		// y position
} bullet_t;

/* singleton struct representing army */
static struct {
	alien_t army[ARMY_HEIGHT][ARMY_LENGTH];	// array of aliens
	int32_t x;				// army x position
	uint32_t y;				// army y position
	uint32_t alive;				// how many aliens remain
	uint8_t stance;				// army stance
	uint8_t right;				// = rightmost non-empty column + 1
	uint8_t left;				// = leftmost non-empty column
} army;

/* data for each alien bullet */
/* if the bullet is not alive, the wait count will decrement with every tick */
/* once the wait count reaches 0, the bullet is fired from a random column */
static bullet_t bullets[BULLETS_ALLOWED];

static uint32_t move_counter;	// when this hits 0, the army marches
static bool direction;		// which direction is the army moving

/* draws a generic image the size of an alien at (x, y) */
void draw(const uint8_t * image, uint32_t x, uint32_t y) {
	int row, col, scalar;			// counters
	bool pixels[ALIEN_WIDTH*ALIEN_SCALE];	// represents a single line of pixels

	for (row = 0; row < ALIEN_HEIGHT; ++row) {
		for (col = 0; col < ALIEN_WIDTH; ++col) {
			/* scalar allows us to make the image bigger by some amount */
			for (scalar = 0; scalar < ALIEN_SCALE; ++scalar) {
				// copy the same pixel ALIEN_SCALE times
				pixels[col*ALIEN_SCALE+scalar] = *image;
			}
			// then move to the next image pixel
			++image;
		}
		// draw the row ALIEN_SCALE times
		for (scalar = 0; scalar < ALIEN_SCALE; ++scalar) {
			video_writeLine(
				x,				// each row has the same x
				y+ALIEN_SCALE*row+scalar,	// y coordinate of this row
				WHITE,				// color
				ALIEN_WIDTH*ALIEN_SCALE,	// row length
				pixels				// the current line
			);
		}
	}
}

/* shortcut method to draw the entire army */
void army_drawAll(){
	const uint8_t * sprite;	// points to the current image
	alien_t * alien;	// points to the current alien
	for (int row = 0; row < ARMY_HEIGHT; row++){
		for (int col = 0; col < ARMY_LENGTH; col++){

			alien = &army.army[row][col];	// store the address
			if (alien->alive) {		// if it's alive, pick the right image
				sprite = sprites[army.stance][alien->type];
			} else if (alien->dying) {	// it it just exploded
				sprite = (const uint8_t *) DEAD;	// erase it
				alien->dying = false;	// don't erase it on the next tick
			} else {
				continue;		// ignore dead aliens
			}

			/* call generic function */
			draw(sprite, army.x + X_OFFSET(col), army.y + Y_OFFSET(row));
		}
	}
}

/* after an alien is killed, this updates the army's right and left edges */
void update_edges(bool side) {
	int row, col;							// counters
	if (side == LEFT) {						// update from the left
		for (col = army.left; col < army.right; ++col) {	// scan left->right
			for (row = 0; row < ARMY_HEIGHT; ++row) {	// check each row
				if (army.army[row][col].alive) {	// if found alive
					army.left = col;		// set edge
					return;				// get outta here
				}
			}
		}
	} else {							// update from the right
		for (col = army.right; col > army.left; --col) {	// scan right->left
			for (row = 0; row < ARMY_HEIGHT; ++row) {	// check each row
				if (army.army[row][col-DRAW_OFF].alive) {	// if found alive
					army.right = col;		// set edge
					return;				// get outta here
				}
			}
		}
	}
}

void revive_army() {
	int row, col;							// counters
	for (row = 0; row < ARMY_HEIGHT; row++){			// for each row
		for (col = 0; col < ARMY_LENGTH; col++){		// for each column
			army.army[row][col].alive = true;		// make alive
			army.army[row][col].dying = false;		// not exploded
			army.army[row][col].type = ALIEN_TYPE(row);	// set type
		}
	}
	army.alive = ARMY_HEIGHT*ARMY_LENGTH;				// all are alive
	army.left = 0;							// set left edge
	army.right = ARMY_LENGTH;					// set right edge
}

void reset_bullet(bullet_t * bullet) {		// called when a bullet dies
	bullet->alive = false;			// make it dead
	bullet->wait = RANDOM_WAIT_TIME;	// wait a while
}

static void draw_bullet(bullet_t * bullet, bool erase) {		// draws a bullet
	const uint8_t * image = erase ? (const uint8_t*) BULLET_ERASE :	// select image
				bullet_images[bullet->orientation][bullet->type];
	int row, col, scalar;						// counters
	bool pixels[BULLET_WIDTH*ALIEN_SCALE];				// single row
	for (row = BULLET_HEIGHT-DRAW_OFF; row >= 0; --row) {			// rows in reversed order
		for (col = 0; col < BULLET_WIDTH; ++col) {		// for each col
			for (scalar = 0; scalar < ALIEN_SCALE; ++scalar) {
				pixels[ALIEN_SCALE*col+scalar] = *image;// multiply by scalar
			}
			++image;					// increment only once
		}
		for (scalar = 0; scalar < ALIEN_SCALE; ++scalar) {	// multiply by scalar
			video_writeLine(				// draw current line
				bullet->x,				// same x position
				bullet->y-ALIEN_SCALE*row+scalar,	// reversed order
				WHITE,					// color
				BULLET_WIDTH*ALIEN_SCALE,		// row width
				pixels					// current row
			);
		}
	}
}

void advance_bullet(bullet_t * bullet) {				// move bullet
	draw_bullet(bullet, ERASE);					// erase it
	bullet->y += BULLET_SPEED*ALIEN_SCALE;				// move
	bullet->orientation = !bullet->orientation;			// reverse orientation
	if (bullet->y > MAX_BULLET_Y) {					// off edge of screen
		reset_bullet(bullet);					// reset it
	} else if (player_receive_fire(bullet->x, bullet->y)) {		// hits tank
		reset_bullet(bullet);					// reset it
	} else if (bunker_receive_fire(bullet->x, bullet->y)) {		// hits tank
		reset_bullet(bullet);					// reset it
	} else {							// carry on
		draw_bullet(bullet, DRAW);				// draw
	}
}

void fire_bullet(bullet_t * bullet) {					// bring a bullet to life
	int8_t row;							// counter
	uint8_t column = random() % (ARMY_LENGTH);			// pick a column

	for (row = ARMY_HEIGHT-DRAW_OFF; row >= 0; --row) {			// check from bottom
		if (army.army[row][column].alive) {			// if alien is alive
			bullet->alive = true;				// fire
			bullet->type = RANDOM_BULLET_TYPE;		// pick a type
			bullet->orientation = DOWN;			// starting orientation
			/* set starting x position */
			bullet->x = army.x + (column*ALIEN_WIDTH+BULLET_OFFSET)*ALIEN_SCALE;
			/* set starting y position */
			bullet->y = army.y + ((row+DRAW_OFF)*ALIEN_HEIGHT+BULLET_HEIGHT)*ALIEN_SCALE;
			draw_bullet(bullet, DRAW);			// draw
			//printf("Firing from column %d\n", column);	// debug
			return;						// break out
		}
	}

	reset_bullet(bullet);						// column is empty
}

bool army_receive_fire(uint32_t x, uint32_t y) {			// check collisions
	int8_t row, col;						// where bullet hits
	alien_t * alien;						// alien pointer
	x = (x - army.x) / ALIEN_SCALE;					// normalize x
	y = (y - army.y) / ALIEN_SCALE;					// normalize y

	if (x >= army.left * ALIEN_WIDTH && x < army.right * ALIEN_WIDTH	// if within army
				&& y >= 0 && y < ARMY_HEIGHT * ALIEN_HEIGHT) {
		col = x / ALIEN_WIDTH;					// compute col
		row = y / ALIEN_HEIGHT;					// compute row
		alien = &army.army[row][col];				// set pointer
		if (alien->alive) {					// if alive
			x %= ALIEN_WIDTH;				// normalize x to alien
			y %= ALIEN_HEIGHT;				// normalize y

			/* must be a direct hit */
			if (x >= PADDING(alien->type) && x < ALIEN_WIDTH-PADDING(alien->type) &&
										y >= AIR_SPACE) {
				alien->alive = false;			// kill it
				alien->dying = true;			// will erase explosion

				draw(
					(const uint8_t *) EXPLOSION,	// draw explosion
					X_OFFSET(col)+army.x,		// compute x
					Y_OFFSET(row)+army.y		// compute y
				);
				scoreboard_add(POINTS[alien->type]);	// get points

				if ( !(--army.alive) ) {		// if all dead
					army.y = Y;
					revive_army();			// revive
					scoreboard_add_life();		// gain a life
				} else if (col == army.left) {		// alien on left edge
					update_edges(LEFT);		// update from left
				} else if (col+DRAW_OFF == army.right) {	// alien on right edge
					update_edges(RIGHT);		// update from right
				}
				return true;				// indicate hit
			}
		}
	}
	return false;							// missed
}

void army_tick(){
	int i;								// counter
	bullet_t * bullet;						// temporary variable
	if ( !(--move_counter) ) {					// it's time to move
		if (army.x + X_OFFSET(army.right) > RIGHT_EDGE) {	// check edge collision
			direction = LEFT;				// reverse direction
			army.x = RIGHT_EDGE - X_OFFSET(army.right);	// reset position
			army.y += DROP_DISTANCE;			// drop
		} else if (army.x + X_OFFSET(army.left) < LEFT_EDGE) {	// check left edge
			direction = RIGHT;				// reverse direction
			army.x = LEFT_EDGE - X_OFFSET(army.left);	// reset position
			army.y += DROP_DISTANCE;			// drop
		} else {						// normal
			army.x += (direction == RIGHT) ? ALIEN_SCALE : -ALIEN_SCALE;	// move
			if (army.stance == IN) {			// check stance
				army.stance = OUT;			// reverse stance
			} else {					// check stance
				army.stance = IN;			// reverse stance
			}
		}
		army_drawAll();						// redraw army
		move_counter = TICKS_PER_MARCH;				// reset move count
	}

	for (i = 0; i < BULLETS_ALLOWED; ++i) {				// check all bullets
		bullet = &bullets[i];					// set pointer
		//printf("%d: %d\n", i, bullet->wait);
		if (!bullet->alive) {					// if not on screen
			if ( !(--bullet->wait) ) {			// if ready to fire
				//printf("Firing bullet %d\n", i);
				fire_bullet(bullet);			// fire
			}
		} else {						// on screen
			advance_bullet(bullet);				// move
		}
	}
}

void army_init(){
	int i;					// counter
	srand(time(NULL));			// set seed
	revive_army();				// initialize army
	for (i = 0; i < BULLETS_ALLOWED; ++i) {	// for each bullet
		reset_bullet(&bullets[i]);	// reset
	}
	army.x = LEFT_EDGE;			// set initial x
	army.y = Y;				// set initial y
	army.stance = IN;			// set initial stance
	army_drawAll();				// draw army

	move_counter = TICKS_PER_MARCH;		// set movement counter
	direction = RIGHT;			// set initial direction
}
