#ifndef _SHM_H_
#define _SHM_H_

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>


#ifndef LOG
#define LOG printf
#endif


#define SHM_KEY (('D'<<24)|('A'<<16)|('I'<<8)|('L'<<0))

struct service_shared_mem{
	volatile gint speed;
	volatile gint tach;
};


#define TO_MEMBER(mem,member) (&(((struct service_shared_mem*)mem)->member))

static inline void* open_share_mem()
{
	int shm_id;
	int first_create;
	void* share_mem;

	/*first open share memery*/
	shm_id = shmget( SHM_KEY , 
			sizeof(struct service_shared_mem) ,
			IPC_CREAT|IPC_EXCL|0666 );
	
	/**/
	first_create =  (-1 != shm_id);
	
	/*share memery is already exist!!!*/
	if( -1 == shm_id ){
	
		/*if other error happen!,we only to return NULL!!*/
		if( EEXIST != errno )
			return NULL;
		
		/*second get shm,but no IPC_EXCL flag*/	
		shm_id = shmget( SHM_KEY , 
			sizeof(struct service_shared_mem) ,
			IPC_CREAT|0666 );
	}		

	/**/
	if( -1 == shm_id ){
		LOG("create share memory error!\n");
		return NULL;
	}

	share_mem = shmat( shm_id , 0 , 0 );
	if( NULL == share_mem ){
		LOG("attch share memory error!\n");
		return NULL;
	}
	
	/*first open shm, you need to clear the share memory*/
	if( first_create )
		memset( share_mem , 0x00 , sizeof(struct service_shared_mem) );
	

	return share_mem;
}


static   inline int service_get_speed(void* mem)
{
	return g_atomic_int_get( TO_MEMBER(mem,speed) );
}

static inline int service_get_tach(void* mem)
{
	return g_atomic_int_get( TO_MEMBER(mem,tach) );
}

static inline void service_set_speed(void* mem, int spd)
{
	g_atomic_int_set( TO_MEMBER(mem,speed) , spd );
}

static inline void service_set_tach(void* mem, int ta)
{
	g_atomic_int_set( TO_MEMBER(mem,tach) , ta );
}



/*

int main(int argc, char** argv)
{
	if(argc < 2){
		printf("usage:%s <1/0>\n", argv[0] );
		return 1;
	}

	int rf = -1;

	if( strcmp(argv[1], "1") == 0 ){
		printf("read\n");
		rf = 1;
	}else if( strcmp(argv[1] , "0") == 0 ){
		printf("write\n");
		rf = 0;
	}else{
		printf("error arg:%s\n", argv[1] );
		return -1;
	}


	void* mem = open_share_mem();
	if(!mem){
		printf("open share mem error\n");
		return -1;
	}

	if( 0 == rf ){
		
		int i = 0;
		for( i = 0 ; 1 ; i++ ){
			service_set_speed( mem , i );
			service_set_tach(  mem , i );
			sleep(1);
		}
	
	}else{
		while(1){
			printf("speed%d ---- tach%d\n", service_get_speed(mem) , service_get_tach(mem) );
			usleep( 100 * 1000 );
		}
	
	}


}
*/

#endif 
