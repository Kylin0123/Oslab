/*************************************************************************
	> File Name: sleep.c
	> Author: 
	> Mail: 
	> Created Time: Mon 10 Apr 2017 07:08:17 PM CST
 ************************************************************************/

#include"types.h"
#include"lib.h"

/*int __attribute__ ((noinline)) sleep(uint32_t time){
    asm volatile(
            "movl $162, %%eax;\
             movl %0, %%ebx;\
             int $0x80;\
            "::"m"(time)
            );
    return 0;
}*/
