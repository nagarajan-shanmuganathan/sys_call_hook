sudo make -C /lib/modules/$(uname -r)/build M=$PWD clean
clear
sudo rmmod hook
sudo make -C /lib/modules/$(uname -r)/build M=$PWD modules
sudo insmod ./hook.ko


sudo dmesg -C

ls

dmesg
