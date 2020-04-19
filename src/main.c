#include "norandom.h"

static int g_major;		/* g_major number assigned to our device driver */

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
        g_major = register_chrdev(0, DEVICE_NAME, &random_fops);

	if (g_major < 0) {
	  printk(KERN_ALERT "Registering char device failed with %d\n", g_major);
	  return g_major;
	}

	printk(KERN_INFO "norandom major: %d\n", g_major);

	return SUCCESS;
}

void cleanup_module(void)
{
	printk(KERN_INFO "norandom device destroyed\n");
	unregister_chrdev(g_major, DEVICE_NAME);
}