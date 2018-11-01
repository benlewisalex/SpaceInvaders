#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "icons.h"
#include "red_alien.h"
#include "scoreboard.h"
#include "video.h"

#define Y			18	// base offset for y

#define TICKS_PER_MOTION	2	// ticks allowed before red alien moves

#define RED_ALIEN_WIDTH		16	//width of the red alien
#define RED_ALIEN_HEIGHT	7	// height of the red alien
#define RED_ALIEN_SCALE		2	//scale of the red alien
#define MAX_POSITION		(COLUMNS-X-(RED_ALIEN_WIDTH*RED_ALIEN_SCALE))	//max x position allowed for the red alien to be

#define RANDOM_DIRECTION	(random() % 2)	// randomly chooses a direction for the red alien to travel
#define RANDOM_POINTS_VALUE	((random() % 6 + 1) * 50)	// randomly chooses point value for killing red alien
#define RANDOM_WAIT_TIME	((random() % 300) + 200)	// randomly chooses amount of time to wait for red alien

#define DRAW	false	// indicates that we are drawing
#define ERASE	true	//indicates that we are erasing

#define INIT_POSITION 10	//initial position of the red alien

#define RIGHT 1	//indicates the alien is going right
#define LEFT 0	// indicates the alien is going left

#define RIGHT_EDGE 298	// the right edge that the alien could hit
#define LEFT_EDGE 10	//the left edge that the alien could hit

#define MIN_COORD 0	// minimum coordinate of the red alien

#define X_OFF1 2	// the offset index to indicate how to erase an alien
#define X_OFF2 1 // another offset index to indicate how to erase an alien

#define SCALE 2	//the scale that we are erasing the alien

#define DRAW_BIT_MASK 0x01	// the bit mask used to write pixels

#define SHIFT 1	// the amount we are shifting to check pixels

#define INDEX1 0	// first index in our pixel array
#define INDEX2 1	// second index in our pixel array

static uint32_t position;	// the x position of the alien
static uint32_t tick_count;	// keeps track of how many ticks have occured
static bool is_flying;	// indicates if the alien is currently flying
static uint32_t wait_count;	//keeps track of how long we've waited
static uint32_t going_right;	//indicates if alien is heading right

//draws or erases an entire red alien
void draw_red_alien(bool erase){
	uint32_t color = erase ? BLACK : RED;//determines color
	for (uint32_t row = 0; row < RED_ALIEN_HEIGHT; row++){//loops through each row in alien sprite
		uint32_t red_alien_row = saucer_16x7[row];//find a alien row one at a time
		bool bytes[RED_ALIEN_WIDTH*RED_ALIEN_SCALE];//creates an array of bools to draw
		for(uint32_t col = 0; col < RED_ALIEN_WIDTH; col++){//loops through each col in alien sprite
			if (red_alien_row & DRAW_BIT_MASK) {//if pixel is on
				bytes[col*SCALE] = true;//set the pixel to be drawn
				bytes[(col*SCALE) + X_OFF2] = true;//set the next pixel to be drawn (scaled by 2)
			}
			else{
				bytes[col*SCALE] = true;//set the pixel to not be drawn
				bytes[(col*SCALE) + X_OFF2] = true;//set the next pixel to not be drawn (scaled by 2)
			}
			red_alien_row = red_alien_row >> SHIFT;//shifts our row over
		}
		video_writeLine(SCALE*position, (SCALE*(Y + row)), color, RED_ALIEN_WIDTH * SCALE, bytes);//writes one line
		video_writeLine(SCALE*position, (SCALE*(Y + row)) + X_OFF2, color, RED_ALIEN_WIDTH * SCALE, bytes);//writes the next line
	}
}
//erases the trailing part of an alien
void erase_red_alien(bool erase){
	uint32_t color = erase ? BLACK : RED;//determines color
	for (uint32_t row = 0; row < RED_ALIEN_HEIGHT; row++){//loops through each row in alien sprite
		bool bytes[RED_ALIEN_SCALE];//creates an array of bools to draw
		if(going_right){//if alien is going right
			bytes[INDEX1] = false;//set the pixel to be drawn
			bytes[INDEX2] = false;//set the next pixel to be drawn (scaled by 2)
			video_writeLine(SCALE*position, (SCALE*(Y + row)), color, SCALE, bytes);//writes one line
			video_writeLine(SCALE*position, (SCALE*(Y + row)) + X_OFF2, color, SCALE, bytes);//writes the next line
		}
		else{
			bytes[INDEX1] = false;//set the pixel to be drawn
			bytes[INDEX12] = false;//set the next pixel to be drawn (scaled by 2)
			video_writeLine(SCALE*(position + RED_ALIEN_WIDTH)-X_OFF, (SCALE*(Y + row)), color, SCALE, bytes);//writes one line
			video_writeLine(SCALE*(position + RED_ALIEN_WIDTH)-X_OFF, (SCALE*(Y + row)) + X_OFF2, color, SCALE, bytes);//writes the next line
		}
	}
}
//checks to see if a red alien was hit
bool red_alien_receive_fire(uint32_t x, uint32_t y) {
	if (!is_flying) {
		return false;//if its not even flying, return false
	}
	x = x / RED_ALIEN_SCALE - position;//scales x
	y = y / RED_ALIEN_SCALE - Y;//scales y

	if (x >= MIN_COORD && x < RED_ALIEN_WIDTH && y >= MIN_COORD && y < RED_ALIEN_HEIGHT) {//if we've hit the alien
		draw_red_alien(ERASE);//erase the alien
		is_flying = false;//set flying to false
		wait_count = RANDOM_WAIT_TIME;//randomize wait count
		going_right = RANDOM_DIRECTION;//randomize direction
		scoreboard_add(RANDOM_POINTS_VALUE);//randomize points added and add them to score
		return true;//return true because a hit happened
	}
	return false;//no hit happend return false
}
//tick function for the red alien
void red_alien_tick() {
	if (!is_flying){//if not flying
		tick_count++;//increment count
		if (tick_count >= wait_count){//if waited long enough
			is_flying = true;//fly awawy!
			if(going_right == RIGHT){//if going right
				position = INIT_POSITION;//set at INIT position
			}
			else{
				position = RIGHT_EDGE;//go to right edge
			}
			tick_count = 0;//reset tick count
		}
	}
	else{
		tick_count++;//increment tick count
		if (tick_count >= TICKS_PER_MOTION){//if waited long enough
			erase_red_alien(ERASE);//erase aliuen
			if(going_right == RIGHT){//if going right
				position++;//increment position
			}
			else{
				position--;//decrement position
			}
			if((position < RIGHT_EDGE && going_right == RIGHT) || (position > LEFT_EDGE && going_right == LEFT)){//if not reached edge
				draw_red_alien(DRAW);//draw alien
			}
			else{
				draw_red_alien(ERASE);//erase alien
				if(going_right == RIGHT){//if going right
					position--;//decrement position
				}
				else{
					position++;//increment position
				}
				draw_red_alien(ERASE);//erase alien
				is_flying = false;//not flying
				wait_count = RANDOM_WAIT_TIME;//randmoize wait count
				going_right = RANDOM_DIRECTION;//randomize direction
			}
			tick_count = 0;//reset tick count

		}
	}
}
//initialize the red alien
void red_alien_init(){
	position = INIT_POSITION;//initialize positionm
	is_flying = false;//currently not flying
	wait_count = RANDOM_WAIT_TIME;//randmoize wait time
	going_right = RANDOM_DIRECTION;//randomize direction
}
