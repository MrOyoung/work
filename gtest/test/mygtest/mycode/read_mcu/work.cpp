#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

#include "uart.h"
#include "debug.h"
#include "config.h"


/************************ uart func ****************************/
static void read_uart(const char *dev);

int writen_size = 0;
int sleep_time = 0;

int main_work(int argc, char **argv)
{
    if (argc < 4)
    {
        printf("Usage : %s /dev/xxx return-size sleep-time(usec)\n", argv[0]);
        return -1;
    }

   	const char *dev = argv[1];
    writen_size = atoi(argv[2]);
    sleep_time = atoi(argv[3]);

    printf("writen_size = %d\ndev = %s\n", writen_size, dev);

    read_uart(dev);

    return 0;
}

unsigned long get_time()
{

	return 0;
}


static void read_uart(const char *dev)
{
    int ret_size = 0;
    int mcu_fd = 0;
    unsigned char mcudata_buf[1024] = {0};

    int index = 0;

    while (1)
    {
        if (-1 == (mcu_fd = open_uart(dev)))
        {
            //PERROR_EXIT("open uart error\n", (void)0);
            PRINT_MSG("open uart error\n");
            usleep(200);
            continue;
        }

        printf("open %s success\n", dev);

		char writen_buf[1024] = {0};

		for (index = 0;index < 50; index++)
		{
			writen_buf[index] = index;
			printf("%02x ", writen_buf[index]);
		}
		printf("\n");
	
        while(1)
        {
        	if (writen_size)
                {
                    ret_size = write(mcu_fd, writen_buf, writen_size);
                    if (ret_size > 0)
                    {
                        printf("write success\n");
                    }
                }
        
            /* read mcu data */
            ret_size = read(mcu_fd, mcudata_buf, 1024);
            if (-1 == ret_size)
            {
                PRINT_MSG("read error %s\n", strerror(errno));
                close_uart(mcu_fd);
                break;
            }
            else if (ret_size > 0)
            {
                printf("return size = %d\n", ret_size);
                for (index = 0; index < ret_size; index++)
                {
                    printf("%02x ", mcudata_buf[index]);
                }
                printf("\n");
            }
            
           usleep(sleep_time);
            
        }//end of while() - read uart

    }//end of while()

    return;
}

