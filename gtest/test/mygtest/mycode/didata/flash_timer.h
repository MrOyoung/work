#ifndef _FLASH_TIMER_H_
#define _FLASH_TIMER_H_

#include "util.h"
#include "timer.h"
#include "Table_Def.h"




int flash_timer_cancel( struct MsgAttr* msg );

int flash_timer_create( struct MsgAttr* msg , DBusConnection* dbus , int index );


#endif
