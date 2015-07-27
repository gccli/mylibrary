obj-m += mytty.o
obj-m += kprobe.o
obj-m += jprobe.o
obj-m += myproc.o
obj-m += mycdev.o
obj-m += kthread.o
obj-m += monitor_tcprecv.o
obj-m += monitor_tcpsend.o
mycdev-objs = mycdev_main.o mycdev_fops.o


# KDIR is the location of the kernel source.  The current standard is  
# to link to the associated source tree from the directory containing  
# the compiled modules.  
# KDIR  := /lib/modules/2.6.15/build  
KDIR := /lib/modules/$(shell uname -r)/build

# PWD is the current working directory and the location of our module  
# source files.  
PWD := $(shell pwd)  

# default is the default make target.  The rule here says to run make  
# with a working directory of the directory containing the kernel  
# source and compile only the modules in the PWD (local) directory.  
default:  
	$(MAKE) -C $(KDIR) M=$(PWD) modules  
clean:  
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	@$(RM) Module.*
