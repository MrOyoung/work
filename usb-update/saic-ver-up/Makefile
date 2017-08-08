
TARGET=saic_ver_up

CFLAGS+=-I../../../../sysroots/x86_64-linux/usr/include/dbus-1.0
CFLAGS+=-I../../../../sysroots/x86_64-linux/usr/lib/dbus-1.0/include
CFLAGS+=-I../../../../sysroots/x86_64-linux/usr/include/glib-2.0
CFLAGS+=-I../../../../sysroots/x86_64-linux/usr/lib/glib-2.0/include

CFLAGS+=-Wall

LDFLAGS+=-ldbus-1 -lglib-2.0 -ldbus-glib-1 -lpthread

all:
	$(CC) $(CFLAGS) -c main.c
	$(CC) $(CFLAGS) -c usb.c
	$(CC) $(CFLAGS) -c config.c
	$(CC) $(CFLAGS) -c misc.c
	$(CC) $(CFLAGS) -c update.c
	$(CC) $(CFLAGS) -c dbus.c
	$(CC) -o $(TARGET) main.o usb.o config.o misc.o update.o dbus.o $(LDFLAGS)

clean:
	rm -rf *.o
	rm -rf $(TARGET)

