#include "uart.h"
#include "stdio.h"
#include "stdlib.h"
#include "Message_Def.h"
#include <unistd.h>
#include "pthread.h"
#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>
#include "works.h"
#include "util.h"
#include "debug.h"
#include <signal.h>

#define dbus_error(ret) (unsigned int)(ret)|(ERROR_DBUS_WRITE|ERROR_DBUS_READ)
#define uart_error(ret) (unsigned int)(ret)|(ERROR_UART_WRITE|ERROR_UART_READ)



#ifdef UART_DOWN_DEBUG
extern int catch_down_debug_print;
#endif

#ifdef UART_UP_DEBUG
extern int catch_up_debug_print;
#endif

int main_bak(int argc, char **argv)
{
   pthread_t thread_read;
   pthread_t thread_write;
   pthread_t thread_ipc;

   void *ret_read;
   void *ret_write;
   void *ret_ipc;

   int fd;
   struct thread_data *thread_data_ptr;
   int ret;
   DBusConnection *bus;

   /*初始化日志系统*/
   DEBUG_LOG_INIT();
   LOG("usage:%s [1/0](print up uart data) [1/0](print down uart data)\n", argv[0] );

   /*入口参数处理*/
#ifdef UART_UP_DEBUG
   if( argc >= 2 )
      catch_up_debug_print = atoi( argv[1] );
#endif

#ifdef UART_DOWN_DEBUG
   if( argc >= 3 )
      catch_down_debug_print = atoi( argv[2] );
#endif

   /*初始化环形缓冲区*/
   init_buffer();

   /*链接dbus总线*/
   if( !(bus = get_dbus("com.saic.IpcUartData"))  )
   {
      LOG("dbus get error!!!\n");
      return 1;
   }

   if( dbus_add_match(bus , "type='signal',path='/com/saic/IpcUartData'")  < 0 )
   {
      LOG( "dbus add match error!!!\n");
      return 1;
   }

   /*打开串口*/
__open_uart:
   set_all_thread_run();

   UART_LOG( "begin open uart\n" );
   fd = open_uart( "/dev/ttymxc1" );
   if( fd < 0 )
   {
      LOG( "open_uart error\n" );
      return 1;
   }
   UART_LOG( "open uart ok\n" );

   /*应用程序主循环*/
   while(1)
   {
      thread_data_ptr = (struct thread_data *)malloc( sizeof(struct thread_data) );
      if(!thread_data_ptr)
      {
         LOG( "malloc error!!!\n" );
         return 1;
      }

      ret = pthread_create( &thread_read , NULL , uart_read_thread , (void *)fd );
      if( ret < 0 )
      {
         LOG( "create uart thread error!!\n" );
         return 1;
      }

      ret = pthread_create( &thread_ipc , NULL , ipc_work_thread , (void *)bus );
      if( ret < 0 )
      {
         LOG( "create ipc thread error!!!\n" );
         return 1;
      }


      thread_data_ptr->data1 = (void *)fd;
      thread_data_ptr->data2 = (void *)bus;
      /*该线程需要多个参数传入*/
      ret = pthread_create( &thread_write , NULL , uart_write_thread , (void *)thread_data_ptr );
      if( ret < 0 )
      {
         LOG( "create uart write thread error!!!\n" );
         return 1;
      }

      pthread_join( thread_write 	,	&ret_write 	);
      pthread_join( thread_read	,	&ret_read 	);
      pthread_join( thread_ipc 	, 	&ret_ipc 	);

      LOG("all thread exit\n");

      if( uart_error(ret_write)  || uart_error(ret_read) )
      {
         LOG("uart is error!!! will reopen uart and restart all work thread...\n");
         close( fd );
         goto __open_uart;
      }

   }

   return 0;
}

