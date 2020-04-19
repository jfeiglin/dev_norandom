#include "norandom.h"

int device_open(struct inode *inode, struct file *file)
{
	try_module_get(THIS_MODULE);
	return SUCCESS;
}

EXPORT_SYMBOL(device_open);

int device_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);
	return 0;
}

EXPORT_SYMBOL(device_release);

ssize_t
device_read(struct file *filp,	/* see include/linux/fs.h   */
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

EXPORT_SYMBOL(device_read);

ssize_t
dummy_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	return (ssize_t)len;
}

EXPORT_SYMBOL(dummy_write);

unsigned int 
dummy_poll(struct file *file, poll_table * wait){
	return 1;
}

EXPORT_SYMBOL(dummy_poll);

long 
dummy_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	int __user *p = (int __user *)arg;
	switch (cmd) {
		//fake entropy read
		case RNDGETENTCNT:
			put_user(4000, p);
	}
	return 0;
}

EXPORT_SYMBOL(dummy_ioctl);
