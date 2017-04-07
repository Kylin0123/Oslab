#include "x86.h"
#include "device.h"

extern int scrX,scrY;

void __attribute__ ((noinline)) sys_write(struct TrapFrame *tf){
    int val_ascii = tf->ebx;
    if(val_ascii == '\n'){ scrX++; scrY = 0; return; }
    asm volatile("movl %0, %%eax;\
            imull $80, %%eax;\
            addl %1, %%eax;\
            imull $2, %%eax;\
            movl %%eax, %%edi;\
            movb $0x0c, %%ah;\
            movb %2, %%al;\
            movw %%ax, %%gs:(%%edi)\
            "::"m"(scrX), "m"(scrY), "m"(val_ascii)
            );            //在第x行第y列打印
    scrY++;
    if(scrY >= 80) { scrX++; scrY = 0; }
}

void syscallHandle(struct TrapFrame *tf);

void GProtectFaultHandle(struct TrapFrame *tf);

void irqHandle(struct TrapFrame *tf) {
	/*
	 * 中断处理程序
	 */
	/* Reassign segment register */
    asm volatile("\
            movw $0x20,%ax;\
            movw %ax,%ds;\
            movw %ax,%es;\
            movw $0x30,%ax;\
            movw %ax,%gs;\
            ");
	switch(tf->irq) {
		case -1:{
			break;
        }
		case 0xd:{
			GProtectFaultHandle(tf);
			break;
        }
		case 0x80:{
			syscallHandle(tf);
			break;
        }
		default:assert(0);
	}
}

void syscallHandle(struct TrapFrame *tf) {
	/* 实现系统调用*/
    switch(tf->eax){
        case _NR_write:{
            sys_write(tf);
            break;
        }
        default:assert(0);
    }
}

void GProtectFaultHandle(struct TrapFrame *tf){
	assert(0);
	return;
}
