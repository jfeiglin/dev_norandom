#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/limits.h>
#include <linux/sched.h>
//#include <asm/uaccess.h>
#include <linux/uaccess.h>


#define CHANGE_NORANDOM_BUFFER_IOCTL (1337)
#define NORANDOM_BUFFER_LEN (32)
#define RANDOM_READ_FNAME ("random_read")
#define URANDOM_READ_FNAME ("urandom_read")
#define RANDOM_IOCTL_FNAME ("random_ioctl")

/* per-instance kprobe private data */
struct random_buf_data {
	char *random_buffer;
    size_t random_buffer_len;
};

int register_all_probes(void);
void unregister_all_probes(void);