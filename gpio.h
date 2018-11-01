#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

#define GPIO_FAILED	NULL		// return value of gpio_init if it fails

#define GPIO_DATA_OFFSET	0x0000	// from GPIO documentation
#define GPIO_TRI_OFFSET		0x0004	// likewise
#define GPIO_GIER_OFFSET	0x011c	// likewise
#define GPIO_ISR_OFFSET		0x0120	// likewise
#define GPIO_IER_OFFSET		0x0128	// likewise

#define GPIO_MODE_OUTPUT	0	// argument to gpio_set_mode
#define GPIO_MODE_INPUT		1	// likewise

#define CHANNEL_1_MASK	0x00000001	// for enabling/acknowledging interrupts
#define GIER_MASK	0x80000000	// bit mask for global interrupt enable register

/* returns pointer to mapped address space */
/* 'device' is the file path of the device */
/* 'fd' will be set to the value of the open file descriptor */
void * gpio_init(const char * device, int * fd);

/* perform clean up */
void gpio_exit(void * gpio_mem, int fd);

/* acknowledges an interrupt */
void gpio_ack_interrupt(void * gpio_mem);

/* enables interrupts */
void gpio_int_enable(void * gpio_mem);

/* disables interrupts */
void gpio_int_disable(void * gpio_mem);

/* reads the value in the data register */
uint32_t gpio_read(void * gpio_mem); 

/* sets input/output mode for mask */
void gpio_set_mode(void * gpio_mem, uint32_t mask, int mode);

#endif
