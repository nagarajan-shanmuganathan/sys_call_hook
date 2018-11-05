#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <asm/unistd.h>

#define __NR_syscall_max 547
#define STATE_FILE_NAME "sys_table"
int main(){
	
	unsigned long state[__NR_syscall_max];
	unsigned long old_state[__NR_syscall_max];
	int flag = 0;
	printf("%d\n", __NR_syscall_max);

	long ret = syscall(333, state);
	printf("%ld\n", ret);
	int i = 0;
	/*for(i = 0; i < __NR_syscall_max; i++) {
		printf("Value at %d: %p\n", i, (void *)state[i]);
	}*/
      	
	FILE *fp;
	
	printf("Check if file exists\n");
	if(access(STATE_FILE_NAME, F_OK) != -1){
		printf("Compare with a newly read system call table\n");
    		// file exists
		fp = fopen(STATE_FILE_NAME, "rb");
		fread(old_state, sizeof(unsigned long), __NR_syscall_max, fp);
		fclose(fp);

		for(i = 0; i < __NR_syscall_max; i++) {
			if(i == 333 || i == 78) {
				if(old_state[i] == state[i]) {
					printf("Same --> Original: %p, modified: %p, syscall number: %d\n", (void *)old_state[i],(void*) state[i], i); 
				}
				else {
					printf("Modified spaces --> Original: %p, modified: %p, syscall number: %d\n",(void*) old_state[i],(void*) state[i], i); 

				}
			}
		}
		fp = fopen(STATE_FILE_NAME, "wb");
		fwrite(state, sizeof(unsigned long), __NR_syscall_max, fp);
		fclose(fp);
	}
	else {
    		// file doesn't exist
		printf("Obtained a system call table\n");
		for(i = 0; i < __NR_syscall_max; i++) {
			if(i == 333 || i == 78){
				printf("Value at %d: %p\n", i, (void *) state[i]);
			}
		}

		fp = fopen(STATE_FILE_NAME, "wb");
		fwrite(state, sizeof(unsigned long), __NR_syscall_max, fp);

		fclose(fp);
	}
	return 0;
}
