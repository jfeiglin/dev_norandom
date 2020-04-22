MODULE_MAME = norandom

SRCS = src/main.c src/norandom.c src/syscall_hook.c src/execve_utils.c

INCLUDE_DIR = -I$(src)/include

ccflags-y := $(INCLUDE_DIR)

OBJS =  $(SRCS:.c=.o)

obj-m += $(MODULE_MAME).o
$(MODULE_MAME)-y = $(OBJS)


all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -shared -fPIC preload_so/preload_so.c -o preload_so.so
	

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f *.so
	