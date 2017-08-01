#include "ipc.h"
#include <string.h>


IpcObject_t* IPCOBJ_connect(const char* well_known_name , const char* listen_rules )
{
    int ret;
    DBusError err;  
    DBusConnection * connection;

    dbus_error_init(&err);  
    connection = dbus_bus_get(DBUS_BUS_SESSION ,&err );  
    if( TRUE == dbus_error_is_set(&err)){  
        fprintf(stderr,"ConnectionErr : %s\n",err.message);  
        dbus_error_free(&err);  
        return NULL;
    }  
    if( NULL == connection )  
        return NULL;  

    ret = dbus_bus_request_name(connection, well_known_name ,DBUS_NAME_FLAG_REPLACE_EXISTING,&err);  
    if( TRUE == dbus_error_is_set(&err) ){  
        fprintf(stderr,"Name Err :%s\n",err.message);  
        dbus_error_free(&err);  
        goto __failed_get;
    }  

    if(ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)  
        goto __failed_get;


	dbus_bus_add_match( connection , listen_rules , &err);  
    if( TRUE == dbus_error_is_set(&err)){  
        fprintf(stderr,"Match Error%s\n",err.message);  
        dbus_error_free(&err);  
        goto __failed_get;
    }
    
    dbus_connection_flush(connection); 
	
    return (IpcObject_t*)connection;

__failed_get:
    dbus_connection_unref( connection );
    return NULL;
}


inline void IPCOBJ_disconnect(IpcObject_t*  obj )
{
	dbus_connection_unref( obj );
}


inline result_t IPCMQ_init_for_send(struct IpcMessageQueue* queue , const char* path , const char* interface)
{
    DBusMessage* msg = dbus_message_new_signal( path , interface , "d" );

    if( NULL == msg ){
    	printf("error:dbus_message_new_signal -- path:%s -- interface:%s", path , interface);
    	return RESULT_ERR;
    }
        

    dbus_message_iter_init_append(  msg , &queue->it );
    queue->m = msg;
    queue->cnt = 0;
    return RESULT_OK;
}


inline result_t IPCMQ_put_message( struct IpcMessageQueue* queue, const void* message , int size )
{
    DBusMessageIter sub;
    if( FALSE == dbus_message_iter_open_container( 
                &queue->it ,
               DBUS_TYPE_ARRAY,
               DBUS_TYPE_BYTE_AS_STRING,
             &sub ) )
       return RESULT_ERR;
    
    if( FALSE == dbus_message_iter_append_fixed_array( &sub , DBUS_TYPE_BYTE , &message , size ) )
        return RESULT_ERR;

    if( FALSE == dbus_message_iter_close_container( &queue->it , &sub ) )
        return RESULT_ERR;
        
    queue->cnt++;

    return RESULT_OK;
}


inline int IPCMQ_size( struct IpcMessageQueue* queue )
{
	return queue->cnt;
}


inline result_t IPCMQ_send( struct IpcMessageQueue* queue , IpcObject_t* __obj )
{
	if( TRUE == dbus_connection_send(  __obj , queue->m , 0 ) ){
		dbus_connection_flush( __obj );
    	return RESULT_OK;
	}
	
	return RESULT_ERR;
}


inline result_t IPCMQ_init_and_recv(struct IpcMessageQueue* queue ,IpcObject_t* __obj)
{
    DBusMessage* msg;

    msg = dbus_connection_pop_message( __obj );  
    if( !msg )
        return RESULT_ERR;

    if( DBUS_MESSAGE_TYPE_SIGNAL != dbus_message_get_type(msg) ){
        dbus_message_unref(msg);  
        return RESULT_ERR;
    }
    
    if( FALSE == dbus_message_iter_init( msg ,  &queue->it ) ){
    	dbus_message_unref( msg );
    	return RESULT_ERR;
    }

	queue->m = msg;
	queue->cnt = 0;
    return RESULT_OK;
}


inline bool_t IPCMQ_get_message(struct IpcMessageQueue* queue , void** message , int* size)
{
    DBusMessageIter sub;

    if( DBUS_TYPE_ARRAY != dbus_message_iter_get_arg_type( & queue->it ) ){
        return B_FALSE;
    }
    
    dbus_message_iter_recurse( &queue->it , &sub );
    dbus_message_iter_get_fixed_array( &sub , message , size );
    dbus_message_iter_next( &queue->it );

    return B_TRUE;
}


inline void IPCMQ_free( struct  IpcMessageQueue* queue )
{
	dbus_message_unref( queue->m );
	queue->m = NULL;
	queue->cnt = 0;
}


inline void IPCMQ_init( struct  IpcMessageQueue* queue )
{
	queue->m = NULL;
	queue->cnt = 0;
}


inline bool_t IPCMQ_is_valid( struct  IpcMessageQueue* queue )
{
	return (bool_t)((queue->m) != NULL);
}


int IPCMQ_dispatch( IpcObject_t* __obj , IPCMSG_handler_t __handler, int __timeout )
{
	int ret;						/*处理了多少个消息*/
	unsigned int* msg_buffer_ptr;	/*指向msg消息的缓冲区指针*/
	int msg_buffer_size;			/*指向msg消息的缓冲区字节大小*/
	struct IpcMessageQueue queue;		/*消息结构体*/
	
	ret = 0;
	
	//先去尝试读取dbus缓冲区中的数据
	if( 0 != __timeout ){
	
		/*如果有数据则直接处理*/
		if( RESULT_OK == IPCMQ_init_and_recv(  &queue , __obj )  ){
			goto __label_get_message_loop;
		}
	
		/*没有收到数据则超时等待，在等待后的第二次接收*/
	    if( FALSE == dbus_connection_read_write( __obj , __timeout ) )
			return -1;
	}
	
	/*没有接收到数据则返回*/
	if( RESULT_OK != IPCMQ_init_and_recv(  &queue , __obj )  )
		return -1;
	
__label_get_message_loop:		
	while( B_TRUE == IPCMQ_get_message( &queue , (void**)&msg_buffer_ptr , &msg_buffer_size ) ){
		/*调用传入的msg处理函数*/
		__handler(  
			__obj ,										/*ipc dbus 通信对象*/    
			msg_buffer_ptr[0] ,								/*msgid*/
			&(msg_buffer_ptr[1]) ,							/*除去msgid后的其他数据*/
			msg_buffer_size - sizeof(msg_buffer_ptr[0])		/*除去msgid后的其他数据的大小，以字节为单位*/ 
			);
			
		ret++;/*处理的消息数量++*/
	}
	
	/*释放这个消息*/
	IPCMQ_free( &queue );
	
	return ret;
}
