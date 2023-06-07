# To build modules outside of the kernel tree, we run "make"
# in the kernel source tree; the Makefile these then includes this
# Makefile once again.
# This conditional selects whether we are being included from the
# kernel Makefile or not.
# sudo rmmod ftdi_sio
obj-m += DriverTest1.o
all:
	
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	sudo insmod DriverTest1.ko 
	sudo dmesg
clear:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	sudo rmmod DriverTest1
test:
	gcc -o Test Test.c 
	sudo ./Test