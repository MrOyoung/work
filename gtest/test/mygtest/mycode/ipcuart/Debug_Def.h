#ifndef _DEBUG_DEF_H_
#define _DEBUG_DEF_H_ /*UTF8 格式测试*/ /*Notepad++ 中文注释测试*/

#include <syslog.h>


#define __DEBUG_HEX_ARRAY( name , array , size ) \
    do{ \
    	static char hexs[] = {\
    	'0','1','2','3','4','5','6','7','8','9',\
    	'a','b','c','d','e','f'};\
    	int i; \
    	printf( MODULE_NAME ":-----------%s------------\n" , name );\
    	for( i = 0; i < size ; i++ ){\
        	if( (0==(i%16)) ){\
        		if( i > 0 )\
        			putchar('\n');\
        		printf( MODULE_NAME ":" );\
        	}\
        putchar( '0' );\
        putchar( 'x' );\
        putchar( hexs[((array[i])>>4)&0xf]  );\
        putchar( hexs[((array[i])>>0)&0xf]  );\
        putchar( ' ' );\
    }\
    putchar('\n');\
    }while(0);

#define debug_hex_array( name , array , size)  __DEBUG_HEX_ARRAY( (name) , (array) , (size) )


/*系统调试日志*/

#define DEBUG_LOG_INIT() openlog(MODULE_NAME , LOG_CONS|LOG_NDELAY|LOG_PID|LOG_PERROR, LOG_USER )

/*用于调试的标号*/
#ifdef _DEBUG_
#define LOG(format,...) syslog( LOG_DEBUG , format , ##__VA_ARGS__ )
#define _DEBUG_BEGIN_(...)   __VA_ARGS__
#define DEBUG_BEGIN(...) __VA_ARGS__
#define DEBUG(...) __VA_ARGS__
#define _DEBUG_END_
#define DEBUG_END
#define ASSERT( expr ) assert( expr )
#else
#define LOG(format,...)
#define _DEBUG_BEGIN_(...)
#define DEBUG_BEGIN(...)
#define DEBUG(...)
#define _DEBUG_END_
#define DEBUG_END
#define ASSERT( expr )
#endif


#endif
