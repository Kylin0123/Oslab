#include "common.h"
#include "x86.h"
#include "device.h"

int scrX,scrY;    //screen x & y
void initxy(){ scrX = 0; scrY = 0; }

extern void initTimer();

void kEntry(void) {
    initxy();

	initSerial();// initialize serial port
	initIdt(); // initialize idt
	initIntr(); // iniialize 8259a
    initTimer(); //TODO: initialize timer
	initSeg(); // initialize gdt, tss
    enableInterrupt(); //TODO: enable interrupt
	uint32_t entry = (uint32_t)loadUMain(); // load user program, enter user space
    initPcb(entry);
    enterUserSpace(entry);
	while(1);
	assert(0);
}
