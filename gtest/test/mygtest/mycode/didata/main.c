#include <stdlib.h>  
#include <string.h>  
#include <dbus/dbus-glib.h>  
#include <dbus/dbus.h>  
#include <unistd.h>  
#include <pthread.h>
#include "util.h"
#include "table.h"
#include "timer.h"
#include "Message_Def.h"
#include "common.h"
#include "debug.h"
#include "Module_Def.h"
#include "popup.h"

/* test for main() */
#include "main.h"


_DEBUG_BEGIN_(
	extern int catch_msg_id;
)_DEBUG_END_

#include <signal.h>

static volatile  unsigned int running = 1;

void handle_ctrl_c(int signum, siginfo_t *info, void *data)
{
	printf("receive SIGINT!\n");

	running = 0;	
}


unsigned int set_ctrl_c_handler(void)
{
	struct sigaction act;

	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = handle_ctrl_c;

	sigaction(SIGINT, &act, NULL);

	return 0;
}


void* work_thread(void* args)
{
    DBusMessage* msg;
    byte* array;
    int len;

    DBusConnection* dbus = (DBusConnection*)args;
    DBusError err;

	
    dbus_error_init(&err);  
    int wait_time = -1;
    int no_msg_cnt = 0;/*连续没有接收到消息的次数*/

	
    //while(1){
    while(running){

        /*连续5次收不到消息，则进入等待，等待时间为距当期最近的定时器的时间*/
        if( no_msg_cnt > 5 )
		{
			printf("[ LINE:%d ] running = %d wait_time = %d\n", __LINE__, running, wait_time);
            dbus_connection_read_write( dbus,  wait_time  );  
			printf("[ LINE:%d ] running = %d wait_time = %d\n", __LINE__, running, wait_time);
		}
	
        /*检查是否有消息*/
        msg = dbus_connection_pop_message (dbus);  
        if( NULL == msg ) {
            no_msg_cnt ++;
            goto __timer;
        }
        no_msg_cnt = 0;

        /*检查该消息是否为信号*/
        if( DBUS_MESSAGE_TYPE_SIGNAL != dbus_message_get_type(msg) )  
            goto __free_msg;

        /*检查该消息内的参数*/
        if( FALSE == dbus_message_get_args( msg, &err ,DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &array, &len, DBUS_TYPE_INVALID ) ){
            dbus_error_free( &err );
            dbus_error_init( &err );
            goto __free_msg;
        }

        /*缓冲区头部为一个int，解释为MSGID*/
        switch( *((int*)array) ){
        	/*通用上行桢*/
            case IPC_UART_DATA_GENERAL_UP:
				//LOG("recv general frame\n");
                do_table( dbus , GeneralMsgTable , general_size ,array + 4 );
                break;

			/*特殊上行桢*/
            case IPC_UART_DATA_SPECIAL_UP:
				//LOG("recv special frame\n");
                do_table( dbus , SpecialMsgTable , special_size, array + 4 );
                break;
              
            /*复位功能*/    
            case APP_CLUSTER_SELFCHECK_READY:
            LOG("recv APP_CLUSTER_SELFCHECK_READY\n");
            	{
            		int i;
            		for( i = 0 ; i < general_size ; i++ )
            			GeneralMsgTable[i].reset( &GeneralMsgTable[i] );
            		for( i = 0 ; i < special_size ; i++ )
            			SpecialMsgTable[i].reset( &SpecialMsgTable[i] );
            		
            		/*clear popup list*/
            		popup_clear_list();
            	}
            	break;
            

            default:
                LOG("di data:wrong msgid:%x \n",*((int*)array) );
                break;
        }

__free_msg:/*释放消息*/
        dbus_message_unref(msg);  

__timer:/*执行定时器操作*/
        wait_time = (int)TIMER_loop();/*返回值为最近一次定时器操作所要等待的时间*/
    }

    return 0;
}


int main_bak( int argc , char ** argv)
{  
    DBusConnection* dbus;
    pthread_t work_id;
    
    DEBUG_LOG_INIT();
   

    dbus = get_dbus( MODULE_SERVICE_DI_DATA_ANALISIS_NAME );
    if( !dbus ){
        LOG( "get_dbus error!!\n");
		perror("error msg ");
        return 1;
    }

    if( dbus_add_match(dbus,"type='signal',path='" MODULE_SERVICE_DI_DATA_ANALISIS_PATH "'" ) < 0 )
        return 1;


	set_ctrl_c_handler();

//    while(1){
        
        if( pthread_create(&work_id , NULL , work_thread , dbus ) ){
            LOG(  "send thread create error!!\n"  );
            return 1;
        }

        pthread_join( work_id , NULL );
//    }
    return 0;  
}  
