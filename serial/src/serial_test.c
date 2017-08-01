#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "usr_serial.h"

struct uart_option test_uart_option =
{
	.baud = 38400,
	.bits = 7,
	.stopbits = 2,
	.parity = 'e',
};

struct uart_option test_uart_option_ok =
{
	.baud = 115200,
	.bits = 8,
	.stopbits = 1,
	.parity = 'n',
};


void serial_test(void)
{
	void *serial_dev = NULL;
	char read_buffer[512] = {0};
	int i,ret;
	int j = 10;

	serial_dev = uart_open();

	if(!serial_dev)
	{
		printf("open uart failed\n");
		return;
	}
/*
	printf ("\n*********default option test start!************\n");
	while(j --)
	{
		memset(read_buffer, 0x0, 512);
		ret = uart_read(serial_dev, read_buffer,sizeof (read_buffer));
		if(ret > 0){		
			for(i = 0; i < ret; i++)
				printf("%x ", read_buffer[i]);;
		}	
		usleep(10000);	
	}

	printf ("\n*******change to 38400 option test start!**********\n");
	j = 10;
	uart_set(serial_dev, &test_uart_option);
	while(j --)
	{
		memset(read_buffer, 0x0, 512);
		ret = uart_read(serial_dev, read_buffer,sizeof (read_buffer));
		if(ret > 0){		
			for(i = 0; i < ret; i++)
				printf("%x ", read_buffer[i]);;
		}	
		usleep(10000);	
	}
*/
	printf ("\n*******change to 115200 option test start!*******\n");
	j = 10;
	uart_set (serial_dev, &test_uart_option_ok);
	while(1)
	{
		memset(read_buffer, 0x0, 512);
		ret = uart_read(serial_dev, read_buffer,sizeof (read_buffer));
		if(ret > 0){		
			for(i = 0; i < ret; i++)
			{
				printf("%x ", read_buffer[i]);
			}

		}
		usleep(20 * 1000);	
	}

	uart_close(serial_dev);
}


int main(void)
{
	serial_test();
	return 0;
}

