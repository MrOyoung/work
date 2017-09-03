#include <iostream>
#include "Message_Def.h"
#include "gtest/gtest.h"

#include "Module_Def.h"
#include "Message_Def.h"

#include "util.h"

/* popup priority test */
#include "popup_priority.h"
/* popup on test */
#include "popup_on.h"
/* main test */
#include "main.h"

using namespace std;

//#define POPUP_PRIORITY_TEST
//#define POPUP_ON_TEST
#define MAIN_TEST

int msg_id_classA_on[] = 
{
	SERVICE_DI_DOOR_LF	,
	SERVICE_DI_DOOR_RF	,
	SERVICE_DI_DOOR_LB	,
	SERVICE_DI_DOOR_RB	,
	SERVICE_DI_DOOR_EC	,
	SERVICE_DI_DOOR_T	,
	SERVICE_DI_DOOR_LF_OPEN_RUN,
	SERVICE_DI_DOOR_RF_OPEN_RUN,
	SERVICE_DI_DOOR_LB_OPEN_RUN,
	SERVICE_DI_DOOR_RB_OPEN_RUN,
	SERVICE_DI_DOOR_EC_OPEN_RUN,
	SERVICE_DI_DOOR_T_OPEN_RUN,
	SERVICE_DI_ACC_TAKE_OVER_MESSAGE,
	SERVICE_DI_FCW_WARNING_ECHO_MESSAGE,
};

int msg_id_classA_off[] =
{
	SERVICE_DI_DOOR_RF_OPEN_POWEROFF,
	SERVICE_DI_DOOR_LF_OPEN_POWEROFF,
	SERVICE_DI_DOOR_LB_OPEN_POWEROFF,
	SERVICE_DI_DOOR_RB_OPEN_POWEROFF,
	SERVICE_DI_DOOR_EC_OPEN_POWEROFF,
	SERVICE_DI_DOOR_T_OPEN_POWEROFF,
	SERVICE_DI_PUT_SHIFTER_TO_PARK_MESSAGE,
	SERVICE_DI_IGNITION_KEY_ON_MESSAGE,
	SERVICE_DI_LIGHTS_ON_KEY_OUT_WARNING_MESSAGE,
	SERVICE_DI_PRESS_BRAKE_MESSAGE,
	SERVICE_DI_NO_SMART_KEY_DETECTED_MESSAGE,
	SERVICE_DI_NO_KEY_DETECTED_PRESS_CLUTCH_MESSAGE,
	SERVICE_DI_PUT_KEY_INTO_BACKUP_POSITION_MESSAGE,
	SERVICE_DI_PEPS_ANTENNA_FAULT_MESSAGE,
	SERVICE_DI_ESCL_FAULT_LEVEL2_ECHO_MESSAGE,
	SERVICE_DI_NO_KEY_DETECTED_PRESS_BRAKE_MESSAGE,
	SERVICE_DI_TAKE_SMART_KEY_MESSAGE,
	SERVICE_DI_KEY_OFF_WHEEL_NOT_STRAIGHT_REMINDER_MESSAGE,
	SERVICE_DI_ALARM_TRIGGERED_MESSAGE
};

int msg_id_classB[] = 
{
	SERVICE_DI_USE_KEY_TO_START_MESSAGE,
	SERVICE_DI_SELECT_NEUTRAL_TO_RESTART_MESSAGE,
	SERVICE_DI_LOW_WASHER_FLUID_MESSAGE,
	SERVICE_DI_LONG_PRESS_BUTTON_ENGINE_OFF_MESSAGE,
	SERVICE_DI_PRESS_BUTTON_ENGINE_OFF_MESSAGE,
	SERVICE_DI_PRESS_BRAKE_SHIFT_MESSAGE,
	SERVICE_DI_ENGAGE_PARK_OR_NEUTRAL_TO_START_MESSAGE,
	SERVICE_DI_PRESS_CLUTCH_MESSAGE,
	SERVICE_DI_DOUBLE_PRESS_BUTTON_ENGINE_OFF_MESSAGE
};                       


