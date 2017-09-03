#ifndef _TABLE_H_
#define _TABLE_H_

#include "Message_Def.h"
#include "Table_Def.h"
#include "common.h"
#include "util.h"


//遍历每个一个表格项
extern void do_table( DBusConnection* dbus,struct MsgAttr* table , int table_size ,byte* frame );

//遍历单个表格项
extern void do_msg_entry(DBusConnection* dbus,struct MsgAttr* msg, byte* frame);

/*特殊帧消息表*/
extern struct MsgAttr SpecialMsgTable[];

/*特殊帧消息表大小*/
extern const int special_size;

/*通用帧*/
extern struct MsgAttr GeneralMsgTable[];

extern const int general_size;


extern void register_health_msg_func( struct MsgAttr* msg);

extern void init_msg_table_function( struct MsgAttr* Msgtable , int table_size );




static inline void DIDATA_do_general_table(DBusConnection* dbus , byte* buffer )
{
	do_table( dbus , GeneralMsgTable , general_size , buffer );
}


static inline void DIDATA_do_special_table(DBusConnection* dbus , byte* buffer )
{
	do_table( dbus , SpecialMsgTable , special_size, buffer );
}
  
static inline unsigned long DIDATA_do_timer_loop()
{
	return TIMER_loop();
}





#endif
