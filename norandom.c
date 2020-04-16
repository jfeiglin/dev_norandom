#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <uapi/linux/types.h>
#include <asm/uaccess.h>	/* for put_user */

/*  
 *  Prototypes - this would normally go in a .h file
 */
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t dummy_write(struct file *, const char *, size_t, loff_t *);
static unsigned int dummy_poll(struct file *file, poll_table * wait);
static long dummy_ioctl(struct file *f, unsigned int cmd, unsigned long arg);

#define SUCCESS 0
#define DEVICE_NAME "norandom"	/* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80		/* Max length of the message from the device */
#define RANDOM_BYTE (0x41)

/* 
 * Global variables are declared as static, so are global within the file. 
 */
static int Major;		/* Major number assigned to our device driver */

const struct file_operations random_fops = {
	.read  = device_read,
	.write = dummy_write,
	.poll  = dummy_poll,
	.unlocked_ioctl = dummy_ioctl,
	.llseek = noop_llseek,
	.open = device_open,
	.release = device_release
};

int init_module(void)
{
        Major = register_chrdev(0, DEVICE_NAME, &random_fops);

	if (Major < 0) {
	  printk(KERN_ALERT "Registering char device failed with %d\n", Major);
	  return Major;
	}

	printk(KERN_INFO "norandom device created w major number %d\n", Major);
	printk(KERN_INFO "create a dev file with: \n");
	printk(KERN_INFO "\t'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);

	return SUCCESS;
}

void cleanup_module(void)
{
	printk(KERN_INFO "norandom device destroyed w major number %d\n", Major);
	unregister_chrdev(Major, DEVICE_NAME);
}

static int device_open(struct inode *inode, struct file *file)
{
	try_module_get(THIS_MODULE);
	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);
	return 0;
}

static ssize_t device_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
	int bytes_read = 0;
	while (length) {
		//needed because buffer is a user mode buffer
		put_user(RANDOM_BYTE, buffer++);
		length--;
		bytes_read++;
	}

	return bytes_read;
}

static ssize_t
dummy_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	return (ssize_t)len;
}

static unsigned int 
dummy_poll(struct file *file, poll_table * wait){
	return 0;
}

static long 
dummy_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	return 0;
}

