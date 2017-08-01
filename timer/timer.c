#include "list.h"
#include "timer.h"
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

/*一次枚举每一个列表项*/
#define list_for_each_timer_entry(__it,__head)          list_for_each_entry(__it,__head,node)

/*安全的枚举每一个列表项*/
#define list_for_each_timer_entry_safe(__it,__s,__head) list_for_each_entry_safe(__it,__s,__head,node)

/*将该列表项转换为一个timer_entry结构体*/
#define to_timer_entry(__p)                             container_of(__p,struct timer_entry,node);


/*
 *定时器数据结构
 */
struct timer_entry{

    /*定时器链表项*/
    struct list_head node;

    /*超时时间*/
    unsigned long timeout;

    /*定时器标志*/
    int   flag;

    /*定时器处理函数*/
    timer_handler handler;

    /*自定义数据1*/
    void* data1;

    /*自定义数据2*/
    void* data2;
};




/*静态申请的定时器数组*/
static struct timer_entry timer_pool[TIMERS_COUNT];

//#define LIST_HEAD(name) \
	struct list_head name = { &(name), &(name) }

/*管理定时器链表的头部*/
static LIST_HEAD(timer_pool_head);
//在timer_pool_head后挂上timer_pool的所有成员
__attribute((constructor)) void __timer_entry_pool_init(){
    int i;
    for( i = 0 ; i < TIMERS_COUNT ; i++ ){
        //list_add:在第二个参数节点和其之后的节点之间插入第一个参数节点
        list_add( &(timer_pool[i].node) , &timer_pool_head );
    }
}


/*
**功能:动态申请内存，定时器节点
**
**
**
**
*/
static inline struct timer_entry* __timer_entry_alloc()
{
    struct list_head* __r;
	
    if( list_empty(&timer_pool_head) )//timer_pool_head未定义？
        return NULL;
    
    __r = timer_pool_head.next;
    list_del( __r );

    return to_timer_entry(__r);
}

static inline void __timer_entry_free(struct timer_entry* timer)
{
   list_add( &(timer->node), &timer_pool_head ); 
}

static inline unsigned long get_tick_count(){
    struct timespec t;
    clock_gettime( CLOCK_MONOTONIC_COARSE , &t );
    return t.tv_sec * 1000 + t.tv_nsec / (1000*1000);
}

/*当前正在活动着的定时器链表*/
static LIST_HEAD(timer_list);

static inline void __add_timer_entry(struct timer_entry* entry)
{
    struct timer_entry* iter;
    /**/
    list_for_each_timer_entry(iter,&timer_list){
        if( (entry->timeout) <= (iter->timeout) )
            break;
    }
    
    list_add_tail( &(entry->node) , &(iter->node) );
}


/*
**功能描述:
	在定时器注册任务
**输入:
		handler:		回调函数
		elapse:			定时间隔
		data1/data2:	传入数据
**
**
**
**
**/
struct timer_entry* TIMER_register(timer_handler handler, /**/
                                   unsigned long elapse,
                                   void*         data1,
                                   void*         data2
                                   )
{
    struct timer_entry* entry;//创建节点，保存该定时任务信息，结构体定义见上文

    entry = __timer_entry_alloc();
    if(NULL == entry)
        return NULL;

    entry->timeout  = get_tick_count() + elapse;
    entry->handler  = handler;
    entry->data1    = data1;
    entry->data2    = data2;

    __add_timer_entry(entry);
    return entry;
}

static inline void __remove_timer_entry_free(struct timer_entry* entry)
{
	list_del( &(entry->node) );
	__timer_entry_free( entry );
}

/*注销一个定时器*/
void TIMER_unregister(struct timer_entry* entry)
{
    __remove_timer_entry_free(entry);
}


unsigned long TIMER_loop(void)
{
    unsigned long       next;
    unsigned long 		now;

    LIST_HEAD(reuse_list);
    struct timer_entry* iter;
    struct timer_entry* safe;
    
    /*获取当前时间*/
    now = get_tick_count();

    list_for_each_timer_entry_safe( iter , safe, &timer_list ){
        if( iter->timeout > now )
            break;

        /*执行定时器操作*/
        next = iter->handler( iter->data1 , iter->data2 );

        /*返回0则表示这个定时器不再执行*/
        if( 0 == next ){
            __remove_timer_entry_free(iter);

        } else{
            /*更新定时器的下次超时时间*/
            iter->timeout = now + next;
            list_move( &(iter->node) , &reuse_list );
        }
    }

    /*将重复使用的定时器再次加入定时器链表之中*/
    list_for_each_timer_entry_safe( iter , safe , &reuse_list )
        __add_timer_entry( iter );
    
    if( list_empty(&timer_list) )
        return (~0);

    /*返回第一个要达到的定时器和当前时间的差*/
    iter = to_timer_entry(timer_list.next);
    return iter->timeout - now;
}
