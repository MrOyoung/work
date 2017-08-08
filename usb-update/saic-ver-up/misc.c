#include "base.h"
#include "cfg.h"


extern char *get_u_disk_dev(void);

#define VER_FILE_SET_SIZE	1024*1024

int get_version_fileset(void)
{
	if (!get_u_disk_dev()) {
		reply_dbus_msg(-1, "udisk is not plug");
		return -1;
	}

	/* mount udisk /dev/sdx /tmp//tmp/usb_disk_mnt */
	if (mount(get_u_disk_dev(), UDISK_MNT_DIR, UDISK_FS_TYPE, 0, 0)) {
		reply_dbus_msg(-1, "mount udisk fail");
		return -1;
	}

	struct dirent* ent = NULL;
	DIR* pDir = opendir(UDISK_MNT_DIR);
	if (NULL == pDir) {
		reply_dbus_msg(-1, "opendir udisk mnt dir fail");
		return -1;
	}

	int filecnt = 0, len = 0;
	char *fileset = malloc(VER_FILE_SET_SIZE);
	if (!fileset) {
		closedir(pDir);
		reply_dbus_msg(-1, "memory not enough");
		return -1;
	}
	memset(fileset, 0, VER_FILE_SET_SIZE);

	while (NULL != (ent=readdir(pDir))) {
		if (!strncmp(ent->d_name, BOOTLOADER_PERFIX_NAME, strlen(BOOTLOADER_PERFIX_NAME)) ||
			!strncmp(ent->d_name, KERNEL_PREFIX_NAME, strlen(KERNEL_PREFIX_NAME)) ||
			!strncmp(ent->d_name, SYSTEM_PREFIX_NAME, strlen(SYSTEM_PREFIX_NAME))) {
			filecnt++;
			len += sprintf(fileset + len, "%s,", ent->d_name);
		}
	}

	closedir(pDir);
	umount(UDISK_MNT_DIR);

	if (debug_mode)
		printf("[debug] get_version_fileset, fileset:%s\n", fileset);

	if (strlen(fileset))
		fileset[strlen(fileset) - 1] = 0;

	reply_dbus_msg(filecnt, fileset);
	free(fileset);
	return 0;
}

static void check_fileset_valid_init()
{
	extern update_fileset *get_update_fileset(void);
	update_fileset *fileset = get_update_fileset();
	fileset->bootloader = NULL;
	fileset->kernel = NULL;
	fileset->system = NULL;
}

static int check_fileset_valid(char *str)
{
	extern update_fileset *get_update_fileset(void);
	update_fileset *fileset = get_update_fileset();

	if (debug_mode)
		printf("[debug] check fileset valid: %s\n", str);

	if (!strncmp(str, BOOTLOADER_PERFIX_NAME, strlen(BOOTLOADER_PERFIX_NAME))) {
		if (fileset->bootloader)
			return -1;
		fileset->bootloader = str;
	} else if (!strncmp(str, KERNEL_PREFIX_NAME, strlen(KERNEL_PREFIX_NAME))) {
		if (fileset->kernel)
			return -1;
		fileset->kernel = str;
	} else if (!strncmp(str, SYSTEM_PREFIX_NAME, strlen(SYSTEM_PREFIX_NAME))){
		if (fileset->system)
			return -1;
		fileset->system = str;
	} else {
		return -1;
	}

	return 0;	
}

int parse_ver_up_file(char *str)
{
	char *delim = ",";
	char *ptr;

	if (!strlen(str)) {
		reply_dbus_msg(-1, "update filename empty");
		return -1;
	}

	check_fileset_valid_init();

	ptr = strtok(str, delim);
	if (!ptr || check_fileset_valid(ptr)) {
		reply_dbus_msg(-1, "update file wrong");
		return -1;
	}

	while((ptr = strtok(NULL, delim))) {
		if (check_fileset_valid(ptr)) {
			reply_dbus_msg(-1, "update file wrong");
			return -1;
		}
	}

	return 0;
}

/* udisk mnt dir : /tmp/usb_disk_mnt */
static int create_version_update_dir(void)
{
	char cmdbuf[256];

	sprintf(cmdbuf, "rm -rf %s", UDISK_MNT_DIR);
	if (system(cmdbuf)) {
		print_error(cmdbuf);
		return -1;
	}

	sprintf(cmdbuf, "mkdir %s", UDISK_MNT_DIR);
	if (system(cmdbuf)) {
		print_error(cmdbuf);
		return -1;
	}

	return 0;
}

int version_update_init(void)
{
	if (create_version_update_dir())
		return -1;

	extern int read_cfg_from_mmc(void);
	if (read_cfg_from_mmc()) {
		printf("read cfg from mmc fail\n");
		return -1;
	}

	extern int dbus_init(void);
	if (dbus_init())
		return -1;

	pthread_t th;
	extern void *udisk_hotplug_monitor(void*);

	/* create thread : monitor udisk hotplug */
	if (pthread_create(&th, NULL, udisk_hotplug_monitor, NULL) != 0) {
		print_error("pthread_create");
		return -1;
	}

	return 0;
}

