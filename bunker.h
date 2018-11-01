#ifndef BUNKER_H
#define BUNKER_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

//initializes the bunkers
void bunker_init();
//calculates to see if a bunker is under fire
bool bunker_receive_fire(uint32_t x, uint32_t y);
#endif
