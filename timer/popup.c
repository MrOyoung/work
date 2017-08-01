#include "Message_Def.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#define ROUND(__v,__a) (((__v)+(__a-1))&(~(__a-1)))
#define UINT_BITS (32)

struct POPUP_LIST_ENTRY{
	struct POPUP_LIST_ENTRY* next;
	unsigned int			 messageid;
};


int get_messageid_priority(unsigned int);


/**/
static struct POPUP_LIST_ENTRY popup_entry_pool[256];
/*第一个可用的entry*/
static struct POPUP_LIST_ENTRY* pool_head;

void init_popup_pool()
{
	int i;
	for( i = 0; i < ARRAY_SIZE(popup_entry_pool) - 1 ; i++ ){
		popup_entry_pool[i].next = &popup_entry_pool[i+1];
	}

	popup_entry_pool[ ARRAY_SIZE(popup_entry_pool) - 1 ].next = 0;
	pool_head = &popup_entry_pool[0];
}

struct POPUP_LIST_ENTRY* alloc_popup_entry()
{
	struct POPUP_LIST_ENTRY* entry;

	/*已经没有可用的entry*/
	if( !pool_head ){
		printf("the pool has no entry!!!\n");
		return NULL;

	}

	entry = pool_head;
	pool_head = pool_head->next;

	return entry;
}


void free_popup_entry(struct POPUP_LIST_ENTRY* entry)
{
	entry->next = pool_head;
	pool_head = entry;
}

/**/
static unsigned int popup_flags[ROUND(SERVICE_DI_MESSAGE_MAX&0xffff,32)/32] = {0};

int is_this_popup_on(unsigned int messageid)
{
	int idx;
	int bit;

	idx = 0xffff & messageid;
	bit = 1 << (idx % UINT_BITS);

	return 0 != (popup_flags[ idx / UINT_BITS ] & bit);
}

void set_this_popup_on(unsigned int messageid)
{
	int idx;
	int bit;

	idx = 0xffff & messageid;
	bit = 1 << (idx % UINT_BITS);

	popup_flags[ idx / UINT_BITS] |= bit;
}

void set_this_popup_off(unsigned int messageid)
{
	int idx;
	int bit;

	idx = 0xffff & messageid;
	bit = 1 << (idx % UINT_BITS);

	popup_flags[ idx / UINT_BITS ] &= (~bit);
}

struct POPUP_LEVEL{
	int count;
	struct POPUP_LIST_ENTRY* head;
};

enum{
	POPUP_PRIORITY_LEVEL_NONE,
	POPUP_PRIORITY_LEVEL_C,
	POPUP_PRIORITY_LEVEL_B,
	POPUP_PRIORITY_LEVEL_A,
	POPUP_PRIORITY_LEVEL_MAX,
};

enum{
	POPUP_PRIORITY_LEVEL_OFF_A,
	POPUP_PRIORITY_LEVEL_OFF_B,
	POPUP_PRIORITY_LEVEL_OFF_C,
	POPUP_PRIORITY_LEVEL_OFF_MAX,
};



static struct POPUP_LEVEL popup_on_hash[POPUP_PRIORITY_LEVEL_MAX-1] = {
	{0, 0} ,  
};

/*当前最高popup等级*/
static int max_priority_popup = 0;

unsigned int get_max_priority_popup_message()
{
	if( POPUP_PRIORITY_LEVEL_NONE == max_priority_popup )
		return 0;

	return popup_on_hash[max_priority_popup - 1].head->messageid;
}

int add_messageid_to_hash(const unsigned int messageid)
{
	int pri_idx;
	struct POPUP_LEVEL* level;
	struct POPUP_LIST_ENTRY* entry;

	/*验证该popup是否当前正打开着*/
	if( is_this_popup_on(messageid) ){
		printf("this popup message is already on\n");
		return -1;
	}
	
	/*获取一个记录的列表项*/
	entry = alloc_popup_entry();
	if( NULL == entry )
		return -1;

	/*获取该popup的优先级*/
	pri_idx = get_messageid_priority( messageid );
	
	/*通过优先级获取该popup所在的hash*/
	level =   &popup_on_hash[pri_idx - 1];
	
	entry->messageid = messageid;
	/*记录将最新的popup列表项目放入相应的hash*/
	entry->next		 = level->head;
	level->head      = entry;

	set_this_popup_on( messageid );

	if( pri_idx > max_priority_popup )
		max_priority_popup = pri_idx;

	return 0;
}

void __free_message_entry( struct POPUP_LIST_ENTRY* entry )
{
	set_this_popup_off( entry->messageid );
	free_popup_entry( entry );
}

void __update_current_max_priority()
{
	int index =  ARRAY_SIZE(popup_on_hash) - 1;

	while( index >= 0 ){
		if( NULL !=  popup_on_hash[index].head )
			break;

		index--;
	}

	max_priority_popup = index + 1;
}

