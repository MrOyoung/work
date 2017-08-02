/*
 * The MIT License (MIT)
 *
 * Copyright © 2014 faith
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

/*======================================
	Header include
======================================*/

//需要链接的库
//LIBS:=-ldbus-glib-1 -ldbus-1 -lgobject-2.0 -lglib-2.0
//LIBS+=-lpthread
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include "common.h"
#include "util.h"
#include <dbus/dbus-glib.h>
#include <pthread.h>

#include <dbus/dbus.h> 

/*======================================
	Constant
======================================*/

#define FPS_INTERVAL	5000	/* calc fps per 5000 [ms] */

/*======================================
	Variable
======================================*/

static int frame_count = -1;
static struct timeval old_time, new_time;


/*======================================
	Public function
======================================*/

void
util_show_fps(void)
{
	unsigned int ms;

	if (frame_count < 0) {
		gettimeofday(&old_time, NULL);
		frame_count = 0;
	}

	frame_count++;

	gettimeofday(&new_time, NULL);
	ms = (new_time.tv_sec * 1e3 + new_time.tv_usec * 1e-3)
	   - (old_time.tv_sec * 1e3 + old_time.tv_usec * 1e-3);
	if (ms >= FPS_INTERVAL) {
		printf("%d frames in %d sec. fps = %f\n", frame_count, (int)(FPS_INTERVAL * 1e-3), frame_count / (FPS_INTERVAL * 1e-3));
		frame_count = 0;
		old_time = new_time;
	}
}

static int runing = 1;
bool is_running()
{

	return runing;
}
void* work_thread(void* args)
{
	DBusMessage* msg;
	char *word = NULL;
	DBusConnection* dbus = args;
   	DBusError err;
	
	dbus_error_init(&err);

	while(1){
        	dbus_connection_read_write(dbus,-1);  
        	msg = dbus_connection_pop_message (dbus);

        	printf("my_dbus_connection_pop_messag\n");
        	if(msg == NULL)
            	continue;


        	printf("dbus_message_get_type\n");
        	if( DBUS_MESSAGE_TYPE_SIGNAL == dbus_message_get_type(msg) ){

                printf("dbus_message_get_type_enter\n");
            	if( FALSE == dbus_message_get_args( msg, &err ,DBUS_TYPE_STRING, &word, DBUS_TYPE_INVALID ) ){
                 printf("dbus_message_get_args_enter\n");
                dbus_error_free( &err );
                goto __free_msg;
           	}
             	printf("receive message %s\n",word);

		if(word == "runing")
			runing = 0;
		else
			runing = 1;			

            }
        
  __free_msg:
        dbus_message_unref(msg);
    }

    return 0;
}

pthread_t work_id;
pthread_t dbus_init()
{
 	DBusConnection* dbus;
	//pthread_t work_id;
  
    	dbus = get_dbus( "com.saic.ServiceCamera" );
  
    	if( !dbus ){
        printf("get_dbus error!!\n");
      	return 1;
 	}

    	if( dbus_add_match(dbus,"type='signal',path='/com/saic/ServiceCamera'" ) < 0 )
      	 return 1;

	if( pthread_create(&work_id , NULL , work_thread , dbus ) ){
                        printf( "send thread create error!!\n"  );
                        return 1;
                  }


     //    pthread_join(work_id , NULL);
         
	return 0;                

}

void dbus_join()
{

	pthread_join(work_id , NULL);
}

DBusConnection* get_dbus(const char* well_known_name)
{
    int ret;
    DBusError err;  
    DBusConnection * connection;

    dbus_error_init(&err);  

    connection = dbus_bus_get(DBUS_BUS_SESSION ,&err );  
    if(dbus_error_is_set(&err)){  
        fprintf(stderr,"ConnectionErr : %s\n",err.message);  
        dbus_error_free(&err);  
        return NULL;
    }  
    if(connection == NULL)  
        return NULL;  

    ret =dbus_bus_request_name(connection, well_known_name ,DBUS_NAME_FLAG_REPLACE_EXISTING,&err);  
    if(dbus_error_is_set(&err)){  
        fprintf(stderr,"Name Err :%s\n",err.message);  
        dbus_error_free(&err);  
        goto __failed_get;
    }  

    if(ret !=DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)  
        goto __failed_get;

    return connection;

__failed_get:
    dbus_connection_unref( connection );
    return NULL;
}



