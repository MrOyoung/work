/*
 * The MIT License (MIT)
 *
 * Copyright © 2014 faith
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */
#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>
#include <dbus/dbus-glib.h> 
#include <dbus/dbus.h>  
#include <unistd.h>
#include <stdbool.h>

#include "common.h"
/*======================================
	Header include
======================================*/

/*======================================
	Prototype
======================================*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void util_show_fps(void);
DBusConnection* get_dbus(const char* well_known_name);

int dbus_add_match(DBusConnection* dbus, const char* rules );

int dbus_send(DBusConnection* dbus,void* array,int size,const char* path,const char*interface );

int dbus_send_noblocking( DBusConnection* dbus, void* array,int size , const char* path, char*interface   );

int dbus_get_message(DBusConnection* dbus, void* message, int* length);

int dbus_get_message_timeout(DBusConnection* dbus, void* message, int* length, int ms_timeout );

void debug_hex_array(const char* name , const byte* array , int size );

pthread_t dbus_init();
void dbus_join();
bool is_running();

unsigned long get_tick_count();


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _UTIL_H */
