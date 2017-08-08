#ifndef __BASE__H__
#define __BASE__H__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/types.h>
#include <linux/netlink.h>
#include <arpa/inet.h>
#include <netinet/in.h>


#define print_error(msg)		\
	printf(" %s: errno:%d, %s, %d\n", msg, errno, __func__, __LINE__)

extern int debug_mode;

void *udisk_hotplug_monitor(void *arg);

#endif

