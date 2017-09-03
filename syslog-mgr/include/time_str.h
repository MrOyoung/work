/************************************************************
**filename:SLMGR_time.c
**autor:YinDP
**date:2017/06/23
**description:get current time
************************************************************/

#ifndef _TIME_STR_H
#define _TIME_STR_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <sys/time.h>

/******************* time ******************/
#define DIG1 1
#define DIG2 10
#define DIG3 100
#define DIG4 1000
#define DIG5 10000
#define DIG6 100000

#define DIGITAL( num , dig ) ((((num) / DIG##dig) % 10) + '0' )

#define DIGITAL4_INC( __p , num ) \
	*__p++ = DIGITAL(num,4);\
	*__p++ = DIGITAL(num,3);\
	*__p++ = DIGITAL(num,2);\
	*__p++ = DIGITAL(num,1);\

#define DIGITAL2_INC( __p , num ) \
	*__p++ = DIGITAL(num,2);\
	*__p++ = DIGITAL(num,1);\

#define DIGITAL3_INC( __p , num ) \
	*__p++ = DIGITAL(num,6);\
	*__p++ = DIGITAL(num,5);\
	*__p++ = DIGITAL(num,4);\


static inline unsigned int make_current_time_str(char* buf)
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
static char *get_time(void)
{
	struct timeval usec;
	struct tm tv;
	time_t ltime;

	static char timebuf[128] = {0};

	ltime = time(NULL);
	localtime_r(&ltime, &tv);
	
	gettimeofday(&usec, NULL);

sprintf(timebuf, "%04d-%02d-%02d_%02d-%02d-%02d-%03ld",
			tv.tm_year + 1900, tv.tm_mon + 1, tv.tm_mday,
			tv.tm_hour, tv.tm_min, tv.tm_sec, (usec.tv_usec / 1000));

	return timebuf;
}


static inline unsigned long get_time_tick()
{
	struct timeval utime;

	gettimeofday(&utime, NULL);

	return (1000000 * utime.tv_usec + utime.tv_sec);
}

#endif //_TIME_STR_H
