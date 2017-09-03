/************************************************************
**filename:SLMGR_time.c
**autor:YinDP
**date:2017/06/23
**description:get current time
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <sys/time.h>

#include "base.h"
#include "config.h"

unsigned int make_current_time_str(char* buf)
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

unsigned long get_time_tick()
{
	struct timeval utime;

	gettimeofday(&utime, NULL);

	return (1000000 * utime.tv_usec + utime.tv_sec);
}

