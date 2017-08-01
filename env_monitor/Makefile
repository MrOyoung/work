CC=gcc

CFLAGS:=
CFLAGS+=-lpthread -Wall

CSOURCE:=env_main.c
CSOURCE+=env_api.c

BINFILE=env_main

$(BINFILE):$(CSOURCE)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm $(BINFILE)
