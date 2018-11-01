#include <stdio.h>
#include <stdlib.h>

#include "audio_adau1761.h"
#include "sound.h"

#define I2C_INDEX	0
#define ARG_COUNT	2

int main(int argc, char *argv[]) {
	int status;
	if (argc < ARG_COUNT) {
		fprintf(stderr, "Usage: %s <wav_file>\n", argv[0]);
		return EXIT_FAILURE;
	}

	// send I2C configuration commands
	config_audio_pll(I2C_INDEX);
	config_audio_codec(I2C_INDEX);

	// enable sound
	unmute(I2C_INDEX);

	// try initializing sound driver
	if ((status = sound_init("/dev/audio")) < 0) {
		// probably forgot to do insmod
		return status;
	}

	sound_play(argv[1]);	// play wav file
	sound_wait();		// wait for driver to finish
	sound_play(argv[1]);	// play again
	sound_exit();		// clean up
}
