#include "popup_priority.h"
#include "pool_template.h"
#include "list.h"
#include "popup_on.h"
#include "timer.h"
#include "ticks.h"
#include "popup.h"
#include <stdio.h>


struct popup_entry{

	struct list_head node;		/*add in show list, if this is not empty , indicate this entry is showing or be be show*/
	struct list_head show_node;	/*add in all list, it must in all list, power off or power on*/

	unsigned long start_time;   /*start show time,if 0 ,this popup is not showing*/
	unsigned long rest_time;    /*remain show time*/

	int			  level;		/*priority A,B,C*/
	void*		  data;			/*user privaite data*/
	
	struct popup_operation* pop;/*show and hide action*/

	unsigned int  messageid;	/*this popup message id*/
	union{
			int   padding;		/*not use*/
		struct {
			short arise;		/*if 1 , this popup alread happen*/
			short sound;		/*if 1 , this popup has sound*/
		};
	};
};

/**/
POOL_TEMPLATE( popup_entry , 256 , popup_entry_alloc , popup_entry_free );

#define node_to_popup_entry(__p) container_of(__p,struct popup_entry,node)
#define show_to_popup_entry(__p) container_of(__p,struct popup_entry,show_node)
#define list_for_each_popup_entry_by_node(iter,head) list_for_each_entry(iter,head,node)
#define list_for_each_popup_entry_by_show_node(iter,head) list_for_each_entry(iter,head,show_node) 

#define TIMEOUT_VERY_LONG (unsigned long)(1000*60*60*8)
#define TIMEOUT_INFINITE  (unsigned long)(~0)

static const unsigned long g_timeouts[] = {
	0,					/*nothing*/
	1500,				/*priority C*/
	3000,				/*priority B*/
	TIMEOUT_VERY_LONG,	/*priority A*/
};

/*power mode*/
static int	g_power_mode = 0;

/*popup show list*/
static struct list_head g_popup_show_list[] = {
	LIST_HEAD_INIT( g_popup_show_list[0] ), /*power off*/
	LIST_HEAD_INIT( g_popup_show_list[1] ), /*power on*/
};

/*popup all list*/
static struct list_head g_popup_all_list[] = {
	LIST_HEAD_INIT( g_popup_all_list[0] ), /*power off*/
	LIST_HEAD_INIT( g_popup_all_list[1] ), /*power on*/
};

/*show list in current power mode*/
#define popup_show_list (g_popup_show_list[g_power_mode])

/*all list in current power mode*/
#define popup_all_list  (g_popup_all_list[g_power_mode])

/*timer handle*/
static struct timer_entry* timer = NULL;

/**/
static inline void popup_start_show(struct popup_entry* e)
{
	if( (e->pop) && (e->pop->show) )
	{
		e->pop->show( e->data );
	}
}


static inline void popup_finish_show(struct popup_entry* e)
{
	if( (e->pop) && (e->pop->hide) )
	{
		e->pop->hide( e->data );	
	}
}


void popup_show_timer_cleanup(void* data1 , void* data2)
{
	(void)data1;
	(void)data2;
	
	timer = NULL;
}


unsigned long popup_show_timer_handler( void* data1 , void* data2 , unsigned long now )
{
	(void)data1;
	(void)data2;
	
	struct popup_entry* e;

	/*no suspend popup show!!!*/
	if( list_empty( &popup_show_list ) )
		return 0;

	/*fetch first popup entry*/
	e	= show_to_popup_entry( popup_show_list.next );

	/*if start_time = 0, this popup is has not been shown*/
	if( 0 == e->start_time ){
		popup_start_show( e ); 
		/*set this popup show*/
		e->start_time = now;
		return e->rest_time;
	}

	/*if the show is timeout,we must remove this entry from show list*/
	if( now >= (e->start_time + e->rest_time) ){

		/*move it to all list*/
		list_del( &(e->show_node)  );

		/*stop show it*/
		popup_finish_show( e );

		/*if show list is empty,then turn the timer off */
		if( list_empty( &popup_show_list ) )
			return 0;
		else
			return 500;
	}

	/*never go here!!!*/
	///printf("popup timer handler error!!!\n");
	return (e->start_time + e->rest_time) - now;
}


