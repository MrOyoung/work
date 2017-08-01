#include <stdio.h>
#include <stdlib.h>
#include "ipc.h"



void handler( IpcObject_t* __obj ,unsigned int __msgid , void* __data , int __size )
{
	struct timeval end_time;
	struct timeval *send_time = (struct timeval *)__data;
	
	gettimeofday(&end_time, NULL);
	printf("message id:%d --- size:%d \n" , __msgid , __size );
	
	//printf("time = %ld\n", (end_time.tv_sec - send_time->tv_sec) * 1000000 + (end_time.tv_usec - send_time->tv_usec));
}


int main(int argc,char** argv)
{
    (void)argc;(void)argv;
    
    IpcObject_t* dbus = IPCOBJ_connect("com.saic.AppCluster" , MAKE_RULES_STR("/com/saic/AppCluster") );

    if( !dbus ){
    	printf("cant not connect ipc\n");
    	return 1;
    }
 
 
 	while(1){
 		IPCMQ_dispatch( dbus , handler  , -1 );
		
	}
 
 
    return 0;
}
