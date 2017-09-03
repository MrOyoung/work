
CC = gcc

CFLAGS := -lpthread \
		  -Wall \
		  -fgnu89-inline

TARGET = main

$(TARGET) : hello.o main.o
	@echo $(LIBS)
	$(CC) $^ -o $@ 

include Makefile-bak

%.o : %.c
	$(CC) $(CFLAGS) -c $^

.PHONY:clean
clean:
	-rm *.o $(TARGET)
	
