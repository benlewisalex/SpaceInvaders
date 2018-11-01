#include <stdint.h>

#define LETTER_WIDTH	5	// letter width
#define LETTER_HEIGHT	5	// letter height
#define LETTER_SCALE	2	// real pixels per image pixel
#define LETTER_SPACE	2	// pixels between characters
#define PADDED_WIDTH	((LETTER_WIDTH+LETTER_SPACE)*LETTER_SCALE)//width between letters
#define LARGE_LETTER_SCALE	8	//scale for large letters
#define LARGE_PADDED_WIDTH	((LETTER_WIDTH+LETTER_SPACE)*LARGE_LETTER_SCALE)//width for large letters

#define ALPHABET_SIZE	26
#define NUMBER_SYSTEM	10

static const uint8_t LETTER_A[LETTER_HEIGHT][LETTER_WIDTH] = {
	{0,1,1,1,0},
	{1,0,0,0,1},
	{1,1,1,1,1},
	{1,0,0,0,1},
	{1,0,0,0,1}
};

static const uint8_t LETTER_B[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,1,1,1,0},
	{1,0,0,0,1},
	{1,1,1,1,1},
	{1,0,0,0,1},
	{1,1,1,1,0}
};

static const uint8_t LETTER_C[LETTER_HEIGHT][LETTER_WIDTH] = {
	{0,1,1,1,1},
	{1,0,0,0,0},
	{1,0,0,0,0},
	{1,0,0,0,0},
	{0,1,1,1,1}
};

static const uint8_t LETTER_D[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,1,1,1,0},
	{1,0,0,0,1},
	{1,0,0,0,1},
	{1,0,0,0,1},
	{1,1,1,1,0}
};

static const uint8_t LETTER_E[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,1,1,1,1},
	{1,0,0,0,0},
	{1,1,1,1,0},
	{1,0,0,0,0},
	{1,1,1,1,1}
};

static const uint8_t LETTER_F[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,1,1,1,1},
	{1,0,0,0,0},
	{1,1,1,1,0},
	{1,0,0,0,0},
	{1,0,0,0,0}
};

static const uint8_t LETTER_G[LETTER_HEIGHT][LETTER_WIDTH] = {
	{0,1,1,1,1},
	{1,0,0,0,0},
	{1,0,0,1,1},
	{1,0,0,0,1},
	{0,1,1,1,0}
};

static const uint8_t LETTER_H[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,0,0,0,1},
	{1,0,0,0,1},
	{1,1,1,1,1},
	{1,0,0,0,1},
	{1,0,0,0,1}
};

static const uint8_t LETTER_I[LETTER_HEIGHT][LETTER_WIDTH] = {
	{0,0,1,0,0},
	{0,0,1,0,0},
	{0,0,1,0,0},
	{0,0,1,0,0},
	{0,0,1,0,0}
};

static const uint8_t LETTER_J[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,1,1,1,1},
	{0,0,1,0,0},
	{0,0,1,0,0},
	{0,0,1,0,0},
	{1,1,1,0,0}
};

static const uint8_t LETTER_K[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,0,0,0,1},
	{1,0,0,1,0},
	{1,1,1,0,0},
	{1,0,0,1,0},
	{1,0,0,0,1}
};

static const uint8_t LETTER_L[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,0,0,0,0},
	{1,0,0,0,0},
	{1,0,0,0,0},
	{1,0,0,0,0},
	{1,1,1,1,1}
};

static const uint8_t LETTER_M[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,0,0,0,1},
	{1,1,0,1,1},
	{1,0,1,0,1},
	{1,0,0,0,1},
	{1,0,0,0,1}
};

static const uint8_t LETTER_N[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,0,0,0,1},
	{1,1,0,0,1},
	{1,0,1,0,1},
	{1,0,0,1,1},
	{1,0,0,0,1}
};

static const uint8_t LETTER_O[LETTER_HEIGHT][LETTER_WIDTH] = {
	{0,1,1,1,0},
	{1,0,0,0,1},
	{1,0,0,0,1},
	{1,0,0,0,1},
	{0,1,1,1,0}
};

static const uint8_t LETTER_P[LETTER_HEIGHT][LETTER_WIDTH] = {
	{0,1,1,1,0},
	{1,0,0,0,1},
	{1,1,1,1,1},
	{1,0,0,0,0},
	{1,0,0,0,0}
};

static const uint8_t LETTER_Q[LETTER_HEIGHT][LETTER_WIDTH] = {
	{0,1,1,1,0},
	{1,0,0,0,1},
	{1,0,0,0,1},
	{1,0,0,1,0},
	{0,1,1,0,1}
};

static const uint8_t LETTER_R[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,1,1,1,0},
	{1,0,0,0,1},
	{1,1,1,1,0},
	{1,0,0,0,1},
	{1,0,0,0,1}
};

static const uint8_t LETTER_S[LETTER_HEIGHT][LETTER_WIDTH] = {
	{0,1,1,1,1},
	{1,0,0,0,0},
	{0,1,1,1,0},
	{0,0,0,0,1},
	{1,1,1,1,0}
};

static const uint8_t LETTER_T[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,1,1,1,1},
	{0,0,1,0,0},
	{0,0,1,0,0},
	{0,0,1,0,0},
	{0,0,1,0,0}
};

static const uint8_t LETTER_U[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,0,0,0,1},
	{1,0,0,0,1},
	{1,0,0,0,1},
	{1,0,0,0,1},
	{0,1,1,1,0}
};

