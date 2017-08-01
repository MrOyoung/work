#ifndef _TIMER_H_
#define _TIMER_H_


/*最大支持多少个定时器同时工作*/
#ifndef TIMERS_COUNT
#define TIMERS_COUNT 64
#endif

struct timer_entry;

/*
 *@定时器回调函数
 *@return value：返回0则表示该定时器不再执行,返回其他值则为下一次超时累加的时间
 *@ticks:当前时间
 *@data1:用户自定义数据1(对应struct timer_entry中的data1成员)
 *@data2:用户自定义数据2(对应struct timer_entry中的data2成员)
 */
typedef unsigned long (*timer_handler)(void* data1 ,void* data2);

/*
 *
 * 定时器循环
 * 应用程序需要每隔一段时间调用该函数，从而可以执行可以执行注册的定时器函数
 *
 */
unsigned long TIMER_loop(void);

/*
 *
 * 注销一个定时器
 * 
 *
 */
void TIMER_unregister(struct timer_entry* entry);

/*
 *
 * 注册一个定时器操作
 *
 *
 */
struct timer_entry* TIMER_register(timer_handler handler, /**/
                                   unsigned long elapse,
                                   void*         data1,
                                   void*         data2
                                   );


#endif
