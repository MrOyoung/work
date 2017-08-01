#include "Message_Def.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#define ROUND(__v,__a) (((__v)+(__a-1))&(~(__a-1)))
#define UINT_BITS (32)

#define POPUP_POOL_SIZE 256

struct popup_entry{
	struct list_head node;
	unsigned int     messageid;
	unsigned int     data	  ;
};

#define to_popup_entry(__p) container_of(__p,struct popup_entry,node)
#define list_for_each_popup_entry(iter,head) list_for_each_entry(iter,head,node)

int get_messageid_priority(unsigned int);

static LIST_HEAD(popup_pool_head);
static struct popup_entry popup_entry_pool[POPUP_POOL_SIZE];

__attribute((constructor)) void __popup_entry_pool_init()
{
	int i;
	for(i = 0;i < POPUP_POOL_SIZE;i++ )
		list_add( &(popup_entry_pool[i].node) , &popup_pool_head );
}


static inline struct popup_entry* __popup_entry_alloc()
{	
	struct list_head* __p;
	if( list_empty(&popup_pool_head) )
		return NULL;
	
	__p = popup_pool_head.next;
	list_del(__p);
	
	return to_popup_entry(__p);
}

static inline void __popup_entry_free(struct popup_entry* __e)
{
	list_add(&(__e->node),&popup_pool_head);
}

/**/
static unsigned int popup_flags[ROUND(SERVICE_DI_MESSAGE_MAX&0xffff,32)/32] = {0};

static inline int is_this_popup_on(unsigned int messageid)
{
	int idx;
	int bit;

	idx = 0xffff & messageid;
	bit = 1 << (idx % UINT_BITS);

	return 0 != (popup_flags[ idx / UINT_BITS ] & bit);
}

static inline void set_this_popup_on(unsigned int messageid)
{
	int idx;
	int bit;

	idx = 0xffff & messageid;
	bit = 1 << (idx % UINT_BITS);

	popup_flags[ idx / UINT_BITS] |= bit;
}

static inline void set_this_popup_off(unsigned int messageid)
{
	int idx;
	int bit;

	idx = 0xffff & messageid;
	bit = 1 << (idx % UINT_BITS);

	popup_flags[ idx / UINT_BITS ] &= (~bit);
}

enum{
	POPUP_PRIORITY_LEVEL_NONE = 0,

	POPUP_PRIORITY_LEVEL_C,
	POPUP_PRIORITY_LEVEL_B,
	POPUP_PRIORITY_LEVEL_A,
};

#define POPUP_POWER_ON      (1<<4)
#define POWER(p)			(((p)>>4) & 0x01)
#define PRI(p)				(((p)&0xf)) 

/*
*	POPUP_PRIORITY_LEVEL_C 	[0]:|head|->[entry]->[entry].....
*   POPUP_PRIORITY_LEVEL_B  [1]:|head|->[entry]->[entry]->[entry]->[entry]->[entry].....
*	POPUP_PRIORITY_LEVEL_A	[2]:|head|->[entry]->[entry].....
*/

static struct list_head popup_power_on_hash[] = {
	LIST_HEAD_INIT( popup_power_on_hash[0]  ),
	LIST_HEAD_INIT( popup_power_on_hash[1]  ),
	LIST_HEAD_INIT( popup_power_on_hash[2]  )
};

static struct list_head popup_power_off_hash[] = {
	LIST_HEAD_INIT( popup_power_off_hash[0]  ),
	LIST_HEAD_INIT( popup_power_off_hash[1]  ),
	LIST_HEAD_INIT( popup_power_off_hash[2]  )
};

/*当前最高popup等级*/
static struct list_head* popup_hash[] = {
	popup_power_off_hash,
	popup_power_on_hash,
};

static int max_priority_popup[] = {
	POPUP_PRIORITY_LEVEL_NONE,
	POPUP_PRIORITY_LEVEL_NONE
};

