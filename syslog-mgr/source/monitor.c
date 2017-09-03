/************************************************************
 **filename		: syslog_mgr.c
 **description	: receive msg from dbus
 **autor		: Rocky
 **date			: 2017/05/24
 ************************************************************/

#include <stdio.h>

#include "Module_Def.h"
#include "config.h"
#include "base.h"

#include "socket.h"

/* callback func */
static void *SLMGR_handler(void *data, unsigned int size)
{
	int msgid = *(int *)data;
	
	SLMGR_record_log(msgid,
					(unsigned char *)(data + sizeof(int)),
					size - sizeof(int));
	return NULL;
}

void *SLMGR_start_record(void *arg)
{
	struct socket_info server;

	server.backlog	= MODULE_SUM;
	server.type		= SOCKET_SERVER;

	if (-1 == socket_create(&server, LOCAL_IP, LOCAL_PORT))
		return NULL;

	/* get msg from socket */
	socket_dispatch_timeout(server, SLMGR_handler, NULL);

	socket_close(server.sockfd);

	return NULL;
}

