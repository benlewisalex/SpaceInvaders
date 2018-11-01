#ifndef UIO_H
#define UIO_H

#include <sys/mman.h>

/* read a word from memory */
#define UIO_READ(ptr, offset)		( *((volatile uint32_t*) (ptr + offset)) )

/* write a word to memory */
#define UIO_WRITE(ptr, offset, val)	( *((volatile uint32_t*) (ptr + offset)) = val )

/* set bits in memory */
#define UIO_SET(ptr, offset, mask)	( *((volatile uint32_t*) (ptr + offset)) |= mask ) 

/* clear bits in memory */
#define UIO_UNSET(ptr, offset, mask)	( *((volatile uint32_t*) (ptr + offset)) &= ~mask )

#define MMAP_SIZE	0x200				// size of mmap call
#define MMAP_RW		(PROT_READ | PROT_WRITE)	// flags argument to mmap
#define MMAP_OFFSET	0				// offset argument to mmap

/* shortcut call to mmap */
#define UIO_MMAP(fd)	mmap(NULL, MMAP_SIZE, MMAP_RW, MAP_SHARED, fd, MMAP_OFFSET)

#endif
