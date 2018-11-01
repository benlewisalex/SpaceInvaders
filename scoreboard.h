#include <stdint.h>

/* initialize SCORE and LIVES */
void scoreboard_init();

/* remove a life; trigger game over screen on the last life*/
void scoreboard_add_life();

/* add a life; max 5 */
void scoreboard_remove_life();

/* increment score by 'points' */
void scoreboard_add(uint32_t points);
