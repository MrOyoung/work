#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>

//#define _STARTUP_DEBUG_
#define _STARTUP_RELEASE_

#ifdef _STARTUP_DEBUG_
	#define _PRINTF(fmt, ...) 	printf("file:%s func:%s() line:%d : "fmt"", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
	#define _PERROR(fmt, ...) 	perror(fmt)
#endif

#ifdef _STARTUP_RELEASE_
	#define _PRINTF(fmt, ...) \
		syslog(LOG_DEBUG, fmt, ##__VA_ARGS__)
	#define _PERROR(fmt, ...) \
		syslog(LOG_DEBUG, fmt, ##__VA_ARGS__)
#endif


#define PRINT_RET( msg, val ) \
	do { _PRINTF("%s", msg); return val; } while (0)

#define PERROR_RET( msg, val ) \
	do { _PERROR( msg); return val; } while (0)

#define PRINT_EXIT( msg, val ) \
	do { _PRINTF("%s", msg); exit(val); } while(0)

#define PERROR_EXIT( msg, val ) \
	do { _PERROR( msg ); exit(val); } while(0)

#define PRINT_MSG(fmt, ...) \
	_PRINTF(fmt, ##__VA_ARGS__)

#define _ARG_OUT_
#define _ARG_IN_


#endif //_DEBUG_H_
