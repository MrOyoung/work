#ifndef _DOWN_TABLE_H_
#define _DOWN_TABLE_H_
#include "common.h"
#include "Message_Def.h"
#include "debug.h"

struct down_data
{
   const char *name;
   unsigned int messageid;
   unsigned int byteindex;
   unsigned int bitmask;
   unsigned int bitshift;

   unsigned int reset;
};

#define RESET_SIZE 64
unsigned int reset_queue[RESET_SIZE];
unsigned int queue_count = 0;

/*messageid号，数据段字节序列，多少位长度，从第几位开始*/
#define DOWN_ENTRY(id,index,bitlength,shift) [id&0xffff] = { #id ,id,index,MASK_##bitlength,shift,0}
#define DOWN_ENTRY_RESET(id,index,bitlength,shift) [id&0xffff] = { #id ,id,index,MASK_##bitlength,shift,1}



const struct down_data app_table[] =
{
   /*general down frame*/
   DOWN_ENTRY_RESET( APP_CLUSTER_TRIPMETERA_CLEAR, 0, 1, 2),
   DOWN_ENTRY_RESET( APP_CLUSTER_AVERAGE_SPEEDA_CLEAR, 0, 1, 3),
   DOWN_ENTRY_RESET( APP_CLUSTER_AVERAGE_FUELA_CLEAR, 0, 1, 4),
   DOWN_ENTRY_RESET( APP_CLUSTER_BACKLIGHT_SET, 0, 2, 6),
   DOWN_ENTRY_RESET( APP_CLUSTER_TRIPMETERC_CLEAR, 1, 1, 0),
   DOWN_ENTRY_RESET( APP_CLUSTER_AVERAGE_SPEEDC_CLEAR, 1, 1, 1),
   DOWN_ENTRY_RESET( APP_CLUSTER_AVERAGE_FUELC_CLEAR, 1, 1, 2),

   /**/
   DOWN_ENTRY_RESET( APP_CLUSTER_TIME_SET, 0, 32, 0),

   /*special down frame*/
   DOWN_ENTRY_RESET( APP_CLUSTER_TRAVEL_TIMEA_CLEAR, 0, 1, 4),
   DOWN_ENTRY_RESET( APP_CLUSTER_TRAVEL_TIMEC_CLEAR, 0, 1, 5),
   DOWN_ENTRY( APP_CLUSTER_TOPIC_SET, 2, 1, 5),
   DOWN_ENTRY( APP_CLUSTER_TIMESTD_SET, 2, 1, 6),
   DOWN_ENTRY( APP_CLUSTER_TIMEFORMAT_SET, 2, 1, 7),
   DOWN_ENTRY( APP_CLUSTER_SPEEDLIMIT_SET, 3, 8, 0),
   DOWN_ENTRY( APP_CLUSTER_LANGUAGE_SET, 4, 1, 0),
   DOWN_ENTRY( APP_CLUSTER_ODOUNIT_SET, 4, 1, 1),
   DOWN_ENTRY( APP_CLUSTER_TEMPUNIT_SET, 4, 1, 2),
   DOWN_ENTRY( APP_CLUSTER_FUELUNIT_SET, 4, 2, 3),
   DOWN_ENTRY( APP_CLUSTER_TABINDEX_CHG, 4, 3, 5),
   DOWN_ENTRY_RESET( APP_CLUSTER_NEXT_MAINTEN_DIST, 5, 16, 0),
   //  DOWN_ENTRY_RESET( APP_CLUSTER_NEXT_MAINTEN_DATE,7,32,0),
   //   DOWN_ENTRY( APP_CLUSTER_POPUP_CHIME,11,8,0)
};


const struct down_data audio_table[] =
{
   DOWN_ENTRY( SERVICE_AUDIOMGR_CHIME, 11, 8, 0)
};


const int app_table_size = array_size( app_table );
const int audio_table_size = array_size( audio_table );


void add_to_reset_queue(unsigned int id)
{
   if( queue_count < RESET_SIZE )
      reset_queue[queue_count++] = id;
   else
      LOG("too more message\n");
}

void update_frame(byte *p, unsigned int data, const struct down_data *table )
{
   int bi;
   unsigned int temp;

   bi = table->byteindex;

   temp =  ( (p[bi + 0]) << 0) |
           ( (p[bi + 1]) << 8) |
           ( (p[bi + 2]) << 16) |
           ( (p[bi + 3]) << 24);

   temp = temp & (~ ( (table->bitmask) << (table->bitshift) ));
   temp |= ( ((table->bitmask) & data) << (table->bitshift) );

   p[bi + 0] = ((temp >> 0) & 0xff);
   p[bi + 1] = ((temp >> 8) & 0xff);
   p[bi + 2] = ((temp >> 16) & 0xff);
   p[bi + 3] = ((temp >> 24) & 0xff);
}

void do_message_to_me(byte *g_ptr , byte *s_ptr , unsigned int *message );


