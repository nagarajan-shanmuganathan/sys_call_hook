#pragma once
#include <linux/fs.h>
//unsigned long **p_sys_call_table = (void*)0xffffffff81e001a0;
struct file_operations* proc_modules_operations = (struct file_operations*)0xffffffff81e1cec0;