int remove_messageid_from(unsigned int messageid)
{
	int pri_idx;
	struct POPUP_LEVEL* level;
	struct POPUP_LIST_ENTRY* entry;
	struct POPUP_LIST_ENTRY* previous;

	/*如果这个messageid的标志没有开则不需要进行下面的操作*/
	if( !is_this_popup_on(messageid) ){
		printf("this message is alway off\n");
		return -1;
	}

	/*获取该popup的优先级*/
	pri_idx = get_messageid_priority( messageid );
	
	/*通过优先级获取该popup所在的hash*/
	level =   &popup_on_hash[pri_idx - 1];

	entry = level->head;
	/*如果链表的第一项就是要找的messageid*/
	if(  messageid ==  entry->messageid ){
		level->head = entry->next;
		__free_message_entry( entry );
		__update_current_max_priority();
		return 0;
	}

	previous = entry;
	entry    = entry->next;

	while( entry ){
		if(messageid == entry->messageid){
			previous->next = entry->next;
			__free_message_entry( entry );
			__update_current_max_priority();
			return 0;
		}

		previous = entry;
		entry    = entry->next;
	}

	assert(0);
	return 0;
}

int get_messageid_priority(unsigned int messageid)
{
#define _DECLARE_MESSAGE_PRIORITY_( __id , __p ) [__id &0xfff] = POPUP_PRIORITY_LEVEL_##__p

	static unsigned char popup_priority_table[SERVICE_DI_MESSAGE_MAX&0xffff] = {
			_DECLARE_MESSAGE_PRIORITY_( SERVICE_DI_ENGINE_COOLANT_TEMP_ECHO_MESSAGE , C ),
			_DECLARE_MESSAGE_PRIORITY_( SERVICE_DI_USE_KEY_TO_START_MESSAGE , B ),
			_DECLARE_MESSAGE_PRIORITY_( SERVICE_DI_SELECT_NEUTRAL_TO_RESTART_MESSAGE , B ),
			_DECLARE_MESSAGE_PRIORITY_( SERVICE_DI_LONG_PRESS_BUTTON_ENGINE_OFF_MESSAGE , B),
			_DECLARE_MESSAGE_PRIORITY_( SERVICE_DI_PRESS_BUTTON_ENGINE_OFF_MESSAGE , B),
			_DECLARE_MESSAGE_PRIORITY_( SERVICE_DI_DOUBLE_PRESS_BUTTON_ENGINE_OFF_MESSAGE , B ),
			_DECLARE_MESSAGE_PRIORITY_( SERVICE_DI_START_STOP_BUTTON_FAILED_MESSAGE , C),
			_DECLARE_MESSAGE_PRIORITY_( SERVICE_DI_PRESS_BRAKE_SHIFT_MESSAGE , B),
			_DECLARE_MESSAGE_PRIORITY_( SERVICE_DI_PUT_SHIFTER_TO_PARK_MESSAGE , A),
			_DECLARE_MESSAGE_PRIORITY_( SERVICE_DI_ENGAGE_PARK_OR_NEUTRAL_TO_START_MESSAGE , B),
			_DECLARE_MESSAGE_PRIORITY_( SERVICE_DI_PRESS_CLUTCH_MESSAGE ,B ),
			_DECLARE_MESSAGE_PRIORITY_( SERVICE_DI_PRESS_BRAKE_MESSAGE , B),
			_DECLARE_MESSAGE_PRIORITY_( SERVICE_DI_CLUTCH_SWITCH_FAULT_MESSAGE , C),
			_DECLARE_MESSAGE_PRIORITY_( SERVICE_DI_IGNITION_KEY_ON_MESSAGE , A),
			_DECLARE_MESSAGE_PRIORITY_( SERVICE_DI_NO_SMART_KEY_DETECTED_MESSAGE ,B ),
			_DECLARE_MESSAGE_PRIORITY_( SERVICE_DI_KEY_BATTERY_LOW_MESSAGE ,C ),
			_DECLARE_MESSAGE_PRIORITY_( SERVICE_DI_NO_KEY_DETECTED_PRESS_CLUTCH_MESSAGE ,B ),
			_DECLARE_MESSAGE_PRIORITY_( SERVICE_DI_PUT_KEY_INTO_BACKUP_POSITION_MESSAGE ,B ),
			_DECLARE_MESSAGE_PRIORITY_( SERVICE_DI_RIGHT_REGULATE_STEERING_MESSAGE ,C ),
			_DECLARE_MESSAGE_PRIORITY_( SERVICE_DI_LEFT_REGULATE_STEERING_MESSAGE , C),
	};
#undef _DECLARE_MESSAGE_PRIORITY_

	return (int)(popup_priority_table[ messageid & 0xffff ]);
}


struct MESSAGE_ATTR{
	unsigned int id;
	const char*  name;
	int          refcnt;
};

