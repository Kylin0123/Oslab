#ifndef __TYPES_H__
#define __TYPES_H__

typedef unsigned int   uint32_t;
typedef          int   int32_t;
typedef unsigned short uint16_t;
typedef          short int16_t;
typedef unsigned char  uint8_t;
typedef          char  int8_t;
typedef unsigned char  boolean;

typedef uint32_t size_t;
typedef int32_t  pid_t;
typedef int32_t  sem_t;

typedef char* va_list;
#define _INTSIZEOF(n) ((sizeof(n)+sizeof(int)-1)&~(sizeof(int)-1))
#define va_start(ap,v) (ap = (va_list)&(v) + _INTSIZEOF(v))
#define va_arg(ap,t) (*(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#define va_end(ap) (ap = (va_list)0)

#define _NR_exit    1
#define _NR_fork    2
#define _NR_read    3
#define _NR_write   4
#define _NR_open    5
#define _NR_close   6
#define _NR_fwrite  7
#define _NR_seminit 64
#define _NR_sempost 65
#define _NR_semwait 66
#define _NR_semdestroy 67
#define _NR_sleep   162

#define O_RDONLY    00000000
#define O_RDWR      00000002
#define O_CREAT     00000100	


#endif
