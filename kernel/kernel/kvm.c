#include "x86.h"
#include "device.h"
#include<string.h>

SegDesc gdt[NR_SEGMENTS];
TSS tss;

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
	setGdt(gdt, sizeof(gdt));

    gdt[6] = SEG(STA_W,0xb8000,0xffffffff,DPL_KERN);
	/*
	 * 初始化TSS
	 */

    tss.ss0 = 0x10;
    tss.esp0 = 0x8000000;
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
                  pushl $0x7f00000;\
                  pushf;\
                  pushl $0x1b;\
                  pushl %0;\
                  "::"m"(entry));
	asm volatile("iret");
}

void loadUMain(void) {

	/*加载用户程序至内存*/
    char buf[12*SECTSIZE];    //size of uMain.elf
    int i = 0;
    for(; i < 12; i++){
        readSect(buf + SECTSIZE * i, 201+i);
    }
    struct ELFHeader *elf = (void *)buf;
    struct ProgramHeader *ph = (void *)buf + elf->phoff;
    memcpy((void *)ph->vaddr, buf + ph->off, ph->memsz);
    //void (*uEntry_addr)(void) = (void *)elf->entry;
    //uEntry_addr();
    enterUserSpace(elf->entry);
}
