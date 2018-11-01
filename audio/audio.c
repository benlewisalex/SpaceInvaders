#include <asm/io.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your name");
MODULE_DESCRIPTION("ECEn 427 Audio Driver");

#define MIN(x,y)	((x) < (y) ? (x) : (y))

#define MODULE_NAME	"audio"

#define SUCCESS		0

#define FIRST_MINOR	0
#define DEVICE_COUNT	1
#define CHRDEV_NAME	MODULE_NAME

#define CDEV_COUNT	1
#define PARENT		NULL
#define RES_INDEX	0
#define IRQ_FLAGS	0

#define I2S_STATUS_REG	0x10
#define INT_EN		0x00000001

#define I2S_DATA_TX_R_REG	0x08
#define I2S_DATA_TX_L_REG	0x0C

#define WORD_SIZE	4
#define FIFO_SIZE	1024
#define FIFO_CHUNK_SIZE	((3 * FIFO_SIZE / 4) / WORD_SIZE)

// Function declarations
static int audio_init(void);
static void audio_exit(void);

static int audio_probe(struct platform_device *pdev);
static int audio_remove(struct platform_device * pdev);

static ssize_t audio_read(struct file * f, char __user * buf, size_t buflen, loff_t * offset);
static ssize_t audio_write(struct file * f, const char __user * buf, size_t buflen, loff_t * offset);

static void enable_interrupts(void);
static void disable_interrupts(void);
static irqreturn_t int_handler(int irq, void * dev_id);

module_init(audio_init);
module_exit(audio_exit);

struct audio_device {
	int minor;			// Device minor number
	struct cdev cdev;		// Character device structure
	struct platform_device * pdev;	// Platform device pointer
	struct device* dev;		// device (/dev)

	phys_addr_t phys_addr;		// Physical address
	u32 mem_size;			// Allocated mem space size
	u32* virt_addr;			// Virtual address
	unsigned int irq;		// IRQ Number

	uint32_t * buffer;		// pointer to audio buffer
	size_t buflen;			// length of audio buffer
	size_t bufidx;			// index of next audio to be written to the FIFO
};

static struct of_device_id audio_of_match[] = {
  { .compatible = "byu,ecen427-audio_codec", },
  {}
};

MODULE_DEVICE_TABLE(of, audio_of_match);

static struct platform_driver driver = {
  .probe = audio_probe,
  .remove = audio_remove,
  .driver = {
    .name = MODULE_NAME,
    .owner = THIS_MODULE,
    .of_match_table = audio_of_match,
  },
};

struct file_operations file_ops = {
	.owner = THIS_MODULE,
	.read = audio_read,
	.write = audio_write,
};

static int major;
static struct class * cls;
static struct audio_device device;

// This is called when Linux loads your driver
static int audio_init(void) {
	int status;
	dev_t dev;

	pr_info("%s: Initializing Audio Driver\n", MODULE_NAME);

	// Get a major number for the driver -- alloc_chrdev_region; // pg. 45, LDD3.
	status = alloc_chrdev_region(&dev, FIRST_MINOR, DEVICE_COUNT, CHRDEV_NAME);
	if (status < 0) {
		pr_err("%s: Major number could not be allocated\n", MODULE_NAME);
		goto err_alloc;
	}

	major = MAJOR(dev);
	device.minor = MINOR(dev);
	pr_info("%s: Major: %d; Minor: %d\n", MODULE_NAME, major, device.minor);

	// Create a device class. -- class_create()
	cls = class_create(THIS_MODULE, MODULE_NAME);

	// Register the driver as a platform driver -- platform_driver_register
	status = platform_driver_register(&driver);
	if (status < 0) {
		pr_err("%s: Could not register platform driver\n", MODULE_NAME);
		goto err_pdreg;
	}

	return SUCCESS;					// no errors

err_pdreg:		// platform_driver_register failed
	class_unregister(cls);				// unregister class
	class_destroy(cls);				// destroy class
	unregister_chrdev_region(dev, DEVICE_COUNT);	// clean up major/minor numbers

err_alloc:		// error allocating major/minor numbers
	return status;
}

// This is called when Linux unloads your driver
static void audio_exit(void) {
	// platform_driver_unregister
	platform_driver_unregister(&driver);
	// class_unregister and class_destroy
	class_unregister(cls);
	class_destroy(cls);
	// unregister_chrdev_region
	unregister_chrdev_region(MKDEV(major, device.minor), DEVICE_COUNT);

	pr_info("%s: Audio Driver Removed\n", MODULE_NAME);
	return;
}

