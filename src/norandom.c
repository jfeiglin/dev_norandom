#include "norandom.h"

int extract_crng_entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	struct random_buf_data *data;
    long unsigned int arg;

	if (!current->mm)
		return 1;	/* Skip kernel threads */

    //buffer is located at first arg, which is ax in 32 bit and di in 64 bit.
#if defined(__i386__)
    arg=regs->ax; 
#elif defined(__amd64__)
    arg=regs->di;
#endif
    
	printk(KERN_INFO "crng_extract(0x%lx) called by %s\n",
            arg,
            current->comm
        );

	data = (struct random_buf_data *)ri->data;
	data->random_buffer = (char *)arg;
    //extract_crng always fills the buffer with random data of length CHACHA_BLOCK_SIZE (64)
    data->random_buffer_len = 64;
	return 0;
}

int extract_crng_ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	struct random_buf_data *data = (struct random_buf_data *)ri->data;
    printk(KERN_INFO "Unrandomizing buffer at 0x%lx with len: %zu\n", 
        (unsigned long int)data->random_buffer,
        data->random_buffer_len
    );
    //memset here
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
    
	printk(KERN_INFO "u/random(0x%lx, %ld) called by %s\n",
            arg2,
            arg3,
            current->comm
        );

	data = (struct random_buf_data *)ri->data;
	data->random_buffer = (char *)arg2;
    data->random_buffer_len = arg3;
	return 0;
}

int random_file_read_ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	struct random_buf_data *data = (struct random_buf_data *)ri->data;
    printk(KERN_INFO "Unrandomizing buffer at 0x%lx with len: %zu\n", 
        (unsigned long int)data->random_buffer,
        data->random_buffer_len
    );
    if(0!=clear_user(data->random_buffer, data->random_buffer_len)){
        printk(KERN_INFO "Couldn't unrandomize buffer\n");
    }
	return 0;
}

struct kretprobe extract_crng_kretprobe = {
	.handler		= extract_crng_ret_handler,
	.entry_handler	= extract_crng_entry_handler,
	.data_size		= sizeof(struct random_buf_data),
	/* Probe up to 20 instances concurrently. */
	.maxactive		= 20,
};

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

int crng_probed = -999;
int urandom_probed = -999;
int random_probed = -999;

int register_all_probes(void){
	extract_crng_kretprobe.kp.symbol_name = EXTRACT_CRNG_FNAME;
	crng_probed = register_kretprobe(&extract_crng_kretprobe);

    if(crng_probed < 0){
        printk(KERN_INFO "registering extract_crng kprobe failed, returned %d\n", crng_probed);
        // if we failed to register the crng_extract probe, then register probes on reading the files
        urandom_file_read_kretprobe.kp.symbol_name = URANDOM_READ_FNAME;
        random_file_read_kretprobe.kp.symbol_name = RANDOM_READ_FNAME;
        urandom_probed = register_kretprobe(&urandom_file_read_kretprobe);
        random_probed = register_kretprobe(&random_file_read_kretprobe);
        if(urandom_probed < 0 || random_probed < 0){
            printk(KERN_INFO "registering (u)random read kprobe failed, random=%d, urandom=%d\n", random_probed, urandom_probed);
            return -1;
        }
        else{
            printk(KERN_INFO "Successfully probed (u)random reads\n");
        }
    }
    else{
        printk(KERN_INFO "Successfully probed extract_crng\n");
    }
    return 0;
}

void unregister_all_probes(void){
    if(0 == crng_probed){
	    unregister_kretprobe(&extract_crng_kretprobe);
	    printk(KERN_INFO "extract_crng_kretprobe at %p unregistered\n",
            extract_crng_kretprobe.kp.addr
        );
        printk(KERN_INFO "Missed probing %d instances of %s\n",
		    extract_crng_kretprobe.nmissed, 
            extract_crng_kretprobe.kp.symbol_name
        );
    }

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
}