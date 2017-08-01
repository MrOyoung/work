VERBOSE = @
CC      = $(CROSS_COMPILE)gcc
DBG_ENABLE   = 0
SRC_PATH   := .
TARGET     := usr_serial
SRCS       += $(wildcard $(SRC_PATH)/*.c)
OBJS       := $(SRCS:.c=.o)


LIBS :=

INCLUDE_PATH := .
LIBRARY_PATH :=

## debug for debug info, when use gdb to debug
ifeq (1, ${DBG_ENABLE}) 
    CFLAGS += -D_DEBUG -O0 -g -DDEBUG=1
else
    CFLAGS += -O2
endif

LDFLAGS += -static

CFLAGS  += $(foreach dir, $(INCLUDE_PATH), -I$(dir))
LDFLAGS += $(foreach lib, $(LIBRARY_PATH), -L$(lib))
LDFLAGS += $(foreach lib, $(LIBS), -l$(lib))


all: $(TARGET)

%.o:%.c
	$(VERBOSE)$(CC) $(CFLAGS) -c $<
	
$(TARGET): $(OBJS)
	$(VERBOSE)$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -rf $(TARGET) *.o

