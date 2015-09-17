#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/time.h>
#include <asm/current.h>
#include <asm/cputime.h>
#include <asm/uaccess.h>
#include <asm/errno.h>
#include "processinfo.h"

unsigned long **sys_call_table;

asmlinkage long (*ref_sys_cs3013_syscall2)(void);

asmlinkage long new_sys_cs3013_syscall2(struct processinfo *info) {
	struct list_head *task_head;
	struct processinfo current_proc_info;
	struct task_struct *task;
	pid_t temp_pid;
	struct task_struct *current_proc = current;

	current_proc_info.pid = current_proc->pid;
	current_proc_info.state = current_proc->state;
	current_proc_info.parent_pid = current_proc->parent->pid;
	current_proc_info.uid = current_uid().val;

	(current_proc_info.youngest_child = (!list_empty(&current_proc->children)) ? 
		list_last_entry(&current_proc->children, struct task_struct, children)->pid : -1);

	temp_pid = (list_next_entry(current_proc, sibling)->pid);
	(current_proc_info.younger_sibling = (temp_pid > current_proc_info.pid) ? temp_pid : -1);

	temp_pid = (list_prev_entry(current_proc, sibling)->pid);
	(current_proc_info.older_sibling = (temp_pid < current_proc_info.pid) ? temp_pid : -1);

	current_proc_info.start_time = timespec_to_ns(&current_proc->start_time);
	current_proc_info.user_time = cputime_to_usecs(&current_proc->utime);
	current_proc_info.sys_time = cputime_to_usecs(&current_proc->stime);
	current_proc_info.cutime = 0;
	current_proc_info.cstime = 0;

	if(current_proc_info.youngest_child != -1) {
		list_for_each(task_head, &current_proc->children) {
			task = list_entry(task_head, struct task_struct, children);
			current_proc_info.cutime += cputime_to_usecs(&task->utime);
			current_proc_info.cstime += cputime_to_usecs(&task->stime);
		}
	}

	if (copy_to_user(info, &current_proc_info, sizeof current_proc_info)) {
		return EFAULT;
	}
	return 0;
}

static unsigned long **find_sys_call_table(void) {
  unsigned long int offset = PAGE_OFFSET;
  unsigned long **sct;
  
  while (offset < ULLONG_MAX) {
    sct = (unsigned long **)offset;

    if (sct[__NR_close] == (unsigned long *) sys_close) {
      printk(KERN_INFO "Interceptor: Found syscall table at address: 0x%02lX",
                (unsigned long) sct);
      return sct;
    }
    
    offset += sizeof(void *);
  }
  
  return NULL;
}

static void disable_page_protection(void) {
  /*
    Control Register 0 (cr0) governs how the CPU operates.

    Bit #16, if set, prevents the CPU from writing to memory marked as
    read only. Well, our system call table meets that description.
    But, we can simply turn off this bit in cr0 to allow us to make
    changes. We read in the current value of the register (32 or 64
    bits wide), and AND that with a value where all bits are 0 except
    the 16th bit (using a negation operation), causing the write_cr0
    value to have the 16th bit cleared (with all other bits staying
    the same. We will thus be able to write to the protected memory.

    It's good to be the kernel!
   */
  write_cr0 (read_cr0 () & (~ 0x10000));
}

static void enable_page_protection(void) {
  /*
   See the above description for cr0. Here, we use an OR to set the 
   16th bit to re-enable write protection on the CPU.
  */
  write_cr0 (read_cr0 () | 0x10000);
}

static int __init interceptor_start(void) {
  /* Find the system call table */
  if(!(sys_call_table = find_sys_call_table())) {
    /* Well, that didn't work. 
       Cancel the module loading step. */
    return -1;
  }
  
  /* Store a copy of all the existing functions */
  ref_sys_cs3013_syscall2 = (void *)sys_call_table[__NR_cs3013_syscall2];

  /* Replace the existing system calls */
  disable_page_protection();

  sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)new_sys_cs3013_syscall2;
  
  enable_page_protection();
  
  /* And indicate the load was successful */
  printk(KERN_INFO "Loaded interceptor!");

  return 0;
}

static void __exit interceptor_end(void) {
  /* If we don't know what the syscall table is, don't bother. */
  if(!sys_call_table)
    return;
  
  /* Revert all system calls to what they were before we began. */
  disable_page_protection();
  sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)ref_sys_cs3013_syscall2;
  enable_page_protection();

  printk(KERN_INFO "Unloaded interceptor!");
}

MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);
