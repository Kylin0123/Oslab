#include "lib.h"
#include "types.h"
/*
 * io lib here
 * 库函数写在这
 */
int32_t syscall(int num, uint32_t a1,uint32_t a2,
		uint32_t a3, uint32_t a4, uint32_t a5)
{
	//int32_t ret = 0;
    /*asm volatile("movl %0,%%eax;\
                movl %1,%%ecx;\
                movl %2,%%edx;\
                movl %3,%%ebx;\
                int $0x80;\
                 "::"r"(num),"r"(a1),"r"(a2),"r"(a3)
                );
    */
	/* 内嵌汇编 保存 num, a1, a2, a3, a4, a5 至通用寄存器*/
    asm volatile("movl %0,%%eax;\
                movl %1,%%ebx;\
                movl %2,%%ecx;\
                movl %3,%%edx;\
                movl %4,%%edi;\
                movl %5,%%esi;\
                int $0x80;\
                 "::"m"(num),"m"(a1),"m"(a2),"m"(a3),"m"(a4),"m"(a5)
                );
    
	return 0;//ret;
}

void __attribute__ ((noinline)) printch(char ch){
    asm volatile("movl %0, %%eax;\
                movl %1, %%ebx;\
                int $0x80\
                 "::"i"(_NR_write),"m"(ch));
}

void printstr(char * str){
    while(*str){
        printch(*str);
        str++;
    } 
}

void printint(int volatile val){
    if(val == 0x80000000){
        printstr("-2147483648");
        return;
    }
    if(val < 0){
        val = -val;
        printch('-');
    } 
    else if(val == 0)
        printch('0');
    char volatile buff[32] = {0};
    int volatile size = 0;
    while(val){
        buff[size++] = val%10;
        val /= 10;
    }
    int volatile i = size - 1;
    for(;i >= 0; i--)
        printch('0'+buff[i]);
}

void printx(uint32_t xval){
    if(xval == 0)
        printch('0');
    char buff[32] = {0};
    int size = 0;
    while(xval){
        buff[size++] = xval%16;
        xval /= 16;
    }
    int i = size - 1;
    for(;i >= 0; i--){
        if(buff[i] >= 10)
            printch('a'-10+buff[i]);
        else
        printch('0'+buff[i]);
    }
}

void printf(const char * volatile format,...){
    va_list volatile arg_ptr = (void *)&format;
    va_start(arg_ptr, format);
    //printch(*format);
    //printch(*(format+1));
    while(*format){
        if(*format == '%'){
            format++;
            switch(*format){
                case 's':{
                    char *str = va_arg(arg_ptr, char*);
                    printstr(str);
                    break;
                }
                case 'c':{
                    char ch = va_arg(arg_ptr, char);
                    printch(ch);
                    break;
                }
                case 'x':{
                    uint32_t xval = va_arg(arg_ptr, uint32_t);
                    printx(xval);
                    break;
                }
                case 'd':{
                    int val = va_arg(arg_ptr, int);
                    printint(val);
                    break;
                }
                default:{
                    printch(*format);
                    break;
                }
            }
            format++;
        }
        else{
            printch(*format);
            format++;
        }

    }
    va_end(arg_ptr);
    //asm volatile("int $0x80");
}
