#include "util.h"
#include "Message_Def.h"
#include "Table_Def.h"
#include "Module_Def.h"
#include "debug.h"
#include "timer.h"
#include "popup.h"

#include "shm.h"

#include <math.h>


static int runMode = 0;
static int powerMode = 0;
static DBusConnection* gdbus_ptr;



/*function in table.c*/
unsigned int get_data( byte* frame , struct DataAttr* entry );
int send_to_modules( const char** paths , int nr , struct SignalData* data, int size , DBusConnection* dbus );
int do_single_data_msg_entry(DBusConnection* dbus,struct MsgAttr* msg,byte* frame );

//////int do_multiple_data_msg_entry_with_mask(DBusConnection* dbus, struct MsgAttr* msg,byte* frame , int mask );

int do_multiple_data_msg_entry_with_mask( 
									DBusConnection* dbus , 
									struct MsgAttr* msg ,
									byte* frame , 
									int mask ,
									void (*addit_op)(DBusConnection*,struct MsgAttr* ,struct SignalData*)
									 );
/*end*/


void popup_addition_op( DBusConnection*dbus, struct MsgAttr* table , struct SignalData* sgl );


/*power off*/
int do_doors_with_power_off_msg_entry(DBusConnection* dbus,struct MsgAttr* msg, byte* frame)
{	
	/*(!powerMode) &&*/  
	return do_multiple_data_msg_entry_with_mask(dbus, msg,frame , (!powerMode) , popup_addition_op );
}

/*power on and car is run*/
int do_doors_with_car_run_msg_entry(DBusConnection* dbus,struct MsgAttr* msg, byte* frame)
{
	/*powerMode && runMode &&*/
	return do_multiple_data_msg_entry_with_mask(dbus,  msg, frame , (powerMode && runMode) , popup_addition_op );	
}

/*power on and car is stop*/
int do_doors_with_car_stop_msg_entry(DBusConnection* dbus,struct MsgAttr* msg, byte* frame )
{
	/*powerMode && !runMode &&*/
	return do_multiple_data_msg_entry_with_mask(dbus, msg, frame , (powerMode && !runMode) , popup_addition_op );
}



int do_speed_msg_entry(DBusConnection* dbus,struct MsgAttr* msg, byte* frame)
{
	int ret =  do_single_data_msg_entry( dbus, msg, frame );
	runMode =  (msg->attrs[0].previous) > 0;
	return ret;
}



int do_power_mode_msg_entry(DBusConnection* dbus,struct MsgAttr* msg, byte* frame)
{
	int ret =  do_single_data_msg_entry( dbus, msg, frame );
	powerMode =  (msg->attrs[0].previous) > 1;
	popup_set_power_mode( powerMode );
	return ret;
}

#define INIT_MSG_FUNC( msg , id , func ) \
	if( ( id == msg->msgid) ){  msg->MsgFunc = func ; return; }

    
struct damp_data{
	float target;
	float current;
	float factor;
	float delta;
	
	/*pointer to share memory*/
	volatile gint* value;
};


#define SPEED_DAMPING_FACTOR (float)(20)
#define TACH_DAMPING_FACTOR (float)(20)

struct damp_data speed_damp_data = {
	.factor = SPEED_DAMPING_FACTOR,
	.delta =  1.0f/*SPEED_DAMPING_FACTOR*/,
};
    
struct damp_data tach_damp_data = {
	.factor = TACH_DAMPING_FACTOR,
	.delta =  10.0f/*TACH_DAMPING_FACTOR*/,
};

unsigned long damp_effect_handler(void * data1, void* data2 , unsigned long now)
{
	unsigned long next_timeout;
	float			  diff;
	unsigned int  msg_buf[2];

	struct damp_data* damp = (struct damp_data*)data1;
	struct MsgAttr*   m    = (struct MsgAttr*)data2;

	diff = damp->target - damp->current;
		
	if( fabs( diff ) < damp->delta  ){
		damp->current = damp->target;
		next_timeout  = 0;
	}else{
		damp->current += (diff / damp->factor);
		next_timeout  = 20;
	}

	/*send message begin*/
	msg_buf[0] = m->msgid;
	msg_buf[1] = (unsigned int)(damp->current + 0.5f);

	/*share mem set*/
	g_atomic_int_set( damp->value , msg_buf[1] );
	
	/*send data*/
	send_to_modules( 
			m->target , 
			m->target_cnt, 
			(struct SignalData*)msg_buf , 
			sizeof(msg_buf) , 
			gdbus_ptr );

	/*send message end*/

	return next_timeout;
}

void damp_effect_cleanup(void* data1, void* data2)
{
	struct MsgAttr*   m    = (struct MsgAttr*)data2;
	m->timer = NULL;
}




