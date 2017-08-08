#include "base.h"
#include "cfg.h"


#define MMC_BOOT_DEV		MMC_NAME"boot0"
#define	UBOOT_START			1024UL

static unsigned long up_size_total;
static unsigned long up_size_curr;

static int copy_file_with_size(int fd_src, int fd_dst, unsigned long size)
{
	unsigned long copy_cnt;
	char buff[4096];

	for (copy_cnt = 0; copy_cnt < size;) {
		int len = read(fd_src, buff, 4096);

		if (len < 0)
			return -1;
		else if (len == 0)
			break;

		if (write(fd_dst, buff, len) != len)
			return -1;

		copy_cnt += len;
		up_size_curr += len;
	}

	return (copy_cnt == size) ? 0 : -1;
}

static int copy_file_to_mmc(char *mmc_dev, unsigned long mmc_offset, char *file)
{
	int fd_src, fd_dst;
	int ret = -1;

	if ((fd_dst = open(mmc_dev, O_WRONLY)) < 0)
		return -1;

	if (lseek(fd_dst, mmc_offset, SEEK_SET) == -1)
		goto err1;

	if ((fd_src = open(file, O_RDONLY)) < 0)
		goto err1;

	struct stat statbuff;  
    if(stat(file, &statbuff) < 0)  
        goto err2;

	if (debug_mode)
		printf("[debug] copy_file_to mmc, size:%lu\n", statbuff.st_size);

	ret = copy_file_with_size(fd_src, fd_dst, statbuff.st_size);

err2:
	close(fd_src);
err1:
	close(fd_dst);
	return ret;
}

static int update_bootloader(const char *filename)
{
	char path1[128], path2[128];
	char cmdbuf[256];
	int ret;

	if (!filename)
		return 0;

	sprintf(path1, "/dev/%s", MMC_BOOT_DEV);
	sprintf(path2, "%s/%s", UDISK_MNT_DIR, filename);

	if (debug_mode) {
		printf("[debug] update bootloader dst:%s(offset:%lu) src:%s\n", path1, UBOOT_START, path2);
	}

	sprintf(cmdbuf, "echo 0 > /sys/block/%s/force_ro", MMC_BOOT_DEV);
	system(cmdbuf);

	ret = copy_file_to_mmc(path1, UBOOT_START, path2);

	sprintf(cmdbuf, "echo 1 > /sys/block/%s/force_ro", MMC_BOOT_DEV);
	system(cmdbuf);
	return ret;
}

static int update_kernel(const char* filename, int ver_id)
{
	char path1[128], path2[128];
	unsigned long mmc_offset;

	sprintf(path1, "/dev/%s", MMC_NAME);
	sprintf(path2, "%s/%s", UDISK_MNT_DIR, filename);

	if (filename) {
		struct stat statbuff;
		if(stat(path2, &statbuff) < 0)
			return -1;

		extern int set_ver_kernel_size(int ver_id, unsigned long size);
		set_ver_kernel_size(ver_id, statbuff.st_size);

		/* write kernel image to mmc */
		mmc_offset = MMC_KERNEL_START + MMC_KERNEL_SIZE * (ver_id - 1);
		if (debug_mode)
			printf("[debug] update kernel image dst:%s(offset:%lu), src:%s\n", path1, mmc_offset, path2);

		return copy_file_to_mmc(path1, mmc_offset, path2);
	}

	/* move valid kernel to ver_id kernel */
	int fd_src, fd_dst;
	int ret = -1;

	if (debug_mode)
		printf("[debug] update kernel dst:%s, src:%s\n", path1, path1);

	if ((fd_dst = open(path1, O_WRONLY)) < 0)
		return -1;

	mmc_offset = MMC_KERNEL_START + MMC_KERNEL_SIZE * (ver_id - 1);
	if (lseek(fd_dst, mmc_offset, SEEK_SET) == -1)
		goto err1;

	if ((fd_src = open(path1, O_RDONLY)) < 0)
		goto err1;

	extern int get_valid_ver_id(void);
	mmc_offset = MMC_KERNEL_START + MMC_KERNEL_SIZE * (get_valid_ver_id()- 1);
	if (lseek(fd_src, mmc_offset, SEEK_SET) == -1)
		goto err2;

	up_size_total += MMC_KERNEL_SIZE;
	ret = copy_file_with_size(fd_src, fd_dst, MMC_KERNEL_SIZE);

err2:
	close(fd_src);
err1:
	close(fd_dst);
	return ret;
}

