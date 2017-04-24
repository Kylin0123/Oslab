#ifndef __TYPES_H__
#define __TYPES_H__

/* 定义数据类型 */
typedef unsigned int   uint32_t;
typedef          int   int32_t;
typedef unsigned short uint16_t;
typedef          short int16_t;
typedef unsigned char  uint8_t;
typedef          char  int8_t;
typedef unsigned int   size_t;

#define _NR_exit    1
#define _NR_fork    2
#define _NR_read    3
#define _NR_write   4
#define _NR_open    5
#define _NR_close   6

#define _NR_sleep   162

#endif
