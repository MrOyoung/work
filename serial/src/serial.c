#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "serial.h"

void *uart_open (void)
{
	void *base = NULL;
	struct serial_device *dev = NULL;
	int memfd = -1;
	void *uart_base = NULL;
	
	dev = get_current_serial_device();
	if (dev->base)
		return (void *)dev;

	uart_base = (void *)dev->uart_phy_base;


    memfd = open("/dev/mem", O_RDWR|O_SYNC);
    if (memfd < 0) {
        return NULL;
    }
	
	base = mmap(NULL, 0x1000, PROT_READ|PROT_WRITE, 
				 MAP_SHARED, memfd, (off_t)uart_base);
	if (!base)
	{
		close (memfd);
		return NULL;
	}
	
	dev->base = (volatile void *)base;
	dev->start(dev->base);
	dev->memfd = memfd;
	
	return (void *)dev;	
}

int uart_read(void *serial, char *buffer, unsigned size)
{
	unsigned i = 0;
	char c;
	struct serial_device *dev = (struct serial_device *)serial;
	if (!dev || !buffer)
		return 0;

	while (size)
	{
		c = (char)dev->getc(dev->base);
		buffer[i++] = (char)c;
		size--;
	}
	return i;
}

int uart_write(void *serial, char *buffer, unsigned size)
{
	unsigned i = 0;
	struct serial_device *dev = (struct serial_device *)serial;
	
	if (!dev || !buffer)
		return 0;
	
    while (size--)
    {
      dev->putc(dev->base, *buffer++);
	  i ++;
    }
	return i;
}

int uart_set(void *serial, struct uart_option *option)
{
	unsigned i = 0;
	struct serial_device *dev = (struct serial_device *)serial;
	
	if (!dev || !option)
		return -1;
	
	dev->set_termios(dev->base, option);
	return 0;
}

void uart_close(void *serial)
{
  struct serial_device *dev = (struct serial_device *)serial;
  if (!dev)
  	return;

  if (!dev->base)
  	return;
  
  dev->stop(dev->base);
  munmap ((void *)dev->base, 0x1000); 
  dev->base = (volatile void *)NULL;
  close(dev->memfd);
}


