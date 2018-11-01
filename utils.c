#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

#define WIDTH 8

/* prints a section of memory in hex */
void print_bytes(uint8_t * ptr, size_t len)
{
	size_t i;				// counter variable
	for (i = 0; i < len;) {			// go until we run out of bytes
		printf("%02x", ptr[i++]);	// print byte as hex
		if (i % WIDTH)			// print newline every 8th byte
			putchar(' ');
		else
			putchar('\n');
	}
}

/* print error message */
/* location should tell you where the error comes from */
/* func should be the name of the function call that failed */
void print_error(const char * location, const char * func)
{
	fprintf(stderr, "%s: failed call to %s(): %s\n", location, func, strerror(errno));
}
