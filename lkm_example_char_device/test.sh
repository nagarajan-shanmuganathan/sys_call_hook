make clean
make
rmmod lkm_char_dev_example
make test

rm /dev/lkm_char_dev_example
mknod /dev/lkm_char_dev_example c 243 0

gcc lkm_char_dev_ioctl_call.c -o ioctl_call
./ioctl_call

dmesg
