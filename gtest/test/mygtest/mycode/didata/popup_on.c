#include "popup_on.h"
#include "Message_Def.h"

#include <stdio.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

/* 字节对齐 */
#define ROUND(__v,__a) \
	( (__v) + ((__a) - 1)  - ( ((__v) + ((__a)) - 1) % (__a) ) )


#define UINT_BITS (32)

/* 确保申请32字节对齐的内存空间，成员个数：11 */
static unsigned int popup_flags[ROUND(SERVICE_DI_MESSAGE_MAX&0xffff,32)/32] = {0};


/* with - 1, without - 0 */
int is_this_popup_on(unsigned int messageid)
{
	int idx;
	int bit;

	idx = 0xffff & messageid;		/* messageid 成员序号 */
	bit = 1 << (idx % UINT_BITS);	/* messageid 在成员的第几位 */

	return 0 != (popup_flags[ idx / UINT_BITS ] & bit);
}

void set_this_popup_on(unsigned int messageid)
{
	int idx;
	int bit;

	idx = 0xffff & messageid;
	bit = 1 << (idx % UINT_BITS);

	popup_flags[ idx / UINT_BITS] |= bit;
}

void set_this_popup_off(unsigned int messageid)
{
	int idx;
	int bit;

	idx = 0xffff & messageid;
	bit = 1 << (idx % UINT_BITS);

	popup_flags[ idx / UINT_BITS ] &= (~bit);
}

