#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include "uart.h"
#include "debug.h"
#include <sys/ioctl.h>

//#define _DEBUG_
#undef _DEBUG_

#ifdef _DEBUG_

	#define __REPORT_ERROR__ \
    	printf("file:%s function:%s line:%d\n",__FILE__,__FUNCTION__,__LINE__)

	#define UART_LOG(fmt, ...) \
		printf("file:%s func:%s line:%d - %s\n", __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#else
	#define __REPORT_ERROR__ 
	#define UART_LOG(fmt, ...)

#endif //_DEBUG_

int open_uart( const char* port )
{
    int fd;
    
	if (NULL == port) return -1;

    UART_LOG( "call open function\n" );
    fd = open( port, O_RDWR|O_NOCTTY|O_NDELAY );
    if ( fd < 0 ) {
        __REPORT_ERROR__;
        return -1;
    }
    
    UART_LOG( "call fcntl function\n" );
    if(fcntl(fd, F_SETFL, 0 ) < 0) {
        __REPORT_ERROR__;
        close(fd);
        return -1;
    }     
    //测试是否为终端设备 
    UART_LOG( "call isatty function\n" );   
    if(0 == isatty( fd )) {
        __REPORT_ERROR__;
        close( fd );
        return -1;
    }

    //if( uart_setup( fd , 115200 , 0 , 8 , 1 , 'n' ) ){
    if( uart_setup( fd , 921600 , 0 , 8 , 1 , 'n' ) ){
        close(fd);
        return -1;
    }

    return fd;
}


int speed_to_flag(int speed)
{
    static int baudflag_arr[] = {
        B921600, B460800, B230400, B115200, B57600, B38400,
        B19200,  B9600,   B4800,   B2400,   B1800,  B1200,
        B600,    B300,    B150,    B110,    B75,    B50
    };
    static int speed_arr[] = {
        921600,  460800,  230400,  115200,  57600,  38400,
        19200,   9600,    4800,    2400,    1800,   1200,
        600,     300,     150,     110,     75,     50
    };

    int i;
    for (i = 0;  i < sizeof(speed_arr)/sizeof(int);  i++) {
        if (speed == speed_arr[i]) {
            return baudflag_arr[i];
        }
    }

    fprintf(stderr, "Unsupported baudrate, use 9600 instead!\n");
    return B9600;
}


int uart_setup(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity)
{
    struct termio term_attr;

	UART_LOG( "call ioctl get function\n" );  
    /* Get current setting */
    if (ioctl(fd, TCGETA, &term_attr) < 0) {
    	__REPORT_ERROR__;
        return -1;
    }

    term_attr.c_iflag &=0;// ~(IXON | IXOFF | IXANY);
    term_attr.c_cflag &= ~CRTSCTS;

    //        term_attr.c_iflag &= ~(INLCR | IGNCR | ICRNL | ISTRIP);
    //        term_attr.c_oflag &= ~(OPOST | ONLCR | OCRNL);
    //        term_attr.c_lflag &= ~(ISIG | ECHO | ICANON | NOFLSH);
    term_attr.c_cflag &= ~CBAUD;
    term_attr.c_cflag |= (CLOCAL | CREAD);

    term_attr.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    term_attr.c_oflag &= ~OPOST;
    term_attr.c_cflag |= speed_to_flag(speed);

    /* Set databits */
    term_attr.c_cflag &= ~(CSIZE);
    switch (databits) {
        case 5:
            term_attr.c_cflag |= CS5;
            break;

        case 6:
            term_attr.c_cflag |= CS6;
            break;

        case 7:
            term_attr.c_cflag |= CS7;
            break;

        //case 8:
        default:
            term_attr.c_cflag |= CS8;
            break;
    }

    /* Set parity */
    switch (parity) {
        case 'o': case 'O':   /* Odd parity */
            term_attr.c_cflag |= (PARENB | PARODD);
            break;

        case 'e': case 'E':   /* Even parity */
            term_attr.c_cflag |= PARENB;
            term_attr.c_cflag &= ~(PARODD);
            break;

        //case 'n': case 'N':   /* None parity */
        default:
            term_attr.c_cflag &= ~(PARENB);
            break;
    }

    /* Set stopbits */
    switch (stopbits) {
        case 2:   /* 2 stopbits */
            term_attr.c_cflag |= CSTOPB;
            break;

        //case 1:   /* 1 stopbits */
        default:
            term_attr.c_cflag &= ~CSTOPB;
            break;
    }

    term_attr.c_cc[VMIN] = 0;
    term_attr.c_cc[VTIME] = 0;

	UART_LOG( "call ioctl set function\n" ); 
    if (ioctl(fd, TCSETAW, &term_attr) < 0){
    	__REPORT_ERROR__;
    	return -1;
    }
        
    UART_LOG( "call ioctl flush function\n" ); 
    if (ioctl(fd, TCFLSH, 2) < 0){
    	__REPORT_ERROR__;
    	return -1;
    }


    return 0;
}

int close_uart(int fd)
{
	if (fd)
	{
		close(fd);
	}
	
	return 0;
}
