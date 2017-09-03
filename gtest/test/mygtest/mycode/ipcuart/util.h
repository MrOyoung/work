#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
//#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>

#include "common.h"

DBusConnection *get_dbus(const char *well_known_name);

int dbus_add_match(DBusConnection *dbus, const char *rules );

int dbus_send(DBusConnection *dbus, void *array, int size, const char *path, const char *interface );

int dbus_send_noblocking( DBusConnection *dbus, void *array, int size , const char *path, char *interface   );

int dbus_get_message(DBusConnection *dbus, void *message, int *length);

int dbus_get_message_timeout(DBusConnection *dbus, void *message, int *length, int ms_timeout );




unsigned long get_tick_count();

#endif
