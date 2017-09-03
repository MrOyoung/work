#include "works.h"
#include <string.h>
#include <unistd.h>
#include "util.h"
#include <sys/time.h>
#include "Message_Def.h"
#include "Module_Def.h"
#include "down_table.h"
#include "common.h"
#include "debug.h"


#ifdef UART_DOWN_DEBUG
int catch_down_debug_print = 0;
#endif

#ifdef UART_UP_DEBUG
int catch_up_count = 0;
int catch_up_size = 0;

int catch_up_debug_print = 0;
#endif


volatile int uart_read_run = 0;
volatile int ipc_run = 0;
volatile int uart_write_run = 0;

void stop_all_thread()
{
   uart_read_run = 0;
   ipc_run = 0;
   uart_write_run = 0;
}


void set_all_thread_run()
{
   uart_read_run = 1;
   ipc_run = 1;
   uart_write_run = 1;
}


static struct cycle_buffer cycle;

void init_buffer()
{
   /*在这里会强制检测环形缓冲区的大小，大小必须为2的n次方如4096*/
   ASSERT( ( (sizeof(cycle.buf) - 1) & sizeof(cycle.buf) ) == 0 );

   cycle.rp = 0;
   cycle.wp = 0;
   cycle.size = 0;
   sem_init( &cycle.sem  , 0, 1);
}

int add(int a , int b)
{
	return a + b;
}


const static word crc16_half_byte[16] =
{
   0x0000, 0x1021, 0x2042, 0x3063,
   0x4084, 0x50a5, 0x60c6, 0x70e7,
   0x8108, 0x9129, 0xa14a, 0xb16b,
   0xc18c, 0xd1ad, 0xe1ce, 0xf1ef
};

static int do_crc( int pos , int len )
{
   word samplecrc;
   word crc;
   word da;

   samplecrc = ((cycle.buf[__round_add(pos, 2)]) << 0) |
               ((cycle.buf[__round_add(pos, 3)]) << 8);

   crc = 0;
   pos = __round_add(pos , 4);
   len = len - 4 - 1;

   while(len-- != 0)
   {
      da = ((byte)(crc / 256)) / 16;
      crc <<= 4;
      crc ^= crc16_half_byte[da ^ (cycle.buf[pos] / 16)];
      da = ((byte)(crc / 256) / 16);
      crc <<= 4;
      crc ^= crc16_half_byte[da ^ (cycle.buf[pos] & 0x0f)];
      pos = __round_add(pos, 1);
   }


   return samplecrc == crc;
}

typedef word U16;
typedef byte U8;

static U16 api_cal_crc16(U8 *ptr, U8 len)
{
   U8  da = 0;
   U16     crc = 0;
   while(len-- != 0)
   {
      da = ((U8)(crc / 256)) / 16;
      crc <<= 4;
      crc ^= crc16_half_byte[da ^ (*ptr / 16)];
      da = ((U8)(crc / 256) / 16);
      crc <<= 4;
      crc ^= crc16_half_byte[da ^ (*ptr & 0x0f)];
      ptr++;
   }
   return(crc);
}




static int do_error()
{
   int error_nr = errno;
   LOG("uart error:%d\n", error_nr );

   switch( error_nr )
   {
   case    EAGAIN:
      return 0;

      //       case    EWOULDBLOCK:
   case    EBADF:
   case    EFAULT :
   case    EINTR  :
   case    EINVAL :
   case    EIO    :
   case    EISDIR :
      return -1;
   default:
      return -1;
   }
   return 0;
}


void *ipc_work_thread(void *);
void *uart_write_thread(void *args);
void *uart_read_thread(void *args)
{
   int fd;
   int linear_sz;
   int ret;

   fd = *(int*)args;

   int no_uart_data_count = 0;


   UART_LOG( "start uart read thread\n" );

   while(uart_read_run)
   {

      sem_wait(&cycle.sem);/*获取信号量*/
      if( BUFFER_LEN == cycle.size )//数据已满
         goto __loop_once_end;

      /*计算连续的缓冲区尺寸*/
      if( cycle.wp >= cycle.rp )
         linear_sz = BUFFER_LEN - cycle.wp;
      else
         linear_sz = cycle.rp - cycle.wp;


      ret = read( fd , cycle.buf + cycle.wp , linear_sz );
      if( ret <= 0 ) /*错误处理机制*/
      {

         if( 0 == ret )
         {
            no_uart_data_count++;
            LOG("uart recv 0 byte cnt:%d\n" , no_uart_data_count );
         }

         if( (no_uart_data_count > 10) || ((ret < 0) && (do_error() < 0)) )
         {
            sem_post( &cycle.sem );//该线程需要退出，需先释放信号量
            stop_all_thread();
            LOG("uart error!!!\n");
            pthread_exit( (void *)ERROR_UART_READ ); /*uart通信发生错误*/
         }
         goto __loop_once_end;
      }

      no_uart_data_count = 0;

#ifdef UART_UP_DEBUG
      if( catch_up_debug_print )
         debug_hex_array( "recv from uart" , cycle.buf + cycle.wp ,  ret );

      catch_up_size += ret;
      if( (++catch_up_count) == 10 )
      {
         LOG("read uart byte size:%d\n" , catch_up_size );
         catch_up_size = 0;
         catch_up_count = 0;
      }
#endif

      __buffer_data_add_mod1( &cycle , ret  );
__loop_once_end:
      sem_post( &cycle.sem );/*释放信号量*/
      usleep( 20 * 1000 );
   }

   return 0;
}


