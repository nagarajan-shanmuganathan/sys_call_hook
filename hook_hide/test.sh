sudo make -C /lib/modules/$(uname -r)/build M=$PWD clean
clear
sudo rmmod hook_hide
sudo make -C /lib/modules/$(uname -r)/build M=$PWD modules
sudo insmod ./hook_hide.ko

ls -la

dmesg

