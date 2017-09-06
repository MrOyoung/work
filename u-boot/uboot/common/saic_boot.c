#include <common.h>
#include <mmc.h>


#define BOOT_CFG_MMC_DEV		1
#define BOOT_CFG_MMC_PART		2
#define BOOT_CFG2_OFFSET		4096    //backup cfg offset

#define MMC_KERNEL_START		(1UL<<20)   //1M
#define MMC_KERNEL_SIZE			(32UL<<20)  //32M

#ifdef DEBUG_VER
#define saic_bootargs	"mem=512M console=ttyLv0 root=/dev/mmcblk0p%d rw zpv_dtb=rom/imx6q-saicecu-a1.dtb video=mxcfb0:dev=ldb,LDB-XGA,if=RGB24,bpp=32 zpv_cpus_map=1,0 lpj=775168"
#else
#define saic_bootargs	"mem=512M console=ttyLv0 root=/dev/mmcblk0p%d rw zpv_dtb=rom/imx6q-saicecu-a1.dtb video=mxcfb0:dev=ldb,LDB-XGA,if=RGB24,bpp=32 zpv_cpus_map=1,0 lpj=775168 loglevel=0"
#endif
#define saic_bootcmd	"mmc read 0x12000000 0x%lx 0x%lx; bootm 0x12000000"

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

static update_cfg g_cfg;

static char buff[1024];
#define ALIGNED_SZ 64

static unsigned int checksum_cfg(update_cfg *cfg)
{
	unsigned int sum = 0;

	sum += cfg->valid_version + cfg->run_version + cfg->next_version;
	sum += cfg->system_id_map[0] + cfg->system_id_map[1] + cfg->system_id_map[2];
	sum += cfg->kernel_size[0] + cfg->kernel_size[1] + cfg->kernel_size[2];

	return sum;
}

static void recover_orig_cfg(update_cfg *cfg)
{
	memset(cfg, 0, sizeof(*cfg));
	cfg->valid_version = 1;
	cfg->run_version = 1;
	cfg->system_id_map[0] = 1;
	cfg->kernel_size[0] = MMC_KERNEL_SIZE;
	cfg->checksum = checksum_cfg(cfg);
}

static int write_cfg_to_mmc(struct mmc *mmc)
{
	if (mmc->part_num != BOOT_CFG_MMC_PART)
		mmc_switch_part(BOOT_CFG_MMC_DEV, BOOT_CFG_MMC_PART);

	char *ptr = (char *)(((unsigned long)buff + ALIGNED_SZ) & (~(ALIGNED_SZ - 1)));

	g_cfg.checksum = checksum_cfg(&g_cfg);
	memcpy(ptr, &g_cfg, sizeof(g_cfg));

	//write cfg
	mmc->block_dev.block_write(BOOT_CFG_MMC_DEV, 0, 1, ptr);
	//write backup cfg
	mmc->block_dev.block_write(BOOT_CFG_MMC_DEV, BOOT_CFG2_OFFSET/mmc->read_bl_len, 1, ptr);

	if (mmc->part_num != BOOT_CFG_MMC_PART)
		mmc_switch_part(BOOT_CFG_MMC_DEV, mmc->part_num);

	return 0;
}

static int read_cfg_from_mmc(struct mmc *mmc)
{
	if (mmc->part_num != BOOT_CFG_MMC_PART)
		mmc_switch_part(BOOT_CFG_MMC_DEV, BOOT_CFG_MMC_PART);

	char *ptr = (char *)(((unsigned long)buff + ALIGNED_SZ) & (~(ALIGNED_SZ - 1)));
	//read cfg
	mmc->block_dev.block_read(BOOT_CFG_MMC_DEV, 0, 1, ptr);
	memcpy(&g_cfg, ptr, sizeof(g_cfg));

	if (g_cfg.checksum != checksum_cfg(&g_cfg)) {
		//read backup cfg
		mmc->block_dev.block_read(BOOT_CFG_MMC_DEV, BOOT_CFG2_OFFSET/mmc->read_bl_len, 1, ptr);
		memcpy(&g_cfg, ptr, sizeof(g_cfg));

		if (g_cfg.checksum != checksum_cfg(&g_cfg))
			recover_orig_cfg(&g_cfg);

		write_cfg_to_mmc(mmc);
	}

	if (mmc->part_num != BOOT_CFG_MMC_PART)
		mmc_switch_part(BOOT_CFG_MMC_DEV, mmc->part_num);

	return 0;
}

static void set_new_boot_env(struct mmc *mmc)
{
	char cmdbuf[512];
	int kernel_id = g_cfg.run_version;
	int system_id = g_cfg.system_id_map[kernel_id - 1];

	printf("saic auto boot kernel:%d, system:%d\n", kernel_id, system_id);

	sprintf(cmdbuf, saic_bootargs, system_id);
	setenv("bootargs", cmdbuf);	

	unsigned long mmc_start = MMC_KERNEL_START + (kernel_id - 1)*MMC_KERNEL_SIZE;
	unsigned long blkstart = mmc_start / mmc->read_bl_len;
	unsigned long blkcnt = (g_cfg.kernel_size[kernel_id - 1] + mmc->read_bl_len - 1) / mmc->read_bl_len;
	sprintf(cmdbuf, saic_bootcmd, blkstart, blkcnt);
	setenv("bootcmd", cmdbuf);
}

static int cfg_for_roolback(void)
{
	if (g_cfg.valid_version == g_cfg.run_version) {
		g_cfg.next_version = 0;
	} else {
		g_cfg.next_version = g_cfg.run_version;
		g_cfg.run_version = g_cfg.valid_version;
	}

	return 0;
}

int saic_auto_boot_set(void)
{
	struct mmc *mmc = find_mmc_device(BOOT_CFG_MMC_DEV);
	if (!mmc) {
		printf("no mmc device at slot %x\n", BOOT_CFG_MMC_DEV);
		return -1;
	}

	read_cfg_from_mmc(mmc);
	set_new_boot_env(mmc);

	cfg_for_roolback();
	write_cfg_to_mmc(mmc);

#ifndef DEBUG_VER
	/* enable watchdog */
	extern void hw_watchdog_init(void);
	hw_watchdog_init();
#endif

	return 0;
}