int dbus_send(DBusConnection* dbus,void* array,int size,const char* path,const char*interface )
{
    int ret = 0;
    int serial;
    void *v_ARRAY = array;

    DBusMessage* msg =dbus_message_new_signal( path ,interface ,"data");
    if( !msg )
        return -1;

    if( ! dbus_message_append_args(msg, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &v_ARRAY, size , DBUS_TYPE_INVALID) ){
        ret = -1;
        goto __unref_msg;
    }

    if( !dbus_connection_send(dbus,msg,&serial)){  
        fprintf(stderr,"Out of Memory!\n");  
        ret = -1;
        goto __unref_msg;
    }  

    dbus_connection_flush(dbus); 

__unref_msg:
    dbus_message_unref( msg );
    return ret;
}

int dbus_send_noblocking( DBusConnection* dbus, void* array,int size , const char* path, char*interface   )
{
    int ret = 0;
    int serial;
    void *v_ARRAY = array;

    DBusMessage* msg =dbus_message_new_signal( path ,interface ,"data");
    if( !msg )
        return -1;

    if( ! dbus_message_append_args(msg, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &v_ARRAY, size , DBUS_TYPE_INVALID) ){
        ret = -1;
        goto __unref_msg;
    }

    if( !dbus_connection_send(dbus,msg,&serial)){  
        fprintf(stderr,"Out of Memory!\n");  
        ret = -1;
        goto __unref_msg;
    }  

__unref_msg:
    dbus_message_unref( msg );
    return ret;
    
}


int dbus_get_message_timeout(DBusConnection* dbus, void* message, int* length, int ms_timeout )
{
    DBusMessage* msg;
    int ret = 0;

    void* array;
    int len;

    DBusError err;
    dbus_error_init(&err);  

    if( FALSE == dbus_connection_read_write(dbus,ms_timeout) )
        return -1;

    msg =dbus_connection_pop_message (dbus);  
    if( !msg )
        return -1;

    if( DBUS_MESSAGE_TYPE_SIGNAL != dbus_message_get_type(msg) ){
        ret = -1;
        goto __unref_msg;
    }
    
    if(  FALSE == 
            dbus_message_get_args( msg, &err ,DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &array, &len, DBUS_TYPE_INVALID  ) ){
        dbus_error_free( &err );
        ret = -1;
        goto __unref_msg;
    }
    
    *length = len;
    memcpy( message , array , len );


__unref_msg:
    dbus_message_unref(msg);  
    return ret;
}


int dbus_get_message(DBusConnection* dbus, void* message, int* length)
{
    DBusMessage* msg;
    int ret = 0;

    void* array;
    int len;

    DBusError err;
    dbus_error_init(&err);  

    if( FALSE == dbus_connection_read_write(dbus,0) )
        return -1;

    msg =dbus_connection_pop_message (dbus);  
    if( !msg )
        return -1;

    if( DBUS_MESSAGE_TYPE_SIGNAL != dbus_message_get_type(msg) ){
        ret = -1;
        goto __unref_msg;
    }
    
    if(  FALSE == 
            dbus_message_get_args( msg, &err ,DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &array, &len, DBUS_TYPE_INVALID  ) ){
        dbus_error_free( &err );
        ret = -1;
        goto __unref_msg;
    }
    
    *length = len;
    memcpy( message , array , len );


__unref_msg:
    dbus_message_unref(msg);  
    return ret;
}


int dbus_add_match(DBusConnection* dbus, const char* rules )
{
    DBusError err;
    dbus_error_init(&err);  
    dbus_bus_add_match(dbus, rules , &err);  
    if(dbus_error_is_set(&err)){  
        fprintf(stderr,"Match Error%s\n",err.message);  
        dbus_error_free(&err);  
        return -1;
    }

    dbus_connection_flush(dbus);  
    return 0;
}


static char hexs[] = {
    '0','1','2','3','4','5','6','7','8','9',
    'a','b','c','d','e','f'
};



void debug_hex_array(const char* name , const byte* array , int size )
{
    int i;
    puts("+++++++++++++++++++++++++++++++++++++++++++++++++");
    puts(name);
    puts("-------------------------------------------------");
    for( i = 0; i < size ; i++ ){
        if( (0==(i%16)) && i)
            putchar('\n');


        putchar( '0' );
        putchar( 'x' );
        putchar( hexs[((array[i])>>4)&0xf]  );
        putchar( hexs[((array[i])>>0)&0xf]  );
        putchar( ' ' );
    }

    putchar('\n');
}




unsigned long get_tick_count()
{  
    struct timespec ts;  
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);  
  
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);  
} 

