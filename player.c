#include <stdint.h>
#include <stdio.h>

#include "army.h"
#include "player.h"
#include "red_alien.h"
#include "scoreboard.h"
#include "video.h"
#include "bunker.h"

#define STARTING_X		70	// initial tank position
#define Y			450	// tank y position

#define TICKS_PER_MOTION	1	// ticks per tank movement
#define TICKS_PER_FLASH		7	// for flashing explosion
#define FIRST_FLASH		2	// first explosion should draw almost right away
#define	EXPLODE_FLASHES		10	// how many times to flash

#define TANK_MIDDLE		((TANK_WIDTH-1)*TANK_SCALE/2)	// for bullet offset
#define AIR_SPACE		3	// padding above tank
#define BULLET_WIDTH		1	// bullet width
#define	BULLET_HEIGHT		10	// bullet height
#define BULLET_SPEED		3	// pixels per motion
#define BULLET_CEILING		20	// minimum y value

#define X			10					// left edge
#define MAX_POSITION		(COLUMNS-X-(TANK_WIDTH*TANK_SCALE))	// right edge

/* standard tank image */
static const uint8_t TANK[TANK_HEIGHT][TANK_WIDTH] = {
	{0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0},
	{0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
	{0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
	{0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
	{0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
	{0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0}
};

/* one exploding tank image */
static const uint8_t EXPLOSION_1[TANK_HEIGHT][TANK_WIDTH] = {
	{0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
	{0,0,1,0,0,0,0,1,0,0,1,0,0,0,0},
	{0,0,0,0,1,0,1,0,0,1,0,1,0,0,0},
	{0,0,1,0,0,0,0,0,0,0,0,0,1,0,0},
	{0,0,0,0,0,1,0,1,1,0,0,0,0,0,0},
	{1,0,0,1,1,1,1,1,1,1,1,0,0,0,0},
	{0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,0,0}
};

/* the other exploding tank image */
static const uint8_t EXPLOSION_2[TANK_HEIGHT][TANK_WIDTH] = {
	{1,0,0,0,0,0,1,0,0,0,0,0,1,0,0},
	{0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
	{0,1,0,0,0,0,1,0,0,0,0,0,1,0,0},
	{0,0,0,0,0,1,0,0,0,0,1,0,0,0,1},
	{1,0,0,0,1,1,0,0,1,0,0,0,0,0,0},
	{0,0,0,1,1,1,1,1,1,1,0,0,1,0,0},
	{0,0,1,1,1,1,1,1,1,1,1,0,0,0,0},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,0,0}
};

#define EXPLOSIONS 2			// number of explosion images
static const uint8_t * TANKS[EXPLOSIONS] = {	// array of explosion image pointers
	(const uint8_t *) EXPLOSION_1,		// first image
	(const uint8_t *) EXPLOSION_2		// second image
};

static int32_t position;		// tank x
static bool dying;			// exploding
static uint8_t flash_count;		// how many more flashes
static uint32_t tank_ticks;		// all-purpose tick counter

int32_t direction;			// really more of velocity
bool left;				// left button is pressed
bool right;				// right button is pressed
uint8_t firing;				// how many firing buttons are pressed

struct {				// struct representing tank bullet
	bool active;			// on screen
	uint32_t x;			// x position
	uint32_t y;			// y position
} bullet;

/* generic tank drawer */
static void draw(const uint8_t * tank, uint32_t x, uint32_t y, bool erase) {
	int row, col, scalar;						// counters
	uint32_t color = erase ? BLACK : GREEN;				// pick a color
	bool pixels[TANK_WIDTH*TANK_SCALE];				// current row

	for (row = 0; row < TANK_HEIGHT; row++){			// for each row
		for(col = 0; col < TANK_WIDTH; col++){			// for each col
			for (scalar = 0; scalar < TANK_SCALE; ++scalar) {
				/* multiply width of image */
				pixels[col*TANK_SCALE + scalar] = *tank;
			}
			++tank;						// move to next pixel
		}
		for (scalar = 0; scalar < TANK_SCALE; ++scalar) {	// multiple height
			video_writeLine(				// draw current line
				x,					// all have same x
				y+TANK_SCALE*row+scalar,		// y pos of current line
				color,					// color
				TANK_WIDTH*TANK_SCALE,			// row width
				pixels					// current row
			);
		}
	}
}

/* this function is used by the scoreboard */
void player_draw_tank(uint32_t x, uint32_t y, bool erase) {		// draw a normal tank
	draw((const uint8_t *) TANK, x, y, erase);			// call helper function
}

static void draw_tank(bool erase) {					// draw the player's tank
	player_draw_tank(position, Y, erase);				// call generic function
}

static void draw_explosion() {						// draw current explosion
	draw(TANKS[flash_count%EXPLOSIONS], position, Y, DRAW);		// pick image and draw it
}

static void draw_bullet(uint32_t x, uint32_t y, bool erase) {		// draw tank bullet
	int row, col, scalar;						// counters
	uint32_t color = erase ? BLACK : WHITE;				// pick a color
	bool pixels[BULLET_WIDTH*TANK_SCALE];				// current row
	for (row = 0; row < BULLET_HEIGHT; ++row) {			// for each row
		for (col = 0; col < BULLET_WIDTH; ++col) {		// for each col
			for (scalar = 0; scalar < TANK_SCALE; ++scalar) {
				/* multiply width */
				pixels[TANK_SCALE*col+scalar] = true;
			}
		}
		for (scalar = 0; scalar < TANK_SCALE; ++scalar) {	// multiply height
			video_writeLine(				// write a line
				x,					// all have same x
				y+(TANK_SCALE*row)+scalar,		// calculate y position
				color,					// color
				BULLET_WIDTH*TANK_SCALE,		// row width
				pixels					// current row
			);
		}
	}
}

/* handle button press */
static void update_direction() {
	if (left == right) {						// no or both buttons
		direction = 0;						// don't move
	} else {							// one button
		direction = left ? -TANK_SCALE : TANK_SCALE;		// set velocity
	}
}

static void reset_tank() {						// move to start
	position = STARTING_X;						// set x
	dying = false;							// stop exploding
	tank_ticks = 1;							// ready to move
	draw_tank(DRAW);						// draw
}

void player_left_button(bool pressed) {					// handle debounced btn
	left = pressed;							// set flag
	update_direction();						// update velocity
}

void player_right_button(bool pressed) {				// handle debounced btn
	right = pressed;						// set flag
	update_direction();						// update velocity
}

void player_fire_button(bool pressed) {					// handle debounced btn
	if (pressed) {							// pressed
		++firing;						// increment # firing
	} else {							// released
		--firing;						// decrement # firing
	}
}

bool player_receive_fire(uint32_t x, uint32_t y) {			// check if you get shot
	if (dying) {							// if exploding
		return false;						// can't be killed
	}

	x = (x - position) / TANK_SCALE;				// normalize x
	y = (y - Y) / TANK_SCALE;					// normalize y

	if (x >= 0 && x < TANK_WIDTH && y >= AIR_SPACE && y < TANK_HEIGHT) {	// check position
		scoreboard_remove_life();				// die
		dying = true;						// explode
		tank_ticks = FIRST_FLASH;				// set up for flash
		flash_count = EXPLODE_FLASHES;				// flash a bunch of times
		draw_tank(ERASE);					// erase
		return true;						// indicate hit
	}
	return false;							// missed
}

void player_tick() {							// heartbeat
	int32_t new_position;						// temporary variable
	if (!dying) {							// alive
		if (left || right) {					// some button pressed
			if ( !(--tank_ticks) ) {			// decrement ticks
				new_position = position + direction;	// calculate new position
				if (new_position < X) {			// check left edge
					new_position = X;		// reset to edge
				} else if (new_position > MAX_POSITION) {	// check right
					new_position = MAX_POSITION;	// reset to edge
				}

				if (position != new_position) {		// if moved
					position = new_position;	// move
					draw_tank(DRAW);		// draw
				}
				tank_ticks = TICKS_PER_MOTION;		// reset counter
			}
		} else {
			// this causes the tank to move immediately when a button is pressed
			tank_ticks = 1;
		}
	} else {
		if ( !(--tank_ticks) ){					// count down
			if (--flash_count) {				// not done exploding
				draw_explosion();			// draw
				tank_ticks = TICKS_PER_FLASH;		// reset counter
			} else {
				draw_tank(ERASE);			// erase
				reset_tank();				// reset
			}
		}
		//printf("%d %d\n", flash_count, tank_ticks);
	}

	if (bullet.active) {						// bullet on screen
		draw_bullet(bullet.x, bullet.y, ERASE);			// erase
		bullet.y -= BULLET_SPEED*TANK_SCALE;			// move
		if (bullet.y <= BULLET_CEILING) {			// check out of bounds
			bullet.active = false;				// deactivate
		} else if (army_receive_fire(bullet.x, bullet.y)) {	// shoot army
			bullet.active = false;				// deactivate
		} else if (bunker_receive_fire(bullet.x, bullet.y)) {	// shoot army
			bullet.active = false;				// deactivate
		} else if (red_alien_receive_fire(bullet.x, bullet.y)) {	// check saucer
			bullet.active = false;				// deactivate
		} else {
			draw_bullet(bullet.x, bullet.y, DRAW);		// redraw
		}
	} else if (firing) {						// fire a new one
		bullet.x = position + TANK_MIDDLE;			// set x
		bullet.y = Y - (BULLET_HEIGHT*TANK_SCALE);		// set y
		bullet.active = true;					// on screen
		draw_bullet(bullet.x, bullet.y, DRAW);			// draw
	}
}

void player_init(){		// initialze stuff
	reset_tank();		// reset tank

	left = right = false;	// no buttons pressed
	firing = 0;		// no buttons pressed
	bullet.active = false;	// not on screen
	update_direction();	// set velocity
}
