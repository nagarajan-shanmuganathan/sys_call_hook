obj-m += lkm_char_dev_example.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

test:
	-sudo rmmod lkm_char_dev_example.ko
	sudo dmesg -C
	sudo insmod lkm_char_dev_example.ko
	dmesg
