/************************************************************
** filename:env_main.c
** autor:Rocky
** date:2017/05/03
** description:env service module
************************************************************/
#include "env_api.h"

#ifdef _USE_DBUS
void env_handler( IpcObject_t* __obj ,unsigned int __msgid , void* __data , int __size )
{
	struct timeval end_time;
	struct timeval *send_time = (struct timeval *)__data;
	
	gettimeofday(&end_time, NULL);
	//printf("message id:%d --- size:%d \n" , __msgid , __size );
	
	//printf("time = %ld\n", (end_time.tv_sec - send_time->tv_sec) * 1000000 + (end_time.tv_usec - send_time->tv_usec));

	return;
}
#endif


int main(int argc, char *argv[])
{
#ifdef _USE_DBUS
	IpcObject_t* dbus = IPCOBJ_connect("com.saic.AppCluster" , MAKE_RULES_STR("/com/saic/AppCluster") );

	if( !dbus ){
		printf("cant not connect ipc\n");
		return 1;
	}


 	while(1){
 		IPCMQ_dispatch( dbus , env_handler  , -1 );
		
	}
#endif
	
	init();	
	
	unsigned char tmp[128] = {0};
	MSG_ID a = ENV_SERVICE_SET;
	
	memcpy(tmp, &a, sizeof(a));
	memcpy(tmp + sizeof(a), "PATH=/usr/bin", strlen("USER=/usr/bin") + 1);
	
	env_test(tmp, sizeof(a) + strlen("USER=/usr/bin"));
	
	memset(tmp, 0, 128);
	a = ENV_SERVICE_GET;
	
	memcpy(tmp, &a, sizeof(a));
	memcpy(tmp + sizeof(a), "HOME=/usr/bin", strlen("USER=/usr/bin") + 1);
	
	env_test(tmp, sizeof(a) + strlen("USER=/usr/bin"));
	
    return 0;
}


