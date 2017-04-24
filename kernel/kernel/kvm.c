#include "x86.h"
#include "device.h"
#include<string.h>

SegDesc gdt[NR_SEGMENTS];
TSS tss;
struct ProcessTable pcb[MAX_PCB_NUM];
int current_pcb;

#define SECTSIZE 512

void waitDisk(void) {
	while((inByte(0x1F7) & 0xC0) != 0x40); 
}

void readSect(void *dst, int offset) {
	int i;
	waitDisk();
	
	outByte(0x1F2, 1);
	outByte(0x1F3, offset);
	outByte(0x1F4, offset >> 8);
	outByte(0x1F5, offset >> 16);
	outByte(0x1F6, (offset >> 24) | 0xE0);
	outByte(0x1F7, 0x20);

	waitDisk();
	for (i = 0; i < SECTSIZE / 4; i ++) {
		((int *)dst)[i] = inLong(0x1F0);
	}
}

void initSeg() {
	gdt[SEG_KCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_KERN);
	gdt[SEG_KDATA] = SEG(STA_W,         0,       0xffffffff, DPL_KERN);
	gdt[SEG_UCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_USER);
	gdt[SEG_UDATA] = SEG(STA_W,         0,       0xffffffff, DPL_USER);
	gdt[SEG_TSS] = SEG16(STS_T32A,      &tss, sizeof(TSS)-1, DPL_KERN);
	gdt[SEG_TSS].s = 0;
    gdt[SEG_VIDEO] = SEG(STA_W,0xb8000,0xffffffff,DPL_KERN);
	setGdt(gdt, sizeof(gdt));

	/*
	 * 初始化TSS
	 */

    tss.ss0 = 0x10;
    //tss.esp0 = 0x8000000;
    tss.esp0 = 0x7f00000;
	asm volatile("ltr %%ax":: "a" (KSEL(SEG_TSS)));

	/*设置正确的段寄存器*/

    asm volatile("\
            movw $0x10,%ax;\
            movw %ax,%ds;\
            movw %ax,%es;\
            movw %ax,%ss;\
            movw %ax,%fs;\
            movw $0x30,%ax;\
            movw %ax,%gs;\
            ");
	lLdt(0);
	
}

void __attribute__((noinline)) enterUserSpace(uint32_t entry) {
	/*
	 * Before enter user space 
	 * you should set the right segment registers here
	 * and use 'iret' to jump to ring3
	 */
    asm volatile("movl $0x23,%%eax;\
                  movl %%eax,%%ds;\
                  movl %%eax,%%es;\
                  pushl %%eax;\
                  pushl $0x7d00000;\
                  pushf;\
                  pushl $0x1b;\
                  pushl %0;\
                  "::"m"(entry));
    enableInterrupt();
	asm volatile("iret");
}

uint32_t loadUMain(void) {

	/*加载用户程序至内存*/
    char buf[12*SECTSIZE];    //size of uMain.elf
    int i = 0;
    for(; i < 12; i++){
        readSect(buf + SECTSIZE * i, 201+i);
    }
    struct ELFHeader *elf = (void *)buf;
    struct ProgramHeader *ph = (void *)buf + elf->phoff;
    i = 0;
    for(;i < elf->phnum;i++,ph++)
        memcpy((void *)ph->vaddr, buf + ph->off, ph->memsz);
    //void (*uEntry_addr)(void) = (void *)elf->entry;
    //uEntry_addr();
    //enterUserSpace(elf->entry);
    return elf->entry;
}

void IDLE_process(){
    while(1)
        waitForInterrupt();
}

extern void init_all_pcbs();
extern int index_of_no_use_pcb();
extern void del_pcb_by_index(int i);

void initPcb(uint32_t entry){
    init_all_pcbs();
    //initial IDLE PCB
    int i = index_of_no_use_pcb();
    pcb[i].pid = 0;
    pcb[i].sleepTime = 0;
    pcb[i].timeCount = 10;
    pcb[i].state = RUNNABLE;
    pcb[i].tf.cs = 0x8;
    pcb[i].tf.eip = (uint32_t)IDLE_process;
    pcb[i].tf.eflags = 0x202;
    pcb[i].tf.ss = 0x10;
    pcb[i].tf.esp = 0x7e00000;

    pcb[i].tf.ebp = 0x7e00000;

    //inital UserPr PCB
    i = index_of_no_use_pcb();
    pcb[i].pid = 1;
    pcb[i].sleepTime = 0;
    pcb[i].timeCount = 10;
    pcb[i].state = RUNNING;
    pcb[i].tf.cs = 0x1b;
    pcb[i].tf.eip = (uint32_t)entry;
    pcb[i].tf.eflags = 0x202;
    pcb[i].tf.ss = 0x23;
    pcb[i].tf.esp = 0x7d00000;

    pcb[i].tf.ebp = 0x7d00000;

    current_pcb = 1; 
}


