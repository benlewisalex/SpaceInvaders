#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <string.h>

#define FAIL_INT(x)	( (x) < 0 )		// system call returning an int failed
#define FAIL_PTR(x)	( (x) == NULL )		// system call returning a pointer failed

/* prints a section of memory in hex */
void print_bytes(uint8_t * ptr, size_t len);

/* print error message */
/* location should tell you where the error comes from */
/* func should be the name of the function call that failed */
void print_error(const char * location, const char * func);

#endif
