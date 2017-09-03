#include "util.h"
#include <string.h>
#include <time.h>
#include "common.h"


DBusConnection *get_dbus(const char *well_known_name)
{
   int ret;
   DBusError err;
   DBusConnection *connection;

   dbus_error_init(&err);

   connection = dbus_bus_get(DBUS_BUS_SESSION , &err );
   if(dbus_error_is_set(&err))
   {
      fprintf(stderr, "ConnectionErr : %s\n", err.message);
      dbus_error_free(&err);
      return NULL;
   }
   if(connection == NULL)
      return NULL;

   ret = dbus_bus_request_name(connection, well_known_name , DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
   if(dbus_error_is_set(&err))
   {
      fprintf(stderr, "Name Err :%s\n", err.message);
      dbus_error_free(&err);
      goto __failed_get;
   }

   if(ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
      goto __failed_get;

   return connection;

__failed_get:
   dbus_connection_unref( connection );
   return NULL;
}



int dbus_send(DBusConnection *dbus, void *array, int size, const char *path, const char *interface )
{
   int ret = 0;
   int serial;
   void *v_ARRAY = array;
   DBusMessage *msg = dbus_message_new_signal( path , interface , "x");


   if( ! dbus_message_append_args(msg, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &v_ARRAY, size , DBUS_TYPE_INVALID) )
   {
      ret = -1;
      goto __unref_msg;
   }

   if( !dbus_connection_send(dbus, msg, &serial))
   {
      fprintf(stderr, "Out of Memory!\n");
      ret = -1;
      goto __unref_msg;
   }

   dbus_connection_flush(dbus);

__unref_msg:
   dbus_message_unref( msg );
   return ret;
}

int dbus_send_noblocking( DBusConnection *dbus, void *array, int size , const char *path, char *interface   )
{
   int ret = 0;
   int serial;
   void *v_ARRAY = array;

   DBusMessage *msg = dbus_message_new_signal( path , interface , "data");
   if( !msg )
   {
      printf("dbus_message_new_signal error\n");
      return -1;
   }


   if( ! dbus_message_append_args(msg, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &v_ARRAY, size , DBUS_TYPE_INVALID) )
   {
      ret = -1;
      printf("dbus_message_append_args error\n");
      goto __unref_msg;
   }

   if( !dbus_connection_send(dbus, msg, &serial))
   {
      printf("dbus_connection_send error\n");
      ret = -1;
      goto __unref_msg;
   }

__unref_msg:
   dbus_message_unref( msg );
   return ret;

}


int dbus_get_message_timeout(DBusConnection *dbus, void *message, int *length, int ms_timeout )
{
   DBusMessage *msg;
   int ret = 0;

   void *array;
   int len;

   DBusError err;
   dbus_error_init(&err);

   if( FALSE == dbus_connection_read_write(dbus, ms_timeout) )
      return -1;

   msg = dbus_connection_pop_message (dbus);
   if( !msg )
      return -1;

   if( DBUS_MESSAGE_TYPE_SIGNAL != dbus_message_get_type(msg) )
   {
      ret = -1;
      goto __unref_msg;
   }

   if(  FALSE ==
         dbus_message_get_args( msg, &err , DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &array, &len, DBUS_TYPE_INVALID  ) )
   {
      dbus_error_free( &err );
      ret = -1;
      goto __unref_msg;
   }

   *length = len;
   memcpy( message , array , len );


__unref_msg:
   dbus_message_unref(msg);
   return ret;
}


int dbus_get_message(DBusConnection *dbus, void *message, int *length)
{
   DBusMessage *msg;
   int ret = 0;

   void *array;
   int len;

   DBusError err;
   dbus_error_init(&err);

   if( FALSE == dbus_connection_read_write(dbus, 0) )
      return -1;

   msg = dbus_connection_pop_message (dbus);
   if( !msg )
      return -1;

   if( DBUS_MESSAGE_TYPE_SIGNAL != dbus_message_get_type(msg) )
   {
      ret = -1;
      goto __unref_msg;
   }

   if(  FALSE ==
         dbus_message_get_args( msg, &err , DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &array, &len, DBUS_TYPE_INVALID  ) )
   {
      dbus_error_free( &err );
      ret = -1;
      goto __unref_msg;
   }

   *length = len;
   memcpy( message , array , len );


__unref_msg:
   dbus_message_unref(msg);
   return ret;
}


int dbus_add_match(DBusConnection *dbus, const char *rules )
{
   DBusError err;
   dbus_error_init(&err);
   dbus_bus_add_match(dbus, rules , &err);
   if(dbus_error_is_set(&err))
   {
      fprintf(stderr, "Match Error%s\n", err.message);
      dbus_error_free(&err);
      return -1;
   }

   dbus_connection_flush(dbus);
   return 0;
}

#if 1
unsigned long get_tick_count()
{
   struct timespec ts;
//   clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
   clock_gettime(CLOCK_MONOTONIC, &ts);
   

   return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

#endif


