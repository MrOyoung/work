CC = gcc

TARGET = dbus_mcudata

SOURCES := util.o
SOURCES += send_mcudata.o 

CFLAGS := -g -Wextra -pthread
CFLAGS += $(shell pkg-config dbus-glib-1 --cflags)

LIBS := $(shell pkg-config dbus-glib-1 --libs)
LIBS += -lpthread -lrt

all:$(SOURCES)
	$(CC) $^ $(LIBS) $(CFLAGS) -o $(TARGET)
	
%.o : %.c
	$(CC) $(CFLAGS) $(LIBS) -c $< -o $@ 


clean:
	rm $(TARGET) $(SOURCES)
