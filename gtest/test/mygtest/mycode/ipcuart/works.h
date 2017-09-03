#ifndef _WORKS_H_
#define _WORKS_H_

#include <semaphore.h>
#include <stdlib.h>
#include "common.h"
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
//#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>
#include <unistd.h>

#define ERROR_DBUS_WRITE (1<<0)
#define ERROR_DBUS_READ (1<<1)
#define ERROR_UART_WRITE (1<<2)
#define ERROR_UART_READ (1<<3)

struct thread_data
{
   void *data1;
   void *data2;
   void *data3;
   void *data4;
};

struct cycle_buffer
{
   /*缓冲区必须为2的n次方,这个缓冲区大小会在init_buffer函数中进行检测*/
#define BUFFER_LEN 4096
#define PADDING_LEN 64

   /*缓冲区前部的填充,volatile为防止编译器优化*/
   volatile byte fpadding[PADDING_LEN];

   /*缓冲区*/
   byte buf[BUFFER_LEN];

   /*缓冲区后部的填充，volatile为防止编译器优化*/
   volatile byte bpadding[PADDING_LEN];

   int rp;

   int wp;

   int size;

   sem_t sem;
};



/*
*宏后面有_modX(X表示会修改第几个参数)后缀的表明该宏会修改输入的参数
*_mod1表示会修改第一个参数,_mod2表示会修改第二个参数，以此类推...
*没有_mod后缀则表示不会修改传入的参数
*/

#define __round_add(p ,s) ((p)+(s))&(BUFFER_LEN-1)

#define __round_add_mod1(p ,s) \
    (p) = __round_add((p),s)

#define __buffer_data_add_mod1(buffer,sz) \
    __round_add_mod1( (buffer)->wp ,sz  );\
    (buffer)->size += sz;

#ifdef __cplusplus
extern "C"{
#endif

/*初始化缓冲区*/
void init_buffer();

/*接收数据的线程*/
extern void *uart_read_thread(void *args);

/*数据解析工作线程*/
extern void *ipc_work_thread(void *args);

extern void *uart_write_thread(void *args);

extern volatile int uart_read_run;
extern volatile int ipc_run;
extern volatile int uart_write_run;


int add(int a , int b);

void stop_all_thread();
void set_all_thread_run();

int move_buffer( int head, int length );

#ifdef __cplusplus
}
#endif

#endif