static inline void func_add_to_show_list( struct popup_entry* e , struct list_head* head)
{
	struct popup_entry* iter = NULL;

	/*find a entry priority less or equal 'e'*/
	list_for_each_popup_entry_by_show_node( iter , head  ){
		if( e->level >= iter->level ){
			list_add_tail( &(e->show_node) , &(iter->show_node) );
			return;
		}
	}

	/**/
	list_add_tail( &(e->show_node) , head );
}


void __insert_popup_to_show_list(struct popup_entry* e)
{
	struct popup_entry* iter;
	/*'iter' is pointer to first one of show list*/
	iter = show_to_popup_entry( popup_show_list.next );

	/*if e's priority greater then iter's priority, and iter is showing*/
	if( (e->level >= iter->level) && (iter->start_time > 0) )
	{

		/*pause old popup show*/
		iter->rest_time = (iter->start_time + iter->rest_time) - get_tick_count();
		iter->start_time = 0;
		popup_finish_show( iter );

		/*add 'e' into show list,as first one*/
		list_add( &(e->show_node) , &popup_show_list );

		/*update timer*/
		TIMER_unregister( timer );
		timer = TIMER_register( 
						popup_show_timer_handler , 
						0,
						0,
						0,
						popup_show_timer_cleanup 
						);

		return;
	}
	
	/*&popup_show_list  */
	func_add_to_show_list( e , &popup_show_list );
}


static inline void func_insert_message_current_power_mode( struct popup_entry* e  )
{
	/*if show list is empty,add it ,and turn on timer*/
	if( list_empty( &popup_show_list ) ){
	
		list_add( &(e->show_node) , &popup_show_list );
		timer = TIMER_register( 
						popup_show_timer_handler,
						0,
						0,
						0,
						popup_show_timer_cleanup
					);
	}else{

		__insert_popup_to_show_list( e );
	}

	/*add this to all list*/
	list_add( &(e->node) , &popup_all_list );
}


int popup_insert_message(unsigned int messageid, void* data, struct popup_operation* op )
{
	int priority = 0;
	int power    = 0;
	
	/*check this popup is on?*/
	if( is_this_popup_on( messageid )  )
		return -1;

	struct popup_entry* e = NULL;
	e = popup_entry_alloc();
	if( !e )
		return -1;

	/*alloc and init*/
	node_init( &(e->node) );
	node_init( &(e->show_node) );
	
	/*fetch popup priority*/
	priority = get_messageid_priority( messageid );

	/*seting*/
	e->start_time= 0;
	e->messageid = messageid;
	e->arise     = 1;
	e->sound	 = 1;
	e->data      = data;
	e->pop		 = op;
	e->level	 = priority & 0xf;
	e->rest_time = g_timeouts[e->level];
	
	/*get this message show in which power mode*/
	power = (priority >> 4) & 0x01;
	
	/**/
	if( power == g_power_mode )
	{
		func_insert_message_current_power_mode(  e  );
	}
	else
	{
		/*add to show list*/
		func_add_to_show_list( e , &g_popup_show_list[power] );
		/*add to all list*/
		list_add( &(e->node) ,     &g_popup_all_list [power] );
	}
	
	set_this_popup_on( messageid );
	return 0;
}


static inline struct popup_entry* func_find_entry_from_all_list( unsigned int messageid , struct list_head* all_head )
{
	struct popup_entry* iter = NULL;
	/*find this popup message in all list,it has showed*/
	list_for_each_popup_entry_by_node( iter , all_head )
	{
		if( messageid == iter->messageid )
		{
			return iter;
		}
	}
	
	return NULL;
}


static inline void func_delete_message_current_power_mode(unsigned int messageid)
{
	struct popup_entry* iter = NULL;
	/*find this popup message in all list,it has showed*/
	iter = func_find_entry_from_all_list( messageid , &popup_all_list );

	/*if this popup entry is in show list,mean it is showing or to be show*/
	if( !node_empty( &(iter->show_node) )  )
	{
		/*remove it from show list*/
		list_del( &iter->show_node );

		/*if this message is showing???*/
		if( iter->start_time  )
		{
			/*stop show it*/
			popup_finish_show( iter );

			/*shutdown timer*/
			TIMER_unregister( timer );

			/*if show list is not empty,you must turn on popup timer*/
			if( !list_empty(&popup_show_list ) )
				timer = TIMER_register( 
						popup_show_timer_handler , 
						0,
						0,
						0,
						popup_show_timer_cleanup 
						);
		}
	}

	/*remove from all list*/
	list_del( &iter->node );
	popup_entry_free( iter );
}