static int power_mode = 0;

#define current_max_priority (max_priority_popup[power_mode])
#define current_hash         (popup_hash[power_mode])


void popup_set_power_mode( int pwr ) {
	power_mode = !!(pwr);
}

int popup_get_power_mode() {
	return power_mode;
}

unsigned int get_max_priority_popup_message( )
{
	struct list_head* p;
	
	if( 0 == current_max_priority )
		return 0;

	p = current_hash[current_max_priority - 1].next;
	
	return to_popup_entry(p)->messageid;
}

int popup_add_message(const unsigned int messageid, unsigned int data)
{
	int t;
	int pri;
	int pwr;
	
	struct popup_entry* entry;

	/*验证该popup是否当前正打开着*/
	if( is_this_popup_on(messageid) ){
		printf("this popup message is already on\n");
		return 0;
	}
	
	/*获取该popup的优先级*/
	t = get_messageid_priority( messageid );
	/*return 0,this message is not a popup!!!*/
	if(0 == t)
		return -1;

	/*获取一个记录的列表项*/
	entry = __popup_entry_alloc();
	if( NULL == entry )
		return -1;

	/*获取该消息为poweron吗？*/
	pwr  = POWER(t);
	/*获取优先级*/
	pri  = PRI(t);
	
	/*记录下message*/
	entry->messageid = messageid;
	entry->data      = data;

	///加入到相应的优先级链表中
	list_add( &(entry->node) , &((popup_hash[pwr])[pri - 1]) );

	/*设置该消息已经开启*/
	set_this_popup_on( messageid );

	/*更新最高优先级*/
	if( pri >  max_priority_popup[pwr] )
		max_priority_popup[pwr] = pri;

	return 0;
}

static inline void __update_current_max_priority( int pwr)
{
	int i =  max_priority_popup[pwr] - 1;

	while( (i >= 0) && list_empty(&((popup_hash[pwr])[i])) )
		i--;

	max_priority_popup[pwr] = i + 1;
}

int popup_remove_message(unsigned int messageid)
{
	int t;
	int pwr;
	int pri;
	struct popup_entry* it;
	struct list_head* list;

	/*如果这个messageid的标志没有开则不需要进行下面的操作*/
	if( !is_this_popup_on(messageid) ){
		printf("this message is alway off\n");
		return -1;
	}

	/*获取该popup的优先级*/
	t = get_messageid_priority( messageid );
	
	/*fetch power mode*/
	pwr = POWER(t);
	/*fetch message priority*/
	pri = PRI(t);

	list = &((popup_hash[pwr])[pri - 1]);
	
	list_for_each_popup_entry(it , list ){
		if(messageid == it->messageid){
			
			///设置该popup消息关闭
			set_this_popup_off( messageid );

			///释放这个列表项
			list_del( &(it->node) );
			__popup_entry_free( it  );

			///只有当要去除的消息为最高优先级的消息时，更新当前最高优先级
			if( pri == max_priority_popup[pwr] )
				__update_current_max_priority( pwr );

			break;
		}
	}

	return 0;
}

