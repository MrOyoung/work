#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <zre/re/c/util/cap_alloc.h>
#include <zre/re/c/namespace.h>
#include <zre/re/c/parent.h>
#include <zre/io/io.h>
#include <zre/libserial/serial_lib.h>
#include <zre/libvideo/zre_video.h>
#include <pthread-zre.h>

#include <zre/libhealth_mgr/health_mgr.h>
#include <zre/sys/cache.h>
#include <zre/libipu/zre_ipu.h>

#define HELPER_ID    1

#define IDLE 1
#define RUN 2

static void 		*serial_dev = NULL;
static void 		*pic_bg_handle;
static unsigned int should_stop = 0;

static unsigned char **pic_init(void);
static unsigned int pic_free(unsigned char **pic_handler);
static int get_speed_from_data(unsigned char* data , int size );
static unsigned int show_pic(const unsigned int speed, unsigned char **pic_handler);

static void * helper_func(void *args)
{
	unsigned int serial_data_speed = 0 ;

	int i = 0, ret = 0;
	char read_buffer[512] = {0};

	void **mcu_handler = &serial_dev;

	/* init the picture handler */
	unsigned char **pic_arr = pic_init();

	pic_bg_handle = zrepic_open("rom/bg.bmp");

	while (1)
	{
		while(should_stop) 
		{
			ret = uart_read( *mcu_handler, read_buffer,sizeof (read_buffer));
#if 1
			if(ret > 0){		
				for(i = 0; i < ret; i++)
					printf("%x ", read_buffer[i]);
				printf("\n");
			}
#endif
			serial_data_speed = get_speed_from_data((unsigned char*)read_buffer , ret );
			printf("\n\nret = %d, serial_data_speed = %d\n\n", ret, serial_data_speed);

			if( -1 == serial_data_speed )
				continue;

			/* show the picture according to the speed that read from uart */
			show_pic(serial_data_speed, pic_arr);
		}

		sleep(1);
	}

	return NULL;
}


static void helper_main(unsigned long vram_virt)
{
	pthread_t th;
	char helper_name[32];
	long ret;
	zre_cap_idx_t ns;
	int msg = RUN;

	printf("helper vram_virt is 0x%lx\n", vram_virt);

	ns = zcore_env_get_cap("shared");
	if(ns == ZRE_INVALID_CAP)
	{
		printf("helper get shared cap failed\n");
		return;
	}

	sprintf(helper_name, "helper%d", HELPER_ID);
	ret = zcore_ns_register_obj_srv(ns, helper_name, pthread_getzrecap(pthread_self()), 0);
	printf("register %s ret %ld\n", helper_name, ret);

	zcore_parent_register_helper(HELPER_ID);

	if (0 != pthread_create(&th, NULL, helper_func, NULL))
	{
		perror("thread create error");
	}

	while(1)
	{
		msg = health_mgr_get_msg();
		if(msg < 0)
			continue;
		if(msg == RUN)
		{
			printf("helper recve mesg to run\n");
			init_ipu(); 

			serial_dev = uart_open_raw(1);
			if(!serial_dev)
			{
				printf("open raw uart failed 2\n");
			}

			ret = zrepic_display_bg(pic_bg_handle);
			printf("zrepic_display_bg return %d\n", ret);

			should_stop = 1;
			asm volatile("":::"memory");
		}
		else if(msg == IDLE)
		{
			printf("helper recve mesg to stop\n");
			should_stop = 0;

			asm volatile("":::"memory");
		}
	}

}


int main(void)
{
	unsigned long vram_virt=0;
	printf("begin emergy_dash\n");

	vram_virt = vram_init();
	if(!vram_virt)
	{
		printf("map vram failed\n");
		return 0;
	}

	helper_main(vram_virt);

	return 0;
}

static int get_speed_from_data(unsigned char* data , int size )
{
	unsigned char* p = data;

	while( ( *p != 0xaa ) && (p < (data + size)) )
		p++;

	if( p >= (data + size) )
		return -1;

	int frame_size = p[1];

	if( p[frame_size + 8] != 0x55 ){
		return -1;
	}

	int magic =	( p[4] << 0 ) |
		( p[5] << 8 ) |
		( p[6] << 16) |
		( p[7] << 24);

	if( 0x01 != magic )
		return -1;

	return p[8 + 8];
}

#define PICTURE_NUM 11

/* 11rst is "rom/unit.bmp" */
static char *bmp_pic[] =
{
	"rom/0.bmp", "rom/1.bmp", "rom/2.bmp", "rom/3.bmp",
	"rom/4.bmp", "rom/5.bmp", "rom/6.bmp", "rom/7.bmp",
	"rom/8.bmp", "rom/9.bmp", "rom/unit.bmp"
};


/* 0.bmp ~ 9.bmp unit.bmp */
static unsigned char **pic_init(void)
{
	int index = 0;
	static unsigned char *array[PICTURE_NUM] = {0};
	
	if (array[0])
	{
		return array;
	}

	for (; index < PICTURE_NUM; index++)
	{
		array[index] = zrepic_open(bmp_pic[index]);
	}

	return array;
}

static unsigned int pic_free(unsigned char **pic_handler)
{
	int index = 0;

	for (; index < 10; index++)
	{
		zrepic_close(pic_handler[index]);
	}

	return 0;
}

struct digit_picture
{
	unsigned char *pic_h;		//hundred
	unsigned char *pic_d;		//decade
	unsigned char *pic_u;		//unit
	unsigned char *pic_unit; 	//unit - km/h
};


#define PIC_U_X 	920
#define PIC_D_X 	620
#define PIC_H_X		320
#define PIC_UNIT_X	1250

#define PIC_U_Y 	172
#define PIC_D_Y 	172
#define PIC_H_Y		172
#define PIC_UNIT_Y	320

static unsigned int show_pic(const unsigned int speed, unsigned char **pic_handler)
{
	struct digit_picture pic;

	pic.pic_h  	 = pic_handler[(speed / 100)]; 
	pic.pic_d 	 = pic_handler[(speed / 10 % 10)];
	pic.pic_u 	 = pic_handler[(speed % 10)];
	pic.pic_unit = pic_handler[PICTURE_NUM - 1];

	printf("speed = %d\n", speed);

	if (speed > 99)
	{
		zrepic_display(pic.pic_h, PIC_H_X, PIC_H_Y);
		zrepic_display(pic.pic_d, PIC_D_X, PIC_D_Y);
	}
	else if ((speed > 9) && (speed < 100))
	{
		zrepic_clear(pic.pic_h, PIC_H_X, PIC_H_Y);
		zrepic_display(pic.pic_d, PIC_D_X, PIC_D_Y);
	}
	else /* speed < 10 */
	{
		zrepic_clear(pic.pic_h, PIC_H_X, PIC_H_Y);
		zrepic_clear(pic.pic_d, PIC_D_X, PIC_D_Y);
	}

	zrepic_display(pic.pic_u, PIC_U_X, PIC_U_Y);
	zrepic_display(pic.pic_unit, PIC_UNIT_X, PIC_UNIT_Y);

	return 0;
}

