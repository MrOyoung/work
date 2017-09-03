#include "popup.h"
#include "debug.h"
#include "Table_Def.h"
#include "Module_Def.h"

int send_to_modules(const char** paths,int nr ,struct SignalData* data, int size ,DBusConnection* dbus );


DBusConnection* g_dbus = NULL;

static inline void func_popup_show( struct MsgAttr* m , int show )
{
	static const char* popup_path[] = {
		KANZIAPP_POPUP_PATH
	};

	
	struct SignalData data = {
		.msgid = m->msgid,
		.s     = {
			(unsigned short)show,
			(unsigned short)show,
		},
	};

	send_to_modules( 
			/**/popup_path ,
			/**/1,
			/**/&data,
			/**/sizeof(unsigned int) * 2,
			/**/g_dbus
			);
}


void popup_show(void* arg)
{
	func_popup_show( (struct MsgAttr*)arg, 1/*show*/ );
}

void popup_hide(void* arg)
{
	func_popup_show( (struct MsgAttr*)arg, 0/*hide*/ );
}


static struct popup_operation popup_op = {
	.show =  popup_show,
	.hide =  popup_hide,
};



void popup_addition_op( DBusConnection*dbus, struct MsgAttr* table , struct SignalData* sgl )
{
	g_dbus = dbus;
	
	LOG("%s OPERATION:%s\n" , table->name , __FUNCTION__ );

	if( (sgl->s[0]) )
		popup_insert_message( table->msgid ,  (void*)table , &popup_op );
	else
		popup_delete_message( table->msgid );
		
		
	if( 1 == (table->attr_cnt) ){
	
		unsigned int temp;
		temp      = (unsigned int)(sgl->s[0]);
		sgl->i[0] = temp;
	}
}


/*
*
*/
int do_multiple_data_msg_entry_with_mask( 
									DBusConnection* dbus , 
									struct MsgAttr* msg ,
									byte* frame , 
									int mask ,
									void (*addit_op)(DBusConnection*,struct MsgAttr* ,struct SignalData*)
									 );
									 
/*
*
*/
int	do_popup_msg_entry(DBusConnection* dbus , struct MsgAttr* msg ,byte* frame )							 
{
	return do_multiple_data_msg_entry_with_mask( dbus , msg , frame , (~0) , popup_addition_op );
}


void register_popup_msg_func( struct MsgAttr* m )
{
	if(  (0 != get_messageid_priority( m->msgid )) && ( NULL == m->MsgFunc ) ){
		m->MsgFunc = do_popup_msg_entry;  
		///printf("%s data size:%d\n", m->name , m->attr_cnt );
	}
}
