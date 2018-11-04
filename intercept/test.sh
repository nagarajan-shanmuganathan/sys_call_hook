sudo make -C /lib/modules/$(uname -r)/build M=$PWD clean
clear
sudo rmmod interceptor
sudo make -C /lib/modules/$(uname -r)/build M=$PWD modules

sudo dmesg -C
sudo insmod ./interceptor.ko
dmesg

