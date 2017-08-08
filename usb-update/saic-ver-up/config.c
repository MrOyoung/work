#include "base.h"
#include "cfg.h"


#define MMC_CFG_DEV			MMC_NAME"boot1"
#define BOOT_CFG2_OFFSET	4096    //backup cfg offset

static update_cfg global_update_cfg;
static update_fileset global_update_fileset;

static void show_update_cfg(void)
{
	update_cfg *cfg = &global_update_cfg;

	printf("[debug]     valid_version:    %x\n", cfg->valid_version);
	printf("[debug]     run_version:      %x\n", cfg->run_version);
	printf("[debug]     next_version:     %x\n", cfg->next_version);
	printf("[debug]     system_id_map[0]: %x\n", cfg->system_id_map[0]);
	printf("[debug]     system_id_map[1]: %x\n", cfg->system_id_map[1]);
	printf("[debug]     system_id_map[2]: %x\n", cfg->system_id_map[2]);
	printf("[debug]     kernel_size[0]: %x\n", cfg->kernel_size[0]);
	printf("[debug]     kernel_size[1]: %x\n", cfg->kernel_size[1]);
	printf("[debug]     kernel_size[2]: %x\n", cfg->kernel_size[2]);
	printf("[debug]     checksum: %x\n", cfg->checksum);
}

static unsigned int checksum_cfg(update_cfg *cfg)
{
	unsigned int sum = 0;

	sum += cfg->valid_version + cfg->run_version + cfg->next_version;
	sum += cfg->system_id_map[0] + cfg->system_id_map[1] + cfg->system_id_map[2];
	sum += cfg->kernel_size[0] + cfg->kernel_size[1] + cfg->kernel_size[2];

	return sum;
}

/* read update cfg from /dev/mmcblk0boot1 */
int read_cfg_from_mmc(void)
{
	update_cfg *cfg = &global_update_cfg;
	char dev_name[128];
	int fd;

	sprintf(dev_name, "/dev/%s", MMC_CFG_DEV);

	if ((fd = open(dev_name, O_RDONLY)) < 0)
		return -1;

	if (read(fd, cfg, sizeof(*cfg)) != sizeof(*cfg)) {
		close(fd);
		return -1;
	}

	close(fd);

	if (debug_mode) {
		printf("[debug] read_cfg_from_mmc\n");
		show_update_cfg();
	}

	return 0;
}

static int write_cfg_to_mmc(void)
{
	update_cfg *cfg = &global_update_cfg;
	char dev_name[128], cmdbuf[256];
	int fd, ret = 0;
	
	sprintf(dev_name, "/dev/%s", MMC_CFG_DEV);

	if ((fd = open(dev_name, O_WRONLY)) < 0) {
		print_error("open");
		return -1;
	}

	cfg->checksum = checksum_cfg(cfg);

	sprintf(cmdbuf, "echo 0 > /sys/block/%s/force_ro", MMC_CFG_DEV);
	system(cmdbuf);

	//write cfg
	if (write(fd, cfg, sizeof(*cfg)) != sizeof(*cfg)) {
		print_error("write");
		ret = -1;
		goto end;
	}

	//write backup cfg
	if (lseek(fd, BOOT_CFG2_OFFSET, SEEK_SET) == -1) {
		print_error("lseek");
		ret = -1;
		goto end;
	}

	if (write(fd, cfg, sizeof(*cfg)) != sizeof(*cfg)) {
		print_error("write");
		ret = -1;
		goto end;
	}

end:
	sprintf(cmdbuf, "echo 1 > /sys/block/%s/force_ro", MMC_CFG_DEV);
	system(cmdbuf);
	close(fd);

	if (debug_mode) {
		printf("[debug] write_cfg_to_mmc\n");
		show_update_cfg();
	}

	return ret;
}

int curr_up_ver_id(void)
{
	if (global_update_cfg.valid_version == 2)
		return 3;
	else
		return 2;
}

int get_valid_ver_id(void)
{
	return global_update_cfg.valid_version;
}

int set_up_ver_run(int type)
{
	update_cfg *cfg = &global_update_cfg;
	update_fileset *fileset = &global_update_fileset;

	if (!fileset->is_kern_up && !fileset->is_sys_up)
		return 0;

	int ver_id = curr_up_ver_id();
	cfg->run_version = ver_id;

	if (fileset->is_sys_up)
		cfg->system_id_map[ver_id - 1] = ver_id;
	else
		cfg->system_id_map[ver_id - 1] = cfg->system_id_map[cfg->valid_version - 1];

	return write_cfg_to_mmc();
}

/* set current version valid 
 *valid_version = next_version;
 *next_version  = 0;
 */
int set_cur_ver_valid(void)
{
	update_cfg *cfg = &global_update_cfg;

	if (cfg->next_version) {
		cfg->valid_version = cfg->next_version;
		cfg->run_version = cfg->next_version;
		cfg->next_version = 0;
	}

	return write_cfg_to_mmc();
}

int set_ver_kernel_size(int ver_id, unsigned long size)
{
	update_cfg *cfg = &global_update_cfg;
	cfg->kernel_size[ver_id - 1] = size;
	return 0;
}

update_fileset *get_update_fileset(void)
{
	return &global_update_fileset;
}

