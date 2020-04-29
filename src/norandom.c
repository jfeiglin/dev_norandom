#include "norandom.h"

unsigned char norandom_buffer[NORANDOM_BUFFER_LEN] = {0};
int urandom_probed = -999;
int random_probed = -999;
int random_ioctl_probed = -999;

int random_file_ioctl_entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    long unsigned int ioctl_cmd;
    long unsigned int ioctl_arg;
    unsigned char * new_buffer_contents_user;

	if (!current->mm)
		return 1;	/* Skip kernel threads */

    //ioctl_cmd is located at 2nd arg and ioctl_arg is at 3rd arg, which is dx/cx in 32 bit and si/dx in 64 bit.
#if defined(__i386__)
    ioctl_cmd=regs->dx;
    ioctl_arg=regs->cx; 
#elif defined(__amd64__)
    ioctl_cmd=regs->si;
    ioctl_arg=regs->dx;
#endif
    if(CHANGE_NORANDOM_BUFFER_IOCTL == ioctl_cmd){
        new_buffer_contents_user = (unsigned char *)ioctl_arg;
        if(0 != copy_from_user(norandom_buffer, new_buffer_contents_user, NORANDOM_BUFFER_LEN)){
            printk(KERN_INFO "Couldn't copy new bytes to norandom buffer from userspace\n");
        }
    }
    return 0;
}

int random_file_read_entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	struct random_buf_data *data;
    long unsigned int arg2;
    long unsigned int arg3;

	if (!current->mm)
		return 1;	/* Skip kernel threads */

    //buffer is located at 2nd arg and length is at 3rd arg, which is dx/cx in 32 bit and si/dx in 64 bit.
#if defined(__i386__)
    arg2=regs->dx;
    arg3=regs->cx; 
#elif defined(__amd64__)
    arg2=regs->si;
    arg3=regs->dx;
#endif
	data = (struct random_buf_data *)ri->data;
	data->random_buffer = (char *)arg2;
    data->random_buffer_len = arg3;
	return 0;
}

int random_file_read_ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	int i = 0;
    struct random_buf_data *data = (struct random_buf_data *)ri->data;

    printk(KERN_INFO "Unrandomizing buffer at 0x%lx with len: %zu\n", 
        (unsigned long int)data->random_buffer,
        data->random_buffer_len
    );

    for(i=0; i<data->random_buffer_len; i++){
        put_user(
            norandom_buffer[i % NORANDOM_BUFFER_LEN],
            data->random_buffer+i
        );
    }
	return 0;
}

struct kretprobe random_file_read_kretprobe = {
	.handler		= random_file_read_ret_handler,
	.entry_handler	= random_file_read_entry_handler,
	.data_size		= sizeof(struct random_buf_data),
	/* Probe up to 20 instances concurrently. */
	.maxactive		= 20,
};

struct kretprobe urandom_file_read_kretprobe = {
	.handler		= random_file_read_ret_handler,
	.entry_handler	= random_file_read_entry_handler,
	.data_size		= sizeof(struct random_buf_data),
	/* Probe up to 20 instances concurrently. */
	.maxactive		= 20,
};

struct kretprobe random_ioctl_kretprobe = {
	.handler		= NULL,
	.entry_handler	= random_file_ioctl_entry_handler,
	.data_size		= 0,
	/* Probe up to 20 instances concurrently. */
	.maxactive		= 20,
};

int register_all_probes(void){

        urandom_file_read_kretprobe.kp.symbol_name = URANDOM_READ_FNAME;
        random_file_read_kretprobe.kp.symbol_name = RANDOM_READ_FNAME;
        random_ioctl_kretprobe.kp.symbol_name = RANDOM_IOCTL_FNAME;
        
        urandom_probed = register_kretprobe(&urandom_file_read_kretprobe);
        random_probed = register_kretprobe(&random_file_read_kretprobe);
        random_ioctl_probed = register_kretprobe(&random_ioctl_kretprobe);
        
        if(urandom_probed < 0 || random_probed < 0){
            printk(KERN_INFO "registering (u)random read kprobe failed, random=%d, urandom=%d\n", random_probed, urandom_probed);
            return -1;
        }
        printk(KERN_INFO "Successfully probed (u)random reads\n");

        if(random_ioctl_probed < 0){
            printk(KERN_INFO "registering random ioctl kprobe failed (%d), changing random buffer will be impossible\n", random_ioctl_probed);
        }


    return 0;
}

void unregister_all_probes(void){
    if(0 == random_probed){
	    unregister_kretprobe(&random_file_read_kretprobe);
	    printk(KERN_INFO "random_file_read_kretprobe at %p unregistered\n",
            random_file_read_kretprobe.kp.addr
        );
        printk(KERN_INFO "Missed probing %d instances of %s\n",
		    random_file_read_kretprobe.nmissed, 
            random_file_read_kretprobe.kp.symbol_name
        );
    }

    if(0 == urandom_probed){
	    unregister_kretprobe(&urandom_file_read_kretprobe);
	    printk(KERN_INFO "urandom_file_read_kretprobe at %p unregistered\n",
            urandom_file_read_kretprobe.kp.addr
        );
        printk(KERN_INFO "Missed probing %d instances of %s\n",
		    urandom_file_read_kretprobe.nmissed, 
            urandom_file_read_kretprobe.kp.symbol_name
        );
    }

    if(0 == random_ioctl_probed){
        unregister_kretprobe(&random_ioctl_kretprobe);
	    printk(KERN_INFO "random_ioctl_kretprobe at %p unregistered\n",
            random_ioctl_kretprobe.kp.addr
        );
        printk(KERN_INFO "Missed probing %d instances of %s\n",
		    random_ioctl_kretprobe.nmissed, 
            random_ioctl_kretprobe.kp.symbol_name
        );
    }
}