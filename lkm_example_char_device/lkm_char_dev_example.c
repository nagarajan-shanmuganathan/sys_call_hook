#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/sched.h>

#include<linux/device.h>
#include<linux/pid.h>
#include<linux/pid_namespace.h>
#include<linux/slab.h>
#include<linux/time.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Abhishek Lolage");
MODULE_DESCRIPTION("First example of Loadable Linux Kernel Module");
MODULE_VERSION("0.01");

#define DEVICE_NAME "lkm_char_dev_example"
#define EXAMPLE_MSG "Hello, World!\n"
#define MSG_BUFFER_LEN 15

typedef struct procinfo{
	pid_t pid;
	pid_t ppid;
	long start_time;
	int num_sib;
}procinfo;

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static long device_ioctl(struct file* f, unsigned int cmd, unsigned long arg);
static procinfo get_process_info(struct task_struct* tstruct);

static int major_num;
static int device_open_count = 0;
static char msg_buffer[MSG_BUFFER_LEN];
static char *msg_ptr;

static struct file_operations file_ops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release,
	.unlocked_ioctl = device_ioctl
};

static long pid_ = 0;

static procinfo get_process_info(struct task_struct* tstruct){
	procinfo pinfo;
	int num_sib = 0;
	
	struct list_head* ptr;

	list_for_each(ptr, &(tstruct->sibling)) {
		num_sib++;
	}
	pinfo.pid = tstruct->pid;
	pinfo.ppid = (tstruct->parent)->pid;
	pinfo.start_time = (tstruct->start_time);
	pinfo.num_sib = num_sib;
	return pinfo;
}
static int __init lkm_char_dev_example_init(void){
	printk(KERN_INFO "%s\n", __FUNCTION__);
	strncpy(msg_buffer, EXAMPLE_MSG, MSG_BUFFER_LEN);	
	msg_ptr = msg_buffer;
	major_num = register_chrdev(0, "lkm_char_dev_example", &file_ops);
	if(major_num < 0){
		printk(KERN_ALERT "Could not register device: %d\n", major_num);
		return major_num;
	}
	else{
		printk(KERN_INFO "lkm_char_dev_example module loaded with device major number %d\n", major_num);
		return 0;
	}
	return 0;
}

static void __exit lkm_char_dev_example_exit(void){
	unregister_chrdev(major_num, DEVICE_NAME);
	printk(KERN_INFO "lkm_char_dev_example module unregistered");
	printk(KERN_INFO "%s\n", __FUNCTION__);
}

static ssize_t device_read(struct file *filp, char *buffer, size_t len, loff_t *offset){
	printk(KERN_ALERT "This operation is not supported.\n");
	return 0;
}

static ssize_t device_write(struct file *filp, const char *buffer, size_t len, loff_t *offset){
	// kstrol
	unsigned int base = 10;
	int conversion;
	conversion = kstrtol(buffer, base, &pid_);
	// get char buffer, convert to int using kstrol
	if(conversion != 0){
		return -EINVAL;
	}
	return 0;
}

static int device_open(struct inode *inode, struct file *file){
	if (device_open_count) {
		return -EBUSY;
	}
	device_open_count++;
	try_module_get(THIS_MODULE);
	return 0;
}

static int device_release(struct inode *inode, struct file *file){
	device_open_count--;
	module_put(THIS_MODULE);
	return 0;
}

static long device_ioctl(struct file* f, unsigned int cmd, unsigned long arg){
	int bytes_not_read = 0;
	struct task_struct* tstruct;
	procinfo pinfo;
	if(pid_ > 0){
		printk(KERN_ALERT "Logging info about process with given PID\n");
		// return info about process with this pid
		tstruct = pid_task(find_get_pid(cmd), PIDTYPE_PID);
		pinfo = get_process_info(tstruct);
	}
	else if(pid_ == 0){
		printk(KERN_ALERT "Logging info about current process\n");
		// return info about current process
		tstruct = current;
		pinfo = get_process_info(tstruct);
	}

	// TODO: cannot pass negative argument, write the custom write function
	else if(pid_ < 0){
		printk(KERN_ALERT "Logging info about parent of current process\n");
		// return info about parent of current process
		tstruct = current->parent;
		pinfo = get_process_info(tstruct);
	}

	bytes_not_read = copy_to_user((void*)arg, &pinfo, sizeof(procinfo));
	// declare the importing struct
	//printk(KERN_ALERT "Bytes not read %d\n", bytes_not_read);
	return 0;
}


module_init(lkm_char_dev_example_init);
module_exit(lkm_char_dev_example_exit);