int msg_id_classC[] = 
{
	SERVICE_DI_EPB_ASSIST1_MESSAGE,
	SERVICE_DI_EPB_ASSIST2_MESSAGE,
	SERVICE_DI_EPB_ASSIST3_MESSAGE,
	SERVICE_DI_EPB_ASSIST4_MESSAGE,
	SERVICE_DI_EPB_ASSIST5_MESSAGE,
	SERVICE_DI_ENGINE_COOLANT_TEMP_ECHO_MESSAGE,
	SERVICE_DI_START_STOP_BUTTON_FAILED_MESSAGE,
	SERVICE_DI_CLUTCH_SWITCH_FAULT_MESSAGE,
	SERVICE_DI_KEY_BATTERY_LOW_MESSAGE,
	SERVICE_DI_LOCK_STATUS_ON_MESSAGE,
	SERVICE_DI_LOCK_STATUS_OFF_MESSAGE,
	SERVICE_DI_LOCK_STATUS_FAIL_MESSAGE,
	SERVICE_DI_HHC_FAIL_MESSAGE,
	SERVICE_DI_AUTOHOLD_FAULT_ECHO_MESSAGE,
	SERVICE_DI_AUTOHOLD_ASSIST1_MESSAGE,
	SERVICE_DI_AUTOHOLD_ASSIST2_MESSAGE,
	SERVICE_DI_AUTOHOLD_ASSIST3_MESSAGE,
	SERVICE_DI_AUTOHOLD_ASSIST4_MESSAGE,
	SERVICE_DI_AUTOHOLD_ASSIST5_MESSAGE,
	SERVICE_DI_ALTERNATOR_CHARGE_ECHO_MESSAGE,
	SERVICE_DI_SEAT_BELT_DRIVER_ECHO_MESSAGE,
	SERVICE_DI_FASTEN_SEATBELT_TO_RESTART_MESSAGE,
	SERVICE_DI_LOW_FUEL_ECHO_MESSAGE_LOW,
	SERVICE_DI_IGNITION_RELAY_FAILED_MESSAGE,
	SERVICE_DI_OVERSPEED_WARNING_MESSAGE,
	SERVICE_DI_POWER_LIFTGATE_SYSTEM_FAULT_MESSAGE,
	SERVICE_DI_POWER_LIFTGATE_SYSTEM_LIMIT_MESSAGE,
	SERVICE_DI_POWER_LIFTGATE_MANUAL_CLOSE_REQUEST_MESSAGE,
	SERVICE_DI_SIA_REMINDER_SUGGEST_MESSAGE,
	SERVICE_DI_FICM_SIA_REMINDER_SUGGEST_MESSAGE,
	SERVICE_DI_DOOR_LF_OPEN_STOP,
	SERVICE_DI_DOOR_RF_OPEN_STOP,
	SERVICE_DI_DOOR_LB_OPEN_STOP,
	SERVICE_DI_DOOR_RB_OPEN_STOP,
	SERVICE_DI_DOOR_EC_OPEN_STOP,
	SERVICE_DI_DOOR_T_OPEN_STOP,
	SERVICE_DI_ATS_MODE_MESSAGE,
	SERVICE_DI_ALL_WHEEL_DRIVE_INSERVICE_DICATION_MESSAGE,
	SERVICE_DI_LEFT_REGULATE_STEERING_MESSAGE,
	SERVICE_DI_LEFT_REGULATE_STREERING_MESSAGE,
	SERVICE_DI_RIGHT_REGULATE_STEERING_MESSAGE,
	SERVICE_DI_LOW_FUEL_ECHO_MESSAGE_CRITICAL,
	SERVICE_DI_FRONT_FOG_ON_ECHO_MESSAGE,
	SERVICE_DI_REAR_FOG_ON_ECHO_MESSAGE,
	SERVICE_DI_MAIN_BEAM_ECHO_MESSAGE,
	SERVICE_DI_ENGINE_DISABLED_MESSAGE,

	/* ADAS功能 begin*/
	SERVICE_DI_ACC_SYSTEM_STAND_BY_MESSAGE,
	SERVICE_DI_ACC_SENSOR_BLOCK_MESSAGE,
	SERVICE_DI_ACC_SYSTEM_CANCEL_REQUEST_MESSAGE,
	SERVICE_DI_ACC_SYSTEM_UNAVAILABLE_MESSAGE,
	SERVICE_DI_ACC_SYSTEM_OFF_MESSAGE,
	SERVICE_DI_ACC_SYSTEM_ON_MESSAGE,
	SERVICE_DI_FVCM_FAULT_MESSASGE,
	SERVICE_DI_AEB_SYSTEM_UNAVAILABE_MESSAGE,
	SERVICE_DI_AEB_SYSTEM_OFF_MESSAGE,
	SERVICE_DI_AEB_SYSTEM_ON_MESSAGE,
	SERVICE_DI_AEB_ACTIVE_MESSAGE,
	SERVICE_DI_LDW_SYSTEM_OFF_ECHO_MESSAGE,
	SERVICE_DI_LDW_STAND_BY_ECHO_MESSAGE,
	//SERVICE_DI_DOOR_LFCE_DI_LDW_SYSTEM_CROSSING_LANE_MESSAGE,
	SERVICE_DI_LDW_UNAVAILABLE_MESSAGE,
	SERVICE_DI_FCW_SYSTEM_OFF_ECHO_MESSAGE,
	SERVICE_DI_FCW_SYSTEM_ON_ECHO_MESSAGE,	
	SERVICE_DI_FCW_SYSTEM_UNAVAILABLE_ECHO_MESSAGE,	
	SERVICE_DI_SAS_SYSTEM_FAULT_MESSAGE,
	SERVICE_DI_SAS_SYSTEM_SPEED_LIMIT_REMINDER_MESSAGE	
};