#define MIN_FRAME_LEN (83+8+1) /*最小帧长度 为何是92 ???*/

int is_cycle_buffer_has_frame(int *head, int *length)
{
   int total;
   int temp;

   if( cycle.size < MIN_FRAME_LEN ) /*不足最小帧尺寸 则为无帧*/
      return 0;

   //1.find frame head... 帧头0xAA
__label_find_header:
   while(  0xaa != cycle.buf[cycle.rp] )
   {
      if( cycle.size < MIN_FRAME_LEN )
         return 0;

      __round_add_mod1( cycle.rp, 1 ); /*读指针++ (cycle.rp) = (((cycle.rp))+(1))&(4096-1)*/
      cycle.size--; /*尺寸--*/
   }

   //2.find frame size
   temp = __round_add(cycle.rp, 1);
   total = cycle.buf[temp] + 1 + 8;
   if( cycle.size < total )
      return 0;

   //3.find tail position
   temp =  __round_add( cycle.rp, (total - 1) );
   //帧尾 is 0x55 
   if( 0x55 != cycle.buf[temp] )
   {
      __round_add_mod1( cycle.rp , 1  );
      cycle.size--;
      goto __label_find_header;
   }

#ifndef NO_CRC_CHECK
   if( !do_crc( cycle.rp, total ) ) //4.if crc is wrong drop this frame...
   {
      __round_add_mod1( cycle.rp , total );
      cycle.size -= total;
      goto __label_find_header;
   }
#endif

   *head = cycle.rp;
   *length = total;
   return 1;
}


int move_buffer( int head, int length )
{
   /*前半部分数据大小*/
   int headhalf = BUFFER_LEN - head;
   /*后半部分大小*/
   int tailhalf = length - headhalf;

   /*
    *如果前半部分 > 后半部分，则将后半部分拷贝到缓冲区末尾，
    *超出缓冲区后的位置有64字节的填充部分，所以不会发送访问出错
    */
   if( headhalf > tailhalf )
   {
      memcpy( cycle.buf + BUFFER_LEN , cycle.buf , tailhalf );
      return head;

      /*
       *如果后半部分 > 前半部分，则将前半部分拷贝至缓冲区头部之前
       *超出缓冲区前部的位置有64字节的填充部分，所以不会发送访问出错
       */
   }
   else
   {
      memcpy( cycle.buf - headhalf , cycle.buf + head , headhalf );
      return -headhalf;
   }
}




static int msgid_up_table[] = { IPC_UART_DATA_GENERAL_UP , IPC_UART_DATA_SPECIAL_UP };

void *ipc_work_thread(void *args)
{
   int head , h , length , l;
   int magic;
   int msgid;

   DBusConnection *dbus = (DBusConnection *)args;
   unsigned long dbuscost;

__wait_for_data:
   if( !ipc_run )/*其他线程通知该线程需要退出*/
      return 0;

   sem_wait( &cycle.sem );/*获取信号量*/

   while( is_cycle_buffer_has_frame( &head , &length )  )
   {
	   LOG("%s->UART Thread running ~~~\n", __func__);
      /*帧头magic*/
      magic =	( (cycle.buf[__round_add(head , 4)]) << 0 ) |
               ( (cycle.buf[__round_add(head , 5)]) << 8 ) |
               ( (cycle.buf[__round_add(head , 6)]) << 16) |
               ( (cycle.buf[__round_add(head , 7)]) << 24);

      //分别处理各种情况 实际只处理了上行帧
      switch( magic )
      {
      case 0x01:
      case 0x03: /*1 3是上行帧 通用上行和特殊上行帧 统一处理 未作区分*/
         /*将UART协议中的桢magic值，转换为ipc通信中的msgid*/
         msgid = msgid_up_table[ (magic >> 1) ];/*magic不是1就是3,然后移位之后就会变成0或者1,对应表中的id号*/

         h = __round_add( head , 8 );/*去除头部的8个字节后的在缓冲区中的位置*/
         l = length - 8 - 1;/*去除帧头8个字节，1字节帧尾*/

         /*如果发送的缓冲区一半在缓冲区后面，一半在缓冲区头部，移动缓冲区中较少的一部分*/
         if( (h + l) > BUFFER_LEN )
            h = move_buffer( h , l );

         /*替换掉msgid*/
         memcpy( cycle.buf + h - sizeof(int) , &msgid , sizeof(int) );

         /*发送*/
         dbus_send( 	dbus ,
                     cycle.buf + h - sizeof(int) ,
                     l + sizeof(int) ,
                     MODULE_SERVICE_DI_DATA_ANALISIS_PATH ,
                     MODULE_IPC_UART_DATA_INTERFACE );
         break;

      default:
         LOG("error magic:%d\n", magic);
         break;
      }

      /*更新缓冲区*/
      cycle.rp =  __round_add( head , length  );
      cycle.size -= length;
   }
   sem_post(&cycle.sem);/*释放信号量*/

   usleep(20 * 1000); /*20ms 执行周期*/
   goto __wait_for_data;

   return 0;
}




