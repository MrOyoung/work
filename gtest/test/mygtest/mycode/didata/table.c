#include "table.h"
#include "stdio.h"
#include "util.h"
#include "Table_Def.h"
#include "flash_timer.h"
#include "common.h"
#include "debug.h"
#include "Module_Def.h"
    

#define INVAILED_VAULE 0xffffffff



#define DEBUG_POSITION() LOG("%s line:%d\n", __FUNCTION__ , __LINE__ );


unsigned int get_data( byte* frame , struct DataAttr* entry )
{
    unsigned char* p;
    unsigned int data,value;

    p = frame + entry->byteindex;
    data = (p[0])|(p[1]<<8)|(p[2]<<16)|(p[3]<<24);
    value = (data >> entry->bitshift) & entry->bitmask;

	/*最大值最小值处理*/
	if( unlikely( value > entry->max ) )
		return entry->max;
		
	
	if( unlikely( value < entry->min ) )
		return INVAILED_VAULE;	/*0xffffffff是无效值*/
	
	/*返回正常值*/
    return value;
}

int send_to_modules( const char** paths , int nr , struct SignalData* data, int size , DBusConnection* dbus )
{
	int i = 0;
	
	do{
		dbus_send( dbus , 
				(void*)data, size, 
				paths[i] , /*目标模块路径*/
				MODULE_SERVICE_DI_DATA_ANALISIS_INTERFACE /*本模块的接口*/   );
				
		i++;
	}while( unlikely( i < nr ) );/*大多数情况下数据只会发送给一个模块*/
	
	
	
	return 0;
}

int do_flash_data_msg_entry( DBusConnection* dbus,struct MsgAttr* msg,byte* frame )
{
	int flashing;/*是否正在闪烁*/
	struct FlashAttr* f;/*闪烁信息的数组*/
	struct SignalData sgl;/*需发送的信号数据结构*/
    sgl.i[0] = get_data( frame , msg->attrs );

    /*当前值和之前的值相同*/
    if( sgl.i[0] == (msg->attrs[0].previous) )
        return 0;
   
    /*调试打印*/
    LOG("%s\nprevious:%d----current:%d\n" , msg->name , msg->attrs[0].previous, sgl.i[0] );

	f = msg->attrs[0].flash;
	/*检测之前是否已经开始闪烁*/
	flashing = f[(msg->attrs[0].previous)].oper;
	
	/*保存当前的数值*/
    msg->attrs[0].previous = sgl.i[0];
    sgl.msgid = msg->msgid;
    
    /*先停止当前的闪烁*/
    if( (NULL != msg->timer) || (Ops_On == flashing) ){
    	LOG("------------------end flash------------------------------\n");
        flash_timer_cancel( msg );
    }
    
    /*执行本次的闪烁操作*/
    if( Ops_On == f[(sgl.i[0])].oper ){
    	LOG("------------------begin flash----------------------------\n");
        flash_timer_create( msg , dbus , (sgl.i[0]) ); 
        return 1;
    }
        
    /*消息发送*/
    send_to_modules( msg->target , msg->target_cnt, &sgl , sizeof(unsigned int)+sizeof(unsigned int) , dbus );
	return 1;
}

int do_single_data_msg_entry(DBusConnection* dbus,struct MsgAttr* msg,byte* frame )
{
    struct SignalData sgl;/*需发送的信号数据结构*/

    sgl.i[0] = get_data( frame , msg->attrs );
    
    /*当前值和之前的值相同*/
    if( sgl.i[0] == (msg->attrs[0].previous) )
        return 0;
    
    LOG("%s\nprevious:%d----current:%d\n" , msg->name , msg->attrs[0].previous, sgl.i[0] );

	/*保存当前的数值*/
    msg->attrs[0].previous = sgl.i[0];
    sgl.msgid = msg->msgid;
    

    /*消息发送*/
    send_to_modules( msg->target , msg->target_cnt, &sgl , sizeof(unsigned int) + sizeof(unsigned int) , dbus );
    
	return 1;
}

