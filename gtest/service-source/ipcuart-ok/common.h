#ifndef _COMMON_H_
#define _COMMON_H_
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;

#define _UNUSED_(v) (void)(v)
#define UNUSED(v) _UNUSED_(v)


/*建议编译器最有可能的情况*/
#define unlikely(x) x
#define likely(x) x

//计算数组大小
#define array_size(array) (sizeof(array)/sizeof(array[0]))

#define MASK_0 0x00
#define MASK_1 0x01
#define MASK_2 0x03
#define MASK_3 0x07
#define MASK_4 0x0f
#define MASK_5 0x1f
#define MASK_6 0x3f
#define MASK_7 0x7f
#define MASK_8 0xff
#define MASK_9 0x1ff
#define MASK_10 0x3ff
#define MASK_11 0x7ff
#define MASK_12 0xfff
#define MASK_13 0x1fff
#define MASK_14 0x3fff
#define MASK_15 0x7fff
#define MASK_16 0xffff
#define MASK_17 0x1ffff
#define MASK_18 0x3ffff
#define MASK_19 0x7ffff
#define MASK_20 0xfffff
#define MASK_21 0x1fffff
#define MASK_22 0x3fffff
#define MASK_23 0x7fffff
#define MASK_24 0xffffff
#define MASK_25 0x1ffffff
#define MASK_26 0x3ffffff
#define MASK_27 0x7ffffff
#define MASK_28 0xfffffff
#define MASK_29 0x1fffffff
#define MASK_30 0x3fffffff
#define MASK_31 0x7fffffff
#define MASK_32 0xffffffff

#endif
