CC=gcc
#CC=arm-poky-linux-gnueabi-gcc

target=pkg_log_test

socket_source=socket-api

CSOURCE:=
CSOURCE+=main.c
CSOURCE+=$(socket_source)/socket.c

OBJECTS:=
OBJECTS+=main.o
OBJECTS+=$(socket_source)/socket.o


CFLAGS:=-Wall -g
CFLAGS+=-O2 

CFLAGS+=-DENABLE_DEMO_LOG

#需要链接的库
LIBS:=-lpthread
#LIBS+=-ldbus-glib-1 -ldbus-1 -lgobject-2.0 -lglib-2.0

#######################################################
$(target):$(OBJECTS)
	$(CC) -o $@ $^ $(CFLAG) $(LIBS)
	
#包含依赖文件
#include $(OBJECTS:.o=.d)

#生成相关的依赖文件
%.d:%.c
	@echo create depend file $@...
	@$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ : ]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$

.PHONY:clean
clean:
	rm -rf $(target) $(OBJECTS) $(OBJECTS:.o=.d)
