#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <uapi/linux/types.h>
#include <asm/uaccess.h>	/* for put_user */

#define SUCCESS 0
#define DEVICE_NAME "norandom"	/* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80		/* Max length of the message from the device */
#define RANDOM_BYTE (0x41)
#define RNDGETENTCNT (0x80045200)

int device_open(struct inode *, struct file *);
int device_release(struct inode *, struct file *);
ssize_t device_read(struct file *, char *, size_t, loff_t *);
ssize_t dummy_write(struct file *, const char *, size_t, loff_t *);
unsigned int dummy_poll(struct file *file, poll_table * wait);
long dummy_ioctl(struct file *f, unsigned int cmd, unsigned long arg);
