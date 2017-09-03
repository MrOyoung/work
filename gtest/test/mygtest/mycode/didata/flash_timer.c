#include "timer.h"
#include "util.h"
#include <time.h>
#include "flash_timer.h"
#include <stdio.h>
#include "util.h"
#include "debug.h"
#include "Module_Def.h"
#include "pool_template.h"




struct flash_timer{
	/*发送的消息id*/
    int msgid;
    
    /*消息的名字*/
    const char* name;
    
    /*重复次数*/
    int repeat_cnt;
    
    /*当期的状态*/
    int current;
    
    unsigned int states[2];
    unsigned int ellapse[2];

	/*目的地和源*/
    const char* dst;
    const char* src;
    DBusConnection* dbus;
};

POOL_TEMPLATE( flash_timer  , 64 , flash_timer_alloc , flash_timer_free );



unsigned long flash_timer_handler(void* data1 ,void* data2, unsigned long now)
{
	unsigned long next_time;
	struct flash_timer* tmr;
	struct MsgAttr*     ma;

	tmr = (struct flash_timer*)data1;
	ma  = (struct MsgAttr*)data2;

	/*message needed to send*/
	unsigned int msg[2] = {
		(unsigned int)tmr->msgid ,
		(unsigned int)tmr->states[tmr->current]
	};

	/*send message*/
	LOG("send flash:%s---%x\n" , tmr->name ,  msg[1] );
	dbus_send( tmr->dbus , msg , sizeof(msg) , tmr->dst , tmr->src ); 

	/*next timeout*/
	next_time = tmr->ellapse[tmr->current];

	/*if == -1, this timer is not a infinity timer*/
	if( -1 != tmr->repeat_cnt  ){
		tmr->repeat_cnt -= (tmr->current);

		/*if == 0 ,than this timer must kill it*/
		if( 0 == tmr->repeat_cnt )
			return 0;
	}

	/*update current timer state*/
	tmr->current ^= 1;

	/*return next timeout*/
	return next_time;
}

void flash_cleanup(void* data1 ,void* data2)
{
	struct flash_timer* tmr;
	struct MsgAttr*     ma;

	tmr = (struct flash_timer*)data1;
	ma  = (struct MsgAttr*)data2;

	flash_timer_free( tmr );
    ma->timer = NULL;
    LOG("cancel TIMER:%s\n", ma->name );
}


int flash_timer_cancel( struct MsgAttr* msg )
{
	LOG("begin call TIMER_unregister:%s ptr:%p\n", msg->name , msg->timer );

	if( msg->timer ){
		LOG("call TIMER_unregister:%s\n", msg->name );
		/*2.unregister timer*/
		TIMER_unregister( msg->timer );
		//msg->timer = NULL;
	
	}
}



int flash_timer_create( struct MsgAttr* msg , DBusConnection* dbus, int index )
{
	struct FlashAttr* flash = &(msg->attrs[0].flash[index]);
	struct flash_timer* p = flash_timer_alloc();
	
	p->msgid = msg->msgid;
    p->name  = msg->name;
    p->repeat_cnt = flash->repeat_cnt;
    p->current    = 0;
    
    p->states[0]  = flash->states[0];
    p->states[1]  = flash->states[1];
    
    p->ellapse[0] = flash->timeout[0];
    p->ellapse[1] = flash->timeout[1];
    
    p->dst = msg->target[0];
    p->src = MODULE_SERVICE_DI_DATA_ANALISIS_INTERFACE;
    p->dbus = dbus;
    
	msg->timer = TIMER_register(flash_timer_handler, /**/
                                   0,
                                   p,
                                   msg,
                                   flash_cleanup
                                   );
                                   
    return 0;
}

