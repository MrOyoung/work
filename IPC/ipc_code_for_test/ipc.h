#ifndef _IPC_H_
#define _IPC_H_
#include <stdio.h>
#include <sys/time.h>
#include <dbus/dbus.h>  


/*
 *编译时所需要的额外路径
 *CFLAGS+=-I=/usr/include/dbus-1.0
 *CFLAGS+=-I=/usr/include/glib-2.0
 *CFLAGS+=-I=/usr/lib/dbus-1.0/include
 *CFLAGS+=-I=/usr/lib/glib-2.0/include
 */

/*
 *链接时所需要的额外库
 *LIBS:=-ldbus-glib-1 -ldbus-1 -lgobject-2.0 -lglib-2.0
 */

/*DBusConnection的别名，用于封装dbus*/
typedef DBusConnection IpcObject_t;
typedef DBusMessage    IpcMessage;
typedef DBusMessageIter IpcMessageIter;

/*布尔变量*/
typedef enum{
	B_FALSE = 0,
	B_TRUE = 1,
}bool_t;

/*函数返回值*/
typedef enum{
	RESULT_ERR = -1,
	RESULT_OK = 0,
}result_t;


/*封装用于dbus通信的结构体*/
struct IpcMessageQueue{
	/*dbus消息*/
	IpcMessage* 	m;
	/*dbus消息的迭代器*/
	IpcMessageIter it;
	/*记录当前该结构体已经添加了多少个消息*/
	int 			cnt;
};

typedef struct
{
	int msgid;
	struct timeval now_time;
}msg_unit;

/*
*
*构造一个用于IPCOBJ_connect第二个参数的字符串
* 参数path必须是一个常量字符串
* example:
*		MAKE_RULES_STR( "/com/saic/IpcUartData" )
*/
#define MAKE_RULES_STR( path ) ("type='signal',path='" path "'")



