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
#include "util.h"
#include "Module_Def.h"
#include "time_str.h"

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
static inline const char *DEMO_get_log_level(enum _log_level level)
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

static inline int DEMO_pkg_log_msg(char* buffer, _MODULE_NAME mod,enum _log_level level, const char* fmt, va_list ap)
{
	int size;
	char* __p;
	__p = buffer;

	/*1.module id:4 byte*/
	*((_MODULE_NAME*)__p) = mod;
	__p += sizeof(_MODULE_NAME);

	*__p = '\n';
	__p += 1;

	/*2.time:20 byte*/
	make_current_time_str( __p );
	__p += TIME_LENGTH;

	/*3.log level:8 byte*/
	memcpy( __p , DEMO_get_log_level(level) , LOG_LEVEL_LENGTH  );
	__p += LOG_LEVEL_LENGTH;

	/*4.message string*/
	size = vsnprintf( __p , 1024 , fmt , ap );

	return size + (__p - buffer);
}

static DBusConnection *DEMO_dbus;

static inline void DEMO_set_dbus_object(void *arg)
{
	DEMO_dbus = (DBusConnection *)arg;
}

static inline int DEMO_log( DBusConnection* dbus, 
		const char* path,
		const char* interface,
		_MODULE_NAME mod,
		enum _log_level level,
		const char* fmt, ... )
{
	int size;
	char* p;
	int ret;

	if (NULL == dbus)
	{
		fprintf(stderr, "error:dbus is NULL\n");
		exit( 0 );
	}
	
	p = malloc( 1024 );
	if( !p )
		return -1;

	va_list ap;
	va_start(ap, fmt);
	size = DEMO_pkg_log_msg(p , mod , level , fmt , ap );
	va_end(ap);

	/*dbus send*/
	ret = dbus_send(dbus,
					p,
					size,
					path,
					interface
					);
	free( p );

	return ret;
}


#define DEMO_LOG_LEVEL(level,fmt,...)	\
			DEMO_log( DEMO_dbus , \
					LOG_SERVICE_PATH ,\
					THIS_MODULE_INTERFACE ,\
					THIS_MODULE_ID,\
					level,\
					fmt , ##__VA_ARGS__ )


#define DEMO_LOG_EMERG(fmt, ...) DEMO_LOG_LEVEL( emerg , fmt , ##__VA_ARGS__ )
#define DEMO_LOG_ERROR(fmt, ...) DEMO_LOG_LEVEL( error , fmt , ##__VA_ARGS__ )
#define DEMO_LOG_WARN(fmt, ...) DEMO_LOG_LEVEL( warn , fmt , ##__VA_ARGS__ )
#define DEMO_LOG_INFO(fmt, ...) DEMO_LOG_LEVEL( info , fmt , ##__VA_ARGS__ )

#endif //_VD_DEMO_LOG_H
