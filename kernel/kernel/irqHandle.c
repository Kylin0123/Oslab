#include "x86.h"
#include "device.h"
#include "kernel_string.h" 

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
extern struct semaphore sem;
extern union SuperBlock sb;
extern union GroupDesc gd;
#define SECTSIZE 512
extern uint8_t InodeBitmap[2*SECTSIZE];
extern uint8_t BlockBitmap[2*SECTSIZE];
extern union Inode ids[10];

#define OneBlockRead(SECT) do{ \
    readSect(buf, SECT); \
    readSect(buf + SECTSIZE, SECT+1); \
}while(0)

#define OneBlockWrite(SECT) do{ \
    writeSect(buf, SECT); \
    writeSect(buf + SECTSIZE, SECT+1); \
}while(0)

#define putStr(STR) do{\
    for(int i = 0; STR[i]!=0; i++)\
    putChar(STR[i]);\
}while(0)

void __attribute__ ((noinline)) sys_write(volatile struct TrapFrame *tf){
    volatile char val_ascii = tf->ebx;
    if(val_ascii == '\n'){ scrX++; scrY = 0; return; }
    asm volatile("\
            movl %0, %%eax;\
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
                schedule_pcb();
            }
            else if(current_pcb == 0)
                schedule_pcb();
#ifdef SHOW_CUR_PCB
            putChar('0'+current_pcb);
#endif
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

#define OneBlockWrite2(STRUCT) do{\
    memcpy(buf, &STRUCT, 1024);\
    OneBlockWrite(i);\
    i+=2;\
}while(0)

void file_system_exit(){
    char buf[2 * SECTSIZE];
    int i = 201;
    OneBlockWrite2(sb);
    OneBlockWrite2(gd);
    OneBlockWrite2(InodeBitmap);
    OneBlockWrite2(BlockBitmap);
    int j = 0;
    for(; j < 10; j++)
        OneBlockWrite2(ids[j]);
}

void syscallHandle(struct TrapFrame *tf) {
    /* 实现系统调用*/
    switch(tf->eax){
        case _NR_exit:{
                          del_pcb_by_pid(current_pcb);
                          schedule_pcb();
                          update_pcb();
                          recovery_pcb(tf);
                          file_system_exit();
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
                          pcb[i].tf.esp = pcb[current_pcb].tf.esp;
                          pcb[i].tf.ebp = pcb[current_pcb].tf.ebp;
                          memcpy((void*)pcb[i].tf.esp+0x1000000,
                                  (void*)pcb[current_pcb].tf.esp,
                                  0x6d00000-pcb[i].tf.esp);

                          memcpy((void*)0x1000000 + 0x200000, (void*)0x200000, 0x2000);

                          pcb[current_pcb].tf.eax = pcb[i].pid;

                          //current_pcb = i; 
                          //pcb[current_pcb].state = RUNNING;

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
        case _NR_seminit:{
                             tf->eax = 0;
                             tf->ecx = (int)&sem;
                             sem.value = tf->edx;
                             break;                 
                         }
        case _NR_sempost:{
                             struct semaphore *S = (void *)tf->ecx;
                             S->value++;
                             if(S->value <= 0){
                                 S->list->state = RUNNABLE;
                             }
                             tf->eax = 0;
                             break;
                         }
        case _NR_semwait:{
                             struct semaphore *S = (void *)tf->ecx;
                             S->value--;
                             if(S->value < 0){
                                 save_pcb(tf);
                                 update_pcb();
                                 pcb[current_pcb].state = BLOCKED;
                                 pcb[current_pcb].sleepTime = -1; 
                                 S->list = &pcb[current_pcb];
                                 schedule_pcb();
                                 recovery_pcb(tf);
                             }
                             tf->eax = 0;
                             break;
                         }
        case _NR_semdestroy:{
                                tf->ecx = 0;
                                tf->eax = 0;
                                break;
                            }

        case _NR_open:{
                          //char *path = (void *)tf->ebx;
                          char path[20];
                          memcpy(path, (void *)tf->ebx,20);
                          int flags = tf->ecx;
                          char* cur_file = strtok(path, "/");
                          int cur_inode = 0;
                          do{
                              if(cur_file == NULL) break;
                              //int blockSize = ids[cur_inode].blockCount;
                              if(ids[cur_inode].type == 2){
                                  char buf[2 * SECTSIZE];
                                  OneBlockRead(201+14*2+ids[cur_inode].pointer[0]*2);
                                  union DirEntryTable *det = (void *)buf;
                                  int i = 0;
                                  for(; i < det->size; i++){
                                      if(mystrcmp(det->dirs[i].name, cur_file) == 0){
                                          cur_inode = det->dirs[i].inode;
                                          break;
                                      }
                                  }
                                  if(i == det->size){    //fail to find file
                                      if(flags & O_CREAT){
                                         int new_inode = gd.availInodeNum++;
                                         sb.availInodeNum++;
                                         int new_size = 2 * SECTSIZE;
                                         ids[new_inode].type = 1;
                                         ids[new_inode].linkCount = 0;
                                         ids[new_inode].blockCount = new_size / sb.blockSize;
                                         ids[new_inode].size = 0;
                                         ids[new_inode].pointer[0] = gd.availBlockNum;
                                         gd.availBlockNum += ids[new_inode].blockCount;
                                         sb.availBlockNum += ids[new_inode].blockCount;
                                         tf->eax = new_inode;
                                        
                                         int new_det_file = det->size++;
                                         memcpy(det->dirs[new_det_file].name, cur_file, 20);
                                         det->dirs[new_det_file].inode = new_inode;
                                         OneBlockWrite(201+14*2+ids[cur_inode].pointer[0]*2);
                                         
                                      }
                                      else
                                          tf->eax = -1;
                                      break;
                                  }
                              }
                          }while((cur_file = strtok(NULL, "/")));
                          if(cur_file == NULL) tf->eax = cur_inode;
                          break;                  
                     }
        case _NR_read:{
                          int cur_inode = tf->ebx;
                          char* buffer = (void *)tf->ecx;
                          strcpy(buffer,"");
                          //int size = tf->edx;
                          switch(ids[cur_inode].type){
                              case 1:{
                                         char buf[2 * SECTSIZE];
                                         OneBlockRead(201+14*2+ids[cur_inode].pointer[0]*2);
                                         memcpy(buffer,buf,1024);
                                         break;
                                     }
                              case 2:{
                                         char buf[2 * SECTSIZE];
                                         OneBlockRead(201+14*2+ids[cur_inode].pointer[0]*2);
                                         union DirEntryTable *det = (void *)buf;
                                         int i = 0;
                                         for(; i < det->size; i++){
                                             mystrcat(buffer, det->dirs[i].name);
                                             mystrcat(buffer, "  ");
                                         }      
                                         break;
                                     }
                          }
                          break;
                      }
        case _NR_fwrite:{
                            int cur_inode = tf->ebx;
                            char* buffer = (void *)tf->ecx;
                            int size = tf->edx;
                            int cur_size = ids[cur_inode].size;
                            char buf[2*SECTSIZE];
                            OneBlockRead(201+14*2+ids[cur_inode].pointer[0]*2);
                            memcpy(buf+cur_size,buffer,size);
                            ids[cur_inode].size += size;
                            OneBlockWrite(201+14*2+ids[cur_inode].pointer[0]*2);
                            break;
                        }
        default:assert(0);
    }
}

void GProtectFaultHandle(struct TrapFrame *tf){
	assert(0);
	return;
}
