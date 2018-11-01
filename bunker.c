#include "bunker.h"
#include "video.h"

#define Y_OFF 400  //y offset for the top of the bunkers
#define B_OFF 18  //base x offset for the bunkers make space between sides
#define NUM_BUNKERS 4 //number of bunkers
#define B0_MULT  1  // x multiplier to find bunker0 x offset
#define B1_MULT  3  // x multiplier to find bunker1 x offset
#define B2_MULT  5  // x multiplier to find bunker2 x offset
#define B3_MULT  7  // x multiplier to find bunker3 x offset
#define B_DIV    8  // divider to divide columns of screen to draw bunkers
#define B0_X  ((COLUMNS/B_DIV)* B0_MULT) - B_OFF  //bunker 0 x offset
#define B1_X  ((COLUMNS/B_DIV)* B1_MULT) - B_OFF  //bunker 1 x offset
#define B2_X  ((COLUMNS/B_DIV)* B2_MULT) - B_OFF  //bunker 2 x offset
#define B3_X  ((COLUMNS/B_DIV)* B3_MULT) - B_OFF  //bunker 3 x offset
#define B_SCALE 2 //bunker scale
#define B_HEIGHT 18 //bunker height
#define B_WIDTH 24  //bunker width
#define B_PIECE_DIM 6 //heigh and width of a bunker piece/section
#define B0_INDEX 0  //bunker 0 array index
#define B1_INDEX 1  //bunker 1 array index
#define B2_INDEX 2  //bunker 2 array index
#define B3_INDEX 3  //bunker 3 array index
#define TOP 0 //top row index
#define MID 1 //middle row index
#define BOT 2 //bottom row index
#define LEFT  //left col index
#define MLEFT 1  //middle left col index
#define MRIGHT 2  //middle right col index
#define RIGHT 3  //right col index
#define EDGE_OFF 1  //edge detection offset for bullet detection
#define B_ROWS 3  //number of bunker piece rows
#define B_COLS 4  //number of bunker piece columns
#define COL_TWO 2 //column two x multiplier offset
#define COL_THREE 3 //column three x multplier offset
#define COL_FOUR 4  //column four x multiplier offset

//enum used to indicate the current hit state of a section of a bunker
enum bunker_state{MAX, ONE_HIT, TWO_HITS, THREE_HITS, GONE};

/* struct used to represent a bunker */
typedef struct {
  enum bunker_state sections[B_ROWS][B_COLS];//double array holding all bunker section states
  uint32_t x_location;// x location of the bunker
} bunker_t;

//array of our four bunkers
static bunker_t bunkers[NUM_BUNKERS];

