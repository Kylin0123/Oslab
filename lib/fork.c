/*************************************************************************
	> File Name: fork.c
	> Author: 
	> Mail: 
	> Created Time: Mon 10 Apr 2017 07:02:15 PM CST
 ************************************************************************/

#include"types.h"
#include"lib.h"

/*static inline int fork(){
    int pid = 0;
    asm (
            "movl $0, %%ebx;\
             movl $0x2, %%eax;\
             int $0x80;\
             movl %%eax, %0;\
            ":"=m"(pid)
            );
    return pid;
}*/