int get_messageid_priority(unsigned int messageid)
{
/*power on*/
#define POPUP_PRIORITY_ON_A(id) [id &0xffff] = POPUP_POWER_ON | POPUP_PRIORITY_LEVEL_A
#define POPUP_PRIORITY_ON_B(id) [id &0xffff] = POPUP_POWER_ON | POPUP_PRIORITY_LEVEL_B
#define POPUP_PRIORITY_ON_C(id) [id &0xffff] = POPUP_POWER_ON | POPUP_PRIORITY_LEVEL_C

/*power off*/
#define POPUP_PRIORITY_OFF_A(id) [id &0xffff] = POPUP_PRIORITY_LEVEL_A
#define POPUP_PRIORITY_OFF_B(id) [id &0xffff] = POPUP_PRIORITY_LEVEL_B
#define POPUP_PRIORITY_OFF_C(id) [id &0xffff] = POPUP_PRIORITY_LEVEL_C

/*priority byte array*/
	static unsigned char popup_priority_table[SERVICE_DI_MESSAGE_MAX&0xffff] = {
		//A class
		POPUP_PRIORITY_ON_A( SERVICE_DI_DOOR_LF	),
		POPUP_PRIORITY_ON_A( SERVICE_DI_DOOR_RF	),
		POPUP_PRIORITY_ON_A( SERVICE_DI_DOOR_LB	),
		POPUP_PRIORITY_ON_A( SERVICE_DI_DOOR_RB	),
		POPUP_PRIORITY_ON_A( SERVICE_DI_DOOR_EC	),
		POPUP_PRIORITY_ON_A( SERVICE_DI_DOOR_T	),
		POPUP_PRIORITY_ON_A( SERVICE_DI_DOOR_LF_OPEN_RUN),
		POPUP_PRIORITY_ON_A( SERVICE_DI_DOOR_RF_OPEN_RUN),
		POPUP_PRIORITY_ON_A( SERVICE_DI_DOOR_LB_OPEN_RUN),
		POPUP_PRIORITY_ON_A( SERVICE_DI_DOOR_RB_OPEN_RUN),
		POPUP_PRIORITY_ON_A( SERVICE_DI_DOOR_EC_OPEN_RUN),
		POPUP_PRIORITY_ON_A( SERVICE_DI_DOOR_T_OPEN_RUN),
		POPUP_PRIORITY_ON_A( SERVICE_DI_ACC_TAKE_OVER_MESSAGE),
		POPUP_PRIORITY_ON_A( SERVICE_DI_FCW_WARNING_ECHO_MESSAGE),
	    //B class
		POPUP_PRIORITY_ON_B( SERVICE_DI_USE_KEY_TO_START_MESSAGE),
		POPUP_PRIORITY_ON_B( SERVICE_DI_SELECT_NEUTRAL_TO_RESTART_MESSAGE),
		POPUP_PRIORITY_ON_B( SERVICE_DI_LOW_WASHER_FLUID_MESSAGE),
		POPUP_PRIORITY_ON_B( SERVICE_DI_LONG_PRESS_BUTTON_ENGINE_OFF_MESSAGE),
		POPUP_PRIORITY_ON_B( SERVICE_DI_PRESS_BUTTON_ENGINE_OFF_MESSAGE),
		POPUP_PRIORITY_ON_B( SERVICE_DI_PRESS_BRAKE_SHIFT_MESSAGE),
		POPUP_PRIORITY_ON_B( SERVICE_DI_ENGAGE_PARK_OR_NEUTRAL_TO_START_MESSAGE),
		POPUP_PRIORITY_ON_B( SERVICE_DI_PRESS_CLUTCH_MESSAGE),
		POPUP_PRIORITY_ON_B( SERVICE_DI_DOUBLE_PRESS_BUTTON_ENGINE_OFF_MESSAGE),
	    //C class
		POPUP_PRIORITY_ON_C( SERVICE_DI_EPB_ASSIST1_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_EPB_ASSIST2_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_EPB_ASSIST3_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_EPB_ASSIST4_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_EPB_ASSIST5_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_ENGINE_COOLANT_TEMP_ECHO_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_START_STOP_BUTTON_FAILED_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_CLUTCH_SWITCH_FAULT_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_KEY_BATTERY_LOW_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_LOCK_STATUS_ON_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_LOCK_STATUS_OFF_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_LOCK_STATUS_FAIL_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_HHC_FAIL_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_AUTOHOLD_FAULT_ECHO_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_AUTOHOLD_ASSIST1_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_AUTOHOLD_ASSIST2_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_AUTOHOLD_ASSIST3_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_AUTOHOLD_ASSIST4_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_AUTOHOLD_ASSIST5_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_ALTERNATOR_CHARGE_ECHO_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_SEAT_BELT_DRIVER_ECHO_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_FASTEN_SEATBELT_TO_RESTART_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_LOW_FUEL_ECHO_MESSAGE_LOW),
		POPUP_PRIORITY_ON_C( SERVICE_DI_IGNITION_RELAY_FAILED_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_OVERSPEED_WARNING_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_POWER_LIFTGATE_SYSTEM_FAULT_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_POWER_LIFTGATE_SYSTEM_LIMIT_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_POWER_LIFTGATE_MANUAL_CLOSE_REQUEST_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_SIA_REMINDER_SUGGEST_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_FICM_SIA_REMINDER_SUGGEST_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_DOOR_LF_OPEN_STOP),
		POPUP_PRIORITY_ON_C( SERVICE_DI_DOOR_RF_OPEN_STOP),
		POPUP_PRIORITY_ON_C( SERVICE_DI_DOOR_LB_OPEN_STOP),
		POPUP_PRIORITY_ON_C( SERVICE_DI_DOOR_RB_OPEN_STOP),
		POPUP_PRIORITY_ON_C( SERVICE_DI_DOOR_EC_OPEN_STOP),
		POPUP_PRIORITY_ON_C( SERVICE_DI_DOOR_T_OPEN_STOP),
		POPUP_PRIORITY_ON_C( SERVICE_DI_ATS_MODE_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_ALL_WHEEL_DRIVE_INSERVICE_DICATION_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_LEFT_REGULATE_STEERING_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_LEFT_REGULATE_STREERING_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_RIGHT_REGULATE_STEERING_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_LOW_FUEL_ECHO_MESSAGE_CRITICAL),
		POPUP_PRIORITY_ON_C( SERVICE_DI_FRONT_FOG_ON_ECHO_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_REAR_FOG_ON_ECHO_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_MAIN_BEAM_ECHO_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_ENGINE_DISABLED_MESSAGE),
		/* ADAS功能 begin*/
		POPUP_PRIORITY_ON_C( SERVICE_DI_ACC_SYSTEM_STAND_BY_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_ACC_SENSOR_BLOCK_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_ACC_SYSTEM_CANCEL_REQUEST_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_ACC_SYSTEM_UNAVAILABLE_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_ACC_SYSTEM_OFF_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_ACC_SYSTEM_ON_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_FVCM_FAULT_MESSASGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_AEB_SYSTEM_UNAVAILABE_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_AEB_SYSTEM_OFF_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_AEB_SYSTEM_ON_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_AEB_ACTIVE_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_LDW_SYSTEM_OFF_ECHO_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_LDW_STAND_BY_ECHO_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_LDW_SYSTEM_CROSSING_LANE_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_LDW_UNAVAILABLE_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_FCW_SYSTEM_OFF_ECHO_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_FCW_SYSTEM_ON_ECHO_MESSAGE),	
		POPUP_PRIORITY_ON_C( SERVICE_DI_FCW_SYSTEM_UNAVAILABLE_ECHO_MESSAGE),	
		POPUP_PRIORITY_ON_C( SERVICE_DI_SAS_SYSTEM_FAULT_MESSAGE),
		POPUP_PRIORITY_ON_C( SERVICE_DI_SAS_SYSTEM_SPEED_LIMIT_REMINDER_MESSAGE),	
		///off A
		POPUP_PRIORITY_OFF_A( SERVICE_DI_DOOR_RF_OPEN_POWEROFF),
		POPUP_PRIORITY_OFF_A( SERVICE_DI_DOOR_LF_OPEN_POWEROFF),
		POPUP_PRIORITY_OFF_A( SERVICE_DI_DOOR_LB_OPEN_POWEROFF),
		POPUP_PRIORITY_OFF_A( SERVICE_DI_DOOR_RB_OPEN_POWEROFF),
		POPUP_PRIORITY_OFF_A( SERVICE_DI_DOOR_EC_OPEN_POWEROFF),
		POPUP_PRIORITY_OFF_A( SERVICE_DI_DOOR_T_OPEN_POWEROFF),
		POPUP_PRIORITY_OFF_A( SERVICE_DI_PUT_SHIFTER_TO_PARK_MESSAGE),
		POPUP_PRIORITY_OFF_A( SERVICE_DI_IGNITION_KEY_ON_MESSAGE),
		POPUP_PRIORITY_OFF_A( SERVICE_DI_LIGHTS_ON_KEY_OUT_WARNING_MESSAGE),
		POPUP_PRIORITY_OFF_A( SERVICE_DI_PRESS_BRAKE_MESSAGE),
		POPUP_PRIORITY_OFF_A( SERVICE_DI_NO_SMART_KEY_DETECTED_MESSAGE),
		POPUP_PRIORITY_OFF_A( SERVICE_DI_NO_KEY_DETECTED_PRESS_CLUTCH_MESSAGE),
		POPUP_PRIORITY_OFF_A( SERVICE_DI_PUT_KEY_INTO_BACKUP_POSITION_MESSAGE),
		POPUP_PRIORITY_OFF_A( SERVICE_DI_PEPS_ANTENNA_FAULT_MESSAGE),
		POPUP_PRIORITY_OFF_A( SERVICE_DI_ESCL_FAULT_LEVEL2_ECHO_MESSAGE),
		POPUP_PRIORITY_OFF_A( SERVICE_DI_NO_KEY_DETECTED_PRESS_BRAKE_MESSAGE),
		POPUP_PRIORITY_OFF_A( SERVICE_DI_TAKE_SMART_KEY_MESSAGE),
		POPUP_PRIORITY_OFF_A( SERVICE_DI_KEY_OFF_WHEEL_NOT_STRAIGHT_REMINDER_MESSAGE),
		POPUP_PRIORITY_OFF_A( SERVICE_DI_ALARM_TRIGGERED_MESSAGE),
	};

	return (int)(popup_priority_table[ messageid & 0xffff ]);
}


