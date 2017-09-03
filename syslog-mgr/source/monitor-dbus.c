/************************************************************
 **filename		: syslog_mgr.c
 **description	: receive msg from dbus
 **autor		: Rocky
 **date			: 2017/05/24
 ************************************************************/

#include <stdio.h>

#include "base.h"
#include "ipc.h"

/* 
 *call-back func,the parameter of func:IPCMQ_dispatch() 
 * new dbus api supported by YDP 
 */
static void SLMGR_handler(IpcObject_t *__obj, unsigned int __msgid, void *__data, int __size)
{

	SLMGR_record_log(__msgid, 
			(unsigned char *)(__data), 
			__size
			);

#if 0
	/* check id */

	/* use func isascii() with three point to check data */

	/* record */
	SLMGR_record_log(*((int*)__data), 
			(unsigned char *)(__data + sizeof(_MODULE_NAME) ), 
			__size - sizeof(_MODULE_NAME)
			);
#endif
}

void *SLMGR_start_record(void *arg)
{
	IpcObject_t *dbus;

	dbus = (IpcObject_t *)arg;

	while (1)
	{
		/*get msg from dbus*/
		IPCMQ_dispatch(dbus, SLMGR_handler, -1);
	}

	return NULL;
}

