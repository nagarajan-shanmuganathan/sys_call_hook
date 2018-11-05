#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <asm/paravirt.h>
#include <linux/kallsyms.h>
#include <linux/module.h>
#include <asm/unistd.h>
#include <linux/unistd.h>
#include <asm/syscalls.h>
#include <asm/asm-offsets.h>
#include <linux/sys.h>
#include <linux/fs.h>

#define SYS_CALL_ENTRY 333
#define MAX_ENTRIES __NR_syscall_max //0 - 334

//   /home/kernel/linux-hwe-4.15.0/arch/x86/entry/syscalls - Can see what entries are blank. 

MODULE_LICENSE("GPL");

unsigned long original_cr0;
static unsigned long *p_sys_call_table;


asmlinkage int (*original_syscall)(void);

asmlinkage int custom_call(unsigned long* state){
	//printk(KERN_ALERT "In custom call: system call start address: %p\n", *(p_sys_call_table[0]));
	//printk(KERN_ALERT "In custom call: system call first entry: %p\n", (unsigned long)(*(unsigned long*)(*(p_sys_call_table))));
	//unsigned long* src_add = p_sys_call_table;
	/*printk(KERN_ALERT "Sending: %p\n", p_sys_call_table[0]);
	printk(KERN_ALERT "Sending: %p\n", p_sys_call_table[78]);
	printk(KERN_ALERT "Sending: %p\n", p_sys_call_table[333]);
	printk(KERN_ALERT "In custom call: system call first entry: %p\n", (unsigned long)(*(unsigned long*)(*(p_sys_call_table))))*/;
	copy_to_user(state, p_sys_call_table, __NR_syscall_max * sizeof(unsigned long));
	return 0;
}

int __init init_module(void) 
{
	p_sys_call_table = (void *) kallsyms_lookup_name("sys_call_table");
	printk(KERN_ALERT "p_sys_call_address: %p\n", p_sys_call_table);
	printk(KERN_ALERT "system call table stored from address: %p\n", p_sys_call_table);

	original_cr0 = read_cr0();
	
	write_cr0(original_cr0 & ~0x00010000);
	
	original_syscall = p_sys_call_table[SYS_CALL_ENTRY];
	
	printk(KERN_ALERT "age old address of hello world: %p\n", original_syscall);
	p_sys_call_table[SYS_CALL_ENTRY] = custom_call;
	write_cr0(original_cr0);
	
	return 0;
}

void cleanup_module(void) 
{
	if(!p_sys_call_table) {
		return;
	}
	write_cr0(original_cr0 & ~0x00010000);
	p_sys_call_table[SYS_CALL_ENTRY] = (unsigned long *)original_syscall;
	write_cr0(original_cr0);
}
