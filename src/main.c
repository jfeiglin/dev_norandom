#include "norandom.h"
#include "syscall_hook.h"

int g_major;		/* g_major number assigned to our device driver */
asmlinkage long (*original_getpid)(void);
unsigned long *sys_call_table;

const struct file_operations random_fops = {
	.read  = device_read,
	.write = dummy_write,
	.poll  = dummy_poll,
	.unlocked_ioctl = dummy_ioctl,
	.llseek = noop_llseek,
	.open = device_open,
	.release = device_release
};

asmlinkage int hooked_getpid(void)
{
	if(0 == strncmp(current->comm, PROC_WATCH_NAME, MAX_PROC_NAME_LEN)){
		printk(KERN_INFO "PID %d (%s) called getpid, returning %d", current->pid, current->comm, PROC_WATCH_FAKE_PID);
		return PROC_WATCH_FAKE_PID;
	}
	return original_getpid();
}

int init_module(void)
{
	sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");
	printk(KERN_INFO "System call table located at: %lx\n", (long unsigned int)sys_call_table);
	original_getpid = (void*)sys_call_table[__NR_getpid];
	printk(KERN_INFO "Hooking getpid at offset %d, address %lx\n", __NR_getpid, (long unsigned int)original_getpid);
	hooking_syscall(sys_call_table, hooked_getpid, __NR_getpid);
    
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
	printk(KERN_INFO "Unhooking getpid at offset %d, address %lx\n", __NR_getpid, (long unsigned int)original_getpid);
	unhooking_syscall(sys_call_table, original_getpid, __NR_getpid);

	printk(KERN_INFO "norandom device destroyed\n");
	unregister_chrdev(g_major, DEVICE_NAME);
}
MODULE_LICENSE("GPL");