#ifdef _DEBUG_
void debug_log_mult_data( const char* name , int count , struct SignalData* dbug , struct SignalData* sgl )
{
	switch( count ){
		case 6:LOG("%s\n"\
					   "previous: %hd - %hd - %hd - %hd - %hd - %hd\n"\
					   "current:  %hd - %hd - %hd - %hd - %hd - %hd\n",
					   name , 
					   dbug->s[0] , dbug->s[1] , dbug->s[2] , dbug->s[3] , dbug->s[4] , dbug->s[5],
					   sgl->s[0] , sgl->s[1] , sgl->s[2] , sgl->s[3] , sgl->s[4] , sgl->s[5]);
			   break;
		case 5:LOG("%s\n"\
					   "previous: %hd - %hd - %hd - %hd - %hd\n"\
					   "current:  %hd - %hd - %hd - %hd - %hd\n",
					   name , 
					   dbug->s[0] , dbug->s[1] , dbug->s[2] , dbug->s[3] , dbug->s[4] ,
					   sgl->s[0] , sgl->s[1] , sgl->s[2] , sgl->s[3] , sgl->s[4] );
			   break;
		case 4:LOG("%s\n"\
					   "previous: %hd - %hd - %hd - %hd\n"\
					   "current:  %hd - %hd - %hd - %hd\n",
					   name , 
					   dbug->s[0] , dbug->s[1] , dbug->s[2] , dbug->s[3],
					   sgl->s[0] , sgl->s[1] , sgl->s[2] , sgl->s[3] );
			   break;
		case 3:LOG("%s\n"\
					   "previous: %hd - %hd - %hd\n"\
					   "current:  %hd - %hd - %hd\n",
					   name , 
					   dbug->s[0] , dbug->s[1] , dbug->s[2],
					   sgl->s[0] , sgl->s[1] , sgl->s[2]);
			   break;
		case 2:LOG("%s\n"\
					   "previous: %hd - %hd\n"\
					   "current:  %hd - %hd\n",
					   name , 
					   dbug->s[0] , dbug->s[1] ,
					   sgl->s[0] , sgl->s[1] );
			   break;
	}
}
#else
#define debug_log_mult_data(...)
#endif

int do_multiple_data_msg_entry(DBusConnection* dbus, struct MsgAttr* msg,byte* frame )
{
    int i;
    struct SignalData sgl;/*需发送的信号数据结构*/
    int sglsize;/*需发送的信号数据结构提多少字节是有效的*/
    int value_change;/*当期的数据和之前的数据比较是否有变化*/

    _DEBUG_BEGIN_(
            struct SignalData dbug;
            )_DEBUG_END_;

    value_change = 0;
    /*需要发送多个数据，则以多个unsigned short发送*/
    for( i = 0 ;i < msg->attr_cnt ; i++ ){
        sgl.s[i] = (unsigned short)get_data( frame , &(msg->attrs[i] ) );

        _DEBUG_BEGIN_(
                dbug.s[i] = (unsigned short)msg->attrs[i].previous;
                )_DEBUG_END_;

        if( (unsigned int)(sgl.s[i]) != (msg->attrs[i].previous) ){
            value_change = 1;
            msg->attrs[i].previous = sgl.s[i];
        }
    }

    if( 0 == value_change )
        return 0;

    debug_log_mult_data( msg->name , msg->attr_cnt , &dbug , &sgl );

    /*需发送多个unsigned short型变量*/
    sglsize = sizeof(unsigned int) + (msg->attr_cnt) * sizeof(unsigned short);

    sgl.msgid = msg->msgid;
    send_to_modules( msg->target , msg->target_cnt, &sgl , sglsize , dbus );
	return 1;
}


