#include<stdio.h>
#include<linux/kernel.h>
#include<sys/syscall.h>
#include<unistd.h>

int main(){
	long ret = syscall(333);
	printf("%ld\n", ret);
	return 0;
}
