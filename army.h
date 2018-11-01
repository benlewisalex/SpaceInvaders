#ifndef ARMY_H
#define ARMY_H

#include <stdbool.h>
#include <stdint.h>

/* call on every tick */
void army_tick();

/* draw stuff and set variables */
void army_init();

/* called every time a user bullet moves */
/* return value indicates a hit */
bool army_receive_fire(uint32_t x, uint32_t y);
#endif
