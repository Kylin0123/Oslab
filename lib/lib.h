#ifndef __lib_h__
#define __lib_h__
#include "types.h"

void printf(const char *format,...);

static inline int fork() {
    int pid = 0;
    asm volatile(
            "pushl %%eax;\
             pushl %%ebx;\
             movl $0, %%ebx;\
             movl $0x2, %%eax;\
             int $0x80;\
             movl %%eax, %0;\
             popl %%ebx;\
             popl %%eax;\
            ":"=m"(pid)
            );
    return pid;
}

static inline int sleep(uint32_t time){
    asm volatile(
            "pushl %%eax;\
             pushl %%ebx;\
             movl $162, %%eax;\
             movl %0, %%ebx;\
             int $0x80;\
             popl %%ebx;\
             popl %%eax;\
            "::"m"(time)
            );


    return 0;
}

static inline int exit(){
     asm volatile(
            "pushl %eax;\
             movl $0x1, %eax;\
             int $0x80;\
             popl %eax;\
            ");
    return 0;
}

int sem_init(sem_t *sem, uint32_t value);
int sem_post(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_destroy(sem_t *sem);

int open(char *path, int flags);
int read(int fd, void *buffer, int size);
int write(int fd, void *buffer, int size);
int lseek(int fd, int offset, int whence);
int close(int fd);
int remove(char *path);

void ls(char *path);
void cat(char *path);

#endif
