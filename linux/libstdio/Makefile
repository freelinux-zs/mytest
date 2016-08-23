#obj-m := hello.o
#obj-m := hrtimer.o
#obj-m := mymod.o
#obj-m := led.o

#KERNEL_DIR := /lib/modules/$(shell uname -r)/build
#PWD := $(shell pwd)

#all:
#	make -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules
#clean:
#	make -C $(KERNEL_DIR) SUBDIRS=$(PWD) clean


RM = rm -f

CCFLAG = -c
ARFLAG = -rc

CC = gcc
AR = ar

lib_OBJECTS = mylib.o
lib_SOURCE = myalib.c

LIB  = libtest.a

libtest.a:$(lib_OBJECTS)
	$(AR) $(ARFLAG) -o $@ $^

$(lib_OBJECTS):$(lib_SOURCE)
	$(CC) $(CCFLAG) -o $@ $^


clean:
	$(RM) $(lib_OBJECTS)
	$(RM) $(LIB)
