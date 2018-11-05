#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#define D(x) printf("\n \"%s\": %d\n",#x,x);

typedef struct procinfo{
	pid_t pid;
	pid_t ppid;
	struct timespec start_time;
	int num_sib;
}procinfo;

void application_ioctl_call(int fd, int pid, procinfo* procinfo){

	char pid_to_buffer[32];
	sprintf(pid_to_buffer, "%d", pid);	
	printf("Sending %s to device\n", pid_to_buffer);

	if(write(fd, pid_to_buffer, strlen(pid_to_buffer) + 1)){
		printf("Sending PID to device failed. \n");
		return;
	}

	if(ioctl(fd, pid, procinfo) == -1){
		printf("ioctl failed\n");
	}
	else{
		printf("device_ioctl() was called, check /var/log/syslog\n");
		int x = 0;
		D(procinfo->pid);
		D(procinfo->ppid);
		D(procinfo->num_sib);
	}
	return;
}

int main(){
	char* char_dev_file = "/dev/lkm_char_dev_example";
	int fd = open(char_dev_file, O_RDWR);

	if(fd == -1){
		printf("Could not open /dev/lkm_char_dev_example\n");
		return 2;
	}
	procinfo pinfo;
	application_ioctl_call(fd, 0, &pinfo);
	//application_ioctl_call(fd, 1550, &pinfo);
	//application_ioctl_call(fd, -1, &pinfo);
	close(fd);
	return 0;
}
