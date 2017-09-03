CC=gcc
#CC=arm-poky-linux-gnueabi-gcc

target=demo_log

sourcefile	=	source/

CSOURCE:=
CSOURCE+=main.c
CSOURCE+=$(sourcefile)record.c
CSOURCE+=$(sourcefile)monitor.c
CSOURCE+=$(sourcefile)getkmsg.c
CSOURCE+=$(sourcefile)scan.c

CSOURCE+=socket-api/socket.c

OBJECTS:=
OBJECTS+=main.o
OBJECTS+=$(sourcefile)record.o
OBJECTS+=$(sourcefile)monitor.o
OBJECTS+=$(sourcefile)getkmsg.o
OBJECTS+=$(sourcefile)scan.o

OBJECTS+=socket-api/socket.o

CFLAGS:=-Wall
CFLAGS+=-O2 
CFLAGS+=-g 

CFLAGS+=-I`pwd`/socket-api
CFLAGS+=-I`pwd`/include

#需要链接的库
LIBS:=-lpthread

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
