#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>

#define GET_CURRENT_TIME clock()
#define USER_APP_PID 8665 

typedef struct procinfo{
	pid_t pid;
	pid_t ppid;
	long start_time;
	int num_sib;
}procinfo;

void printProcInfo(procinfo* p){
	printf("#######################\n");	
	printf("PID: %d\n", p->pid);
	printf("Parent's PID: %d\n", p->ppid);
	printf("# of siblings: %d\n", p->num_sib);
	//printf("Start time: %ld\n", (long)((p->start_time).tv_sec));
	printf("Start time in secs: %lf\n", (double)(p->start_time)/(double)10000000000);
}
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
		//printf("device_ioctl() was called, check /var/log/syslog\n");
		printProcInfo(procinfo);
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
	clock_t time;
	time = GET_CURRENT_TIME;
	application_ioctl_call(fd, 0, &pinfo);
	printf("Execution time: %lf\n", (double)((GET_CURRENT_TIME - time)));
	printf("#######################\n\n");	
	time = GET_CURRENT_TIME;
	application_ioctl_call(fd, USER_APP_PID, &pinfo);
	printf("Execution time: %lf\n", (double)((GET_CURRENT_TIME - time)));
	printf("#######################\n\n");	
	time = GET_CURRENT_TIME;
	application_ioctl_call(fd, -1, &pinfo);
	printf("Execution time: %lf\n", (double)((GET_CURRENT_TIME - time)));
	printf("#######################\n\n");	
	close(fd);
	return 0;
}
