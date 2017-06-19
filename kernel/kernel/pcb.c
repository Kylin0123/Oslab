/*************************************************************************
	> File Name: pcb.c
	> Author: 
	> Mail: 
	> Created Time: Mon 10 Apr 2017 08:47:04 PM CST
 ************************************************************************/

#include"x86.h"
#include"device.h"

extern struct ProcessTable pcb[MAX_PCB_NUM];
extern int current_pcb;

void init_all_pcbs(){
    int i = 0;
    for(; i < MAX_PCB_NUM; i++)
        pcb[i].valid = FALSE;
}

int index_of_no_use_pcb(){
    int i = 0;
    for(; i < MAX_PCB_NUM; i++)
        if(pcb[i].valid == FALSE){
            pcb[i].valid = TRUE;
            return i;
        }
    return -1;
}

void del_pcb_by_pid(int pid){
    int i = 0;
    for(; i < MAX_PCB_NUM; i++)
        if(pcb[i].valid == TRUE && pcb[i].pid == pid)
            pcb[i].valid = FALSE;
}

void save_pcb(struct TrapFrame *tf){
    pcb[current_pcb].tf = *tf;
    pcb[current_pcb].state = RUNNABLE;
}

void schedule_pcb(){
    int scheduled = -1;
    int i = 1;
    for(; i < MAX_PCB_NUM; i++){
        if(pcb[i].valid == TRUE && pcb[i].state == RUNNABLE && i != current_pcb)
            scheduled = i;
    }
    if(scheduled == -1) scheduled = 0;
    pcb[scheduled].state = RUNNING;
    pcb[scheduled].timeCount = 10;
    current_pcb = scheduled;
}

void update_pcb(){
    pcb[current_pcb].timeCount--;
    int i = 0;
    for(; i < MAX_PCB_NUM; i++){
        if(pcb[i].valid == TRUE && pcb[i].state == BLOCKED){
            pcb[i].sleepTime --;
        if(pcb[i].sleepTime == 0){
                pcb[i].state = RUNNABLE;
                pcb[i].timeCount = 10;
                //schedule_pcb();
            }
        }
    }
}

extern SegDesc gdt[NR_SEGMENTS];

void recovery_pcb(struct TrapFrame *tf){
    pcb[current_pcb].state = RUNNING;
    *tf = pcb[current_pcb].tf;
    tf->ebp = pcb[current_pcb].tf.ebp;
    if(current_pcb == 2){      //child proc
        gdt[SEG_UDATA].base_15_0 = 0;
        gdt[SEG_UDATA].base_23_16 = 0;
        gdt[SEG_UDATA].base_31_24 = 1;
    }
    else{                      //parent proc
        gdt[SEG_UDATA].base_15_0 = 0;
        gdt[SEG_UDATA].base_23_16 = 0;
        gdt[SEG_UDATA].base_31_24 = 0;
    }
}




