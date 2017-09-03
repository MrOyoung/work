#ifndef _UART_H_
#define _UART_H_

//#define UART_DEV (const char *)"/dev/ttymxc1"


int open_uart(const char* port);

int uart_setup(int fd,int speed,int flow_ctrl,int databits, int stopbits,int parity);

int close_uart(int fd);

#endif
