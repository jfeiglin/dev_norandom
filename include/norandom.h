#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/limits.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

#define RANDOM_BYTE (65)
#define EXTRACT_CRNG_FNAME ("extract_crng")
#define RANDOM_READ_FNAME ("random_read")
#define URANDOM_READ_FNAME ("urandom_read")

/* per-instance kprobe private data */
struct random_buf_data {
	char *random_buffer;
    size_t random_buffer_len;
};

int register_all_probes(void);
void unregister_all_probes(void);