#define _DECLARE_ATTR_(message,NOUSED) {.id = message, .name=#message }
struct MESSAGE_ATTR attrs[] = {
			_DECLARE_ATTR_( SERVICE_DI_ENGINE_COOLANT_TEMP_ECHO_MESSAGE , C ),
			_DECLARE_ATTR_( SERVICE_DI_USE_KEY_TO_START_MESSAGE , B ),
			_DECLARE_ATTR_( SERVICE_DI_SELECT_NEUTRAL_TO_RESTART_MESSAGE , B ),
			_DECLARE_ATTR_( SERVICE_DI_LONG_PRESS_BUTTON_ENGINE_OFF_MESSAGE , B),
			_DECLARE_ATTR_( SERVICE_DI_PRESS_BUTTON_ENGINE_OFF_MESSAGE , B),
			_DECLARE_ATTR_( SERVICE_DI_DOUBLE_PRESS_BUTTON_ENGINE_OFF_MESSAGE , B ),
			_DECLARE_ATTR_( SERVICE_DI_START_STOP_BUTTON_FAILED_MESSAGE , C),
			_DECLARE_ATTR_( SERVICE_DI_PRESS_BRAKE_SHIFT_MESSAGE , B),
			_DECLARE_ATTR_( SERVICE_DI_PUT_SHIFTER_TO_PARK_MESSAGE , A),
			_DECLARE_ATTR_( SERVICE_DI_ENGAGE_PARK_OR_NEUTRAL_TO_START_MESSAGE , B),
			_DECLARE_ATTR_( SERVICE_DI_PRESS_CLUTCH_MESSAGE ,B ),
			_DECLARE_ATTR_( SERVICE_DI_PRESS_BRAKE_MESSAGE , B),
			_DECLARE_ATTR_( SERVICE_DI_CLUTCH_SWITCH_FAULT_MESSAGE , C),
			_DECLARE_ATTR_( SERVICE_DI_IGNITION_KEY_ON_MESSAGE , A),
			_DECLARE_ATTR_( SERVICE_DI_NO_SMART_KEY_DETECTED_MESSAGE ,B ),
			_DECLARE_ATTR_( SERVICE_DI_KEY_BATTERY_LOW_MESSAGE ,C ),
			_DECLARE_ATTR_( SERVICE_DI_NO_KEY_DETECTED_PRESS_CLUTCH_MESSAGE ,B ),
			_DECLARE_ATTR_( SERVICE_DI_PUT_KEY_INTO_BACKUP_POSITION_MESSAGE ,B ),
			_DECLARE_ATTR_( SERVICE_DI_RIGHT_REGULATE_STEERING_MESSAGE ,C ),
			_DECLARE_ATTR_( SERVICE_DI_LEFT_REGULATE_STEERING_MESSAGE , C),
};


static char pirority_chs[] = {'N','C','B','A'};

void print_all_message()
{
	int i;
	printf("******************************************************\n");
	for( i = 0 ; i < ARRAY_SIZE(attrs); i++ )
		printf("MESSAGE INDEX:%d ID:%s\n", i , attrs[i].name );
	printf("******************************************************\n");
}

const char* find_message_name(int id)
{
	int i;
	for( i = 0 ; i < ARRAY_SIZE(attrs); i++ )
		if( attrs[i].id == id )
			return attrs[i].name;

	return NULL;
}

void print_all_message_priority()
{
	int i;
	for( i = 0 ; i < ARRAY_SIZE(attrs); i++ )
		printf("MESSAGE INDEX:%d ID:%s PRIORITY:%c ---COUNT:%d\n ", i , attrs[i].name ,  pirority_chs[get_messageid_priority(attrs[i].id)]  ,attrs[i].refcnt );
}


int main(int argc,char** argv)
{
	init_popup_pool();
	char buf[256];

	while( 1 ){
		print_all_message_priority();
		printf("===================================================================================\n");
		printf("plesae enter index:(--help to print message again,+ add messasge, - remove message)\n");
		printf("===================================================================================\n");
		
		printf("CMD:");
		if( buf != fgets( buf , sizeof(buf) , stdin ) ){
			printf("error input!!!\n");
			continue;
		}
		
		if(0 == strcmp("--help", buf ) ){
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

		printf("OPERATION:%c MESSAGE-----%s\n", buf[0] , attrs[idx].name );

		if( '+' == buf[0] ){
			if(!add_messageid_to_hash( attrs[idx].id ))
				attrs[idx].refcnt++;
		}
		else{
			if(!remove_messageid_from( attrs[idx].id ))
				attrs[idx].refcnt--;
		}

		int max_p_msg = get_max_priority_popup_message();
		if( 0 == max_p_msg ){
			printf("no message\n");
			continue;
		}

		const char* name = find_message_name( max_p_msg );
		if( NULL == name ){
			printf("cant find name\n");
			continue;
		}

		printf("\n\n\t\t+++++++++++++++++++++++++++++++++++++++++++\n");
		printf("\t\tMAX PRIORITY MESSAGE IS %s\n",name );
		printf("\t\t+++++++++++++++++++++++++++++++++++++++++++\n\n\n");
	}

	return 0;
}