static int update_system(const char *filename, int ver_id)
{
	char path1[128], path2[128];

	if (!filename)
		return 0;

	sprintf(path1, "/dev/%sp%d", MMC_NAME, ver_id);
	sprintf(path2, "%s/%s", UDISK_MNT_DIR, filename);

	if (debug_mode)
		printf("[debug] update system dst:%s, src:%s\n", path1, path2);

	return copy_file_to_mmc(path1, 0, path2);
}

static int check_file_size(char *filename, unsigned long max_size)
{
	struct stat statbuff;
	char path[128];

	if (!filename)
		return 0;

	sprintf(path, "%s/%s", UDISK_MNT_DIR, filename);
	if (stat(path, &statbuff))
		return -ENOENT;

	return (statbuff.st_size > max_size) ? -1 : statbuff.st_size;
}

static int check_ver_file_valid(update_fileset *fileset)
{
	int ret;

	if (debug_mode)
		printf("[debug] check file valid, boot:%s, kernel:%s, system:%s\n",
				fileset->bootloader, fileset->kernel, fileset->system);

	up_size_total = 0;
	up_size_curr = 0;

	ret = check_file_size(fileset->bootloader, BOOTLOADER_MAX_SIZE);
	if (ret < 0)
		return ret;
	up_size_total += ret;

	ret = check_file_size(fileset->kernel, KERNEL_MAX_SIZE);
	if (ret < 0)
		return ret;
	up_size_total += ret;

	ret = check_file_size(fileset->system, SYSTEM_MAX_SIZE);
	if (ret < 0)
		return ret;
	up_size_total += ret;

	return 0;
}

int update_version(void)
{
	extern update_fileset *get_update_fileset(void);		
	update_fileset *fileset = get_update_fileset();

	extern int curr_up_ver_id(void);
	int ver_id = curr_up_ver_id();
	int ret;

	if (debug_mode)
		printf("[debug_mode] update_vesion ver_id:%d\n", ver_id);

	extern char *get_u_disk_dev(void);
	if (mount(get_u_disk_dev(), UDISK_MNT_DIR, UDISK_FS_TYPE, 0, 0)) {
		reply_dbus_msg(-1, "mount udisk fail");
		return -1;
	}

	if ((ret = check_ver_file_valid(fileset))) {
		if (ret == -ENOENT)
			reply_dbus_msg(-1, "file not exist");
		else
			reply_dbus_msg(-1, "file size exceed limit ");

		up_size_total = 0;
		goto err;;
	}

	if (update_bootloader(fileset->bootloader)) {
		reply_dbus_msg(-1, "update bootloader fail");
		goto err;
	}

	/* only update bootloader */
	if (!fileset->kernel && !fileset->system)
		goto end;

	if (update_kernel(fileset->kernel, ver_id)) {
		reply_dbus_msg(-1, "update kernel fail");
		goto err;
	}
	fileset->is_kern_up = 1;

	if (update_system(fileset->system, ver_id)) {
		reply_dbus_msg(-1, "update system fail");
		goto err;
	}
	fileset->is_sys_up |= fileset->system ? 1 : 0;

end:
	umount(UDISK_MNT_DIR);
	return 0;

err:
	umount(UDISK_MNT_DIR);
	return -1;
}

int get_update_process(void)
{
	char buf[128];
	sprintf(buf, "%lu:%lu", up_size_curr, up_size_total);

	reply_dbus_msg(0, buf);
	return 0;
}

