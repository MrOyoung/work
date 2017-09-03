#include "Message_Def.h"
#include "Table_Def.h"
#include "debug.h"
#include "util.h"

_DEBUG_BEGIN_(
	extern int catch_msg_id;
)_DEBUG_END_

unsigned int get_data( byte* frame , struct DataAttr* entry );
int send_to_modules( const char** paths , int nr , struct SignalData* data, int size , DBusConnection* dbus );

enum{
	COLOR_GRAY,
	COLOR_GREEN,
	COLOR_YELLOW,
	COLOR_RED,
};


int do_TIME_DISTANCE_POSITION_COLOR_msg_entry(DBusConnection* dbus, struct MsgAttr* msg,byte* frame )
{
    struct SignalData sgl;/*需发送的信号数据结构*/
    
   	unsigned int i = get_data( frame , &(msg->attrs[0] ) );
   	
   	int color = COLOR_GRAY;
   	int cars = 0;
   	
   	if( i & (0x3 << 6) ){
   		color = (i>>6) & 0x3;
   		cars = 4;
   	}else if( i & (0x3 << 4) ){
   		color = (i>>4) & 0x3;
   		cars = 3;
   	}else if( i & (0x3 << 2) ){
   		color = (i>>2) & 0x3;
   		cars = 2;
   	}else if( i & (0x3 << 0) ){
   		color = (i>>0) & 0x3;
   		cars = 1;
   	}

	int temp = (color << 8) | cars;	
    if( temp == msg->attrs[0].previous )
        return 0;

    sgl.i[0]  = temp;
    sgl.msgid = msg->msgid;    

     /*调试打印*/
    LOG("%s\n orgin:%x --- car:%d --- color:%d --- previous:%x----current:%x\n" , 
            	msg->name , 
            	i ,
            	cars,
            	color, 
            	msg->attrs[0].previous, 
            	sgl.i[0] 
            );
   
        
     msg->attrs[0].previous  =  temp;
         
     /*消息发送*/
    send_to_modules( msg->target , msg->target_cnt, &sgl , sizeof(unsigned int) + sizeof(unsigned int) , dbus );
	return 1;
}



int do_DRIVER_SELECT_TARGET_DISTANCE_LEVEL_msg_entry(DBusConnection* dbus, struct MsgAttr* msg,byte* frame )
{
	struct SignalData sgl;/*需发送的信号数据结构*/
    
   	unsigned int low8bit = get_data( frame , &(msg->attrs[0] ) );
   	unsigned int high8bit = get_data( frame , &(msg->attrs[1]) );
   	

    sgl.i[0]  = (high8bit << 8)|low8bit;
    sgl.msgid = msg->msgid;    


	if( msg->attrs[0].previous ==  sgl.i[0] )
		return 0;
	
     /*调试打印*/
     LOG("%s\n previous:%x----current:%x\n" , 
            	msg->name , 
            	msg->attrs[0].previous, 
            	sgl.i[0] 
            );
        
     msg->attrs[0].previous  = sgl.i[0];
         
     /*消息发送*/
    
    send_to_modules( msg->target , msg->target_cnt, &sgl , sizeof(unsigned int) + sizeof(unsigned int) , dbus );
	return 1;
}


struct ADAS_HANDLER{
	unsigned int msg;
	MsgFunctionPtr func;
};


static struct ADAS_HANDLER handlers[] = {
	{ SERVICE_DI_TIME_DISTANCE_POSITION_COLOR , do_TIME_DISTANCE_POSITION_COLOR_msg_entry },
	{ SERVICE_DI_DRIVER_SELECT_TARGET_DISTANCE_LEVEL , do_DRIVER_SELECT_TARGET_DISTANCE_LEVEL_msg_entry }
}; 




void register_adas_msg_func( struct MsgAttr* m )
{
	int i;
	for( i = 0 ; i < array_size(handlers); i++ ){
		if(  (NULL == m->MsgFunc) && (m->msgid == handlers[i].msg) )
			m->MsgFunc = handlers[i].func;	
	}	
}