// Called by kernel when a platform device is detected that matches the 'compatible' name of this driver.
static int audio_probe(struct platform_device *pdev) {
	int status;
	struct resource * res;
	// Initialize the character device structure (cdev_init)
	cdev_init(&device.cdev, &file_ops);

	// Register the character device with Linux (cdev_add)
	status = cdev_add(&device.cdev, MKDEV(major, device.minor), CDEV_COUNT);
	if (status < 0) {
		pr_err("%s: Could not add character device\n", MODULE_NAME);
		goto err_cdev;
	}

	// Create a device file in /dev so that the character device can be accessed from user space (device_create).
	device.dev = device_create(cls, PARENT, MKDEV(major, device.minor), NULL, MODULE_NAME);
	if (IS_ERR(device.dev)) {
		pr_err("%s: Could not create device\n", MODULE_NAME);
		goto err_devcr;
	}

	// Get the physical device address from the device tree -- platform_get_resource
	res = platform_get_resource(pdev, IORESOURCE_MEM, RES_INDEX);

	device.phys_addr = res->start;
	device.mem_size = res->end - res->start + 1;
	pr_info("%s: Physical Address: 0x%08x; Size: 0x%08x\n", MODULE_NAME, device.phys_addr, device.mem_size);

	// Reserve the memory region -- request_mem_region
	res = request_mem_region(device.phys_addr, device.mem_size, MODULE_NAME);
	if (res == NULL) {
		pr_err("%s: Request for I/O memory failed\n", MODULE_NAME);
		goto err_rqmem;
	}

	// Get a (virtual memory) pointer to the device -- ioremap
	device.virt_addr = ioremap(device.phys_addr, device.mem_size);
	if (device.virt_addr == NULL) {
		pr_err("%s: Could not get virtual address\n", MODULE_NAME);
		goto err_iomap;
	}
	pr_info("%s: Virtual Address: 0x%p\n", MODULE_NAME, device.virt_addr);

	// Get the IRQ number from the device tree -- platform_get_resource
	res = platform_get_resource(pdev, IORESOURCE_IRQ, RES_INDEX);
	device.irq = res->start;
	pr_info("%s: IRQ Number: %d\n", MODULE_NAME, device.irq);

	// Register your interrupt service routine -- request_irq
	status = request_irq(device.irq, int_handler, IRQ_FLAGS, MODULE_NAME, &device);
	if (status < 0) {
		pr_err("%s: Request for IRQ failed\n", MODULE_NAME);
		goto err_rqirq;
	}

	device.buffer = NULL;

	enable_interrupts();
	return SUCCESS;					// no errors

err_rqirq:
	iounmap(device.virt_addr);

err_iomap:
	release_mem_region(device.phys_addr, device.mem_size);

err_rqmem:		// request_mem_region failed
	device_destroy(cls, MKDEV(major, device.minor));

err_devcr:		// device_create failed
	cdev_del(&device.cdev);

err_cdev:		// cdev_add failed
	return status;
}

static int audio_remove(struct platform_device * pdev) {
	free_irq(device.irq, &device);
	// iounmap
	iounmap(device.virt_addr);
	pr_info("%s: Unmapped I/O memory\n", MODULE_NAME);

	// release_mem_region
	release_mem_region(device.phys_addr, device.mem_size);
	pr_info("%s: Released I/O memory\n", MODULE_NAME);

	// device_destroy
	device_destroy(cls, MKDEV(major, device.minor));
	pr_info("%s: Destroyed device\n", MODULE_NAME);

	// cdev_del
	cdev_del(&device.cdev);
	pr_info("%s: Deleted character device\n", MODULE_NAME);

	return 0;
}

static unsigned int read_helper(int offset){
	// the typecasts make sure pointer addition is correct
	return ioread32( ((void*) device.virt_addr) + offset);
}

static void write_helper(u32 offset, int value){
	// the typecasts make sure pointer addition is correct
	iowrite32(value, ((void*) device.virt_addr) + offset);
}

static ssize_t audio_read(struct file * f, char __user * buf, size_t buflen, loff_t * offset)
{
	// device.buffer is non-null iff there is audio data
	return device.buffer != NULL;
}

static ssize_t audio_write(struct file * f, const char __user * buf, size_t buflen, loff_t * offset)
{
	// immediately disable interrupts from the audio core
	disable_interrupts();

	// Free the buffer used to store the old sound sample (if applicable) (kfree)
	if(device.buffer != NULL){
		kfree(device.buffer);
		device.buffer = NULL;
	}

	// Allocate a buffer for the new clip (kmalloc)
	device.buffer = kmalloc(buflen, GFP_KERNEL);

	// Copy the audio data from the userspace to your newly allocated buffer() - LDD page 64
	if (copy_from_user(device.buffer, (uint32_t*) buf, buflen)) {
		pr_err("%s: Bad pointer\n", MODULE_NAME);
		kfree(device.buffer);
		device.buffer = NULL;
		return -EFAULT;
	}

	// buflen is the length of the data; bufidx is the next word to be written
	device.buflen = buflen / WORD_SIZE;
	device.bufidx = 0;

	// Make sure the audio core has interrupts enabled
	enable_interrupts();
	return buflen;
}

static void enable_interrupts(void)
{
	unsigned int regval;
	pr_info("%s: Enabling interrupts\n", MODULE_NAME);

	regval = read_helper(I2S_STATUS_REG);			// get current register value
	write_helper(I2S_STATUS_REG, regval | INT_EN);		// just set INT_EN
}

static void disable_interrupts(void)
{
	unsigned int regval;
	pr_info("%s: Disabling interrupts\n", MODULE_NAME);

	regval = read_helper(I2S_STATUS_REG);			// get current register value
	write_helper(I2S_STATUS_REG, regval & ~INT_EN);		// just unset INT_EN
}

static irqreturn_t int_handler(int irq, void * dev_id)
{
	// counter variable
	int i;

	// decide how many samples to write to the FIFO
	size_t chunk_size = MIN(FIFO_CHUNK_SIZE, device.buflen - device.bufidx);

	pr_info("%s: Handling interrupt\n", MODULE_NAME);

	// fill the FIFO one word at a time
	for(i = 0; i < chunk_size; i++){
		write_helper(I2S_DATA_TX_R_REG, device.buffer[device.bufidx]);	// right channel
		write_helper(I2S_DATA_TX_L_REG, device.buffer[device.bufidx]);	// left channel
		device.bufidx++;						// next sample
	}

	// reset driver state once audio has all been queued
	if (device.bufidx == device.buflen) {
		disable_interrupts();
		kfree(device.buffer);
		device.buffer = NULL;
	}

	return IRQ_HANDLED;
}