void clear_down_buffer(byte *g_ptr , byte *s_ptr , unsigned int  id );

void clear_reset_queue( byte *g_ptr, byte *s_ptr )
{
   int i;

   for( i = 0 ; i < queue_count ; i++ )
      clear_down_buffer( g_ptr , s_ptr , reset_queue[i] );

   queue_count = 0;
}




void do_message_to_me(byte *g_ptr , byte *s_ptr , unsigned int *message )
{
   int id = message[0];
   int data = message[1];
   int i = (message[0]) & 0xffff;

   /*特殊情况，单独处理*/
   switch( id )
   {
   case APP_CLUSTER_TIME_SET:/*特殊处理时间*/
   {
      unsigned short *p = (unsigned short *)(message + 1);
      LOG("recv APP_CLUSTER_TIME_SET:sec:%hu-min:%hu-hour:%hu-day:%hu-mouth:%hu-year:%hu\n",
          p[0],/*seconds*/
          p[1],/*minutes*/
          p[2],/*hours*/
          p[3],/*days*/
          p[4],/*mouths*/
          p[5]/*years*/
         );

      g_ptr[2] = (byte)p[0];
      g_ptr[3] = (byte)p[1];
      g_ptr[4] = (byte)p[2];
      g_ptr[5] = (byte)p[3];
      g_ptr[6] = (byte)p[4];
      g_ptr[7] = (byte)p[5];

      add_to_reset_queue( APP_CLUSTER_TIME_SET );
      return;/*look out!!!!!!!!!!!!!*/
   }

   case APP_CLUSTER_NEXT_MAINTEN_DATE:
   {
      unsigned short *p = (unsigned short *)(message + 1);
      LOG("recv APP_CLUSTER_NEXT_MAINTEN_DATE:year:%hu-month:%hu-day:%hu\n",
          p[0] , /*year*/
          p[1] , /*month*/
          p[2]   /*day*/
         );

      s_ptr[7] = (byte)p[0];/*year*/
      s_ptr[8] = (byte)p[1];/*month*/
      s_ptr[9] = (byte)p[2];/*day*/

      add_to_reset_queue( APP_CLUSTER_NEXT_MAINTEN_DATE );
      return;/*look out!!!!!!!!!!!!!*/
   }
   }

   /**/
   switch( id  >> 16 )
   {
   case MODULE_APP_CLUSTER: /*send from app*/
      if( i >= app_table_size  )
      {
         LOG("error msg from MODULE_APP_CLUSTER :%d\n hex:%x  index:%d\n", id , id , i );
         return;
      }

      LOG("recv message:%s  index:%d data:%d\n" ,  app_table[i].name , i , data );
      update_frame( (id > APP_CLUSTER_TIME_SET) ? s_ptr : g_ptr ,  data , &app_table[i]  );

      /*add clear message*/
      if( (app_table[i]).reset  )
         add_to_reset_queue( id );

      break;

   case MODULE_SERVICE_AUDIO_MGR: /*send from audio manager*/
      if( i >= audio_table_size  )
      {
         LOG("error msg from MODULE_SERVICE_AUDIO_MGR :%d\n hex:%x  index:%d\n", id , id , i );
         return;
      }

      LOG("recv message:%s  index:%d data:%d\n" ,  audio_table[i].name , i , data );
      update_frame( s_ptr , data , &audio_table[i]  );

      /*add clear message*/
      if( (audio_table[i]).reset  )
         add_to_reset_queue( id );

      break;
   }
}

void clear_down_buffer(byte *g_ptr , byte *s_ptr , unsigned int  id )
{
   int i = id & 0xffff;
   /*特殊情况，单独处理*/
   switch( id )
   {
   case APP_CLUSTER_TIME_SET:/*特殊处理时间*/
   {
      LOG("clear APP_CLUSTER_TIME_SET \n");

      g_ptr[2] = 0;
      g_ptr[3] = 0;
      g_ptr[4] = 0;
      g_ptr[5] = 0;
      g_ptr[6] = 0;
      g_ptr[7] = 0;
      return;
   }

   case APP_CLUSTER_NEXT_MAINTEN_DATE:
   {
      LOG("clear APP_CLUSTER_NEXT_MAINTEN_DATE\n");
      s_ptr[7] = 0;
      s_ptr[8] = 0;
      s_ptr[9] = 0;
      return;
   }
   }

   /**/
   switch( (id  >> 16) )
   {
   case MODULE_APP_CLUSTER: /*clear app message*/
      update_frame( (id > APP_CLUSTER_TIME_SET) ? s_ptr : g_ptr ,  0 , &app_table[i]  );
      LOG("clear message %s\n" , app_table[i].name );
      break;

   case MODULE_SERVICE_AUDIO_MGR: /*clear audio manager message*/
      update_frame( s_ptr , 0 , &audio_table[i]  );
      LOG("clear message %s\n" , audio_table[i].name );
      break;
   }
}


#endif
