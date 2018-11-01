sound: main.o audio_adau1761.o i2cps.o sound.o

audio_adau1761.o: audio_adau1761.c audio_adau1761.h i2cps.h
i2cps.o: i2cps.c i2cps.h
main.o: main.c sound.h
sound.o: sound.c sound.h

clean:
	rm *.o