/*申明一个缓冲区，包含一个通用下行帧和一个专用下行帧*/
#define DECLARE_DOWN_FRAME(name) \
    volatile byte padding_sxmlidahluehsjnxgakajsda_##name[4];\
byte name[] = {\
    /*通用帧*/\
    0xaa , 8 , 0x00 , 0x00 , 0x02 , 0x00 , 0x00 ,  0x00/*通用*/ ,    /*帧头*/\
    0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 ,0x00 , /*8个字节*/\
    0x55, /*帧位*/\
    /*专用帧*/\
    0xaa ,  12 , 0x00 , 0x00 , 0x04 , 0x00 , 0x00 , 0x00/*专用*/ ,/*帧头*/\
    0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 ,0x00 ,0x00 ,0x00 , 0x00, 0x00,/*专用帧12个字节*/\
    0x55 \
}

#define GENERAL_DOWN_LEN (8+8+1)
#define SPECIAL_DOWN_LEN (8+12+1)
#define CRC_DOWN_FRAME( down ) \
    do{ \
        U16 crc;\
        /*计算通用下行帧crc*/ \
        crc = api_cal_crc16( down + 4 , GENERAL_DOWN_LEN - 4 - 1 ); \
        down[2] = (crc >> 0) & 0xff; \
        down[3] = (crc >> 8) & 0xff; \
        /*计算特殊下行帧crc*/ \
        crc = api_cal_crc16( down + GENERAL_DOWN_LEN + 4 ,  SPECIAL_DOWN_LEN - 4 -1  ); \
        down[2 + GENERAL_DOWN_LEN] = (crc >> 0) & 0xff; \
        down[3 + GENERAL_DOWN_LEN] = (crc >> 8) & 0xff;  \
    }while(0);


void __write_uart( int fd , byte *frame , int size )
{
   /*向uart发送数据*/
   if( 0 > write(fd , frame , size ) )
   {
      LOG("write uart error!!!\n");
      stop_all_thread();
      pthread_exit( (void *)ERROR_UART_WRITE  );
   }

   /*重置需要清空的数据*/
   if(queue_count > 0)
   {
      clear_reset_queue( frame + 8 , frame + GENERAL_DOWN_LEN + 8 );
      CRC_DOWN_FRAME( frame );
   }
}

void __heart_beat(int fd)
{
#define CONFIG_HEART_BEAT_INTERVAL 2

   static byte heart_beat_package[] =
   {
      0xaa , 1 , 0x85 , 0xcd , 0x06 , 0x00 , 0x00 ,  0x00, /*head*/
      0x00 , /*body*/
      0x55   /*tail*/
   };

   static int heart_beat_count = 10 * CONFIG_HEART_BEAT_INTERVAL + 10;

   /**/
   if( heart_beat_count++ <= (10 * CONFIG_HEART_BEAT_INTERVAL) )
      return;

   if( 0 > write( fd , heart_beat_package , sizeof(heart_beat_package) ) )
   {
      LOG("write heart beat error!!!\n");
      stop_all_thread();
      pthread_exit( (void *)ERROR_UART_WRITE  );
   }

   heart_beat_count = 0;
   LOG("heart beat\n");
}


void *uart_write_thread(void *args)
{
   DBusConnection *bus;
   int fd /*串口文件号*/, len;
   unsigned int message[8];/*接受消息的缓冲区*/
   unsigned long tick/*目标滴答数*/;

   int has_message = 0;

   /*取出主线程传入的数据*/
   struct thread_data *p = (struct thread_data *)args;
   fd = *(int *)p->data1;
   bus = (DBusConnection *)(p->data2);


   /*释放内存*/
   free( p );

   static DECLARE_DOWN_FRAME( down  );
   CRC_DOWN_FRAME( down );



   while( uart_write_run )
   {

      /*send heart beat package to mcu*/
      __heart_beat(fd);

      /*当前时间100毫秒后发送*/
      tick = get_tick_count() + 100;

      usleep( (95 * 1000) );
      while( get_tick_count() < tick )
      {

         if( dbus_get_message_timeout(bus , message , &len , 0 )  >= 0 )
         {

            do_message_to_me( down + 8 , down + GENERAL_DOWN_LEN + 8 , message  );

            CRC_DOWN_FRAME( down );

            if( catch_down_debug_print )
            {
               debug_hex_array( "general down frame" , down , GENERAL_DOWN_LEN );
               debug_hex_array( "special down frame" , down + GENERAL_DOWN_LEN ,  sizeof(down) - GENERAL_DOWN_LEN );
            }

            has_message = 1;

         }
         else
         {
            usleep(500);
         }

      }


      if(has_message)
         __write_uart( fd , down , sizeof(down) );

      has_message = 0;

   }
   return 0;
}
