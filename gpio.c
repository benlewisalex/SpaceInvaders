#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "gpio.h"
#include "uio.h"
#include "utils.h"

/* initialize a generic gpio device */
void * gpio_init(const char * device, int * fd)
{
	int tempfd;					// we only set fd if everything succeeds
	void * gpio_mem;				// this will hold the result of mmap
	const char * const location = "gpio_init";	// function name for print_error

	/* open device; handle error */
	if ( FAIL_INT(tempfd = open(device, O_RDWR)) ) {
		print_error(location, "open");		// print error message
		return GPIO_FAILED;			// return -1
	}

	/* call mmap; handle error */
	if ( FAIL_PTR(gpio_mem = UIO_MMAP(tempfd)) ) {

		int error = errno;			// preserve the value of errno
		if ( FAIL_INT(close(tempfd)) ) {	// close file descriptor
			print_error(location, "close");	// print error message
		}
		errno = error;				// restore errno

		print_error(location, "mmap");		// print error message
		return GPIO_FAILED;			// return -1
	}

	/* ignore any interrupts that occurred prior to initialization */
	if (UIO_READ(gpio_mem, GPIO_ISR_OFFSET) & CHANNEL_1_MASK) {
		gpio_ack_interrupt(gpio_mem);
	}

	*fd = tempfd;					// set fd
	return gpio_mem;				// return mmap'd pointer
}

/* perform clean up */
void gpio_exit(void * gpio_mem, int fd)
{
	const char * const location = "gpio_exit";	// function name for print_error

	/* unmap */
	if ( FAIL_INT(munmap(gpio_mem, MMAP_SIZE)) ) {
		print_error(location, "munmap");	// print error, but don't exit
	}

	/* close file */
	if ( FAIL_INT(close(fd)) ) {
		print_error(location, "close");		// print error
	}
}

/* acknowledge an interrupt */
void gpio_ack_interrupt(void * gpio_mem)
{
	/* toggle-on-write to the ISR */
	UIO_WRITE(gpio_mem, GPIO_ISR_OFFSET, CHANNEL_1_MASK);
}

/* enable interrupts */
void gpio_int_enable(void * gpio_mem)
{
	/* enable general interrupts */
	UIO_SET(gpio_mem, GPIO_GIER_OFFSET, GIER_MASK);

	/* enable channel 1 interrupts*/
	UIO_SET(gpio_mem, GPIO_IER_OFFSET, CHANNEL_1_MASK);
}

/* disable interrupts */
void gpio_int_disable(void * gpio_mem)
{
	/* disable channel 1 interrupts*/
	UIO_UNSET(gpio_mem, GPIO_IER_OFFSET, CHANNEL_1_MASK);

	/* disable general interrupts */
	UIO_UNSET(gpio_mem, GPIO_GIER_OFFSET, GIER_MASK);
}

/* read the value of the data register */
uint32_t gpio_read(void * gpio_mem)
{
	return UIO_READ(gpio_mem, GPIO_DATA_OFFSET);
}

/* set a particular pin to input or output mode */
void gpio_set_mode(void * gpio_mem, uint32_t mask, int mode)
{
	/* if mode is 1 */
	if (mode) {
		/* set bits */
		UIO_SET(gpio_mem, GPIO_TRI_OFFSET, mask);
	} else {
		/* unset bits */
		UIO_UNSET(gpio_mem, GPIO_TRI_OFFSET, mask);
	}
}