//sprite representing the entire bunker drawing
static  uint32_t FULL_BUNKER[B_HEIGHT][B_WIDTH] = {
 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
 {0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
 {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
 {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
 {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
 {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
 {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
 {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
 {1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1},
 {1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1},
 {1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1},
 {1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1},
 {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
 {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
 {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
 {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
 {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
 {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1}
};

//sprite representing a full bunker square
static  uint32_t bunker_piece_full[B_PIECE_DIM][B_PIECE_DIM] = {
 {1,1,1,1,1,1},
 {1,1,1,1,1,1},
 {1,1,1,1,1,1},
 {1,1,1,1,1,1},
 {1,1,1,1,1,1},
 {1,1,1,1,1,1}
};

//sprite representing the final damage mapping
static  uint32_t bunkerDamage0_6x6[B_PIECE_DIM][B_PIECE_DIM] = {
 {0,1,1,0,0,0},
 {0,0,0,0,0,1},
 {1,1,0,1,0,0},
 {1,0,0,0,0,0},
 {0,0,1,1,0,0},
 {0,0,0,0,1,0}
};

//sprite representing the third damage mapping
static  uint32_t bunkerDamage1_6x6[B_PIECE_DIM][B_PIECE_DIM] = {
 {1,1,1,0,1,0},
 {1,0,1,0,0,1},
 {1,1,0,1,1,1},
 {1,0,0,0,0,0},
 {0,1,1,1,0,1},
 {0,1,1,0,1,0}
};

//sprite representing the second damage mapping
static  uint32_t bunkerDamage2_6x6[B_PIECE_DIM][B_PIECE_DIM] = {
 {1,1,1,1,1,1},
 {1,0,1,1,0,1},
 {1,1,0,1,1,1},
 {1,1,0,1,1,0},
 {0,1,1,1,0,1},
 {1,1,1,1,1,1}
};

//sprite representing the first damage mapping
static  uint32_t bunkerDamage3_6x6[B_PIECE_DIM][B_PIECE_DIM] = {
 {1,1,1,1,1,1},
 {1,1,1,1,1,1},
 {1,1,1,1,1,1},
 {1,1,1,1,1,1},
 {1,1,1,1,1,1},
 {1,1,1,1,1,1}
};

//sprite representing the upper left section of a bunker
static  uint32_t  bunker_upper_left_gone_6x6[B_PIECE_DIM][B_PIECE_DIM] = {
 {0,0,0,1,1,1},
 {0,0,1,1,1,1},
 {0,1,1,1,1,1},
 {1,1,1,1,1,1},
 {1,1,1,1,1,1},
 {1,1,1,1,1,1}
};

//sprite representing the upper right section of a bunker
static  uint32_t  bunker_upper_right_gone_6x6[B_PIECE_DIM][B_PIECE_DIM] = {
 {1,1,1,0,0,0},
 {1,1,1,1,0,0},
 {1,1,1,1,1,0},
 {1,1,1,1,1,1},
 {1,1,1,1,1,1},
 {1,1,1,1,1,1}
};

//sprite representing the middle row, mid right column section of a bunker
static  uint32_t  bunker_lower_left_gone_6x6[B_PIECE_DIM][B_PIECE_DIM] = {
 {1,1,1,1,1,1},
 {1,1,1,1,1,1},
 {0,0,1,1,1,1},
 {0,0,0,1,1,1},
 {0,0,0,0,1,1},
 {0,0,0,0,0,1}
};

//sprite representing the middle row, mid left column section of a bunker
static  uint32_t  bunker_lower_right_gone_6x6[B_PIECE_DIM][B_PIECE_DIM] = {
 {1,1,1,1,1,1},
 {1,1,1,1,1,1},
 {1,1,1,1,0,0},
 {1,1,1,0,0,0},
 {1,1,0,0,0,0},
 {1,0,0,0,0,0}
};

//draws an entire bunker (all sections)
void draw_full_bunker(uint3a2_t x) {
	int row, col, scalar;//initialize rows and columns and scalar
	bool pixels[B_WIDTH*B_SCALE];//array of bools to represent pixels to be drawn
	for (row = 0; row < B_HEIGHT; ++row) {//loop the height of a bunker
		for (col = 0; col < B_WIDTH; ++col) {//loop the width of a bunker
			for (scalar = 0; scalar < B_SCALE; ++scalar) {//loop through our scalar
				pixels[col*B_SCALE+scalar] = FULL_BUNKER[row][col];//fill pixel aray with value of bunker at current row and col
			}
		}
		for (scalar = 0; scalar < B_SCALE; ++scalar) {//loop through scalar
			video_writeLine(//write a line of the bunker to the screen
				x,//x offset
				Y_OFF+B_SCALE*row+scalar,//y offset
				GREEN,//color
				B_WIDTH*B_SCALE,//length
				pixels//array of pixels to be written
			);
		}
	}
}

//draws a single bunker piece
void draw_bunker_piece(uint32_t x, uint32_t y, uint32_t piece[][B_PIECE_DIM]) {
	int row, col, scalar;//initialize rows and columns and scalar
	bool pixels[B_WIDTH*B_SCALE];//array of bools to represent pixels to be drawn
	for (row = 0; row < B_PIECE_DIM; ++row) {//loop the height of a bunker piece
		for (col = 0; col < B_PIECE_DIM; ++col) {//loop the width of a bunker piece
			for (scalar = 0; scalar < B_SCALE; ++scalar) {//loop through our scalar
				pixels[col*B_SCALE+scalar] = piece[row][col];//fill pixel aray with value of passed in bunker piece sprite
			}
		}
		for (scalar = 0; scalar < B_SCALE; ++scalar) {//loop through scalar
			video_writeLine(//write a line of the bunker to the screen
				x,//x offset
				y+B_SCALE*row+scalar,//y offset
				GREEN,//color
				B_PIECE_DIM*B_SCALE,//length
				pixels//array of pixels to be written
			);
		}
	}
}

//creates the necessary sprite to represent predetermined bunker damage level
void identify_damage(uint32_t x, uint32_t y, uint32_t piece[][B_PIECE_DIM], uint32_t damage[][B_PIECE_DIM]){
  uint32_t new_piece[B_PIECE_DIM][B_PIECE_DIM];//create a new double array to hold the entire value of a bunker piece
  for (uint32_t i = 0; i < B_PIECE_DIM; i++){//loop through height of bunker piece
    for (uint32_t j = 0; j < B_PIECE_DIM; j++){//loop through width of bunker piece
      new_piece[i][j] = piece[i][j] && damage[i][j];//create each new piece pixel by ANDing current piece with damage piece
    }
  }
  draw_bunker_piece(x, y, new_piece);//draw the newly created piece with damage applied
}

//performs calculations to determine which sprites must be ANDed to create newly damaged bunker piece
void redraw_bunker(uint32_t row, uint32_t col, uint32_t bunker_index){
  uint32_t damage_level[B_PIECE_DIM][B_PIECE_DIM];  //create a double array that will represent damage level
  for (uint32_t i = 0; i < B_PIECE_DIM; i++){//loop through bunker piece height
    for (uint32_t j = 0; j < B_PIECE_DIM; j++){//loop through bunker piece width
      if(bunkers[bunker_index].sections[row][col] == ONE_HIT){//if bunker piece has been hit once
        damage_level[i][j] = bunkerDamage2_6x6[i][j];//assign bunker damage level 2 (higher is less damage)
      }
      else if(bunkers[bunker_index].sections[row][col] == TWO_HITS){//if bunker piece has been hit twice
        damage_level[i][j] = bunkerDamage1_6x6[i][j];//assign bunker damage level 1 (higher is less damage)
      }
      else if(bunkers[bunker_index].sections[row][col] == THREE_HITS){//if bunker piece has been hit three times
        damage_level[i][j] = bunkerDamage0_6x6[i][j];//assign bunker damage level 0 (higher is less damage)
      }
      else if(bunkers[bunker_index].sections[row][col] == GONE){//if bunker piece has been destroyed
        damage_level[i][j] = 0;//assign damage to be all zeros, meaning the entire bunker is gone
      }
    }
  }
  if(row == TOP && col == LEFT){
    identify_damage(bunkers[bunker_index].x_location + B_PIECE_DIM*B_SCALE*col, Y_OFF + B_PIECE_DIM*B_SCALE*row, bunker_upper_left_gone_6x6, damage_level);
  }
  else if(row == TOP && col == MLEFT){
    identify_damage(bunkers[bunker_index].x_location + B_PIECE_DIM*B_SCALE*col, Y_OFF + B_PIECE_DIM*B_SCALE*row, bunker_piece_full, damage_level);
  }
  else if(row == TOP && col == MRIGHT){
    identify_damage(bunkers[bunker_index].x_location + B_PIECE_DIM*B_SCALE*col, Y_OFF + B_PIECE_DIM*B_SCALE*row, bunker_piece_full, damage_level);
  }
  else if(row == TOP && col == 3){
    identify_damage(bunkers[bunker_index].x_location + B_PIECE_DIM*B_SCALE*col, Y_OFF + B_PIECE_DIM*B_SCALE*row, bunker_upper_right_gone_6x6, damage_level);
  }
  else if(row == MID && col == LEFT){
    identify_damage(bunkers[bunker_index].x_location + B_PIECE_DIM*B_SCALE*col, Y_OFF + B_PIECE_DIM*B_SCALE*row, bunker_piece_full, damage_level);
  }
  else if(row == MID && col == MLEFT){
    identify_damage(bunkers[bunker_index].x_location + B_PIECE_DIM*B_SCALE*col, Y_OFF + B_PIECE_DIM*B_SCALE*row, bunker_lower_right_gone_6x6, damage_level);
  }
  else if(row == MID && col == MRIGHT){
    identify_damage(bunkers[bunker_index].x_location + B_PIECE_DIM*B_SCALE*col, Y_OFF + B_PIECE_DIM*B_SCALE*row, bunker_lower_left_gone_6x6, damage_level);
  }
  else if(row == MID && col == RIGHT){
    identify_damage(bunkers[bunker_index].x_location + B_PIECE_DIM*B_SCALE*col, Y_OFF + B_PIECE_DIM*B_SCALE*row, bunker_piece_full, damage_level);
  }
  else if(row == BOT && col == LEFT){
    identify_damage(bunkers[bunker_index].x_location + B_PIECE_DIM*B_SCALE*col, Y_OFF + B_PIECE_DIM*B_SCALE*row, bunker_piece_full, damage_level);
  }
  else if(row == BOT && col == RIGHT){
    identify_damage(bunkers[bunker_index].x_location + B_PIECE_DIM*B_SCALE*col, Y_OFF + B_PIECE_DIM*B_SCALE*row, bunker_piece_full, damage_level);
  }

}

//state machine for calculating the state of a bunker piece
bool decrement_bunker_integrity(uint32_t bunker_index, uint32_t row, uint32_t col){
  if (bunkers[bunker_index].sections[row][col] == MAX){//if bunker piece was at MAX
    bunkers[bunker_index].sections[row][col] = ONE_HIT;//set bunker piece to ONE_HIT
    redraw_bunker(row, col, bunker_index);//redraw the bunker piece
    return true;//return true because bunker piece was hit
  }
  else if(bunkers[bunker_index].sections[row][col] == ONE_HIT){//if bunker piece was at ONE_HIT
    bunkers[bunker_index].sections[row][col] = TWO_HITS;//set bunker piece to TWO_HITS
    redraw_bunker(row, col, bunker_index);//redraw the bunker piece
    return true;//return true because bunker piece was hit
  }
  else if(bunkers[bunker_index].sections[row][col] == TWO_HITS){//if bunker piece was at TWO_HITS
    bunkers[bunker_index].sections[row][col] = THREE_HITS;//set bunker piece to THREE_HITS
    redraw_bunker(row, col, bunker_index);//redraw the bunker piece
    return true;//return true because bunker piece was hit
  }
  else if(bunkers[bunker_index].sections[row][col] == THREE_HITS){//if bunker piece was at THREE_HITS
    bunkers[bunker_index].sections[row][col] = GONE;//set bunker piece to GONE
    redraw_bunker(row, col, bunker_index);//redraw the bunker piece
    return true;//return true because bunker piece was hit
  }
  else{
    return false;//return false because the bunker piece was already destroyed, so it can't be hit again
  }
}

//checks to see if a bullet hit a bunker piece area
bool check_bunker_hit(uint32_t x, uint32_t y, uint32_t bunker_index){
  //if in the first column x range
  if (x < bunkers[bunker_index].x_location + (B_WIDTH/B_SCALE)){
    //if in top row y range
    if (y < Y_OFF + (((B_HEIGHT*B_SCALE)/B_ROWS))){
      //if this bunker piece was hit, check to see if the integrity can be decremented further
      if(decrement_bunker_integrity(bunker_index, TOP, LEFT)){
        return true;//return true because there was a successful hit
      }
    }
    //if in middle row y range
    if (y >= Y_OFF + (((B_HEIGHT*B_SCALE)/B_ROWS)) && y < Y_OFF + (((B_HEIGHT*B_SCALE)/B_ROWS)*COL_TWO)){
      //if this bunker piece was hit, check to see if the integrity can be decremented further
      if(decrement_bunker_integrity(bunker_index, MID, LEFT)){
        return true;//return true because there was a successful hit
      }
    }
    //if in bottom row y range
    if (y >= Y_OFF + (((B_HEIGHT*B_SCALE)/B_ROWS)*COL_TWO) && y < Y_OFF + (((B_HEIGHT*B_SCALE)/B_ROWS)*COL_THREE)){
      //if this bunker piece was hit, check to see if the integrity can be decremented further
      if(decrement_bunker_integrity(bunker_index, BOT, LEFT)){
        return true;//return true because there was a successful hit
      }
    }
  }
  //if in the second column x range
  if (x >= bunkers[bunker_index].x_location + (B_WIDTH/B_SCALE) && x < bunkers[bunker_index].x_location + ((B_WIDTH/B_SCALE)*COL_TWO)){
    //if in top row y range
    if (y < Y_OFF + (((B_HEIGHT*B_SCALE)/B_ROWS))){
      //if this bunker piece was hit, check to see if the integrity can be decremented further
      if(decrement_bunker_integrity(bunker_index, TOP, MLEFT)){
        return true;//return true because there was a successful hit
      }
    }
    //if in middle row y range
    else if (y >= Y_OFF + (((B_HEIGHT*B_SCALE)/B_ROWS)) && y < Y_OFF + (((B_HEIGHT*B_SCALE)/B_ROWS)*COL_TWO)){
      //if this bunker piece was hit, check to see if the integrity can be decremented further
      if(decrement_bunker_integrity(bunker_index, MID, MLEFT)){
        return true;//return true because there was a successful hit
      }
    }
    //if in bottom row y range
    else{
      //if this bunker piece was hit, check to see if the integrity can be decremented further
      if(decrement_bunker_integrity(bunker_index, BOT, MLEFT)){
        return true;//return true because there was a successful hit
      }
    }  }
  //if in the third column x range
  if (x >= bunkers[bunker_index].x_location + ((B_WIDTH/B_SCALE)*COL_TWO) && x < bunkers[bunker_index].x_location + ((B_WIDTH/B_SCALE)*COL_THREE)){
    //if in top row y range
    if (y < Y_OFF + (((B_HEIGHT*B_SCALE)/B_ROWS))){
      //if this bunker piece was hit, check to see if the integrity can be decremented further
      if(decrement_bunker_integrity(bunker_index, TOP, MRIGHT)){
        return true;//return true because there was a successful hit
      }
    }
    //if in middle row y range
    else if (y >= Y_OFF + (((B_HEIGHT*B_SCALE)/B_ROWS)) && y < Y_OFF + (((B_HEIGHT*B_SCALE)/B_ROWS)*COL_TWO)){
      //if this bunker piece was hit, check to see if the integrity can be decremented further
      if(decrement_bunker_integrity(bunker_index, MID, MRIGHT)){
        return true;//return true because there was a successful hit
      }
    }
    //if in bottom row y range
    else{
      //if this bunker piece was hit, check to see if the integrity can be decremented further
      if(decrement_bunker_integrity(bunker_index, BOT, MRIGHT)){
        return true;//return true because there was a successful hit
      }
    }  }
  //if in the fourth (last) column x range
  if (x >= bunkers[bunker_index].x_location + ((B_WIDTH/B_SCALE)*B_ROWS) && x < bunkers[bunker_index].x_location + ((B_WIDTH/B_SCALE)*COL_FOUR)){
    //if in top row y range
    if (y < Y_OFF + (((B_HEIGHT*B_SCALE)/B_ROWS))){
      //if this bunker piece was hit, check to see if the integrity can be decremented further
      if(decrement_bunker_integrity(bunker_index, TOP, RIGHT)){
        return true;//return true because there was a successful hit
      }
    }
    //if in middle row y range
    else if (y >= Y_OFF + (((B_HEIGHT*B_SCALE)/B_ROWS)) && y < Y_OFF + (((B_HEIGHT*B_SCALE)/B_ROWS)*COL_TWO)){
      //if this bunker piece was hit, check to see if the integrity can be decremented further
      if(decrement_bunker_integrity(bunker_index, MID, RIGHT)){
        return true;//return true because there was a successful hit
      }
    }
    //if in bottom row y range
    else{
      //if this bunker piece was hit, check to see if the integrity can be decremented further
      if(decrement_bunker_integrity(bunker_index, BOT, RIGHT)){
        return true;//return true because there was a successful hit
      }
    }  }
    return false;//return false because no hit was detected
}

//checks to see if a bullet is within the coordinates where a bunker could be
bool bunker_receive_fire(uint32_t x, uint32_t y){
  if (y > (Y_OFF + (B_SCALE*B_HEIGHT)) || y < Y_OFF){//if not within the y range of a bunker
    return false;//return false, not in bunker range
  }
  if(x >= B0_X-EDGE_OFF && x < (B0_X + (B_WIDTH*B_SCALE))){//if within x range of bunker0
    return check_bunker_hit(x, y, B0_INDEX);  //check to see if a bunker was successfully hit and return result
  }
  if(x >= B1_X-EDGE_OFF && x < (B1_X + (B_WIDTH*B_SCALE))){//if within x range of bunker1
    return check_bunker_hit(x, y, B1_INDEX);  //check to see if a bunker was successfully hit and return result
  }
  if(x >= B2_X-EDGE_OFF && x < (B2_X + (B_WIDTH*B_SCALE))){//if within x range of bunker2
    return check_bunker_hit(x, y, B2_INDEX);  //check to see if a bunker was successfully hit and return result
  }
  if(x >= B3_X-EDGE_OFF && x <= (B3_X + (B_WIDTH*B_SCALE))){//if within x range of bunker3
    return check_bunker_hit(x, y, B3_INDEX);  //check to see if a bunker was successfully hit and return result
  }
  return false;//return false because not within the x range of bunkers
}

//initializes the bunkers and each bunker piece
void initialize_bunker(bunker_t bunker, uint32_t index){
  bunker.sections[TOP][LEFT] = MAX;     //sets top left bunker piece to MAX health
  bunker.sections[TOP][MLEFT] = MAX;    //sets top mid-left bunker piece to MAX health
  bunker.sections[TOP][MRIGHT] = MAX;   //sets top mid-right bunker piece to MAX health
  bunker.sections[TOP][RIGHT] = MAX;    //sets top right bunker piece to MAX health
  bunker.sections[MID][LEFT] = MAX;     //sets middle left bunker piece to MAX health
  bunker.sections[MID][MLEFT] = MAX;    //sets middle mid-left bunker piece to MAX health
  bunker.sections[MID][MRIGHT] = MAX;   //sets middle right bunker piece to MAX health
  bunker.sections[MID][RIGHT] = MAX;    //sets middle right bunker piece to MAX health
  bunker.sections[BOT][LEFT] = MAX;     //sets bottom left bunker piece to MAX health
  bunker.sections[BOT][MLEFT] = GONE;   //sets bottom mid-left bunker piece to GONE health (doesn't exist)
  bunker.sections[BOT][MRIGHT] = GONE;  //sets bottom right bunker piece to GONE health (doesn't exist)
  bunker.sections[BOT][RIGHT] = MAX;    //sets bottom right bunker piece to MAX health
  bunkers[index] = bunker;  //stores the passed-in bunker in our bunkers array at the provided index
}

//initializes the bunkers
void bunker_init(){
  draw_full_bunker(B0_X);//draws bunker0
  draw_full_bunker(B1_X);//draws bunker1
  draw_full_bunker(B2_X);//draws bunker2
  draw_full_bunker(B3_X);//draws bunker3
  bunker_t bunker0;//defines bunker0
  bunker0.x_location = B0_X;//sets bunker0's x location
  initialize_bunker(bunker0, B0_INDEX);//initializes bunker0
  bunker_t bunker1;//defines bunker1
  bunker0.x_location = B1_X;//sets bunker1's x location
  initialize_bunker(bunker0, B1_INDEX);//initializes bunker1
  bunker_t bunker2;//defines bunker2
  bunker0.x_location = B2_X;//sets bunker2's x location
  initialize_bunker(bunker0, B2_INDEX);//initializes bunker2
  bunker_t bunker3;//defines bunker3
  bunker0.x_location = B3_X;//sets bunker3's x location
  initialize_bunker(bunker0, B3_INDEX);//initializes bunker3
}