static const uint8_t LETTER_V[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,0,0,0,1},
	{1,0,0,0,1},
	{1,0,0,0,1},
	{0,1,0,1,0},
	{0,0,1,0,0}
};

static const uint8_t LETTER_W[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,0,0,0,1},
	{1,0,0,0,1},
	{1,0,1,0,1},
	{1,0,1,0,1},
	{0,1,0,1,0}
};

static const uint8_t LETTER_X[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,0,0,0,1},
	{0,1,0,1,0},
	{0,0,1,0,0},
	{0,1,0,1,0},
	{1,0,0,0,1}
};

static const uint8_t LETTER_Y[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,0,0,0,1},
	{0,1,0,1,0},
	{0,0,1,0,0},
	{0,0,1,0,0},
	{0,0,1,0,0}
};

static const uint8_t LETTER_Z[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,1,1,1,1},
	{0,0,0,1,0},
	{0,0,1,0,0},
	{0,1,0,0,0},
	{1,1,1,1,1}
};

static const uint8_t DIGIT_0[LETTER_HEIGHT][LETTER_WIDTH] = {
	{0,1,1,1,0},
	{1,0,0,0,1},
	{1,0,0,0,1},
	{1,0,0,0,1},
	{0,1,1,1,0}
};

static const uint8_t DIGIT_1[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,1,1,0,0},
	{0,0,1,0,0},
	{0,0,1,0,0},
	{0,0,1,0,0},
	{1,1,1,1,1}
};

static const uint8_t DIGIT_2[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,1,1,1,1},
	{0,0,0,0,1},
	{1,1,1,1,1},
	{1,0,0,0,0},
	{1,1,1,1,1}
};

static const uint8_t DIGIT_3[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,1,1,1,1},
	{0,0,0,0,1},
	{1,1,1,1,1},
	{0,0,0,0,1},
	{1,1,1,1,1}
};

static const uint8_t DIGIT_4[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,0,0,0,1},
	{1,0,0,0,1},
	{1,1,1,1,1},
	{0,0,0,0,1},
	{0,0,0,0,1}
};

static const uint8_t DIGIT_5[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,1,1,1,1},
	{1,0,0,0,0},
	{1,1,1,1,1},
	{0,0,0,0,1},
	{1,1,1,1,1}
};

static const uint8_t DIGIT_6[LETTER_HEIGHT][LETTER_WIDTH] = {
	{0,1,1,1,1},
	{1,0,0,0,0},
	{1,1,1,1,1},
	{1,0,0,0,1},
	{1,1,1,1,1}
};

static const uint8_t DIGIT_7[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,1,1,1,1},
	{0,0,0,0,1},
	{0,0,0,0,1},
	{0,0,0,0,1},
	{0,0,0,0,1}
};

static const uint8_t DIGIT_8[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,1,1,1,1},
	{1,0,0,0,1},
	{1,1,1,1,1},
	{1,0,0,0,1},
	{1,1,1,1,1}
};

static const uint8_t DIGIT_9[LETTER_HEIGHT][LETTER_WIDTH] = {
	{1,1,1,1,1},
	{1,0,0,0,1},
	{1,1,1,1,1},
	{0,0,0,0,1},
	{1,1,1,1,1}
};
// this is the array holding our alphabet sprites
static const uint8_t * ALPHABET[ALPHABET_SIZE] = {
	(const uint8_t *) LETTER_A,
	(const uint8_t *) LETTER_B,
	(const uint8_t *) LETTER_C,
	(const uint8_t *) LETTER_D,
	(const uint8_t *) LETTER_E,
	(const uint8_t *) LETTER_F,
	(const uint8_t *) LETTER_G,
	(const uint8_t *) LETTER_H,
	(const uint8_t *) LETTER_I,
	(const uint8_t *) LETTER_J,
	(const uint8_t *) LETTER_K,
	(const uint8_t *) LETTER_L,
	(const uint8_t *) LETTER_M,
	(const uint8_t *) LETTER_N,
	(const uint8_t *) LETTER_O,
	(const uint8_t *) LETTER_P,
	(const uint8_t *) LETTER_Q,
	(const uint8_t *) LETTER_R,
	(const uint8_t *) LETTER_S,
	(const uint8_t *) LETTER_T,
	(const uint8_t *) LETTER_U,
	(const uint8_t *) LETTER_V,
	(const uint8_t *) LETTER_W,
	(const uint8_t *) LETTER_X,
	(const uint8_t *) LETTER_Y,
	(const uint8_t *) LETTER_Z
};
#define ALPHA(chr)	(ALPHABET[chr-'A'])//takes a char and creates an index out of it
// this is the array holding our digit sprites
static const uint8_t * DIGITS[NUMBER_SYSTEM] = {
	(const uint8_t *) DIGIT_0,
	(const uint8_t *) DIGIT_1,
	(const uint8_t *) DIGIT_2,
	(const uint8_t *) DIGIT_3,
	(const uint8_t *) DIGIT_4,
	(const uint8_t *) DIGIT_5,
	(const uint8_t *) DIGIT_6,
	(const uint8_t *) DIGIT_7,
	(const uint8_t *) DIGIT_8,
	(const uint8_t *) DIGIT_9
};

/* draw the given image at standard size */
void text_drawLetter(const uint8_t * letter, uint32_t x, uint32_t y, uint32_t color);

/* draw a large version of the letter */
void text_drawLargeLetter(const uint8_t * letter, uint32_t x, uint32_t y, uint32_t color);