static inline int do_damp_effect_msg_entry (DBusConnection* dbus, struct MsgAttr* msg , byte* frame , struct damp_data* damp )
{
	unsigned int d;
	unsigned long next;
	
	/*parse value*/
	d = get_data( frame , msg->attrs );
	if( d ==  (msg->attrs[0].previous) ){
		return 0;
	}

	/*update previous vaule*/
	msg->attrs[0].previous = d;
	/*update target vaule*/
	damp->target = 	  (float)d;
	
	
	if( msg->timer ){
		/*do nothing ,all work in timer handler*/
		return 1;
	}

	gdbus_ptr	 =     dbus;	
	
	/*call handler */
	next = damp_effect_handler( (void*)damp , (void*)msg , 0 );
	
	/*if next > 0 , need more timer handler*/
	if( next > 0 ){
		msg->timer = TIMER_register(
				damp_effect_handler ,
				next , 
				(void*)damp, 
				(void*)msg , 
				damp_effect_cleanup 
			);
	}

	return 1;
}

int do_speed_damp_effect_msg_entry(DBusConnection* dbus,struct MsgAttr* msg,byte* frame )
{
	int __r = do_damp_effect_msg_entry( dbus , msg , frame , &speed_damp_data );
	runMode = (msg->attrs[0].previous) > 0;
	
	return __r;
}

int do_tach_damp_effect_msg_entry(DBusConnection* dbus, struct MsgAttr* msg,byte* frame )
{
	return do_damp_effect_msg_entry( dbus , msg, frame , &tach_damp_data );
}


static struct service_shared_mem* g_shm_mem = NULL;


void register_speed_pwr_doors_func(struct MsgAttr* msg)
{
	if( NULL == g_shm_mem ){
		LOG("open share mem\n");
		g_shm_mem = (struct service_shared_mem*)open_share_mem();
		
		if( g_shm_mem ){

			LOG("share mem open ok");
			
			g_atomic_int_set( &(g_shm_mem->speed) , 0 );
			g_atomic_int_set( &(g_shm_mem->tach)  , 0 );

			speed_damp_data.value = &(g_shm_mem->speed);
			tach_damp_data.value  = &(g_shm_mem->tach);
		}else{
			LOG("open shm error!!!\n");
		}
	}


	INIT_MSG_FUNC(msg , SERVICE_DI_SPEED, do_speed_damp_effect_msg_entry );
	INIT_MSG_FUNC(msg , SERVICE_DI_TACHOMETER, do_tach_damp_effect_msg_entry );
	
	
	INIT_MSG_FUNC(msg , SERVICE_DI_POWER_MODE , do_power_mode_msg_entry );

	INIT_MSG_FUNC(msg , SERVICE_DI_DOOR_LF_OPEN_RUN , do_doors_with_car_run_msg_entry );
	INIT_MSG_FUNC(msg , SERVICE_DI_DOOR_LF_OPEN_STOP , do_doors_with_car_stop_msg_entry );
	INIT_MSG_FUNC(msg , SERVICE_DI_DOOR_LF_OPEN_POWEROFF , do_doors_with_power_off_msg_entry );

	INIT_MSG_FUNC(msg , SERVICE_DI_DOOR_RF_OPEN_RUN , do_doors_with_car_run_msg_entry );
	INIT_MSG_FUNC(msg , SERVICE_DI_DOOR_RF_OPEN_STOP , do_doors_with_car_stop_msg_entry );
	INIT_MSG_FUNC(msg , SERVICE_DI_DOOR_RF_OPEN_POWEROFF , do_doors_with_power_off_msg_entry );

	INIT_MSG_FUNC(msg , SERVICE_DI_DOOR_LB_OPEN_RUN , do_doors_with_car_run_msg_entry );
	INIT_MSG_FUNC(msg , SERVICE_DI_DOOR_LB_OPEN_STOP , do_doors_with_car_stop_msg_entry );
	INIT_MSG_FUNC(msg , SERVICE_DI_DOOR_LB_OPEN_POWEROFF , do_doors_with_power_off_msg_entry );

	INIT_MSG_FUNC(msg , SERVICE_DI_DOOR_RB_OPEN_RUN , do_doors_with_car_run_msg_entry );
	INIT_MSG_FUNC(msg , SERVICE_DI_DOOR_RB_OPEN_STOP , do_doors_with_car_stop_msg_entry );
	INIT_MSG_FUNC(msg , SERVICE_DI_DOOR_RB_OPEN_POWEROFF , do_doors_with_power_off_msg_entry );

	INIT_MSG_FUNC(msg , SERVICE_DI_DOOR_EC_OPEN_RUN , do_doors_with_car_run_msg_entry );
	INIT_MSG_FUNC(msg , SERVICE_DI_DOOR_EC_OPEN_STOP , do_doors_with_car_stop_msg_entry );
	INIT_MSG_FUNC(msg , SERVICE_DI_DOOR_EC_OPEN_POWEROFF , do_doors_with_power_off_msg_entry );

	INIT_MSG_FUNC(msg , SERVICE_DI_DOOR_T_OPEN_RUN , do_doors_with_car_run_msg_entry );
	INIT_MSG_FUNC(msg , SERVICE_DI_DOOR_T_OPEN_STOP , do_doors_with_car_stop_msg_entry );
	INIT_MSG_FUNC(msg , SERVICE_DI_DOOR_T_OPEN_POWEROFF , do_doors_with_power_off_msg_entry );
}
