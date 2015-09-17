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
	struct list_head *head;
	struct task_struct *task_info = current;
	struct processinfo temp;
	struct task_struct *older_sibling;
	struct task_struct *younger_sibling;
	struct task_struct *youngest_child;

	temp.pid = task_info->pid;
	temp.state = task_info->state;
	temp.parent_pid = task_info->parent->pid;

	if(!list_empty(&task_info->children))
	{
		youngest_child = list_last_entry(&task_info->children, struct task_struct, children);
		temp.youngest_child = youngest_child->pid;
	}
	else
	{
		temp.youngest_child = -1;
	}
	if(list_entry(task_info->sibling.next, struct task_struct, sibling)->pid > temp.pid)
	{
		younger_sibling = list_entry(task_info->sibling.next, struct task_struct, sibling);
		temp.younger_sibling = younger_sibling->pid;
	}
	else
	{
		temp.younger_sibling = -1;
	}
	if(list_entry(task_info->sibling.prev, struct task_struct, sibling)->pid < temp.pid)
	{
		older_sibling = list_entry(task_info->sibling.prev, struct task_struct, sibling);
		temp.older_sibling = older_sibling->pid;
	}
	else
	{
		temp.older_sibling = -1;
	}

	temp.uid = task_info->real_cred->uid.val;
	temp.start_time = timespec_to_ns(&task_info->start_time);
	temp.user_time = cputime_to_usecs(&task_info->utime);
	temp.sys_time = cputime_to_usecs(&task_info->stime);
	temp.cutime = 0;
	temp.cstime = 0;

	if(!list_empty(&task_info->children))
	{
		list_for_each(head, &task_info->children)
		{
			struct task_struct *t;
			t = list_entry(head, struct task_struct, children);
			temp.cutime += cputime_to_usecs(&t->utime);
			temp.cstime += cputime_to_usecs(&t->stime);
		}
	}

	if (copy_to_user(info, &temp, sizeof temp)) {
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
