/*
 * The MIT License (MIT)
 *
 * Copyright © 2014 faith
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */
#ifndef _COMMON_H
#define _COMMON_H

/*======================================
	Header include
======================================*/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;
/*======================================
	Macro
======================================*/

#define LOG_ERROR(fmt, ...) fprintf(stderr, "\e[91m[%s:%s:%d] " fmt "\e[0m\n", __FILE__, __func__, __LINE__,  ## __VA_ARGS__)
#define LOG_DEBUG(fmt, ...) printf("[%s:%s:%d] " fmt "\n", __FILE__, __func__, __LINE__, ## __VA_ARGS__)

#define LOG_PERROR(str) LOG_ERROR("%s error %d, %s", str, errno, strerror(errno))


//user
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

#endif /* _COMMON_H */
