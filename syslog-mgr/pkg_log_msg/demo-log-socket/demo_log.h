/************************************************************
 **filename:syslog_mgr.c
 **autor:Rocky
 **date:2017/05/26
 **description:pkg log msg
 ************************************************************/

#ifndef _VD_LOG_H
#define _VD_LOG_H

#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <stdarg.h>

#include "Module_Def.h"
#include "time_str.h"
#include "socket-api/socket.h"

#include "config.h"

#define _arg_in
#define _arg_out

enum _log_level
{
	info,
	warn,
	error,
	emerg
};

//获取level标签，格式为[ xxxx ]
static inline const char *DEMO_get_level(enum _log_level level)
{
	static const char* level_strs[] = {
		[info]  = "[ info ]",
		[warn]  = "[ warn ]",
		[error] = "[ error]",
		[emerg] = "[ emerg]",
	};

	return level_strs[level];
}

#define MODULE_NAME_LENGTH (sizeof(_MODULE_NAME))
#define LOG_LEVEL_LENGTH   (8)
#define TIME_LENGTH		   (20)

static int DEMO_pkg_log(char* buffer, enum _log_level level, const char* fmt, va_list ap)
{
	int size;
	char* __p;
	__p = buffer;

	/*1.module id:4 byte*/
	*((_MODULE_NAME*)__p) = THIS_MODULE_ID;
	__p += sizeof(_MODULE_NAME);

	*__p = '\n';
	__p += 1;

	/*2.time:20 byte*/
	make_current_time_str( __p );
	__p += TIME_LENGTH;

	/*3.log level:8 byte*/
	memcpy( __p , DEMO_get_level(level) , LOG_LEVEL_LENGTH  );
	__p += LOG_LEVEL_LENGTH;

	/*4.message string*/
	size = vsnprintf( __p , 64 , fmt , ap );
	//size = vsnprintf( __p , 1024 , fmt , ap );

	return size + (__p - buffer);
}

static struct socket_info server;


int DEMO_socket_init(void)
{
	server.type = SOCKET_CLIENT;

	if (-1 == socket_create(&server, SERVER_IP, SERVER_PPORT))
		return -1;

	if (-1 == socket_connect(server))
	{
		perror("connect to socket error");
		socket_close(server.sockfd);
		server.sockfd = 0;
		return -1;
	}

	fprintf(stderr, "socket connect success...\n");

	return 0;
}


static int DEMO_send_log(enum _log_level level, const char* fmt, ... )
{
	int size;
	int ret;
	char* str;

	if (!server.sockfd)
	{
		if (-1 == DEMO_socket_init())
			return -1;
	}

	str = calloc(1, BUFFER_SIZE);

	va_list astr;
	va_start(astr, fmt);
	size = DEMO_pkg_log(str, level, fmt, astr );
	va_end(astr);

	fprintf(stderr, "size = %d msg is %s\n", size, str + sizeof(int) + 1);

	/* socket send */
	socket_send(&server, (unsigned char *)str, size);	
	if (-1 == server.retsize)
	{
		perror("demo log send error");
		socket_close(server.sockfd);
		server.sockfd = 0;
		ret = -1;
	}

	free( str );

	return ret;
}


#define DEMO_LOG_LEVEL(level,fmt,...)	\
	DEMO_send_log(level, fmt, ##__VA_ARGS__)

#ifndef ENABLE_DEMO_LOG
	#define DEMO_LOG_EMERG(fmt, ...)
	#define DEMO_LOG_ERROR(fmt, ...)
	#define DEMO_LOG_WARN(fmt, ...)
	#define DEMO_LOG_INFO(fmt, ...)
#endif
#ifdef ENABLE_DEMO_LOG
	#define DEMO_LOG_EMERG(fmt, ...) DEMO_LOG_LEVEL( emerg , fmt , ##__VA_ARGS__ )
	#define DEMO_LOG_ERROR(fmt, ...) DEMO_LOG_LEVEL( error , fmt , ##__VA_ARGS__ )
	#define DEMO_LOG_WARN(fmt, ...) DEMO_LOG_LEVEL( warn , fmt , ##__VA_ARGS__ )
	#define DEMO_LOG_INFO(fmt, ...) DEMO_LOG_LEVEL( info , fmt , ##__VA_ARGS__ )
#endif

#endif //_VD_DEMO_LOG_H
