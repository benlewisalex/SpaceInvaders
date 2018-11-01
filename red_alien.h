#ifndef RED_ALIEN_H
#define RED_ALIEN_H

#include <stdbool.h>
#include <stdint.h>

/* initialize player position, bunkers */
void red_alien_init();

/* takes care of motion, bullet firing */
void red_alien_tick();

/* return value indicates a hit */
bool red_alien_receive_fire(uint32_t x, uint32_t y);

#endif
