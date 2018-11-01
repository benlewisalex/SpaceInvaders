#include <stdio.h>
#include <string.h>

#include "game_over.h"
#include "text.h"
#include "video.h"

#define FLASH_RATE 25				//tick rate for flashes to occur
#define NAME_SIZE 5	//oversize name for buffer space
#define TRUE_NAME_SIZE 3	//true size for name
#define NUM_HIGH_SCORES 10	//number of high scores to be used
#define SCORE_DIGITS	5	//num digits in a score
#define LETTER_INIT_X 290	// initial x coordinate for the first letter
#define LETTER_INIT_Y 120	// initial y coordinate for the first letter
#define MAX_NAME_INDEX 2	//2 is max index for letters (0-2)
#define HS_OFF 1	//high scores x offset
#define NUM_SCORES 10	//number of high scores
#define NAME_X_INIT 260	//initial x of the names
#define NAME_Y_INIT 150 //initial y of the names
#define NAME_Y_INC 20 //standard y offset for each new name
#define GAP 6	//gap between the name and the score
#define LETTER_ALTER 1	//increments or decrements a letter value
#define FLASH_ON 3	//time letter is flashed on
#define FLASH_OFF 2	//time letter is flashed off
#define Y_OFF_EYN 100	//y offset for ENter Your Name
#define Y_OFF_GO 50	//y offset for GAME OVER
#define X_OFF_EYN 200	//x offset for ENTER YOUR NAME
#define X_OFF_GO 75	// x offset for GAME OVER
#define HALF_SCALE 2	// used to half known, constant sizes


static uint32_t name_index;	//current letter we are on
static char old_top_names[NUM_HIGH_SCORES][NAME_SIZE];//array of old high score names
static uint32_t old_top_scores[NUM_HIGH_SCORES]; //array of old high score scores
static char new_top_names[NUM_HIGH_SCORES][NAME_SIZE];//array of new high score names
static uint32_t new_top_scores[NUM_HIGH_SCORES];//array of new high score scores
static bool is_initialized;	//indicates if the game over scenario has been initialized
static uint32_t tick_count;	//coutns the ticks that have ocurred
static uint32_t current_letter_x;	//x coordinate of current letter we are on
static uint32_t current_letter_y; //y coordinate of current letter we are on
static char current_name[TRUE_NAME_SIZE] = {'A','A','A'};//current name as displayed on the screen
static uint32_t hs_offset_x;//the high scores x offset being used
static uint32_t hs_offset_y;//the high scores y offset being used
static uint32_t player_score;//the player's score