int msg_id_negative[] = 
{
	SERVICE_DI_SEAT_BELT_WARNING_SOUND, /* 0 */
	SERVICE_DI_DIRECTION_INDICATOR_LEFT_HAND_LAMP_TICK_TOCK, /* 1 */
	SERVICE_DI_STOP_START_ON_MESSAGE, /* 87 */
	SERVICE_DI_USER_ODO_UNIT, /* 237 */                                      
	16843073 /* out of range */
};


#ifdef POPUP_PRIORITY_TEST
/********************************** popup_priority test start *****************************/

#define A_POWERON	0x13
#define B_POWERON	0x12
#define C_POWERON	0x11
#define A_POWEROFF	0x03
#define POPUP_NULL	0x00

/********************************************
 **SERVICE_DI_MessageID_Tag
 **NUM:321
 **MIN:SERVICE_DI_SEAT_BELT_WARNING_SOUND   --  16842752
 **MAX:SERVICE_DI_MESSAGE_MAX			   --  16843073
 ********************************************/

/* 测试用例 */
class TestClassAPowerOn :
	public ::testing::Test, 
	public ::testing::WithParamInterface<int> /* 模板类 <T>为传入数据类型 */
{
};

/* 测试特例 GetParam()方法获取框架指定的参数 */
/* popup_priority.c A class power on */
TEST_P(TestClassAPowerOn, ClassA_poweron)
{
	EXPECT_EQ(A_POWERON , get_messageid_priority( GetParam() ));
}

/* 
 **	使用宏向框架注册“定制化测试” 
 **	第一个参数是测试前缀，第二个参数是测试类名，第三个参数时参数生成规则 
 */
INSTANTIATE_TEST_CASE_P(TestClassAPowerOn, TestClassAPowerOn, testing::ValuesIn(msg_id_classA_on));


