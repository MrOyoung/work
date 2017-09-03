#include "common.h"
#include "Message_Def.h"
#include "Table_Def.h"
#include "debug.h"

/*长显示信息的重置*/
unsigned int longdisplay[] = {
    SERVICE_DI_TACHOMETER ,   
    SERVICE_DI_DATE ,
    SERVICE_DI_ENGINE_COOLANT_TEMP , 
    SERVICE_DI_GEAR , 
    SERVICE_DI_BACKLIGHT , 
    SERVICE_DI_ODOMETER , 
    SERVICE_DI_AVERAGE_SPEED_A , 
    SERVICE_DI_INSTRUMENT_TOPIC ,
    SERVICE_DI_MILAGE , 
    SERVICE_DI_TRIPMETER_A , 
    SERVICE_DI_FUEL_GAUGE , 
    SERVICE_DI_SPEED_LIMIT , 
    SERVICE_DI_DIS_TO_NEXT_MAINTENANCE , 
    SERVICE_DI_OUTSIDE_AIR_TEMP , 
    SERVICE_DI_AVERAGE_FUEL_CONSUMPTION_A1 , 
    SERVICE_DI_INSTANTANEOUS_FUEL_CONSUMPTION1 , 
    SERVICE_DI_LCD_TEMP , 
    SERVICE_DI_DRIVE_TIME_A , 
    SERVICE_DI_TRIPMETER_C , 
    SERVICE_DI_AVERAGE_FUEL_CONSUMPTION_C1 , 
    SERVICE_DI_AVERAGE_SPEED_C , 
    SERVICE_DI_DRIVE_TIME_C , 
    SERVICE_DI_AVERAGE_FUEL_CONSUMPTION_TREND_SAMPING , 
    SERVICE_DI_STEERING_WHEEL_ANGLE , 
    SERVICE_DI_BATTERY_STATUS_DISPLAY , 
    SERVICE_DI_BATTERY_STATUS_SOC_STATE , 
    SERVICE_DI_TYRE_PRESSURE , 
    SERVICE_DI_THROTTLE_POSITION , 
    SERVICE_DI_WORKMODE , 
    SERVICE_DI_AT_OR_MT ,
    SERVICE_DI_POWER_MODE,
    SERVICE_DI_SPEED, 
    
    /*ADAS*/
    SERVICE_DI_LDW_LEFT_LANE_STATUS,
    SERVICE_DI_LDW_RIGHT_LANE_STATUS,
    
    SERVICE_DI_TIME_DISTANCE_POSITION_COLOR,
    SERVICE_DI_FRONT_CAR_DISAPPEARED,
    
    SERVICE_DI_ACC_DRIVER_SELECT_TARGET_SPEED,
    SERVICE_DI_ACC_DRIVER_SELECT_TARGET_SPEED_ENABLE
};

/*常显信息重置函数*/
void longdisplay_reset( struct MsgAttr* m )
{
	int i;
	for( i = 0 ;i < m->attr_cnt ; i++ ){
		m->attrs[i].previous = 0xfafafafa;
    }
 
}

/*重置时不需要做任何动作的，按键肯定不需要任何重置操作*/
unsigned int nothing[] = {
    SERVICE_DI_BUTTON_KEY_RIGHT , 
    SERVICE_DI_BUTTON_KEY_LEFT , 
    SERVICE_DI_BUTTON_KEY_UP , 
    SERVICE_DI_BUTTON_KEY_DOWN , 
    SERVICE_DI_BUTTON_KEY_ENTER , 
    SERVICE_DI_BUTTON_KEY_6 , 
};

/*什么也不做的函数*/
void nothing_reset( struct MsgAttr* m )
{
	
}



void __register_reset_func( struct MsgAttr* m , unsigned int* msgs , int count  , MsgResetFunctionPtr func  )
{
	if( !(m->reset) ){
		int i;
		for( i = 0 ; i < count ; i++ ){
			
			if( (m->msgid) == msgs[i] ){
				m->reset = func;
			//	func( m );/*先执行一次*/
			}
		}
	}
}

/*该宏只是将上面的传入的数组自动计算了*/
#define __register_reset_func_auto_size( m , array , func ) \
	__register_reset_func( m , array , array_size(array) , func )


void register_reset_func( struct MsgAttr* m )
{
	__register_reset_func_auto_size( m , longdisplay , longdisplay_reset );
	__register_reset_func_auto_size( m , nothing , nothing_reset );
	
}