struct MESSAGE_ATTR{
	unsigned int id;
	const char*  name;
	const char*  pri;
	int          on;
	int		     pwr;
};


/*power on*/
#define ATTR(__id , __p , __pwr ) { .id = __id , .name = #__id , .pri = #__p , .on = 0, .pwr = __pwr }

#define ATTR_PRIORITY_ON_A(id)  ATTR(id,A,1)
#define ATTR_PRIORITY_ON_B(id)  ATTR(id,B,1)
#define ATTR_PRIORITY_ON_C(id)  ATTR(id,C,1)
#define ATTR_PRIORITY_OFF_A(id) ATTR(id,A,0)
#define ATTR_PRIORITY_OFF_B(id) ATTR(id,B,0)
#define ATTR_PRIORITY_OFF_C(id) ATTR(id,C,0)

struct MESSAGE_ATTR attrs[] = {
		ATTR_PRIORITY_ON_A( SERVICE_DI_DOOR_LF	),
		ATTR_PRIORITY_ON_A( SERVICE_DI_DOOR_RF	),
		ATTR_PRIORITY_ON_A( SERVICE_DI_DOOR_LB	),
		ATTR_PRIORITY_ON_A( SERVICE_DI_DOOR_RB	),
		ATTR_PRIORITY_ON_A( SERVICE_DI_DOOR_EC	),
		ATTR_PRIORITY_ON_A( SERVICE_DI_DOOR_T	),
		ATTR_PRIORITY_ON_A( SERVICE_DI_DOOR_LF_OPEN_RUN),
		ATTR_PRIORITY_ON_A( SERVICE_DI_DOOR_RF_OPEN_RUN),
		ATTR_PRIORITY_ON_A( SERVICE_DI_DOOR_LB_OPEN_RUN),
		ATTR_PRIORITY_ON_A( SERVICE_DI_DOOR_RB_OPEN_RUN),
		ATTR_PRIORITY_ON_A( SERVICE_DI_DOOR_EC_OPEN_RUN),
		ATTR_PRIORITY_ON_A( SERVICE_DI_DOOR_T_OPEN_RUN),
		ATTR_PRIORITY_ON_A( SERVICE_DI_ACC_TAKE_OVER_MESSAGE),
		ATTR_PRIORITY_ON_A( SERVICE_DI_FCW_WARNING_ECHO_MESSAGE),
	    //B class
		ATTR_PRIORITY_ON_B( SERVICE_DI_USE_KEY_TO_START_MESSAGE),
		ATTR_PRIORITY_ON_B( SERVICE_DI_SELECT_NEUTRAL_TO_RESTART_MESSAGE),
		ATTR_PRIORITY_ON_B( SERVICE_DI_LOW_WASHER_FLUID_MESSAGE),
		ATTR_PRIORITY_ON_B( SERVICE_DI_LONG_PRESS_BUTTON_ENGINE_OFF_MESSAGE),
		ATTR_PRIORITY_ON_B( SERVICE_DI_PRESS_BUTTON_ENGINE_OFF_MESSAGE),
		ATTR_PRIORITY_ON_B( SERVICE_DI_PRESS_BRAKE_SHIFT_MESSAGE),
		ATTR_PRIORITY_ON_B( SERVICE_DI_ENGAGE_PARK_OR_NEUTRAL_TO_START_MESSAGE),
		ATTR_PRIORITY_ON_B( SERVICE_DI_PRESS_CLUTCH_MESSAGE),
		ATTR_PRIORITY_ON_B( SERVICE_DI_DOUBLE_PRESS_BUTTON_ENGINE_OFF_MESSAGE),
	    //C class
		ATTR_PRIORITY_ON_C( SERVICE_DI_EPB_ASSIST1_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_EPB_ASSIST2_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_EPB_ASSIST3_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_EPB_ASSIST4_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_EPB_ASSIST5_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_ENGINE_COOLANT_TEMP_ECHO_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_START_STOP_BUTTON_FAILED_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_CLUTCH_SWITCH_FAULT_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_KEY_BATTERY_LOW_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_LOCK_STATUS_ON_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_LOCK_STATUS_OFF_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_LOCK_STATUS_FAIL_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_HHC_FAIL_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_AUTOHOLD_FAULT_ECHO_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_AUTOHOLD_ASSIST1_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_AUTOHOLD_ASSIST2_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_AUTOHOLD_ASSIST3_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_AUTOHOLD_ASSIST4_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_AUTOHOLD_ASSIST5_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_ALTERNATOR_CHARGE_ECHO_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_SEAT_BELT_DRIVER_ECHO_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_FASTEN_SEATBELT_TO_RESTART_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_LOW_FUEL_ECHO_MESSAGE_LOW),
		ATTR_PRIORITY_ON_C( SERVICE_DI_IGNITION_RELAY_FAILED_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_OVERSPEED_WARNING_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_POWER_LIFTGATE_SYSTEM_FAULT_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_POWER_LIFTGATE_SYSTEM_LIMIT_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_POWER_LIFTGATE_MANUAL_CLOSE_REQUEST_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_SIA_REMINDER_SUGGEST_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_FICM_SIA_REMINDER_SUGGEST_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_DOOR_LF_OPEN_STOP),
		ATTR_PRIORITY_ON_C( SERVICE_DI_DOOR_RF_OPEN_STOP),
		ATTR_PRIORITY_ON_C( SERVICE_DI_DOOR_LB_OPEN_STOP),
		ATTR_PRIORITY_ON_C( SERVICE_DI_DOOR_RB_OPEN_STOP),
		ATTR_PRIORITY_ON_C( SERVICE_DI_DOOR_EC_OPEN_STOP),
		ATTR_PRIORITY_ON_C( SERVICE_DI_DOOR_T_OPEN_STOP),
		ATTR_PRIORITY_ON_C( SERVICE_DI_ATS_MODE_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_ALL_WHEEL_DRIVE_INSERVICE_DICATION_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_LEFT_REGULATE_STEERING_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_LEFT_REGULATE_STREERING_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_RIGHT_REGULATE_STEERING_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_LOW_FUEL_ECHO_MESSAGE_CRITICAL),
		ATTR_PRIORITY_ON_C( SERVICE_DI_FRONT_FOG_ON_ECHO_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_REAR_FOG_ON_ECHO_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_MAIN_BEAM_ECHO_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_ENGINE_DISABLED_MESSAGE),
		/* ADAS功能 begin*/
		ATTR_PRIORITY_ON_C( SERVICE_DI_ACC_SYSTEM_STAND_BY_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_ACC_SENSOR_BLOCK_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_ACC_SYSTEM_CANCEL_REQUEST_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_ACC_SYSTEM_UNAVAILABLE_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_ACC_SYSTEM_OFF_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_ACC_SYSTEM_ON_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_FVCM_FAULT_MESSASGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_AEB_SYSTEM_UNAVAILABE_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_AEB_SYSTEM_OFF_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_AEB_SYSTEM_ON_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_AEB_ACTIVE_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_LDW_SYSTEM_OFF_ECHO_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_LDW_STAND_BY_ECHO_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_LDW_SYSTEM_CROSSING_LANE_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_LDW_UNAVAILABLE_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_FCW_SYSTEM_OFF_ECHO_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_FCW_SYSTEM_ON_ECHO_MESSAGE),	
		ATTR_PRIORITY_ON_C( SERVICE_DI_FCW_SYSTEM_UNAVAILABLE_ECHO_MESSAGE),	
		ATTR_PRIORITY_ON_C( SERVICE_DI_SAS_SYSTEM_FAULT_MESSAGE),
		ATTR_PRIORITY_ON_C( SERVICE_DI_SAS_SYSTEM_SPEED_LIMIT_REMINDER_MESSAGE),	
		///off A
		ATTR_PRIORITY_OFF_A( SERVICE_DI_DOOR_RF_OPEN_POWEROFF),
		ATTR_PRIORITY_OFF_A( SERVICE_DI_DOOR_LF_OPEN_POWEROFF),
		ATTR_PRIORITY_OFF_A( SERVICE_DI_DOOR_LB_OPEN_POWEROFF),
		ATTR_PRIORITY_OFF_A( SERVICE_DI_DOOR_RB_OPEN_POWEROFF),
		ATTR_PRIORITY_OFF_A( SERVICE_DI_DOOR_EC_OPEN_POWEROFF),
		ATTR_PRIORITY_OFF_A( SERVICE_DI_DOOR_T_OPEN_POWEROFF),
		ATTR_PRIORITY_OFF_A( SERVICE_DI_PUT_SHIFTER_TO_PARK_MESSAGE),
		ATTR_PRIORITY_OFF_A( SERVICE_DI_IGNITION_KEY_ON_MESSAGE),
		ATTR_PRIORITY_OFF_A( SERVICE_DI_LIGHTS_ON_KEY_OUT_WARNING_MESSAGE),
		ATTR_PRIORITY_OFF_A( SERVICE_DI_PRESS_BRAKE_MESSAGE),
		ATTR_PRIORITY_OFF_A( SERVICE_DI_NO_SMART_KEY_DETECTED_MESSAGE),
		ATTR_PRIORITY_OFF_A( SERVICE_DI_NO_KEY_DETECTED_PRESS_CLUTCH_MESSAGE),
		ATTR_PRIORITY_OFF_A( SERVICE_DI_PUT_KEY_INTO_BACKUP_POSITION_MESSAGE),
		ATTR_PRIORITY_OFF_A( SERVICE_DI_PEPS_ANTENNA_FAULT_MESSAGE),
		ATTR_PRIORITY_OFF_A( SERVICE_DI_ESCL_FAULT_LEVEL2_ECHO_MESSAGE),
		ATTR_PRIORITY_OFF_A( SERVICE_DI_NO_KEY_DETECTED_PRESS_BRAKE_MESSAGE),
		ATTR_PRIORITY_OFF_A( SERVICE_DI_TAKE_SMART_KEY_MESSAGE),
		ATTR_PRIORITY_OFF_A( SERVICE_DI_KEY_OFF_WHEEL_NOT_STRAIGHT_REMINDER_MESSAGE),
		ATTR_PRIORITY_OFF_A( SERVICE_DI_ALARM_TRIGGERED_MESSAGE),
};


