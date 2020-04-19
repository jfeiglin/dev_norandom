#include "syscall_hook.h"

static void unprotect_memory(void){
	unsigned long original_cr0 = read_cr0();
	write_cr0(original_cr0 & (~ 0x10000)); // set write protect flag in cr0 to 0
}

static void protect_memory(void){
	unsigned long original_cr0 = read_cr0();
	write_cr0(original_cr0 | (0x10000)); // set write protect flag in cr0 to 1
}

void hooking_syscall(unsigned long *sys_call_table, void *hook_addr, uint16_t syscall_offset)
{
	unprotect_memory();
	sys_call_table[syscall_offset] = (unsigned long)hook_addr;
	protect_memory();
}

void unhooking_syscall(unsigned long *sys_call_table, void *orig_addr, uint16_t syscall_offset)
{
	unprotect_memory();
	sys_call_table[syscall_offset] = (unsigned long)orig_addr;
	protect_memory();
}
