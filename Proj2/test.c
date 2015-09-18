#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "processinfo.h"

#define LARGE_NUM 500000000L // Used for giving program run time in for loops
#define __NR_cs3013_syscall2 356

/* Function to print statistics to a created 'processinfo' struct */
void print_stats()
{
    struct processinfo* info = (struct processinfo*)malloc(sizeof(struct processinfo));
    printf("----------------------\n");
    printf("[Process]       : %ld\n", (long) syscall(__NR_cs3013_syscall2, info));
    printf("State           : %ld\n", info->state);
    printf("Pid             : %d\n", info->pid);
    printf("Parent Pid      : %d\n", info->parent_pid);
    printf("Youngest Child  : %d\n", info->youngest_child);
    printf("Younger Sibling : %d\n", info->younger_sibling);
    printf("Older Sibling   : %d\n", info->older_sibling);
    printf("UID             : %d\n", info->uid);
    printf("Start time      : %lld\n", info->start_time);
    printf("User time       : %lld\n", info->user_time);
    printf("Sys time        : %lld\n", info->sys_time);
    printf("CUTime          : %lld\n", info->cutime);
    printf("CSTime          : %lld\n\n", info->cstime);
}


int main () {
    int pid; // Pid for children processes
    int i; // Counter
    long l; // Run time counter
    long times[] = {LARGE_NUM * 2, LARGE_NUM * 1, LARGE_NUM * 3}; // Used to make child 1 finish before child 0, showing younger/older siblings
    for (i = 0; i < 3; i++)
    {
        if ((pid = fork()) == 0)
        {
            for (l = 0; l < times[i]; l++); // Run time
            printf("Child %d (%d) info\n", i, getpid());
            print_stats();
            exit(0);
        }
    }
    for (l = 0; l < LARGE_NUM/2; l++); // Run time
    printf("Parent process info\n");
    print_stats();
    while(wait(NULL) > 0); // Wait for all children
    return 0;
}


