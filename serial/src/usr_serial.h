#ifndef __USR_SERIAL_H__
#define __USR_SERIAL_H__

struct uart_option
{
	int baud;
	int bits;
	int stopbits;
	int parity;
};

void *uart_open (void);
int uart_read(void *serial, char *buffer, unsigned size);
int uart_write(void *serial, char *buffer, unsigned size);
void uart_close(void *serial);
int uart_set(void *serial, struct uart_option *option);

#endif

