#ifndef _UART_H_
#define _UART_H_

int open_uart(const char *port);

int uart_setup(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity);

#endif
