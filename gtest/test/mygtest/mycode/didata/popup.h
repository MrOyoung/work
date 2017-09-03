#ifndef _POPUP_H_
#define _POPUP_H_

#include "popup_priority.h"


/*
 *
 *
 *
 */
struct popup_entry;


/*
 *
 *
 */
struct popup_operation{
	void (*show)(void*);
	void (*hide)(void*);
};


/*
 *
 *
 */
int popup_delete_message(unsigned int messageid);

/*
 *
 *
 */
int popup_insert_message(unsigned int messageid, void* data , struct popup_operation* pop );

/*
 *
 *
 */
void popup_set_power_mode(int mode);

/*
 *
 *
 */
void popup_clear_list();

/*
 *
 *
 */
///void debug_print_all( void (*handler)(void*,int)  );


#endif


