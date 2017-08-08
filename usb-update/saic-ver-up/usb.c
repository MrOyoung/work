#include "base.h" 

#define UEVENT_BUFFER_SIZE 2048

static char u_disk_dev[128];

static int init_hotplug_sock()
{
	const int buffersize = 1024;
	struct sockaddr_nl snl;

	bzero(&snl, sizeof(struct sockaddr_nl));
	snl.nl_family = AF_NETLINK;
	snl.nl_pid = getpid();
	snl.nl_groups = 1;

	int sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (sock == -1) {
		print_error("socket");
		return -1;
	}
	setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));

	int ret = bind(sock, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));
	if (ret < 0) {
		print_error("bind");
		close(sock);
		return -1;
	}

	return sock;
} 

static char *get_udisk_name(char *tmp)
{
	if (!(tmp = strstr(tmp, "usb")))
		return NULL;

	if (!(tmp = strstr(tmp, "/block/sd")))
		return NULL;

	if (strlen(tmp = strstr(tmp, "/sd")) <= 4)
		return NULL;

	return strrchr(tmp, '/');
}

static void process_add_device(char *str)
{
	char *tmp = get_udisk_name(str);
	if (!tmp)
		return;

	memset(u_disk_dev, 0, sizeof(u_disk_dev));
	strcpy(u_disk_dev, "/dev");
	strcat(u_disk_dev, tmp);

	if (debug_mode)
		printf("[debug] add U Disk:%s\n", u_disk_dev);
}

static void process_remove_device(char *str)
{
	char *tmp = get_udisk_name(str);
	if (!tmp)
		return;

	if (strstr(u_disk_dev, tmp)) {
		if (debug_mode)
			printf("[debug] remove U Disk:%s\n", u_disk_dev);
		memset(u_disk_dev, 0, sizeof(u_disk_dev));
	}
}

char *get_u_disk_dev(void)
{
	if (!strlen(u_disk_dev))
		return NULL;

	return u_disk_dev;
}

void *udisk_hotplug_monitor(void *arg)
{
	int hotplug_sock = init_hotplug_sock();
	if (hotplug_sock <= 0)
		return NULL;

	while(1) {
		char buf[UEVENT_BUFFER_SIZE * 2] = {0};
		char *tmp;

		if (recv(hotplug_sock, &buf, sizeof(buf), 0) < 0)
			continue;

		if ((tmp = strstr(buf, "add@/devices"))) {
			process_add_device(tmp);
		} else if ((tmp = strstr(buf, "remove@/devices"))) {
			process_remove_device(tmp);
		}
    }

	close(hotplug_sock);
	return NULL;
}

