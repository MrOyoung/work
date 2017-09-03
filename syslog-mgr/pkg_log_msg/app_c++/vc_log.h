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
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include "Module_Def.h"

#define _arg_in
#define _arg_out

enum _log_level
{
	info,
	warn,
	error,
	emerg
};

#define DIG1 1
#define DIG2 10
#define DIG3 100
#define DIG4 1000

#define DIGITAL( num , dig ) ((((num) / DIG##dig) % 10) + '0' )

#define DIGITAL4_INC( __p , num ) \
	*__p++ = DIGITAL(num,4);\
*__p++ = DIGITAL(num,3);\
*__p++ = DIGITAL(num,2);\
*__p++ = DIGITAL(num,1);\

#define DIGITAL2_INC( __p , num ) \
	*__p++ = DIGITAL(num,2);\
*__p++ = DIGITAL(num,1);\


static int make_current_time_str(char* buf)
{
	time_t t0;
	struct tm t1;
	char* p = buf;

	/*get current time*/
	time( &t0 );
	localtime_r( &t0, &t1 );

	/*year*/
	int year = t1.tm_year + 1900;
	DIGITAL4_INC( p , year );
	*p++ = '-';

	/*month*/
	DIGITAL2_INC( p , t1.tm_mon + 1);
	*p++ = '-';

	/*day*/
	DIGITAL2_INC( p , t1.tm_mday);
	*p++ = ' ';

	/*hour*/
	DIGITAL2_INC( p , t1.tm_hour);
	*p++ = ':';

	/*min*/
	DIGITAL2_INC( p , t1.tm_min);
	*p++ = ':';

	/*second*/
	DIGITAL2_INC( p , t1.tm_sec);
	*p++ = ' ';
	*p   = '\0';

	return (p - buf);
}

//获取level标签，格式为[ xxxx ]
static inline const char *syslog_get_level(enum _log_level level)
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

static inline int syslog_msg(char* buffer, _MODULE_NAME mod,enum _log_level level, const char* fmt, va_list ap)
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
	memcpy( __p , syslog_get_level(level) , LOG_LEVEL_LENGTH  );
	__p += LOG_LEVEL_LENGTH;

	/*4.message string*/
	size = vsnprintf( __p , 1024 , fmt , ap );

	return size + (__p - buffer);
}

static inline int VC_log(const char* path,
		const char* interface,
		_MODULE_NAME mod,
		enum _log_level level,
		const char* fmt, ... )
{
	int size;
	char* p;
	int ret;
	
	p = malloc( 1024 );
	if( !p )
		return -1;

	va_list ap;
	va_start(ap, fmt);
	size = syslog_msg(p , mod , level , fmt , ap );
	va_end(ap);

	/*dbus send*/
	ret = dbus_send( DBusConnection::Instance(),
					p,
					size,
					path,
					interface
					);
	free( p );

	return ret;
}


#define LOG_LEVEL(dbus, level,fmt,...)	\
			VC_log( LOG_SERVICE_PATH ,\
					THIS_MODULE_INTERFACE ,\
					THIS_MODULE_ID,\
					level,\
					fmt , ##__VA_ARGS__ )


#define LOG_EMERG(fmt,...) LOG_LEVEL( emerg , fmt , ##__VA_ARGS__ )
#define LOG_ERROR(fmt,...) LOG_LEVEL( error , fmt , ##__VA_ARGS__ )
#define LOG_WARN(fmt,...) LOG_LEVEL( warn , fmt , ##__VA_ARGS__ )
#define LOG_INFO(fmt,...) LOG_LEVEL( info , fmt , ##__VA_ARGS__ )

#endif //


