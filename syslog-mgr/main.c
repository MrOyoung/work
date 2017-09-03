/************************************************************
 **filename:syslog_main.c
 **autor:Rocky
 **date:2017/05/24
 **description:syslog main
 ************************************************************/
#include <stdio.h>
#include <pthread.h>
#include "base.h"
#include "Debug.h"

#define _RECORD_TEST	1
#define _MONITOR_TEST	0
#define _GETXITOSLOG	0


int main(int argc, char *argv[])
{
	int ret, main_running = 1;

	pthread_t monitor_pth;
	pthread_t record_pth;
	pthread_t xitoslog_pth;


	while (main_running)	{
#if _RECORD_TEST
		ret = pthread_create(&record_pth, NULL, SLMGR_start_record, NULL);
		if (0 != ret) 
			errExit("pthread_create record_pth error\n");
		else
			printf("pthread_create record_pth success\n");
#endif

#if _GETXITOSLOG
		ret = pthread_create(&xitoslog_pth, NULL, SLMGR_start_monitor_kmsg, NULL);
		if (0 != ret) 
			errExit("pthread_create xitoslog_pth error\n");
		else
			printf("pthread_create xitoslog_pth success\n");

#endif

#if _MONITOR_TEST
		ret = pthread_create(&monitor_pth, NULL, SLMGR_start_monitor_logdir, NULL);
		if (0 != ret)
			errExit("pthread_create monitor_pth error\n");
		else
			printf("pthread_create monitor_pth success\n");

#endif


#if _RECORD_TEST
		ret = pthread_join(record_pth, NULL);
		if (0 != ret)
			errExit("pthread_join record_pth error\n");
		else
		{
			printf("record_pth exit\n");
			goto _label;
		}
#endif

#if _MONITOR_TEST 
		ret = pthread_join(monitor_pth, NULL);
		if (0 != ret)
			errExit("pthread_join monitor_pth error\n");
		else
		{
			printf("monitor_pth exit\n");
			goto _label;
		}
#endif

#if _GETXITOSLOG
		ret = pthread_join(xitoslog_pth, NULL);
		if (0 != ret)
			errExit("pthread_join xitoslog_pth error\n");
		else
		{
			printf("xitoslog_pth exit\n");
			goto _label;
		}
#endif

_label:
#if _MONITOR_TEST
		pthread_cancel(monitor_pth);
#endif

#if _RECORD_TEST
		pthread_cancel(record_pth);
#endif

#if _GETXITOSLOG
		pthread_cancel(xitoslog_pth);
#endif
	}/* end of while() */


	return 0;
}
