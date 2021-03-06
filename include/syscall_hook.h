#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h> //task_struct definition for current and__NR_getpid (includes asm-generic/unistd.h)

void hooking_syscall(unsigned long *sys_call_table, void *hook_addr, uint16_t syscall_offset);
void unhooking_syscall(unsigned long *sys_call_table, void *orig_addr, uint16_t syscall_offset);