int do_multiple_data_msg_entry_with_mask( 
									DBusConnection* dbus , 
									struct MsgAttr* msg ,
									byte* frame , 
									int mask ,
									void (*addit_op)(DBusConnection*,struct MsgAttr* ,struct SignalData*)
									 )
{
    int i;
    struct SignalData sgl;/*需发送的信号数据结构*/
    int sglsize;/*需发送的信号数据结构提多少字节是有效的*/
    int value_change;/*当期的数据和之前的数据比较是否有变化*/

    _DEBUG_BEGIN_(
            struct SignalData dbug;
            )_DEBUG_END_;

    value_change = 0;
    /*需要发送多个数据，则以多个unsigned short发送*/
    for( i = 0 ;i < msg->attr_cnt ; i++ ){
        sgl.s[i] = mask & (unsigned short)get_data( frame , &(msg->attrs[i] ) );

        _DEBUG_BEGIN_(
                dbug.s[i] = (unsigned short)msg->attrs[i].previous;
                )_DEBUG_END_;

        if( (unsigned int)(sgl.s[i]) != (msg->attrs[i].previous) ){
            value_change = 1;
            msg->attrs[i].previous = sgl.s[i];
        }
    }

    if( 0 == value_change )
        return 0;

    
	debug_log_mult_data( msg->name , msg->attr_cnt , &dbug , &sgl );

    /*需发送多个unsigned short型变量*/
    sglsize = sizeof(unsigned int) + (msg->attr_cnt) * sizeof(unsigned short);
    sgl.msgid = msg->msgid;
    
    /*附加操作*/
    if( addit_op )
    	addit_op( dbus , msg , &sgl );
    
    send_to_modules( msg->target , msg->target_cnt, &sgl , sglsize , dbus );
	return 1;
}


void do_table(DBusConnection* dbus, struct MsgAttr* Msgtable , int table_size ,byte* frame )
{
    int i;
    for( i = 0; likely( i < table_size ) ; i++)
        Msgtable[i].MsgFunc( dbus , &Msgtable[i], frame );
        
}

void popup_reset( struct MsgAttr* m )
{
	int i;
	for( i = 0 ;i < m->attr_cnt ; i++ ){
		if( m->attrs[i].previous != 0 )
			m->attrs[i].previous = 0xfafafafa;
    }
   
}

void lamp_reset( struct MsgAttr* m )
{
	int  i = m->attrs[0].previous;
	int flashing = m->attrs[0].flash[i].oper;
	
	if( Ops_Off == flashing ){/*如果是闪烁的话则更本不需要重置*/
			m->attrs[0].previous = (m->attrs[0].max+1);
	}
}


extern void register_reset_func( struct MsgAttr* m );
extern void register_speed_pwr_doors_func(struct MsgAttr* m);
extern void register_adas_msg_func( struct MsgAttr* m );
extern void register_popup_msg_func( struct MsgAttr* m );

void init_msg_table_function( struct MsgAttr* Msgtable , int table_size )
{
	int i;
	for( i = 0; i < table_size ; i++  ){
	
		/*注册特殊处理重置消息*/
		register_reset_func( &Msgtable[i] );
		
		/*注册和车辆健康相关的函数*/
		///register_health_msg_func( &Msgtable[i] );
		
		/*注册速度，电源模式，车门相关的函数*/
		register_speed_pwr_doors_func( &Msgtable[i] );
		
		/*adas*/
		///register_adas_msg_func( &Msgtable[i] );
		
		register_popup_msg_func( &Msgtable[i] );
	
		/*这里是通用消息处理函数*/
		if( !(Msgtable[i].MsgFunc) ){
									/*这里的函数指针如果未初始化，则使用已经下三个函数进行初始化*/
									/*do_flash_data_msg_entry:处理闪烁消息*/
									/*do_single_data_msg_entry：处理消息子表中只有一项的消息*/
									/*do_multiple_data_msg_entry:处理消息子表中只有多项的消息*/
									/*或者在消息特殊处理函数也会在register_health_msg_func中会进行赋值*/
									/*如果不指定消息处理函数，则默认使用以上三种*/
			if( 1 == Msgtable[i].attr_cnt )
        		Msgtable[i].MsgFunc =  (Msgtable[i].attrs[0].flash) ? do_flash_data_msg_entry : do_single_data_msg_entry;
    		else
        		Msgtable[i].MsgFunc =  do_multiple_data_msg_entry;
		}
		
		/*通用复位函数*/
		if( !(Msgtable[i].reset) ){
        	Msgtable[i].reset =  (Msgtable[i].attrs[0].flash) ? lamp_reset : popup_reset;		
		}

			
	}
}

/* 在main函数之前执行，完成数组的初始化 */
__attribute((constructor)) void init_msg_func_handler()
{
	init_msg_table_function( GeneralMsgTable , general_size );
	init_msg_table_function( SpecialMsgTable , special_size );
}



