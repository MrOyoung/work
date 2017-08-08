#ifndef ___CONFIG_H___
#define ___CONFIG_H___


#define UDISK_MNT_DIR				"/tmp/usb_disk_mnt"
#define UDISK_FS_TYPE				"vfat"

#define MMC_NAME					"mmcblk0"
#define MMC_KERNEL_START			(1UL<<20)   //1M
#define MMC_KERNEL_SIZE				(32UL<<20)  //32M

#define BOOTLOADER_PERFIX_NAME		"bootloader_v"
#define KERNEL_PREFIX_NAME			"kernel_v"
#define SYSTEM_PREFIX_NAME			"system_v"

#define DBUS_NAME					"saic.ver-up.server"
#define DBUS_INTERFACE				"ver.up"

#define BOOTLOADER_MAX_SIZE			512UL<<10	//512K
#define KERNEL_MAX_SIZE				MMC_KERNEL_SIZE	//32M
#define SYSTEM_MAX_SIZE				1UL<<30		//1G

typedef struct {
	unsigned int valid_version;
	unsigned int run_version;
	unsigned int next_version;

	//logic system id <--> real system id
	unsigned int system_id_map[3];

	//kernel uImage size
	unsigned int kernel_size[3];

	//checksum of update_cfg
	unsigned int checksum;
} update_cfg;

typedef struct {
	char *bootloader;

	char *kernel;
	int is_kern_up;

	char *system;
	int is_sys_up;
} update_fileset;

#endif