static char pirority_chs[] = {'C','B','A'};

void print_all_message()
{
	int i;
	printf("******************************************************\n");
	for( i = 0 ; i < ARRAY_SIZE(attrs); i++ )
		printf("MESSAGE INDEX:%d ID:%s\n", i , attrs[i].name );
	printf("******************************************************\n");
}

const char* find_message_name(int id, int *index)
{
	int i;
	for( i = 0 ; i < ARRAY_SIZE(attrs); i++ )
		if( attrs[i].id == id )
		{
			*index = i;

			return attrs[i].name;
		}

	return NULL;
}

void print_all_message_priority()
{
	int i;
	int l;
	for( i = 0 ; i < ARRAY_SIZE(attrs); i++ ){
		l = strlen( attrs[i].name );
		l = 64 - l;
	
		printf("MESSAGE INDEX:%d \t", i );
		printf("ID:%s", attrs[i].name  );

		while(l-- > 0)
			putchar(' ');

		printf("PRIORITY:%s POWERON:%d ON:%d\n", attrs[i].pri , attrs[i].pwr, attrs[i].on );
	
	}
}


int main(int argc,char** argv)
{
	char buf[256];
	while( 1 ){
		printf("plesae enter index:(--help to print message again,+ add messasge, - remove message )\n");
		
		printf("CMD:");
		if( buf != fgets( buf , sizeof(buf) , stdin ) ){
			printf("error input!!!\n");
			continue;
		}
		
		if(0 == strncmp("--help", buf, 6 ) ){
			print_all_message_priority();
			continue;
		}

		if(0 == strncmp("on" , buf , 2 ) ){
			popup_set_power_mode(1);
			printf("set powet on!\n");
			continue;
		}

		if(0 == strncmp("off" , buf , 3 ) ){
			popup_set_power_mode(0);
			printf("set powet off!\n");
			continue;
		}

		if( '-' != buf[0] && '+' != buf[0] ){
			printf("error cmd!!!\n");
			continue;
		}

		if( !isdigit( buf[1] ) ){
			printf("error number\n");
			continue;
		}
		
		int idx = atoi( buf+1 );
		if( (idx < 0) || (idx >= ARRAY_SIZE(attrs)) ){
			printf("error index\n");
			continue;
		}

		printf("operation:%c ----%s\n", buf[0] , attrs[idx].name );

		if( '+' == buf[0] ){
			popup_add_message( attrs[idx].id , 0 );
			attrs[idx].on = 1;
		}
		else{
			popup_remove_message( attrs[idx].id );
			attrs[idx].on = 0;
		}

		int max_p_msg = get_max_priority_popup_message();
		if( 0 == max_p_msg ){
			printf("no message\n");
			continue;
		}

		int index_in_attrs;
		const char* name = find_message_name( max_p_msg, &index_in_attrs );
		if( NULL == name ){
			printf("cant find name\n");
			continue;
		}

		printf("\n>>>>>>>>>>>>>>>>>>>>>>>>>>MAX PRIORITY IS %d : %s , power mode is %s\n\n",index_in_attrs, name , popup_get_power_mode() ? "on" : "off" );
	}

	return 0;
}