//called when the game has ended
void game_over_end_game(uint32_t score){
	player_score = score;//update player score
	game_is_over = true;//set game over flag to true
}
// gets state of game over boolean
bool game_over_get_state(){
  return game_is_over; //return game is over flag
}
// sets the state of game over boolean
void game_over_set_name_state(bool is_selected){
  name_selected = is_selected;	//set the value to what is passed in
}
// returns whether or not a name has been selected
bool game_over_get_name_state(){
  return name_selected;
}
//the true initializer of the game over screen. Called when the game is actually over.
void true_init(){
  video_writeScreen(BLACK);//black the screen
  int i;
	uint32_t offset = X_OFF_GO;//initialize to game over offset
	char * string = "GAME";//create word
	for (i = 0; i < strlen(string); ++i) {
		text_drawLargeLetter(ALPHA(string[i]), offset, Y_OFF_GO, WHITE);//draw word to screen
		offset += LARGE_PADDED_WIDTH;//adjust x_offset
	}
  offset += LARGE_PADDED_WIDTH/HALF_SCALE;//adjust x_offset
  string = "OVER";//create word
	for (i = 0; i < strlen(string); ++i) {
		text_drawLargeLetter(ALPHA(string[i]), offset, Y_OFF_GO, WHITE);//draw word to screen
		offset += LARGE_PADDED_WIDTH;//adjust x_offset
	}
  offset = X_OFF_EYN;//adjust x_offset
  string = "ENTER";//create word
	for (i = 0; i < strlen(string); ++i) {
		text_drawLetter(ALPHA(string[i]), offset, Y_OFF_EYN, WHITE);//draw word to screen
		offset += PADDED_WIDTH;//adjust x_offset
	}
  offset += PADDED_WIDTH;//adjust x_offset
  string = "YOUR";//create word
	for (i = 0; i < strlen(string); ++i) {
		text_drawLetter(ALPHA(string[i]), offset, Y_OFF_EYN, WHITE);//draw word to screen
		offset += PADDED_WIDTH;//adjust x_offset
	}
  offset += PADDED_WIDTH;//adjust x_offset
  string = "NAME";//create word
  for (i = 0; i < strlen(string); ++i) {
    text_drawLetter(ALPHA(string[i]), offset, Y_OFF_EYN, WHITE);//draw word to screen
    offset += PADDED_WIDTH;//adjust x_offset
  }
  offset = current_letter_x;//adjust x_offset
  string = current_name;//create word
  for (i = 0; i < strlen(string); ++i) {
    text_drawLetter(ALPHA(string[i]), offset, current_letter_y  , WHITE);//draw word to screen
    offset += PADDED_WIDTH;//adjust x_offset
  }
}
//flashes the current letter
void flash_name(){
  if(tick_count == FLASH_RATE*FLASH_OFF){//if its time to flash the letter off
    text_drawLetter(ALPHA(current_name[name_index]), current_letter_x, current_letter_y  , BLACK);//flash off
  }
  else if (tick_count >= FLASH_RATE*FLASH_ON){//if its time to flash the letter on
    tick_count = 0;//reset tick counter
    text_drawLetter(ALPHA(current_name[name_index]), current_letter_x, current_letter_y  , WHITE);//flash on
  }
}

