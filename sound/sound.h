/* must be called prior to any other calls */
int sound_init(const char * device);

/* returns 0 on success, <0 on failure */
int sound_play(const char * wav_file);

/* busy-loops until sound is done playing */
void sound_wait();

/* deinitializes and performs clean up */
void sound_exit();
