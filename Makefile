MODULE_MAME = norandom

SRCS = src/main.c src/norandom.c src/syscall_hook.c

INCLUDE_DIR = -I$(src)/include

ccflags-y := $(INCLUDE_DIR)

OBJS =  $(SRCS:.c=.o)

obj-m += $(MODULE_MAME).o
$(MODULE_MAME)-y = $(OBJS)


all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