//decrements the current letter we are on
void game_over_decrement_letter(bool pressed){
  if(pressed){//if button is pressed
    text_drawLetter(ALPHA(current_name[name_index]), current_letter_x, current_letter_y  , BLACK);//erase letter
    if(current_name[name_index] == 'A'){//if at A
      current_name[name_index] = 'Z';//go to z
    }
    else{
      current_name[name_index] = current_name[name_index] - LETTER_ALTER;//decrement letter
    }
    text_drawLetter(ALPHA(current_name[name_index]), current_letter_x, current_letter_y  , WHITE);//draw new letter
  }
}
//increments the current letter we are on
void game_over_increment_letter(bool pressed){
  if(pressed){//if button is pressed
    text_drawLetter(ALPHA(current_name[name_index]), current_letter_x, current_letter_y  , BLACK);//erase letter
    if(current_name[name_index] == 'Z'){//if at Z
      current_name[name_index] = 'A';//go to A
    }
    else{
      current_name[name_index] = current_name[name_index] + LETTER_ALTER;//increment letter
    }
    text_drawLetter(ALPHA(current_name[name_index]), current_letter_x, current_letter_y  , WHITE);//draw new letter
  }
}
//draw high scores to screen
void draw_high_scores() {
  hs_offset_x = NAME_X_INIT;//initilize x coordinate to initial name x
  hs_offset_y = NAME_Y_INIT;//initilize y coordinate to initial name y
  for (uint32_t i = 0; i < NUM_HIGH_SCORES; i++){//loop through each high score
    char * string = new_top_names[i];//grab the name
    for (uint32_t j = 0; j < strlen(string); ++j) {//loop through each character
      text_drawLetter(ALPHA(string[j]), hs_offset_x, hs_offset_y, WHITE);//draw character
      hs_offset_x += PADDED_WIDTH;//adjust x_offset
    }
    hs_offset_x += PADDED_WIDTH* GAP;//adjust x_offset and provide nme and score gap

    uint32_t s = new_top_scores[i];//get the top score
    uint8_t digit;
    for (uint32_t k = SCORE_DIGITS; k > 0; --k) {//loop[ through each digit]
  		hs_offset_x -= PADDED_WIDTH;//adjust x_offset
  		digit = s % NUMBER_SYSTEM;//digit is s mod number of digits (10)
  		s /= NUMBER_SYSTEM; //s divided by 10 is our true digit
  		text_drawLetter(DIGITS[digit], hs_offset_x, hs_offset_y, GREEN);//draw score digit to screen
  	}
    hs_offset_x = NAME_X_INIT;//adjust x_offset
    hs_offset_y += NAME_Y_INC;//adjust y_offset
  }
}
//read high scores
void read_high_scores(){
  FILE *fp;//create file pointer
	char* filename = "high_scores.txt";//define file name

	fp = fopen(filename, "r+");//open file
  rewind(fp);//go to beginning of file
	if (fp == NULL){//if file is null
			printf("Could not open file %s",filename);//file not found
			return;//go back
	}
	for(uint32_t i = 0; i < NUM_SCORES; i++){//loop through number of scores
    char temp_name[NAME_SIZE];//create a temporary name char array
    uint32_t temp_score;//creat a temporary score
    fscanf(fp, "%s", temp_name);//read in the name
    fscanf(fp,"%d", &temp_score);//read in the score
    old_top_scores[i] = temp_score;//write to global array for scores
    strcpy(old_top_names[i], temp_name);//write to global array for names
	}
	fclose(fp);//close the file
	return;//done
}
//compares old scores with current player's scores
void check_new_score(){
  for(uint32_t i = 0; i < NUM_HIGH_SCORES; i++){//loop through current scores
    if(player_score >= old_top_scores[i]){//if player score is better
      strcpy(new_top_names[i], current_name);//squeeze player into this index in the array
      new_top_scores[i] = player_score;//squeeze player's score in, too
      i++;//advance to the next index
      for(uint32_t j = i; j < NUM_HIGH_SCORES; j++){//loop through the rest of the high scores
        strcpy(new_top_names[j], old_top_names[j-HS_OFF]);//move each name one down in the array
        new_top_scores[j] = old_top_scores[j-HS_OFF];//move each score one down in the array
      }
      i = NUM_HIGH_SCORES;//set i to finish for loop immediately
    }
    else{
      strcpy(new_top_names[i], old_top_names[i]);//simply copy name over, no changes
      new_top_scores[i] = old_top_scores[i];//simply copy score over, no changes
    }
  }
}
//writes high scores to text file
void write_high_scores(){
  FILE * fp;//create file pointer
  fp = fopen ("high_scores.txt", "w+");//opens the file
  for(uint32_t i = 0; i < NUM_HIGH_SCORES; i++){//loops through all high scores
    fprintf(fp, "%s %d\n", new_top_names[i], new_top_scores[i]);//writes high score to file
  }

  fclose(fp);//closes file
}
//advances to the next letter to choose
void game_over_advance_letter(bool pressed){
  if(pressed){//if button is pressed
    text_drawLetter(ALPHA(current_name[name_index]), current_letter_x, current_letter_y  , WHITE);//draw letter
    name_index++;//advance to the next letter index in name
    current_letter_x += PADDED_WIDTH;//adjust x offset to the new letter
    if(name_index > MAX_NAME_INDEX){//if we've already done all three letters
      game_over_set_name_state(true);//set name state to true (completed)
      read_high_scores();//read current high scores
      check_new_score();//see if current player should be added to high scores list
      draw_high_scores();//draw high scores to screen
      write_high_scores();//write high scores back to file
    }
  }
}
//tick function for the game over scenario
void game_over_tick(){
  if(!is_initialized){//if the game is over
    true_init();//truly initialize the game over screen
    is_initialized = true;//set the boolean to indicate a name is chosen
  }
  tick_count++;//increment tick count
  flash_name();//flash the name
}


void game_over_init(){
  game_is_over = false;//initialize game over state to be false
  is_initialized = false;//initialize the state of a name being chosen to false
  tick_count = 0;//initialize tick count to 0
  current_letter_x = LETTER_INIT_X;//set x coordinate to initial letter offset
  current_letter_y = LETTER_INIT_Y;//set y coordinate to initial letter offset
  name_index = 0;//initialize name index to the first letter
  game_over_set_name_state(false);//set current state of chosen name to false
  player_score = 0;//initialize player score to 0 as seen by game over scenario
}
