#include <stdio.h>
#include <stdlib.h>
#include "ipc.h"
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#define QUEUE_CNT 100
#define NEW_WAY_ROCKY
#undef OLD_WAY


unsigned long get_tick_count()
{  
    struct timespec ts;  
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);  
  
    return (ts.tv_sec * 1000000 + ts.tv_nsec / 1000);  
} 


int main( int argc , char** argv)
{
	/*struct timeval start_time;
	struct timeval end_time;
	*/
	srand( time(NULL) );//产生随机数
	
	IpcObject_t*  ipc = IPCOBJ_connect( "com.saic.ServiceDiData" , MAKE_RULES_STR("/com/saic/ServiceDiData") );//MAKE_RULES_STR：宏，在ipc.h中定义，返回自身的一个句柄
    
 
 	if( NULL == ipc ){
 		printf("error ipc\n");
 		return 1;
 	}
 	
 	int count = 50;
 	if( argc > 1 ){
 		int c = atoi( argv[1] );
 		if( c > 0 )
 			count = c;
 	}
 	
 	//printf("send size:%d \n", count );
 	
 	
	int i = 1;

#ifdef OLD_WAY/*{{{*/
	for( i = 0 ; i < QUEUE_CNT ; i++  ){
		result_t r = IPCMQ_init_for_send( &queue[i] , "/com/saic/AppCluster" , "ServiceDiData.signal.type" ) ;
		if( r != RESULT_OK  ){
			printf("init ipc message queue error\n");
			return 1;
		}
	}
	
	int index = 0;
	struct IpcMessageQueue queue[QUEUE_CNT];
	
    while( index++ < count ){
  
    	unsigned int msg[] = { index , index , index , index , index , index };
		int size = (rand() % 5) + 2;
		size *= 4;

		//printf("%d\n",index);
		
		
		for( i = 0 ; i < QUEUE_CNT ; i ++  ){
			if( RESULT_OK != IPCMQ_put_message( &queue[i], msg, size ) ){
				printf("put message error:%d\n", index );
				return 1;
			}
		}
		
    }

    //printf("queue_cnt = %d\n", QUEUE_CNT);

	for( i = 0 ; i < QUEUE_CNT ; i ++ ){
		//gettimeofday(&start_time, NULL);
		if( RESULT_OK != IPCMQ_send( &queue[i] , ipc ) ){
			printf("send error\n");
			return 1;
		}
	}

#endif/*}}}*/

#ifdef NEW_WAY_ROCKY
	struct IpcMessageQueue queue;
	msg_unit send_msg;

	int j;
	int index = 0;
	
	//int package_per_100m = 400 / count;
	int package_per_100m = 10;
	
	
	unsigned long before_time;
	
	unsigned long cost_time;
	
	
__label_loop:

 	
 	index = 0;
	while( index++ < package_per_100m )
	{
		before_time = get_tick_count();
		//printf("package_per_100m = %d\n", package_per_100m);
		//gettimeofday(&start_time, NULL);
		
		result_t r = IPCMQ_init_for_send( &queue , "/com/saic/AppCluster" ,"ServiceDiData.signal.type" ) ;
		if( r != RESULT_OK  ){
			printf("init ipc message queue error\n");
			return 1;
		}

		for( j = 0 ; j < count ; j++  ){
			//memset(send_msg, 0, sizeof(msg_unit));
			send_msg.msgid = index;

			//gettimeofday(&send_msg->now_time, NULL);
			
			if( RESULT_OK != IPCMQ_put_message( &queue, &send_msg, sizeof(msg_unit) ) ){
				printf("put message error:%d\n", i );
				return 1;
			}
		}

		if( RESULT_OK != IPCMQ_send( &queue, ipc ) ){
			printf("send error\n");
			return 1;
		}
	
		IPCMQ_free(&queue);
		
		printf("time = %lu\n", (get_tick_count() - before_time) );
		
		//gettimeofday(&end_time, NULL);
		//printf("time = %ld\n", (end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec));
		
	}
	
	
	
/*	
	if( cost_time < (10 * 100) )
		usleep( (10 * 100) - cost_time );
	else
		printf("send too slow\n");
	*/	
	//goto __label_loop;
	
	

#endif


    return 0;
}
