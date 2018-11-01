#ifndef GAME_OVER_H
#define GAME_OVER_H

#include <stdbool.h>
#include <stdint.h>

static bool game_is_over; //indicates if game is over or not
static bool name_selected;  //indicates if a name has been selected or not

//ends the game
void game_over_end_game(uint32_t score);
//tick function for the end game screen
void game_over_tick();
//initializese the game over scenario
void game_over_init();
//gets the current state of game over
bool game_over_get_state();
//sets the state of whether or not we've picked a name
void game_over_set_name_state(bool is_over);
//gets the state of whether we've chosen a name
bool game_over_get_name_state();
//decrements the letter we are on
void game_over_decrement_letter(bool pressed);
//increments the letter we are on
void game_over_increment_letter(bool pressed);
//advances to the next letter
void game_over_advance_letter(bool pressed);

#endif