#ifdef __cplusplus
extern "C" {
#endif


/*===========================================================================*\
 * FUNCTION: IPCOBJ_connect
 *===========================================================================
 * PARAMETERS:
 * well_known_name 	- name
 * listen_rules   - 监听规则，可以使用MAKE_RULES_STR宏来定义
 *
 * RETURN VALUE:
 * 返回一个ipc通信对象，如果失败返回一个NULL
 *
 * --------------------------------------------------------------------------
 * ABSTRACT:
 * --------------------------------------------------------------------------
 * 该函数获取一个ipc通信对象,IPC通信时必须需要一个这样的对象
 * example:
 * IpcObject_t* ipc;
 * ipc = IPCOBJ_connect("com.saic.IpcUartData" , MAKE_RULES_STR("/com/saic/IpcUartData") );
\*===========================================================================*/
IpcObject_t* IPCOBJ_connect(const char* well_known_name , const char* listen_rules );


/*===========================================================================*\
 * FUNCTION: IPCOBJ_disconnect
 *===========================================================================
 * PARAMETERS:
 * obj 	- 一个有效的IpcObject_t对象的指针
 *
 *
 * RETURN VALUE:
 * 0  - disconnect sucessed
 * -1 - 参数为无效参数
 *
 * --------------------------------------------------------------------------
 * ABSTRACT:
 * --------------------------------------------------------------------------
 * 当ipc对象不再需要时，使用该函数关闭连接，并且销毁该对象所占据的内存空间。
 * example:
 * pcObject_t* ipc;
 * ipc = IPCOBJ_connect("com.saic.IpcUartData" , MAKE_RULES_STR("/com/saic/IpcUartData") );
 * //do something.....
 * IPCOBJ_disconnect( ipc ); //free ipc object
\*===========================================================================*/
void IPCOBJ_disconnect(IpcObject_t*  obj );


/*===========================================================================*\
 * FUNCTION: IPCMQ_init_for_send
 *===========================================================================
 * PARAMETERS:
 * queue 	- 一个没有使用过，或者一个使用过但是free过的IpcMessageQueue结构体指针
 * path   -   要发送至的路径
 * interface   - 发送的模块的接口
 *
 * RETURN VALUE:
 * R_OK  - 初始化成功
 * R_ERR - 初始化失败，没有足够的内存空间
 *
 * --------------------------------------------------------------------------
 * ABSTRACT:
 * --------------------------------------------------------------------------
 * 初始化一个IpcMessageQueue结构体，该结构体用于向path发送，在调用IPCMQ_put_message
 * 前，必须调用该函数用于初始化一个用于发送的消息队列结构体，之后便可以调用函数
 * IPCMQ_put_message用于添加消息。
 *
\*===========================================================================*/
result_t IPCMQ_init_for_send(struct IpcMessageQueue* queue , const char* path , const char* interface);



/*===========================================================================*\
 * FUNCTION: IPCMQ_size
 *===========================================================================
 * PARAMETERS:
 * queue 	- 一个为了发送而初始化的IpcMessageQueue指针
 *
 * RETURN VALUE:
 * 已经添加了多少个消息
 *
 * --------------------------------------------------------------------------
 * ABSTRACT:
 * --------------------------------------------------------------------------
 * 返回该消息队列中已经添加爱了多少个消息
 *
\*===========================================================================*/
int IPCMQ_size( struct IpcMessageQueue* queue );



/*===========================================================================*\
 * FUNCTION: IPCMQ_put_message
 *===========================================================================
 * PARAMETERS:
 * queue 	- IpcMessageQueue结构体指针
 * message   - 消息数据的首地址
 * size   - 消息的大小，以字节为单位
 *
 * RETURN VALUE:
 * R_OK  - 添加消息成功
 * R_ERR - 添加消息失败，没有足够的内存空间
 *
 * --------------------------------------------------------------------------
 * ABSTRACT:
 * --------------------------------------------------------------------------
 * 将消息数据添加到消息队列中，该消息队列必须之前调用过IPCMQ_init_for_send函数，
 * 否则后果未定义。
\*===========================================================================*/
result_t IPCMQ_put_message( struct IpcMessageQueue* queue, const void* message , int size );


/*===========================================================================*\
 * FUNCTION: IPCMQ_send
 *===========================================================================
 * PARAMETERS:
 * queue 	- IpcMessageQueue结构体指针
 * __obj   - 一个有效的IpcObject_t对象的指针
 *
 * RETURN VALUE:
 * R_OK  - send sucessed
 * R_ERR - send failed
 *
 * --------------------------------------------------------------------------
 * ABSTRACT:
 * --------------------------------------------------------------------------
 * 将消息队列queue，通过__obj对象发送出去，该queue指向的结构体必须调用函数 
 * IPCMQ_init_for_send初始化过，并且调用函数IPCMQ_put_message添加了消息数据。
 *
\*===========================================================================*/
result_t IPCMQ_send( struct IpcMessageQueue* queue , IpcObject_t* __obj );


/*===========================================================================*\
 * FUNCTION: IPCMQ_get_message
 *===========================================================================
 * PARAMETERS:
 * queue - 一个接收的消息队列指针
 * message - 一个指向消息缓冲区的指针的地址，该指针使用完毕后无需释放，在下次调用函数前为有效
 * size - 获取的消息缓冲区大小
 *
 * RETURN VALUE:
 * B_TRUE  - 获取一个消息
 * B_FALSE -  没有消息获取到
 *
 * --------------------------------------------------------------------------
 * ABSTRACT:
 * --------------------------------------------------------------------------
 * 
\*===========================================================================*/
bool_t IPCMQ_get_message(struct IpcMessageQueue* queue , void** message , int* size);



/*===========================================================================*\
 * FUNCTION: IPCMQ_init_and_recv
 *===========================================================================
 * PARAMETERS:
 * queue - 
 * __obj -
 * RETURN VALUE:
 * 
 *
 * --------------------------------------------------------------------------
 * ABSTRACT:
 * --------------------------------------------------------------------------
 * 
\*===========================================================================*/
result_t IPCMQ_init_and_recv(struct IpcMessageQueue* queue ,IpcObject_t* __obj);


/*===========================================================================*\
 * FUNCTION: IPCMQ_free
 *===========================================================================
 * PARAMETERS:
 * queue 	- IpcMessageQueue结构体指针
 *
 * RETURN VALUE:
 * 0  - free sucessed
 * -1 - free failed
 *
 * --------------------------------------------------------------------------
 * ABSTRACT:
 * --------------------------------------------------------------------------
 * 该函数用以释放调用过IPCMQ_init_and_recv，IPCMQ_init_for_send这两个函数的
 * IpcMessageQueue结构体.
 * 共有两种情况必须调用该函数：
 * 1.调用完IPCMQ_init_and_recv，然后读取其中的数据之后,必须调用该函数以释放空间。
 * 2.调用完IPCMQ_init_for_send ，将该消息队列发送出去后，必须调用该函数以释放空间。
 * If the data does not arrive, it returns immediately
\*===========================================================================*/
void IPCMQ_free(struct IpcMessageQueue* queue );


/*===========================================================================*\
 * FUNCTION: IPCMQ_init
 *===========================================================================
 * PARAMETERS:
 * queue 	- IpcMessageQueue结构体指针
 *
 * RETURN VALUE:
 * 0  - free sucessed
 * -1 - free failed
 *
 * --------------------------------------------------------------------------
 * ABSTRACT:
 * --------------------------------------------------------------------------
 * 初始化一个消息队列，在定义了一个消息队列之后，请必须调用这种带有init相关的函数
\*===========================================================================*/
void IPCMQ_init(struct IpcMessageQueue* queue );


/*===========================================================================*\
 * FUNCTION: IPCMQ_is_vaild
 *===========================================================================
 * PARAMETERS:
 * queue 	- IpcMessageQueue结构体指针
 *
 * RETURN VALUE:
 * 
 *
 * --------------------------------------------------------------------------
 * ABSTRACT:
 * --------------------------------------------------------------------------
 * 返回一个消息队列是否有效
\*===========================================================================*/
bool_t  IPCMQ_is_vaild( struct IpcMessageQueue* queue );


typedef void (*IPCMSG_handler_t)( IpcObject_t* __obj ,unsigned int __msgid , void* __data , int __size );

/*===========================================================================*\
 * FUNCTION: IPCMQ_dispatch
 *===========================================================================
 * PARAMETERS:
 * __obj 	- Must be a valid pointer to the IPC object
 * __handler - 当有消息到来就会调用这个函数
 * __timeout - 等待消息的超时时间以毫秒为单位
 * RETURN VALUE:
 * 处理的消息的个数
 *
 * --------------------------------------------------------------------------
 * ABSTRACT:
 * --------------------------------------------------------------------------
 * 
\*===========================================================================*/
int IPCMQ_dispatch( IpcObject_t* __obj , IPCMSG_handler_t __handler, int __timeout );


#ifdef __cplusplus
}
#endif

#endif
