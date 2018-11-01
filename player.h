#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include <stdint.h>

#define DRAW	false
#define ERASE	true

#define TANK_SCALE		2
#define TANK_WIDTH		17
#define TANK_HEIGHT		8

/* initialize player position, bunkers */
void player_init();

/* takes care of motion, bullet firing */
void player_tick();

/* allows lives.c to draw and erase tanks */
void player_draw_tank(uint32_t x, uint32_t y, bool erase);

/* called when the left button is pressed or released */
void player_left_button(bool pressed);

/* called when the right button is pressed or released */
void player_right_button(bool pressed);

/* called when the fire button is pressed or released */
void player_fire_button(bool pressed);

/* each alien bullet calls this to see if they hit the player */
bool player_receive_fire(uint32_t x, uint32_t y);

#endif