/* popup_priority.c A class power off */
class TestClassAPowerOff :
	public ::testing::Test, 
	public ::testing::WithParamInterface<int> /* 模板类 <T>为传入数据类型 */
{
};

TEST_P(TestClassAPowerOff, ClassA_poweroff)
{
	EXPECT_EQ(A_POWEROFF , get_messageid_priority( GetParam() ));
}
INSTANTIATE_TEST_CASE_P(TestClassAPowerOff, TestClassAPowerOff, testing::ValuesIn(msg_id_classA_off));


/* popup_priority.c B class */
class TestClassB :
	public ::testing::Test, 
	public ::testing::WithParamInterface<int> /* 模板类 <T>为传入数据类型 */
{
};

TEST_P(TestClassB, ClassB)
{
	EXPECT_EQ(B_POWERON , get_messageid_priority( GetParam() ));
}
INSTANTIATE_TEST_CASE_P(TestClassB, TestClassB, testing::ValuesIn(msg_id_classB));


/* popup_priority.c C class */
class TestClassC :
	public ::testing::Test, 
	public ::testing::WithParamInterface<int> /* 模板类 <T>为传入数据类型 */
{
};

TEST_P(TestClassC, ClassC)
{
	EXPECT_EQ(C_POWERON , get_messageid_priority( GetParam() ));
}
INSTANTIATE_TEST_CASE_P(TestClassC, TestClassC, testing::ValuesIn(msg_id_classC));


/* popup_priority.c negative */
class TestNegative :
	public ::testing::Test, 
	public ::testing::WithParamInterface<int> /* 模板类 <T>为传入数据类型 */
{
};

TEST_P(TestNegative, negative)
{
	EXPECT_EQ(POPUP_NULL , get_messageid_priority( GetParam() ));
}
INSTANTIATE_TEST_CASE_P(TestNegative, TestNegative, testing::ValuesIn(msg_id_negative));

#endif

/********************************** popup_priority test end *****************************/


/********************************** popup_on test start *****************************/
#ifdef POPUP_ON_TEST

#define WITHIN	1
#define WITHOUT 0

/* popup_on.c class A/B/C */
class TestFuncIsThisPopupOn :
	public ::testing::Test, 
	public ::testing::WithParamInterface<int> /* 模板类 <T>为传入数据类型 */
{
};

TEST_P(TestFuncIsThisPopupOn, ClassABC)
{
	EXPECT_EQ(WITHOUT , is_this_popup_on( GetParam() ));
}
INSTANTIATE_TEST_CASE_P(TestClassAPowerOn, TestFuncIsThisPopupOn, testing::ValuesIn(msg_id_classA_on));
INSTANTIATE_TEST_CASE_P(TestClassAPowerOff, TestFuncIsThisPopupOn, testing::ValuesIn(msg_id_classA_off));
INSTANTIATE_TEST_CASE_P(TestClassB, TestFuncIsThisPopupOn, testing::ValuesIn(msg_id_classB));
INSTANTIATE_TEST_CASE_P(TestClassC, TestFuncIsThisPopupOn, testing::ValuesIn(msg_id_classC));

INSTANTIATE_TEST_CASE_P(TestNegative, TestFuncIsThisPopupOn, testing::ValuesIn(msg_id_negative));


/* set_this_popup_on() set_this_popup_off() 未测 */

#endif //POPUP_ON_TEST
/********************************** popup_on test end *****************************/



/********************************** reset test start *****************************/
#ifdef REST_TEST

/* longdisplay_reset() nothing_reset() 未测 */

#endif //REST_TEST
/********************************** reset test end *****************************/


/********************************** popup_msg test start *****************************/
#ifdef POPUP_MSG_TEST

/* longdisplay_reset() nothing_reset() 未测 */

#endif //POPUP_MSG_TEST

/********************************** popup_msg test end *****************************/