int popup_delete_message(unsigned int messageid)
{
	int power = 0;

	if( !is_this_popup_on(messageid) )
		return -1;
	
	/*this popup show in which power mode??*/
	power = ( get_messageid_priority( messageid ) >> 4) & 0x01;
	
	/*it is in current power mode???*/
	if( power == g_power_mode )
	{
		func_delete_message_current_power_mode(messageid);
	}
	/*in other power mode???*/
	else
	{
		struct popup_entry* iter;
		/*find this popup message in all list*/
	    iter = func_find_entry_from_all_list( messageid , &g_popup_all_list[power] );
		
		/*delete from all list*/
		list_del( &iter->node );
		
		/*if this node in show list , remove it*/
		if( !node_empty( &iter->show_node) )
		{
			list_del( &iter->show_node );
		}

		/*free this popup entry*/
		popup_entry_free( iter );
	}

	set_this_popup_off( messageid );
	
	return 0;
}



static void func_disable_current_power_mode_popup_show()
{
	struct popup_entry* iter = NULL;
	
	/*if show list is empty, do nothing!!!*/
	if( list_empty( &popup_show_list ) )
	{
		return;
	}

	/*'iter' is pointer to first one of show list*/
	iter = show_to_popup_entry( popup_show_list.next );

	/*if 'iter' is showing*/
	if(  iter->start_time > 0 )
	{
		/*pause it's show*/
		iter->rest_time = (iter->start_time + iter->rest_time) - get_tick_count();
		iter->start_time = 0;
		popup_finish_show( iter );
	}

	/*shutdown popup show timer*/
	if( timer )
	{
		TIMER_unregister( timer );
	}
}


static void func_enable_current_power_mode_popup_show()
{
	if( !list_empty( &popup_show_list ) )
	{
		timer = TIMER_register( 
						popup_show_timer_handler , 
						0,
						0,
						0,
						popup_show_timer_cleanup 
						);
	}
}


void popup_set_power_mode(int mode)
{
	/*if same power mode, do nothing!!!*/
	if( mode == g_power_mode )
		return;

	/*disable current power mode'popup show*/
	func_disable_current_power_mode_popup_show();

	/*change power mode*/
	g_power_mode = mode;

	/*enable current power mode's popup show*/
	func_enable_current_power_mode_popup_show();
}

void popup_clear_list()
{
	struct popup_entry* iter = NULL;
	struct popup_entry* safe = NULL;

	if( !list_empty( &popup_show_list ) )
	{
		/*'iter' is pointer to first one of show list*/
		iter = show_to_popup_entry( popup_show_list.next );

		/*if 'iter' is showing*/
		if(  iter->start_time > 0 )
		{
			popup_finish_show( iter );
		}
	}

	/*shutdown popup show timer*/
	if( timer )
	{
		TIMER_unregister( timer );
	}

	/*clear show list*/
	list_init( &g_popup_show_list[0] );
	list_init( &g_popup_show_list[1] );

	/*free entry in all list*/
	list_for_each_entry_safe( iter , safe , &g_popup_all_list[0] , node )
	{
		set_this_popup_off( iter->messageid );
		//list_del( &iter->node );
		popup_entry_free( iter );
	}

	list_for_each_entry_safe( iter , safe , &g_popup_all_list[1] , node )
	{
		set_this_popup_off( iter->messageid );
		//list_del( &iter->node );
		popup_entry_free( iter );
	}
	
	list_init( &g_popup_all_list[0] );
	list_init( &g_popup_all_list[1] );
}


///void debug_print_all( void (*handler)(void*,int)  )
///{
///	struct popup_entry* iter;
///	list_for_each_popup_entry_by_node( iter , &popup_all_list  ){
///		handler( iter->data , iter->level );
///	}
///}
