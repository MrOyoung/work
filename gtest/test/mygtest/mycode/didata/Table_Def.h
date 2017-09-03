#ifndef _TABLE_DEF_H_
#define _TABLE_DEF_H_

#include <stdlib.h>
#include "common.h"
#include "util.h"
#include "timer.h"

enum{
	Ops_Off,/*先关闭定时器，然后透传*/
	Ops_On,/*只通过定时器传值*/
	Ops_Reset
};

struct FlashAttr{
	/*是否要执行的操作*/
	unsigned int oper;
	/*定时器重复次数*/
	int repeat_cnt;
	/*两种状态所要发送的数值*/
	unsigned int states[2];
	/*两种状态切换的间隔时间*/
	unsigned int timeout[2];
};

#define FLASH_OFF() {\
	.oper = Ops_Off,	\
	.repeat_cnt = 0,	\
	.states = {0,0},	\
	.timeout = {0,0}	\
}

#define FLASH_ON(r,state1,time1,state2,time2) {\
	.oper = Ops_On ,			\
	.repeat_cnt = r,			\
	.states = {state1,state2},	\
	.timeout = {time1,time2}	\
}

#define FLASH_DIR() \
	FLASH_OFF()
	
#define FLASH_RESET() {\
	.oper = Ops_Reset,	\
	.repeat_cnt = 0,	\
	.states = {0,0},	\
	.timeout = {0,0}	\
}


struct DataAttr {
	/*该数据所在数据帧的字节索引*/
	unsigned int byteindex;
	/*该数据有效开始位*/
	unsigned int bitshift;
	/*位掩码*/
	unsigned int bitmask;
	/*最小值*/
	unsigned int min;
	/*最大值*/
	unsigned int max;
	/*之前的数值*/
	unsigned int previous;
	/*处理闪烁*/
	struct FlashAttr* flash;
};

/*该宏内的变量字符串 之前有__双下划线前缀，只是为了避免与结构体内成员名字重名导致编译不通过*/
#define DATA_ENTRY_FLASH( __byteindex , __bitshift, __bitlength, __min, __max , __flash ) {\
	.byteindex = __byteindex,		\
	.bitshift = __bitshift,			\
	.bitmask = MASK_##__bitlength,	\
	.min = __min,					\
	.max = (__flash)?(array_size(__flash)-2):__max,\
	.previous = 0x00,				\
	.flash = __flash				\
}

#define DATA_ENTRY( __byteindex, __bitshift, __bitlength , __min, __max ) \
	DATA_ENTRY_FLASH( __byteindex , __bitshift, __bitlength, __min, __max , NULL )

/*前向申明*/
struct MsgAttr;

typedef int  (*MsgFunctionPtr)(DBusConnection* dbus, struct MsgAttr* table ,byte* frame );
typedef int  (*AbnormalFunctionPtr)(struct MsgAttr* msg);
typedef void (*MsgResetFunctionPtr)(struct MsgAttr* msg);


/*
typedef int  (*message_parse)(struct MsgAttr* table ,byte* frame);
typedef void (*message_dispatch)(struct MsgAttr* table,int change,DBusConnection* dbus);
typedef void (*message_addition)(struct MsgAttr* table,int change);
typedef void (*message_reset)(struct MsgAttr* table);
*/


struct MsgAttr{
	/*message id*/
	int msgid;
	/*message id 名字*/
	const char* name;
	/*该消息内数据子表大小*/
	int attr_cnt;
	/*数据子表*/
	struct DataAttr* attrs;
	/*要发往的目的模块的数量*/
	int target_cnt;
	/*要发往的目的模块的路径字符串数组*/
	const char** target;
	/*处理该消息的函数指针*/
	MsgFunctionPtr MsgFunc;
	/*判断该消息所包含的数据是否异常*/
	AbnormalFunctionPtr abnormal;
	/*特殊作用函数，与其他指针协同处理需求复杂的消息*/
	MsgFunctionPtr special;
	/*用于重置消息中的数值*/
	MsgResetFunctionPtr reset;
	/*定时器*/
	struct timer_entry* timer;
	
	/*
	message_parse 		parse;
	message_dispatch 	dispatch;
	message_addition 	addition;
	message_reset    	reset;
	void* 				pridata;
	*/
};

/*申明消息表项*/
#define MSG_ENTRY_EX( id, targets, attrs_array, func ) {\
	.msgid = id,						\
	.name = #id,						\
	.attr_cnt = array_size(attrs_array),\
	.attrs = attrs_array,				\
	.target_cnt = array_size(targets),	\
	.target =  targets,					\
	.MsgFunc = func,					\
	.special = NULL,					\
	.reset = NULL						\
}

#define MSG_ENTRY(id,targets,attrs_array) \
	MSG_ENTRY_EX(id,targets,attrs_array, NULL)

struct SignalData{
	int msgid;
	union{
		unsigned int i[3];
		unsigned short s[6];
	};
};

#define __DECLARE_DATA_TABLE( msgid ) \
	struct DataAttr msgid##table[]

#define __DECLARE_MSG_ENTRY( msgid , targets )  \
	MSG_ENTRY( msgid ,  targets , msgid##table )

#define __DECLARE_MSG_ENTRY_EX( msgid , targets ,func )  \
	MSG_ENTRY_EX( msgid ,  targets , msgid##table , func )
	
	
	
#define MSG_INIT_FUNC( __msg_init_func ) \
\
extern struct MsgAttr GeneralMsgTable[];\
extern const int general_size;\
extern struct MsgAttr SpecialMsgTable[];\
extern const int special_size;\
__attribute((constructor)) static void __MSG_INIT_FUNC##__msg_init_func()\
{\
	int i;\
	LOG("%s begin\n", __FUNCTION__ );\
	for( i = 0; i < general_size; i++ ){\
		__msg_init_func( &GeneralMsgTable[i] );\
	}\
	\
	for( i = 0; i < special_size; i++ ){\
		__msg_init_func( &SpecialMsgTable[i] );\
	}\
	LOG("%s end\n", __FUNCTION__ );\
}	\


#endif