/********************************** mian test start *****************************/
#ifdef MAIN_TEST

/* 90Byte */
static unsigned char mcudata_general[] = 
{
	0x00, 0x00, 0x00, 0x00,/* IPC_UART_DATA_GENERAL_UP */ 
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xae, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x33, 0x89, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x8c,
	0x88, 0x13, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x4b, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x05, 0x09, 0x0f, 0x64, 0x37, 0x0d, 0x0c, 0x0c, 0x0c, 0x0c 
};

/* 103Byte */
static unsigned char mcudata_special[] = 
{
	0x00, 0x00, 0x00, 0x00,/* IPC_UART_DATA_SPECIAL_UP */ 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x2e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x06, 0x0c, 0x05, 0x00,
	0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
	0x00
};


/* reset func */
static unsigned char reset_frame[4] = {};


#define INSERT( array, nint ) \
	do { \
		array[0] = nint & 0xff; \
		array[1] = (nint >> 8)  & 0xff; \
		array[2] = (nint >> 16) & 0xff; \
		array[3] = (nint >> 24) & 0xff; \
	}while (0)


void *send_mcudata_thread(void *args)
{
	char *type = "UnitTestModule.signal.type";
	DBusConnection *dbus;
	dbus = get_dbus("com.saic.UnitTestModule");
	if (!dbus)
	{
		printf("#########get_dbus error!!\n");
		return NULL;
	}

	int ret = 0;
	int exchange = 0; 
	int size = 0;
	unsigned char *mcudataPtr = mcudata_general;

	INSERT(mcudata_general, IPC_UART_DATA_GENERAL_UP);
	INSERT(mcudata_special, IPC_UART_DATA_SPECIAL_UP);
	INSERT(reset_frame, APP_CLUSTER_SELFCHECK_READY);

	while (1)
	{
		//if (exchange ^= 1)
		//{
		//	printf("mcudata_general\n");
		//	mcudataPtr = mcudata_general;
		//	size = sizeof(mcudata_general);
		//}
		//else
		//{
		//	printf("mcudata_special\n");
		//	mcudataPtr = mcudata_special;
		//	size = sizeof(mcudata_special);
		//}

		if (exchange == 0)
		{
			printf("mcudata_general\n");
			mcudataPtr = mcudata_general;
			size = sizeof(mcudata_general);
			exchange++;
		}
		else if (exchange == 1)
		{
			printf("mcudata_special\n");
			mcudataPtr = mcudata_special;
			size = sizeof(mcudata_special);
			exchange++;
		}
		else if (exchange == 2)
		{
			printf("reset_frame\n");
			mcudataPtr = reset_frame;
			size = sizeof(reset_frame);
			exchange = 0;
		}

		ret = dbus_send(dbus,
				mcudataPtr,
				size, 			   /* size */
				MODULE_SERVICE_DI_DATA_ANALISIS_PATH,  /* path */
				type);
		if (-1 == ret)
		{
			printf("#########dbus_send error!!!\n");
		}
		printf("send success!!\n");

		usleep(200000);
	}
}


#if 0
int main(int argc, char *argv[])
{
	pthread_t pth;

	if (0 != pthread_create(&pth, NULL, send_mcudata_thread, NULL))
	{
		printf("pthread create error!!!\n");	
	}
	
	printf("pthread create successfully!!\n");

	pthread_join(pth, NULL);
	return 0;
}
#endif

TEST(TestMain, positive)
{
	pthread_t pth;
//
//	if (0 != pthread_create(&pth, NULL, send_mcudata_thread, NULL))
//	{
//		printf("pthread create error!!!\n");	
//	}
//	
//	printf("pthread create successfully!!\n");
//
	EXPECT_EQ(0, main_bak(1, NULL));

//	pthread_join(pth, NULL);
}

#endif //POPUP_MSG_TEST
/********************************** mian test end *****************************/


