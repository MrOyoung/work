#COMPILE=arm-mel-linux-
COMPILE=arm-poky-linux-gnueabi-
CC:=$(COMPILE)gcc
#CFLAGS:=--sysroot=/opt/mentor-sysdk/2015.12/sysroots/cortexa9hf-vfp-neon-mel-linux-gnueabi/ -mfloat-abi=hard -mfpu=neon
CFLAGS+=-ldbus-glib-1 -ldbus-1 -lgobject-2.0 -lglib-2.0 -lpthread
CFLAGS+=-mtune=cortex-a9
CFLAGS+=-I=/usr/include/dbus-1.0
CFLAGS+=-I=/usr/include/glib-2.0
CFLAGS+=-I=/usr/lib/dbus-1.0/include
CFLAGS+=-I=/usr/lib/glib-2.0/include
CFLAGS+=-mtune=cortex-a9
CFLAGS+=-Wall
#
##all:signal signal_receiver
##
#fake_data:fake_data.o timer.o
#	$(CC) -o $@ $^ $(CFLAGS)

receiver:receiver.c ipc.c
	$(CC) -o $@ $^ ${CFLAGS}


sender:sender.c ipc.c
	$(CC) -o $@ $^ ${CFLAGS}
	#cp sender /home/rocky/share-nfs/
	
	
prefermance_receiver:receiver_backup.c util.c
	$(CC) -o $@ $^ ${CFLAGS}

mssleep:mssleep.c
	$(CC) -o $@ $^ ${CFLAGS}

prefermance_sender:sender_backup.c util.c
	$(CC) -o $@ $^ ${CFLAGS}
	#`pkg-config --cflags --libs dbus-glib-1`

