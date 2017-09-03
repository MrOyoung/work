#CC=gcc
CC=arm-poky-linux-gnueabi-gcc

target=pkg_log_test

CSOURCE:=
CSOURCE+=util.c
CSOURCE+=main.c

OBJECTS:=
OBJECTS+=main.o
OBJECTS+=util.o

CFLAGS:=-Wall
CFLAGS+=-O2 

CFLAGS+=-I=/usr/include/dbus-1.0
CFLAGS+=-I=/usr/include/glib-2.0
CFLAGS+=-I=/usr/lib/dbus-1.0/include
CFLAGS+=-I=/usr/lib/glib-2.0/include

#需要链接的库
LIBS:=-lpthread
LIBS+=-ldbus-glib-1 -ldbus-1 -lgobject-2.0 -lglib-2.0

#######################################################
$(target):$(OBJECTS)
	$(CC) -o $@ $^ $(CFLAG) $(LIBS)
	
#包含依赖文件
include $(OBJECTS:.o=.d)

#生成相关的依赖文件
%.d:%.c
	@echo create depend file $@...
	@$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ : ]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$

.PHONY:clean
clean:
	rm -rf $(target) $(OBJECTS) $(OBJECTS:.o=.d)
