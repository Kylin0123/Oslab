#include "x86.h"
#include "device.h"
#include <string.h>

extern int scrX,scrY;
extern struct ProcessTable pcb[MAX_PCB_NUM];
extern void schedule_pcb();
extern int current_pcb;
extern void save_pcb(struct TrapFrame * tf);
extern void update_pcb();
extern void recovery_pcb();
extern int index_of_no_use_pcb();
extern void del_pcb_by_pid(int i);
extern SegDesc gdt[NR_SEGMENTS];

void __attribute__ ((noinline)) sys_write(volatile struct TrapFrame *tf){
    volatile char val_ascii = tf->ebx;
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
            movw $0x10,%ax;\
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
        case 0x20:{
            save_pcb(tf);
            update_pcb();
            if(current_pcb != 0 && pcb[current_pcb].timeCount <= 0){
                pcb[current_pcb].state = RUNNABLE;
            }
            schedule_pcb();
            putChar('0'+current_pcb);
            recovery_pcb(tf);
            break;
        }
		case 0x80:{
			syscallHandle(tf);
			break;
        }
		default:assert(0);
	}
    asm volatile("\
            movw %0,%%ds;\
            movw %1,%%es;\
            movw %2,%%gs;\
            "::"m"(tf->ds),"m"(tf->es),"m"(tf->gs));
    
}

void syscallHandle(struct TrapFrame *tf) {
	/* 实现系统调用*/
    switch(tf->eax){
        case _NR_exit:{
            del_pcb_by_pid(current_pcb);
            schedule_pcb();
            update_pcb();
            recovery_pcb(tf);
            break;
        }
        case _NR_fork:{
            save_pcb(tf);
            
            int i = index_of_no_use_pcb();
            pcb[i].tf = pcb[current_pcb].tf;
            pcb[i].tf.eax = 0;
            pcb[i].pid = i;
            pcb[i].timeCount = 10;
            pcb[i].sleepTime = 0;
            pcb[i].state = RUNNABLE;

            //pcb[i].tf.esp = pcb[current_pcb].tf.esp - 0x1000000;
            //pcb[i].tf.ebp = pcb[current_pcb].tf.ebp - 0x1000000;
            pcb[i].tf.esp = pcb[current_pcb].tf.esp;
            pcb[i].tf.ebp = pcb[current_pcb].tf.ebp;
            

            memcpy((void*)pcb[i].tf.esp+0x1000000, (void*)pcb[current_pcb].tf.esp, 0x6d00000-pcb[i].tf.esp);
            
            memcpy((void*)0x1000000 + 0x200000, (void*)0x200000, 0x2000);

            pcb[current_pcb].tf.eax = pcb[i].pid;
            
            current_pcb = i; 
            pcb[current_pcb].state = RUNNING;

            /*Change to user process space*/
           
            recovery_pcb(tf);
            break;
        }
        case _NR_write:{
            sys_write(tf);
            break;
        }
        case _NR_sleep:{
            save_pcb(tf);
            update_pcb();
            pcb[current_pcb].sleepTime = tf->ebx;
            pcb[current_pcb].state = BLOCKED;
            schedule_pcb();
            recovery_pcb(tf);
            break;                   
        }
        default:assert(0);
    }
}

void GProtectFaultHandle(struct TrapFrame *tf){
	assert(0);
	return;
}
