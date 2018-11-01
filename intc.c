#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "intc.h"
#include "uio.h"
#include "utils.h"

#define INIT_FAILED	-1	// return value for intc_init

#define ISR_OFFSET	0x0000	// defined in documentation
#define IPR_OFFSET	0x0004	// likewise
#define IER_OFFSET	0x0008	// likewise
#define IAR_OFFSET	0x000c	// likewise
#define MER_OFFSET	0x001c	// likewise

#define MER_ME_MASK	0x0001	// mask for master enable bit
#define MER_HIE_MASK	0x0002	// mask to enable hardware interrupts

/* this is a const and not a #define because write() requires a void* */
const uint32_t UIO_ENABLE = 1;

static int fd = 0;			// file descriptor for hardware device
static uint8_t * user_intc = NULL;	// pointer to mmap'd address space

int32_t intc_init(const char * filename)
{
	const char * const location = "intc_init";	// string for print_error

	/* open file */
	if ( FAIL_INT(fd = open(filename, O_RDWR)) ) {
		print_error(location, "open");		// print error message
		return INIT_FAILED;			// return -1
	}

	/* call mmap on opened file */
	if ( FAIL_PTR(user_intc = UIO_MMAP(fd)) ) {

		int error = errno;			// preserve value of errno
		if ( FAIL_INT(close(fd)) ) {		// close fd
			print_error(location, "close");	// print error message
		}
		errno = error;				// restore value of errno

		print_error(location, "mmap");		// print error message
		return INIT_FAILED;			// return -1
	}

	/* enable interrupts globally */
	UIO_SET(user_intc, MER_OFFSET, MER_ME_MASK | MER_HIE_MASK);
}

/* perform clean-up */
void intc_exit()
{
	const char * const location = "intc_exit";	// string for print_error

	/* try unmapping */
	if ( FAIL_INT(munmap(user_intc, MMAP_SIZE)) ) {
		print_error(location, "munmap");	// print error message, then continue
	}

	/* try closing file */
	if ( FAIL_INT(close(fd)) ) {
		print_error(location, "close");		// print error message
	}

	user_intc = NULL;				// clear pointer
	fd = 0;						// clear file descriptor
}

// This function will block until an interrupt occurrs
// Returns: Bitmask of activated interrupts
uint32_t intc_wait_for_interrupt()
{
	uint32_t _;					// throwaway variable
	read(fd, &_, sizeof(uint32_t));			// block until interrupt received
	return UIO_READ(user_intc, IPR_OFFSET);		// check which interrupts were raised
}

// Acknowledge interrupt(s) in the interrupt controller
// irq_mask: Bitmask of interrupt lines to acknowledge.
void intc_ack_interrupt(uint32_t irq_mask)
{
	/* write the appropriate bits to the interrupt acknowledge register */
	UIO_WRITE(user_intc, IAR_OFFSET, irq_mask);
}

/* turn interrupts back on after every interrupt */
void intc_enable_uio_interrupts()
{
	/* simply write a 32-bit 1 to the file */
	write(fd, &UIO_ENABLE, sizeof(uint32_t));
}

// Enable interrupt line(s)
// irq_mask: Bitmask of lines to enable
// This function only enables interrupt lines, ie, a 0 bit in irq_mask
//	will not disable the interrupt line
void intc_irq_enable(uint32_t irq_mask)
{
	/* set the bits in the IER */
	UIO_SET(user_intc, IER_OFFSET, irq_mask);
}

// Same as intc_irq_enable, except this disables interrupt lines
void intc_irq_disable(uint32_t irq_mask)
{
	/* clear the bits in the IER */
	UIO_UNSET(user_intc, IER_OFFSET, irq_mask);
}
