obj-m += my_module.o

all:
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules


test:
		gcc user_program.c -o ioctl_driver
		sudo ./ioctl_driver

create_dev:
		sudo mknod /dev/my_device c 700 0

del_device:
		sudo rm /dev/my_device

clean:
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load: all
		sudo insmod my_module.ko
		sudo dmesg -C

unload: clean
		sudo rmmod my_module.ko
		sudo dmesg -C