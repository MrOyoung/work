#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "usr_serial.h"

struct serial_device {
	int memfd;
	volatile void *base;
	void *uart_phy_base;
	int clk_rate;
	int	(*start)(volatile void * base);
	int	(*getc)(volatile void * base);
	void (*putc)(volatile void * base, const char c);
	void (*stop)(volatile void * base);
	void (*set_termios)(volatile void * base, struct uart_option *option);
};

struct serial_device *get_current_serial_device(void);


#endif
