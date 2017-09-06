/*
 * Copyright (C) 2010-2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <div64.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#ifdef CONFIG_UBOOT_LOGO_ENABLE
#include <asm/imx-common/mxc_ipu.h>
#endif

enum pll_clocks {
	PLL_SYS,	/* System PLL */
	PLL_BUS,	/* System Bus PLL*/
	PLL_USBOTG,	/* OTG USB PLL */
#ifdef CONFIG_UBOOT_LOGO_ENABLE
	PLL_VIDEO,
#endif
	PLL_ENET,	/* ENET PLL */
};

#ifdef CONFIG_UBOOT_LOGO_ENABLE
#define PLL5_FREQ_MIN	650000000
#define PLL5_FREQ_MAX	1300000000
#endif

struct mxc_ccm_reg *imx_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

#ifdef CONFIG_MXC_OCOTP
void enable_ocotp_clk(unsigned char enable)
{
	u32 reg;

	reg = __raw_readl(&imx_ccm->CCGR2);
	if (enable)
		reg |= MXC_CCM_CCGR2_OCOTP_CTRL_MASK;
	else
		reg &= ~MXC_CCM_CCGR2_OCOTP_CTRL_MASK;
	__raw_writel(reg, &imx_ccm->CCGR2);
}
#endif

void enable_usboh3_clk(unsigned char enable)
{
	u32 reg;

	reg = __raw_readl(&imx_ccm->CCGR6);
	if (enable)
		reg |= MXC_CCM_CCGR6_USBOH3_MASK;
	else
		reg &= ~(MXC_CCM_CCGR6_USBOH3_MASK);
	__raw_writel(reg, &imx_ccm->CCGR6);

}

#ifdef CONFIG_SYS_I2C_MXC
/* i2c_num can be from 0 - 2 */
int enable_i2c_clk(unsigned char enable, unsigned i2c_num)
{
	u32 reg;
	u32 mask;

	if (i2c_num > 2)
		return -EINVAL;

	mask = MXC_CCM_CCGR_CG_MASK
		<< (MXC_CCM_CCGR2_I2C1_SERIAL_OFFSET + (i2c_num << 1));
	reg = __raw_readl(&imx_ccm->CCGR2);
	if (enable)
		reg |= mask;
	else
		reg &= ~mask;
	__raw_writel(reg, &imx_ccm->CCGR2);
	return 0;
}
#endif

static u32 decode_pll(enum pll_clocks pll, u32 infreq)
{
#ifdef CONFIG_UBOOT_LOGO_ENABLE
	u32 post_div, temp, mfn, mfd;
#endif
	u32 div;

	switch (pll) {
	case PLL_SYS:
		div = __raw_readl(&imx_ccm->analog_pll_sys);
		div &= BM_ANADIG_PLL_SYS_DIV_SELECT;

		return (infreq * div) >> 1;
	case PLL_BUS:
		div = __raw_readl(&imx_ccm->analog_pll_528);
		div &= BM_ANADIG_PLL_528_DIV_SELECT;

		return infreq * (20 + (div << 1));
	case PLL_USBOTG:
		div = __raw_readl(&imx_ccm->analog_usb1_pll_480_ctrl);
		div &= BM_ANADIG_USB1_PLL_480_CTRL_DIV_SELECT;

		return infreq * (20 + (div << 1));

#ifdef CONFIG_UBOOT_LOGO_ENABLE
	case PLL_VIDEO:
		temp = __raw_readl(&imx_ccm->analog_pll_video) & 
			BM_ANADIG_PLL_VIDEO_TEST_DIV_SELECT;
		temp = temp >> BP_ANADIG_PLL_VIDEO_TEST_DIV_SELECT;
		if (temp == 0)
			post_div = 4;
		else if (temp == 1)
			post_div = 2;
		else
			post_div = 1;

		temp = __raw_readl(&imx_ccm->ana_misc2) & 
			BM_ANADIG_ANA_MISC2_CONTROL3;
		temp = temp >> BP_ANADIG_ANA_MISC2_CONTROL3;
		if (temp == 1)
			post_div *= 2;
		else if (temp == 3)
			post_div *= 4;

		div = __raw_readl(&imx_ccm->analog_pll_video) & 
			BM_ANADIG_PLL_VIDEO_DIV_SELECT;
		mfn = __raw_readl(&imx_ccm->analog_pll_video_num) & 
			BM_ANADIG_PLL_VIDEO_NUM_A;
		mfd = __raw_readl(&imx_ccm->analog_pll_video_denon) & 
			BM_ANADIG_PLL_VIDEO_DENOM_B;

		temp = (infreq * div) + ((infreq / mfd) * mfn);
		return (temp / post_div);
#endif

	case PLL_ENET:
		div = __raw_readl(&imx_ccm->analog_pll_enet);
		div &= BM_ANADIG_PLL_ENET_DIV_SELECT;

		return 25000000 * (div + (div >> 1) + 1);
	default:
		return 0;
	}
	/* NOTREACHED */
}
static u32 mxc_get_pll_pfd(enum pll_clocks pll, int pfd_num)
{
	u32 div;
	u64 freq;

	switch (pll) {
	case PLL_BUS:
		if (pfd_num == 3) {
			/* No PFD3 on PPL2 */
			return 0;
		}
		div = __raw_readl(&imx_ccm->analog_pfd_528);
		freq = (u64)decode_pll(PLL_BUS, MXC_HCLK);
		break;
	case PLL_USBOTG:
		div = __raw_readl(&imx_ccm->analog_pfd_480);
		freq = (u64)decode_pll(PLL_USBOTG, MXC_HCLK);
		break;
	default:
		/* No PFD on other PLL					     */
		return 0;
	}

	return lldiv(freq * 18, (div & ANATOP_PFD_FRAC_MASK(pfd_num)) >>
			      ANATOP_PFD_FRAC_SHIFT(pfd_num));
}

static u32 get_mcu_main_clk(void)
{
	u32 reg, freq;

	reg = __raw_readl(&imx_ccm->cacrr);
	reg &= MXC_CCM_CACRR_ARM_PODF_MASK;
	reg >>= MXC_CCM_CACRR_ARM_PODF_OFFSET;
	freq = decode_pll(PLL_SYS, MXC_HCLK);

	return freq / (reg + 1);
}

u32 get_periph_clk(void)
{
	u32 reg, freq = 0;

	reg = __raw_readl(&imx_ccm->cbcdr);
	if (reg & MXC_CCM_CBCDR_PERIPH_CLK_SEL) {
		reg = __raw_readl(&imx_ccm->cbcmr);
		reg &= MXC_CCM_CBCMR_PERIPH_CLK2_SEL_MASK;
		reg >>= MXC_CCM_CBCMR_PERIPH_CLK2_SEL_OFFSET;

		switch (reg) {
		case 0:
			freq = decode_pll(PLL_USBOTG, MXC_HCLK);
			break;
		case 1:
		case 2:
			freq = MXC_HCLK;
			break;
		default:
			break;
		}
	} else {
		reg = __raw_readl(&imx_ccm->cbcmr);
		reg &= MXC_CCM_CBCMR_PRE_PERIPH_CLK_SEL_MASK;
		reg >>= MXC_CCM_CBCMR_PRE_PERIPH_CLK_SEL_OFFSET;

		switch (reg) {
		case 0:
			freq = decode_pll(PLL_BUS, MXC_HCLK);
			break;
		case 1:
			freq = mxc_get_pll_pfd(PLL_BUS, 2);
			break;
		case 2:
			freq = mxc_get_pll_pfd(PLL_BUS, 0);
			break;
		case 3:
			/* static / 2 divider */
			freq = mxc_get_pll_pfd(PLL_BUS, 2) / 2;
			break;
		default:
			break;
		}
	}

	return freq;
}

static u32 get_ipg_clk(void)
{
	u32 reg, ipg_podf;

	reg = __raw_readl(&imx_ccm->cbcdr);
	reg &= MXC_CCM_CBCDR_IPG_PODF_MASK;
	ipg_podf = reg >> MXC_CCM_CBCDR_IPG_PODF_OFFSET;

	return get_ahb_clk() / (ipg_podf + 1);
}

static u32 get_ipg_per_clk(void)
{
	u32 reg, perclk_podf;

	reg = __raw_readl(&imx_ccm->cscmr1);
#if (defined(CONFIG_MX6SL) || defined(CONFIG_MX6SX))
	if (reg & MXC_CCM_CSCMR1_PER_CLK_SEL_MASK)
		return MXC_HCLK; /* OSC 24Mhz */
#endif
	perclk_podf = reg & MXC_CCM_CSCMR1_PERCLK_PODF_MASK;

	return get_ipg_clk() / (perclk_podf + 1);
}

static u32 get_uart_clk(void)
{
	u32 reg, uart_podf;
	u32 freq = decode_pll(PLL_USBOTG, MXC_HCLK) / 6; /* static divider */
	reg = __raw_readl(&imx_ccm->cscdr1);
#if (defined(CONFIG_MX6SL) || defined(CONFIG_MX6SX))
	if (reg & MXC_CCM_CSCDR1_UART_CLK_SEL)
		freq = MXC_HCLK;
#endif
	reg &= MXC_CCM_CSCDR1_UART_CLK_PODF_MASK;
	uart_podf = reg >> MXC_CCM_CSCDR1_UART_CLK_PODF_OFFSET;

	return freq / (uart_podf + 1);
}

static u32 get_cspi_clk(void)
{
	u32 reg, cspi_podf;

	reg = __raw_readl(&imx_ccm->cscdr2);
	reg &= MXC_CCM_CSCDR2_ECSPI_CLK_PODF_MASK;
	cspi_podf = reg >> MXC_CCM_CSCDR2_ECSPI_CLK_PODF_OFFSET;

	return	decode_pll(PLL_USBOTG, MXC_HCLK) / (8 * (cspi_podf + 1));
}

static u32 get_axi_clk(void)
{
	u32 root_freq, axi_podf;
	u32 cbcdr =  __raw_readl(&imx_ccm->cbcdr);

	axi_podf = cbcdr & MXC_CCM_CBCDR_AXI_PODF_MASK;
	axi_podf >>= MXC_CCM_CBCDR_AXI_PODF_OFFSET;

	if (cbcdr & MXC_CCM_CBCDR_AXI_SEL) {
		if (cbcdr & MXC_CCM_CBCDR_AXI_ALT_SEL)
			root_freq = mxc_get_pll_pfd(PLL_BUS, 2);
		else
			root_freq = mxc_get_pll_pfd(PLL_USBOTG, 1);
	} else
		root_freq = get_periph_clk();

	return  root_freq / (axi_podf + 1);
}

static u32 get_emi_slow_clk(void)
{
	u32 emi_clk_sel, emi_slow_podf, cscmr1, root_freq = 0;

	cscmr1 =  __raw_readl(&imx_ccm->cscmr1);
	emi_clk_sel = cscmr1 & MXC_CCM_CSCMR1_ACLK_EMI_SLOW_MASK;
	emi_clk_sel >>= MXC_CCM_CSCMR1_ACLK_EMI_SLOW_OFFSET;
	emi_slow_podf = cscmr1 & MXC_CCM_CSCMR1_ACLK_EMI_SLOW_PODF_MASK;
	emi_slow_podf >>= MXC_CCM_CSCMR1_ACLK_EMI_SLOW_PODF_OFFSET;

	switch (emi_clk_sel) {
	case 0:
		root_freq = get_axi_clk();
		break;
	case 1:
		root_freq = decode_pll(PLL_USBOTG, MXC_HCLK);
		break;
	case 2:
		root_freq =  mxc_get_pll_pfd(PLL_BUS, 2);
		break;
	case 3:
		root_freq =  mxc_get_pll_pfd(PLL_BUS, 0);
		break;
	}

	return root_freq / (emi_slow_podf + 1);
}

#if (defined(CONFIG_MX6SL) || defined(CONFIG_MX6SX))
static u32 get_mmdc_ch0_clk(void)
{
	u32 cbcmr = __raw_readl(&imx_ccm->cbcmr);
	u32 cbcdr = __raw_readl(&imx_ccm->cbcdr);
	u32 freq, podf;

	podf = (cbcdr & MXC_CCM_CBCDR_MMDC_CH1_PODF_MASK) \
			>> MXC_CCM_CBCDR_MMDC_CH1_PODF_OFFSET;

	switch ((cbcmr & MXC_CCM_CBCMR_PRE_PERIPH2_CLK_SEL_MASK) >>
		MXC_CCM_CBCMR_PRE_PERIPH2_CLK_SEL_OFFSET) {
	case 0:
		freq = decode_pll(PLL_BUS, MXC_HCLK);
		break;
	case 1:
		freq = mxc_get_pll_pfd(PLL_BUS, 2);
		break;
	case 2:
		freq = mxc_get_pll_pfd(PLL_BUS, 0);
		break;
	case 3:
		/* static / 2 divider */
		freq =  mxc_get_pll_pfd(PLL_BUS, 2) / 2;
	}

	return freq / (podf + 1);

}
#else
static u32 get_mmdc_ch0_clk(void)
{
	u32 cbcdr = __raw_readl(&imx_ccm->cbcdr);
	u32 mmdc_ch0_podf = (cbcdr & MXC_CCM_CBCDR_MMDC_CH0_PODF_MASK) >>
				MXC_CCM_CBCDR_MMDC_CH0_PODF_OFFSET;

	return get_periph_clk() / (mmdc_ch0_podf + 1);
}
#endif

#ifdef CONFIG_MX6SX
void enable_lvds(uint32_t lcdif_base)
{
	u32 reg = 0;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_GPR_BASE_ADDR;

	/* Turn on LDB DI0 clocks */
	reg = readl(&imx_ccm->CCGR3);
	reg |=  MXC_CCM_CCGR3_LDB_DI0_MASK;
	writel(reg, &imx_ccm->CCGR3);

	/* set LDB DI0 clk select to 011 PLL2 PFD3 200M*/
	reg = readl(&imx_ccm->cs2cdr);
	reg &= ~MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK;
	reg |= (3 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET);
	writel(reg, &imx_ccm->cs2cdr);

	reg = readl(&imx_ccm->cscmr2);
	reg |= MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV;
	writel(reg, &imx_ccm->cscmr2);

	/* set LDB DI0 clock for LCDIF PIX clock */
	reg = readl(&imx_ccm->cscdr2);
	if (lcdif_base == LCDIF1_BASE_ADDR) {
		reg &= ~MXC_CCM_CSCDR2_LCDIF1_CLK_SEL_MASK;
		reg |= (0x3 << MXC_CCM_CSCDR2_LCDIF1_CLK_SEL_OFFSET);
	} else {
		reg &= ~MXC_CCM_CSCDR2_LCDIF2_CLK_SEL_MASK;
		reg |= (0x3 << MXC_CCM_CSCDR2_LCDIF2_CLK_SEL_OFFSET);
	}
	writel(reg, &imx_ccm->cscdr2);

	reg = IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW
		| IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG
		| IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT
		| IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0;
	writel(reg, &iomux->gpr[6]);

	reg = readl(&iomux->gpr[5]);
	if (lcdif_base == LCDIF1_BASE_ADDR)
		reg &= ~0x8;  /* MUX LVDS to LCDIF1 */
	else
		reg |= 0x8; /* MUX LVDS to LCDIF2 */
	writel(reg, &iomux->gpr[5]);
}

void enable_lcdif_clock(uint32_t base_addr)
{
	u32 reg = 0;

	/* Set to pre-mux clock at default */
	reg = readl(&imx_ccm->cscdr2);
	if (base_addr == LCDIF1_BASE_ADDR)
		reg &= ~MXC_CCM_CSCDR2_LCDIF1_CLK_SEL_MASK;
	else
		reg &= ~MXC_CCM_CSCDR2_LCDIF2_CLK_SEL_MASK;
	writel(reg, &imx_ccm->cscdr2);

	/* Enable the LCDIF pix clock, axi clock, disp axi clock */
	reg = readl(&imx_ccm->CCGR3);
	if (base_addr == LCDIF1_BASE_ADDR)
		reg |= (MXC_CCM_CCGR3_LCDIF1_PIX_MASK | MXC_CCM_CCGR3_DISP_AXI_MASK);
	else
		reg |= (MXC_CCM_CCGR3_LCDIF2_PIX_MASK | MXC_CCM_CCGR3_DISP_AXI_MASK);
	writel(reg, &imx_ccm->CCGR3);

	reg = readl(&imx_ccm->CCGR2);
	reg |= (MXC_CCM_CCGR2_LCD_MASK);
	writel(reg, &imx_ccm->CCGR2);
}

static int enable_pll_video(u32 pll_div, u32 pll_num, u32 pll_denom)
{
	u32 reg = 0;
	ulong start;

	debug("pll5 div = %d, num = %d, denom = %d\n",
		pll_div, pll_num, pll_denom);

	/* Power up PLL5 video */
	writel(BM_ANADIG_PLL_VIDEO_POWERDOWN | BM_ANADIG_PLL_VIDEO_BYPASS |
		BM_ANADIG_PLL_VIDEO_DIV_SELECT | BM_ANADIG_PLL_VIDEO_TEST_DIV_SELECT,
		&imx_ccm->analog_pll_video_clr);

	/* Set div, num and denom */
	writel(BF_ANADIG_PLL_VIDEO_DIV_SELECT(pll_div) |
		BF_ANADIG_PLL_VIDEO_TEST_DIV_SELECT(0x2),
		&imx_ccm->analog_pll_video_set);

	writel(BF_ANADIG_PLL_VIDEO_NUM_A(pll_num),
		&imx_ccm->analog_pll_video_num);

	writel(BF_ANADIG_PLL_VIDEO_DENOM_B(pll_denom),
		&imx_ccm->analog_pll_video_denon);

	/* Wait PLL5 lock */
	start = get_timer(0);	/* Get current timestamp */

	do {
		reg = readl(&imx_ccm->analog_pll_video);
		if (reg & BM_ANADIG_PLL_VIDEO_LOCK) {
			/* Enable PLL out */
			writel(BM_ANADIG_PLL_VIDEO_ENABLE,
					&imx_ccm->analog_pll_video_set);
			return 0;
		}
	} while (get_timer(0) < (start + 10)); /* Wait 10ms */

	printf("Lock PLL5 timeout\n");
	return 1;

}

void mxs_set_lcdclk(uint32_t base_addr, uint32_t freq)
{
	u32 reg = 0;
	u32 hck = MXC_HCLK/1000;
	u32 min = hck * 27;
	u32 max = hck * 54;
	u32 temp, best = 0;
	u32 i, j, pred = 1, postd = 1;
	u32 pll_div, pll_num, pll_denom;

	debug("mxs_set_lcdclk, freq = %d\n", freq);

	if (base_addr == LCDIF1_BASE_ADDR) {
		reg = readl(&imx_ccm->cscdr2);
		if ((reg & MXC_CCM_CSCDR2_LCDIF1_CLK_SEL_MASK) != 0)
			return; /*Can't change clocks when clock not from pre-mux */
	} else {
		reg = readl(&imx_ccm->cscdr2);
		if ((reg & MXC_CCM_CSCDR2_LCDIF2_CLK_SEL_MASK) != 0)
			return; /*Can't change clocks when clock not from pre-mux */
	}

	for (i = 1; i <= 8; i++) {
		for (j = 1; j <= 8; j++) {
			temp = freq * i * j;
			if (temp > max || temp < min)
				continue;

			if (best == 0 || temp < best) {
				best = temp;
				pred = i;
				postd = j;
			}
		}
	}

	if (best == 0) {
		printf("Fail to set rate to %dkhz", freq);
		return;
	}

	debug("best %d, pred = %d, postd = %d\n", best, pred, postd);

	pll_div = best / hck;
	pll_denom = 1000000;
	pll_num = (best - hck * pll_div) * pll_denom / hck;

	if (base_addr == LCDIF1_BASE_ADDR) {
		if (enable_pll_video(pll_div, pll_num, pll_denom))
			return;

		/* Select pre-lcd clock to PLL5 */
		reg = readl(&imx_ccm->cscdr2);
		reg &= ~MXC_CCM_CSCDR2_LCDIF1_PRED_SEL_MASK;
		reg |= (0x2 << MXC_CCM_CSCDR2_LCDIF1_PRED_SEL_OFFSET);
		/* Set the pre divider */
		reg &= ~MXC_CCM_CSCDR2_LCDIF1_PRE_DIV_MASK;
		reg |= ((pred - 1) << MXC_CCM_CSCDR2_LCDIF1_PRE_DIV_OFFSET);
		writel(reg, &imx_ccm->cscdr2);

		/* Set the post divider */
		reg = readl(&imx_ccm->cbcmr);
		reg &= ~MXC_CCM_CBCMR_LCDIF1_PODF_MASK;
		reg |= ((postd - 1) << MXC_CCM_CBCMR_LCDIF1_PODF_OFFSET);
		writel(reg, &imx_ccm->cbcmr);

	} else {
		if (enable_pll_video(pll_div, pll_num, pll_denom))
			return;

		/* Select pre-lcd clock to PLL5 */
		reg = readl(&imx_ccm->cscdr2);
		reg &= ~MXC_CCM_CSCDR2_LCDIF2_PRED_SEL_MASK;
		reg |= (0x2 << MXC_CCM_CSCDR2_LCDIF2_PRED_SEL_OFFSET);
		/* Set the pre divider */
		reg &= ~MXC_CCM_CSCDR2_LCDIF2_PRE_DIV_MASK;
		reg |= ((pred - 1) << MXC_CCM_CSCDR2_LCDIF2_PRE_DIV_OFFSET);
		writel(reg, &imx_ccm->cscdr2);

		/* Set the post divider */
		reg = readl(&imx_ccm->cscmr1);
		reg &= ~MXC_CCM_CSCMR1_LCDIF2_PODF_MASK;
		reg |= ((postd - 1) << MXC_CCM_CSCMR1_LCDIF2_PODF_OFFSET);
		writel(reg, &imx_ccm->cscmr1);
	}
}

/* qspi_num can be from 0 - 1 */
void enable_qspi_clk(int qspi_num)
{
    u32 reg = 0;

    /* Enable QuadSPI clock */
	switch (qspi_num) {
	case 0:
		/*disable the clock gate*/
		clrbits_le32(&imx_ccm->CCGR3, MXC_CCM_CCGR3_QSPI1_MASK);

		/* set 50M  : (50 = 396 / 2 / 4) */
		reg = readl(&imx_ccm->cscmr1);
		reg &= ~(MXC_CCM_CSCMR1_QSPI1_PODF_MASK |
			MXC_CCM_CSCMR1_QSPI1_CLK_SEL_MASK);
		reg |= ((1 << MXC_CCM_CSCMR1_QSPI1_PODF_OFFSET) |
				(2 << MXC_CCM_CSCMR1_QSPI1_CLK_SEL_OFFSET));
		writel(reg, &imx_ccm->cscmr1);

		/*enable the clock gate*/
		setbits_le32(&imx_ccm->CCGR3, MXC_CCM_CCGR3_QSPI1_MASK);
		break;
	case 1:
		/*disable the clock gatei*/
		/*QSPI2 and GPMI_BCH_INPUT_GPMI_IO share the same clock gate, disable both of them*/
		clrbits_le32(&imx_ccm->CCGR4, MXC_CCM_CCGR4_QSPI2_ENFC_MASK
				| MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_GPMI_IO_MASK);

		/* set 50M  : (50 = 396 / 2 / 4) */
		reg = readl(&imx_ccm->cs2cdr);
		reg &= ~(MXC_CCM_CS2CDR_QSPI2_CLK_PODF_MASK |
				MXC_CCM_CS2CDR_QSPI2_CLK_PRED_MASK |
				MXC_CCM_CS2CDR_QSPI2_CLK_SEL_MASK);
		reg |= (MXC_CCM_CS2CDR_QSPI2_CLK_PRED(0x1) |
				MXC_CCM_CS2CDR_QSPI2_CLK_SEL(0x3));
		writel(reg, &imx_ccm->cs2cdr);

		/*enable the clock gate*/
		setbits_le32(&imx_ccm->CCGR4, MXC_CCM_CCGR4_QSPI2_ENFC_MASK
				| MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_GPMI_IO_MASK);
		break;
	default:
		break;
	}
}

void enable_enet_clock(void)
{
	u32 reg = 0;

	/* set enet ahb clock 200Mhz
	 * pll2_pfd2_396m-> ENET_PODF-> ENET_AHB
	 */
	reg = __raw_readl(&imx_ccm->chsccdr);
	reg &= ~(MXC_CCM_CHSCCDR_ENET_PRE_CLK_SEL_MASK
		| MXC_CCM_CHSCCDR_ENET_PODF_MASK | MXC_CCM_CHSCCDR_ENET_CLK_SEL_MASK);
	/* PLL2 PFD2 */
	reg |= (4 << MXC_CCM_CHSCCDR_ENET_PRE_CLK_SEL_OFFSET);
	/* Div = 2*/
	reg |= (1 << MXC_CCM_CHSCCDR_ENET_PODF_OFFSET);
	reg |= (0 << MXC_CCM_CHSCCDR_ENET_CLK_SEL_OFFSET);
	writel(reg, &imx_ccm->chsccdr);

	/* Enable enet system clock */
	reg = readl(&imx_ccm->CCGR3);
	reg |= MXC_CCM_CCGR3_ENET_MASK;
	writel(reg, &imx_ccm->CCGR3);
}

void mxs_set_vadcclk()
{
	u32 reg = 0;

	reg = readl(&imx_ccm->cscmr2);
	reg &= ~MXC_CCM_CSCMR2_VID_CLK_SEL_MASK;
	reg |= 0x19 << MXC_CCM_CSCMR2_VID_CLK_SEL_OFFSET;
	writel(reg, &imx_ccm->cscmr2);
}
#endif

#ifdef CONFIG_UBOOT_LOGO_ENABLE
static int config_lvds_clk(u32 ipu, u32 di, u32 freq)
{
	u32 divider;
	unsigned int reg;

	printf("config_ipu_lvds_clk: freq = %d.\r\n", freq);

#ifdef CONFIG_MX6Q
	__raw_writel(BM_ANADIG_PFD_528_PFD2_CLKGATE, &imx_ccm->analog_pfd_528_set);
#endif
	
	/*
	 * Need to follow a strict procedure when changing the LDB
	 * clock, else we can introduce a glitch. Things to keep in
	 * mind:
	 * 1. The current and new parent clocks must be disabled.
	 * 2. The default clock for ldb_dio_clk is mmdc_ch1 which has
	 * no CG bit.
	 * 3. In the RTL implementation of the LDB_DI_CLK_SEL mux
	 * the top four options are in one mux and the PLL3 option along
	 * with another option is in the second mux. There is third mux
	 * used to decide between the first and second mux.
	 * The code below switches the parent to the bottom mux first
	 * and then manipulates the top mux. This ensures that no glitch
	 * will enter the divider.
	 *
	 * Need to disable MMDC_CH1 clock manually as there is no CG bit
	 * for this clock. The only way to disable this clock is to move
	 * it topll3_sw_clk and then to disable pll3_sw_clk
	 * Make sure periph2_clk2_sel is set to pll3_sw_clk
	 */
	reg = __raw_readl(&imx_ccm->cbcmr);
	reg &= ~(1 << 20);
	__raw_writel(reg, &imx_ccm->cbcmr);
	
	/*
	 * Set MMDC_CH1 mask bit.
	 */
	reg = __raw_readl(&imx_ccm->ccdr);
	reg |= 1 << 16;
	__raw_writel(reg, &imx_ccm->ccdr);

	/*
	 * Set the periph2_clk_sel to the top mux so that
	 * mmdc_ch1 is from pll3_sw_clk.
	 */
	reg = __raw_readl(&imx_ccm->cbcdr);
	reg |= 1 << 26;
	__raw_writel(reg, &imx_ccm->cbcdr);

	/*
	 * Wait for the clock switch.
	 */
	while (__raw_readl(&imx_ccm->cdhipr))
		;
	
	/*
	 * Disable pll3_sw_clk by selecting the bypass clock source.
	 */
	reg = __raw_readl(&imx_ccm->ccsr);
	reg |= 1 << 0;
	__raw_writel(reg, &imx_ccm->ccsr);

	/*
	 * Set the ldb_di0_clk and ldb_di1_clk to 111b.
	 */
	reg = __raw_readl(&imx_ccm->cs2cdr);
	reg |= ((7 << 9) | (7 << 12));
	__raw_writel(reg, &imx_ccm->cs2cdr);

	/*
	 * Set the ldb_di0_clk and ldb_di1_clk to 100b.
	 */
	reg = __raw_readl(&imx_ccm->cs2cdr);
	reg &= ~((7 << 9) | (7 << 12));
	reg |= ((4 << 9) | (4 << 12));
	__raw_writel(reg, &imx_ccm->cs2cdr);

	/* Set ldb_di_clk clock source to PLL2 PFD0 */
	reg = __raw_readl(&imx_ccm->cs2cdr);
	if (di == 0) {
		reg &= ~(0x7 << 9);
		reg |= (0x1 << 9);
	} else if (di == 1) {
		reg &= ~(0x7 << 12);
		reg |= (0x1 << 12);
	}
	__raw_writel(reg, &imx_ccm->cs2cdr);

	/*
	 * Unbypass pll3_sw_clk.
	 */
	reg = __raw_readl(&imx_ccm->ccsr);
	reg &= ~(1 << 0);
	__raw_writel(reg, &imx_ccm->ccsr);

	/*
	 * Set the periph2_clk_sel back to the bottom mux so that
	 * mmdc_ch1 is from its original parent.
	 */
	reg = __raw_readl(&imx_ccm->cbcdr);
	reg &= ~(1 << 26);
	__raw_writel(reg, &imx_ccm->cbcdr);

	/*
	 * Wait for the clock switch.
	 */
	while (__raw_readl(&imx_ccm->cdhipr))
		;

	/*
	 * Clear MMDC_CH1 mask bit.
	 */
	reg = __raw_readl(&imx_ccm->ccdr);
	reg &= ~(1 << 16);
	__raw_writel(reg, &imx_ccm->ccdr);

	/* Set PLL2 PFD0 Clock */
	divider = (decode_pll(PLL_BUS, MXC_HCLK) / 1000000) * 18 / (freq / 1000000);
	if(divider < 12)
		divider = 12;
	if(divider > 35)
		divider = 35;

	printf("config_ipu_lvds_clk: set pll2_pfd0 clock to %dMHz, divider = %d.\r\n", decode_pll(PLL_BUS, MXC_HCLK) /1000000  / divider * 18, divider);

#ifdef CONFIG_MX6Q
	__raw_writel(BM_ANADIG_PFD_528_PFD2_CLKGATE, &imx_ccm->analog_pfd_528_clr);
#endif

	reg = __raw_readl(&imx_ccm->analog_pfd_528);
	reg &= ~BM_ANADIG_PFD_528_PFD0_FRAC;
	reg |= (divider << BP_ANADIG_PFD_528_PFD0_FRAC);
	__raw_writel(BM_ANADIG_PFD_528_PFD0_FRAC, &imx_ccm->analog_pfd_528_clr);
	__raw_writel(reg, &imx_ccm->analog_pfd_528_set);


#ifdef LVDS_SPLIT_MODE
	/* Set ipu_di clock to ldb_di_clk/3.5 */
	reg = __raw_readl(&imx_ccm->cscmr2);
	if (di == 0)
		reg &= ~(0x1 << 10);
	else if (di == 1)
		reg &= ~(0x1 << 11);
#else
	/* Set ipu_di clock to ldb_di_clk/7 */
	reg = __raw_readl(&imx_ccm->cscmr2);
	if (di == 0)
		reg |= (0x1 << 10);
	else if (di == 1)
		reg |= (0x1 << 11);
#endif
	__raw_writel(reg, &imx_ccm->cscmr2);

	/* Set ipu_di_clk clock source to ldb_di_clk, and root clock pre-multiplexer from PLL2 PFD0, ipu_di_podf divide by 1 */
	if (ipu == 1) {
		reg = __raw_readl(&imx_ccm->chsccdr);
		if (di == 0) {
			reg &= ~(0x1FF << 0);
			reg |= ((3 << 0) | (3 << 6));
		} else if (di == 1) {
			reg &= ~(0x1FF << 9);
			reg |= ((4 << 9) | (3 << 15));
		}
		__raw_writel(reg, &imx_ccm->chsccdr);
	}

	if (ipu == 2) {
		reg = __raw_readl(&imx_ccm->cscdr2);
		if (di == 0) {
			reg &= ~(0x1FF << 0);
			reg |= ((3 << 0) | (3 << 6));
		} else if (di == 1) {
			reg &= ~(0x1FF << 9);
			reg |= ((4 << 9) | (3 << 15));
		}
		__raw_writel(reg, &imx_ccm->cscdr2);
	}
	
	return 0;
}

static int config_ipu_di_clk(u32 ipu, u32 di, u32 freq)
{
	unsigned int reg;

	u32 pre_div_rate, divider;
	u32 test_div_sel = 2;
	u32 control3 = 0;
	u64 temp64;
	u32 mfn, mfd = 1000000;

	printf("config_ipu_di_clk: freq = %d.\r\n", freq);

	/* Set PLL5 Clock */
	/* Disable the PLL first */
	reg = __raw_readl(&imx_ccm->analog_pll_video);
	reg |= BM_ANADIG_PLL_VIDEO_BYPASS;
	reg &= ~BM_ANADIG_PLL_VIDEO_ENABLE;
	__raw_writel(reg, &imx_ccm->analog_pll_video);

	pre_div_rate = freq * 2;
	while (pre_div_rate < PLL5_FREQ_MIN) {
		pre_div_rate *= 2;
		/*
		 * test_div_sel field values:
		 * 2 -> Divide by 1
		 * 1 -> Divide by 2
		 * 0 -> Divide by 4
		 *
		 * control3 field values:
		 * 0 -> Divide by 1
		 * 1 -> Divide by 2
		 * 3 -> Divide by 4
		 */
		if (test_div_sel != 0)
			test_div_sel --;
		else {
			control3 ++;
			if (control3 == 2)
				control3 ++;
		}
	}
	divider = pre_div_rate / MXC_HCLK;
	temp64 = (u64) (pre_div_rate - (divider * MXC_HCLK));
	temp64 *= mfd;
	do_div(temp64, MXC_HCLK);
	mfn = temp64;

	reg = __raw_readl(&imx_ccm->analog_pll_video)
			& ~(BM_ANADIG_PLL_VIDEO_DIV_SELECT | BM_ANADIG_PLL_VIDEO_TEST_DIV_SELECT);
	reg |= (divider |
		(test_div_sel << BP_ANADIG_PLL_VIDEO_TEST_DIV_SELECT));
	__raw_writel(reg, &imx_ccm->analog_pll_video);
	__raw_writel(mfn, &imx_ccm->analog_pll_video_num);
	__raw_writel(mfd, &imx_ccm->analog_pll_video_denon);

	reg = __raw_readl(&imx_ccm->ana_misc2)
		& ~BM_ANADIG_ANA_MISC2_CONTROL3;
	reg |= control3 << BP_ANADIG_ANA_MISC2_CONTROL3;
	__raw_writel(reg, &imx_ccm->ana_misc2);

	/* Enable the PLL power */
	reg = __raw_readl(&imx_ccm->analog_pll_video);
	reg &= ~BM_ANADIG_PLL_VIDEO_POWERDOWN;
	__raw_writel(reg, &imx_ccm->analog_pll_video);

	/* Wait for PLL to lock */
	while((__raw_readl(&imx_ccm->analog_pll_video) & BM_ANADIG_PLL_VIDEO_LOCK) == 0)
		printf("wait for pll5 lock.\n");

	/* Enable the PLL output */
	reg = __raw_readl(&imx_ccm->analog_pll_video);
	reg &= ~BM_ANADIG_PLL_VIDEO_BYPASS;
	reg |= BM_ANADIG_PLL_VIDEO_ENABLE;
	__raw_writel(reg, &imx_ccm->analog_pll_video);

	printf("config_ipu_di_clk: set pll5 clock to %dHz.\r\n", decode_pll(PLL_VIDEO, MXC_HCLK));

	/* Set ipu_di_clk clock source to pre-muxed ipu di clock, divided by 2, and root clock pre-muxed from PLL5 */
	if (ipu == 1) {
		reg = __raw_readl(&imx_ccm->chsccdr);
		if (di == 0) {
			reg &= ~0x000001FF;
			reg |= ((0 << 0) | (1 << 3) | (2 << 6));
		} else if (di == 1) {
			reg &= ~0x0003FE00;
			reg |= ((0 << 9) | (1 << 12) | (2 << 15));
		}
		__raw_writel(reg, &imx_ccm->chsccdr);
	}

	if (ipu == 2) {
		reg = __raw_readl(&imx_ccm->cscdr2);
		if (di == 0) {
			reg &= ~0x000001FF;
			reg |= ((0 << 0) | (1 << 3) | (2 << 6));
		} else if (di == 1) {
			reg &= ~0x0003FE00;
			reg |= ((0 << 9) | (1 << 12) | (2 << 15));
		}
		__raw_writel(reg, &imx_ccm->cscdr2);
	}

	return 0;
}

/*!
 *
 * @param clk_type  clock type, e.g MXC_IPU1_LVDS_DI0_CLK, MXC_IPU1_DI0_CLK, etc.
 * @param freq		targeted freq in Hz
 * @return          0 if successful; non-zero otherwise
 */
int display_clk_config(u32 clk_type, u32 freq)
{
	switch (clk_type) {
	case MXC_IPU1_LVDS_DI0_CLK:
		if (config_lvds_clk(1, 0, freq))
			return -1;
		break;
	case MXC_IPU1_LVDS_DI1_CLK:
		if (config_lvds_clk(1, 1, freq))
			return -1;
		break;
	case MXC_IPU2_LVDS_DI0_CLK:
		if (config_lvds_clk(2, 0, freq))
			return -1;
		break;
	case MXC_IPU2_LVDS_DI1_CLK:
		if (config_lvds_clk(2, 1, freq))
			return -1;
		break;
	case MXC_IPU1_DI0_CLK:
		if (config_ipu_di_clk(1, 0, freq))
			return -1;
		break;
	case MXC_IPU1_DI1_CLK:
		if (config_ipu_di_clk(1, 1, freq))
			return -1;
		break;
	case MXC_IPU2_DI0_CLK:
		if (config_ipu_di_clk(2, 0, freq))
			return -1;
		break;
	case MXC_IPU2_DI1_CLK:
		if (config_ipu_di_clk(2, 1, freq))
			return -1;
		break;
	default:
		printf("Unsupported or invalid clock type! :(\n");
		return -1;
	}

	return 0;
}
#endif

#ifdef CONFIG_FEC_MXC
int enable_fec_anatop_clock(int fec_id, enum enet_freq freq)
{
	u32 reg = 0;
	s32 timeout = 100000;

	if (freq < ENET_25MHz || freq > ENET_125MHz)
		return -EINVAL;

	reg = readl(&imx_ccm->analog_pll_enet);
	reg &= ~BM_ANADIG_PLL_ENET_DIV_SELECT;

	if (0 == fec_id) {
		reg &= ~BM_ANADIG_PLL_ENET_DIV_SELECT;
		reg |= BF_ANADIG_PLL_ENET_DIV_SELECT(freq);
	} else {
		reg &= ~BM_ANADIG_PLL_ENET2_DIV_SELECT;
		reg |= BF_ANADIG_PLL_ENET2_DIV_SELECT(freq);
	}

	if ((reg & BM_ANADIG_PLL_ENET_POWERDOWN) ||
	    (!(reg & BM_ANADIG_PLL_ENET_LOCK))) {
		reg &= ~BM_ANADIG_PLL_ENET_POWERDOWN;
		writel(reg, &imx_ccm->analog_pll_enet);
		while (timeout--) {
			if (readl(&imx_ccm->analog_pll_enet) & BM_ANADIG_PLL_ENET_LOCK)
				break;
		}
		if (timeout < 0)
			return -ETIMEDOUT;
	}

	/* Enable FEC clock */
	if (0 == fec_id)
		reg |= BM_ANADIG_PLL_ENET_ENABLE;
	else
		reg |= BM_ANADIG_PLL_ENET2_ENABLE;
	reg &= ~BM_ANADIG_PLL_ENET_BYPASS;
#ifdef CONFIG_FEC_MXC_25M_REF_CLK
	reg |= BM_ANADIG_PLL_ENET_REF_25M_ENABLE;
#endif
	writel(reg, &imx_ccm->analog_pll_enet);

	return 0;
}
#endif

static u32 get_usdhc_clk(u32 port)
{
	u32 root_freq = 0, usdhc_podf = 0, clk_sel = 0;
	u32 cscmr1 = __raw_readl(&imx_ccm->cscmr1);
	u32 cscdr1 = __raw_readl(&imx_ccm->cscdr1);

	switch (port) {
	case 0:
		usdhc_podf = (cscdr1 & MXC_CCM_CSCDR1_USDHC1_PODF_MASK) >>
					MXC_CCM_CSCDR1_USDHC1_PODF_OFFSET;
		clk_sel = cscmr1 & MXC_CCM_CSCMR1_USDHC1_CLK_SEL;

		break;
	case 1:
		usdhc_podf = (cscdr1 & MXC_CCM_CSCDR1_USDHC2_PODF_MASK) >>
					MXC_CCM_CSCDR1_USDHC2_PODF_OFFSET;
		clk_sel = cscmr1 & MXC_CCM_CSCMR1_USDHC2_CLK_SEL;

		break;
	case 2:
		usdhc_podf = (cscdr1 & MXC_CCM_CSCDR1_USDHC3_PODF_MASK) >>
					MXC_CCM_CSCDR1_USDHC3_PODF_OFFSET;
		clk_sel = cscmr1 & MXC_CCM_CSCMR1_USDHC3_CLK_SEL;

		break;
	case 3:
		usdhc_podf = (cscdr1 & MXC_CCM_CSCDR1_USDHC4_PODF_MASK) >>
					MXC_CCM_CSCDR1_USDHC4_PODF_OFFSET;
		clk_sel = cscmr1 & MXC_CCM_CSCMR1_USDHC4_CLK_SEL;

		break;
	default:
		break;
	}

	if (clk_sel)
		root_freq = mxc_get_pll_pfd(PLL_BUS, 0);
	else
		root_freq = mxc_get_pll_pfd(PLL_BUS, 2);

	return root_freq / (usdhc_podf + 1);
}

u32 imx_get_uartclk(void)
{
	return get_uart_clk();
}

u32 imx_get_fecclk(void)
{
	return mxc_get_clock(MXC_IPG_CLK);
}

static int enable_enet_pll(uint32_t en)
{
	struct mxc_ccm_reg *const imx_ccm
		= (struct mxc_ccm_reg *) CCM_BASE_ADDR;
	s32 timeout = 100000;
	u32 reg = 0;

	/* Enable PLLs */
	reg = readl(&imx_ccm->analog_pll_enet);
	reg &= ~BM_ANADIG_PLL_SYS_POWERDOWN;
	writel(reg, &imx_ccm->analog_pll_enet);
	reg |= BM_ANADIG_PLL_SYS_ENABLE;
	while (timeout--) {
		if (readl(&imx_ccm->analog_pll_enet) & BM_ANADIG_PLL_SYS_LOCK)
			break;
	}
	if (timeout <= 0)
		return -EIO;
	reg &= ~BM_ANADIG_PLL_SYS_BYPASS;
	writel(reg, &imx_ccm->analog_pll_enet);
	reg |= en;
	writel(reg, &imx_ccm->analog_pll_enet);
	return 0;
}

#ifndef CONFIG_MX6SX
static void ungate_sata_clock(void)
{
	struct mxc_ccm_reg *const imx_ccm =
		(struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* Enable SATA clock. */
	setbits_le32(&imx_ccm->CCGR5, MXC_CCM_CCGR5_SATA_MASK);
}
#else
static void ungate_disp_axi_clock(void)
{
	struct mxc_ccm_reg *const imx_ccm =
		(struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* Enable display axi clock. */
	setbits_le32(&imx_ccm->CCGR3, MXC_CCM_CCGR3_DISP_AXI_MASK);
}
#endif

static void ungate_pcie_clock(void)
{
	struct mxc_ccm_reg *const imx_ccm =
		(struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* Enable PCIe clock. */
	setbits_le32(&imx_ccm->CCGR4, MXC_CCM_CCGR4_PCIE_MASK);
}

#ifndef CONFIG_MX6SX
int enable_sata_clock(void)
{
	ungate_sata_clock();
	return enable_enet_pll(BM_ANADIG_PLL_ENET_ENABLE_SATA);
}
#endif

int enable_pcie_clock(void)
{
	/* PCIe reference clock sourced from AXI. */
	clrbits_le32(&imx_ccm->cbcmr, MXC_CCM_CBCMR_PCIE_AXI_CLK_SEL);

	/*
	 * Here be dragons!
	 *
	 * The register ANATOP_MISC1 is not documented in the Freescale
	 * MX6RM. The register that is mapped in the ANATOP space and
	 * marked as ANATOP_MISC1 is actually documented in the PMU section
	 * of the datasheet as PMU_MISC1.
	 *
	 * Switch LVDS clock source to SATA (0xb), disable clock INPUT and
	 * enable clock OUTPUT. This is important for PCI express link that
	 * is clocked from the i.MX6.
	 */
#define ANADIG_ANA_MISC1_LVDSCLK1_IBEN		(1 << 12)
#define ANADIG_ANA_MISC1_LVDSCLK1_OBEN		(1 << 10)
#define ANADIG_ANA_MISC1_LVDS1_CLK_SEL_MASK	0x0000001F
#ifndef CONFIG_MX6SX
	/* lvds_clk1 is sourced from sata ref on imx6q/dl/solo */
	clrsetbits_le32(&imx_ccm->ana_misc1,
			ANADIG_ANA_MISC1_LVDSCLK1_IBEN |
			ANADIG_ANA_MISC1_LVDS1_CLK_SEL_MASK,
			ANADIG_ANA_MISC1_LVDSCLK1_OBEN | 0xb);

	/* Party time! Ungate the clock to the PCIe. */
	ungate_sata_clock();
	ungate_pcie_clock();

	return enable_enet_pll(BM_ANADIG_PLL_ENET_ENABLE_SATA |
			BM_ANADIG_PLL_ENET_ENABLE_PCIE);
#else
	/* lvds_clk1 is sourced from pcie ref on imx6sx */
	clrsetbits_le32(&imx_ccm->ana_misc1,
			ANADIG_ANA_MISC1_LVDSCLK1_IBEN |
			ANADIG_ANA_MISC1_LVDS1_CLK_SEL_MASK,
			ANADIG_ANA_MISC1_LVDSCLK1_OBEN | 0xa);

	ungate_disp_axi_clock();
	ungate_pcie_clock();
	return enable_enet_pll(BM_ANADIG_PLL_ENET_ENABLE_PCIE);
#endif
}

#ifdef CONFIG_SECURE_BOOT
void hab_caam_clock_enable(void)
{
	struct mxc_ccm_reg *ccm_regs = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	u32 reg = 0;

	/*CG4 ~ CG6, enable CAAM clocks*/
	reg = readl(&ccm_regs->CCGR0);
	reg |= (MXC_CCM_CCGR0_CAAM_WRAPPER_IPG_MASK |
			MXC_CCM_CCGR0_CAAM_WRAPPER_ACLK_MASK |
			MXC_CCM_CCGR0_CAAM_SECURE_MEM_MASK);
	writel(reg, &ccm_regs->CCGR0);

	/* Enable EMI slow clk */
	reg = readl(&ccm_regs->CCGR6);
	reg |= MXC_CCM_CCGR6_EMI_SLOW_MASK;
	writel(reg, &ccm_regs->CCGR6);
}

void hab_caam_clock_disable(void)
{
	struct mxc_ccm_reg *ccm_regs = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	u32 reg = 0;

	/*CG4 ~ CG6, disable CAAM clocks*/
	reg = readl(&ccm_regs->CCGR0);
	reg &= ~(MXC_CCM_CCGR0_CAAM_WRAPPER_IPG_MASK |
			MXC_CCM_CCGR0_CAAM_WRAPPER_ACLK_MASK |
			MXC_CCM_CCGR0_CAAM_SECURE_MEM_MASK);
	writel(reg, &ccm_regs->CCGR0);

	/* Disable EMI slow clk */
	reg = readl(&ccm_regs->CCGR6);
	reg &= ~MXC_CCM_CCGR6_EMI_SLOW_MASK;
	writel(reg, &ccm_regs->CCGR6);
}
#endif

unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_ARM_CLK:
		return get_mcu_main_clk();
	case MXC_PER_CLK:
		return get_periph_clk();
	case MXC_AHB_CLK:
		return get_ahb_clk();
	case MXC_IPG_CLK:
		return get_ipg_clk();
	case MXC_IPG_PERCLK:
	case MXC_I2C_CLK:
		return get_ipg_per_clk();
	case MXC_UART_CLK:
		return get_uart_clk();
	case MXC_CSPI_CLK:
		return get_cspi_clk();
	case MXC_AXI_CLK:
		return get_axi_clk();
	case MXC_EMI_SLOW_CLK:
		return get_emi_slow_clk();
	case MXC_DDR_CLK:
		return get_mmdc_ch0_clk();
	case MXC_ESDHC_CLK:
		return get_usdhc_clk(0);
	case MXC_ESDHC2_CLK:
		return get_usdhc_clk(1);
	case MXC_ESDHC3_CLK:
		return get_usdhc_clk(2);
	case MXC_ESDHC4_CLK:
		return get_usdhc_clk(3);
	case MXC_SATA_CLK:
		return get_ahb_clk();
	default:
		break;
	}

	return -1;
}

/*
 * Dump some core clockes.
 */
int do_mx6_showclocks(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 freq;
	freq = decode_pll(PLL_SYS, MXC_HCLK);
	printf("PLL_SYS    %8d MHz\n", freq / 1000000);
	freq = decode_pll(PLL_BUS, MXC_HCLK);
	printf("PLL_BUS    %8d MHz\n", freq / 1000000);
	freq = decode_pll(PLL_USBOTG, MXC_HCLK);
	printf("PLL_OTG    %8d MHz\n", freq / 1000000);
	freq = decode_pll(PLL_ENET, MXC_HCLK);
	printf("PLL_NET    %8d MHz\n", freq / 1000000);

	printf("\n");
	printf("IPG        %8d kHz\n", mxc_get_clock(MXC_IPG_CLK) / 1000);
	printf("UART       %8d kHz\n", mxc_get_clock(MXC_UART_CLK) / 1000);
#ifdef CONFIG_MXC_SPI
	printf("CSPI       %8d kHz\n", mxc_get_clock(MXC_CSPI_CLK) / 1000);
#endif
	printf("AHB        %8d kHz\n", mxc_get_clock(MXC_AHB_CLK) / 1000);
	printf("AXI        %8d kHz\n", mxc_get_clock(MXC_AXI_CLK) / 1000);
	printf("DDR        %8d kHz\n", mxc_get_clock(MXC_DDR_CLK) / 1000);
	printf("USDHC1     %8d kHz\n", mxc_get_clock(MXC_ESDHC_CLK) / 1000);
	printf("USDHC2     %8d kHz\n", mxc_get_clock(MXC_ESDHC2_CLK) / 1000);
	printf("USDHC3     %8d kHz\n", mxc_get_clock(MXC_ESDHC3_CLK) / 1000);
	printf("USDHC4     %8d kHz\n", mxc_get_clock(MXC_ESDHC4_CLK) / 1000);
	printf("EMI SLOW   %8d kHz\n", mxc_get_clock(MXC_EMI_SLOW_CLK) / 1000);
	printf("IPG PERCLK %8d kHz\n", mxc_get_clock(MXC_IPG_PERCLK) / 1000);

	return 0;
}

#ifndef CONFIG_MX6SX
void enable_ipu_clock(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	int reg;
	reg = readl(&mxc_ccm->CCGR3);
	reg |= MXC_CCM_CCGR3_IPU1_IPU_MASK;
	writel(reg, &mxc_ccm->CCGR3);
}
#endif

/****************** CMD CLOCK **********************************/
#ifdef CONFIG_CMD_CLOCK
#define PLL1_FREQ_MAX	1300000000
#define PLL1_FREQ_MIN	650000000
#define PLL2_FREQ_MAX	528000000
#define PLL2_FREQ_MIN	480000000
#define MAX_DDR_CLK     PLL2_FREQ_MAX
#define AHB_CLK_MAX     132000000
#define IPG_CLK_MAX     (AHB_CLK_MAX >> 1)
#define NFC_CLK_MAX     PLL2_FREQ_MAX


#define SZ_DEC_1M       1000000
#define CONFIG_MX6_HCLK_FREQ	24000000

/* Out-of-reset PFDs and clock source definitions */
#define PLL2_PFD0_FREQ	352000000
#define PLL2_PFD1_FREQ	594000000
#define PLL2_PFD2_FREQ	396000000
#define PLL2_PFD2_DIV_FREQ	198000000
#define PLL3_PFD0_FREQ	720000000
#define PLL3_PFD1_FREQ	540000000
#define PLL3_PFD2_FREQ	508200000
#define PLL3_PFD3_FREQ	454700000
#define PLL3_80M	80000000
#define PLL3_60M	60000000

#define REG_RD(base, reg) \
	(*(volatile unsigned int *)((base) + (reg)))
#define REG_WR(base, reg, value) \
	((*(volatile unsigned int *)((base) + (reg))) = (value))
#define REG_SET(base, reg, value) \
	((*(volatile unsigned int *)((base) + (reg ## _SET))) = (value))
#define REG_CLR(base, reg, value) \
	((*(volatile unsigned int *)((base) + (reg ## _CLR))) = (value))
#define REG_TOG(base, reg, value) \
	((*(volatile unsigned int *)((base) + (reg ## _TOG))) = (value))

#define HW_ANADIG_PLL_SYS	(0x00000000)
#define HW_ANADIG_PLL_SYS_SET	(0x00000004)
#define HW_ANADIG_PLL_SYS_CLR	(0x00000008)
#define HW_ANADIG_PLL_SYS_TOG	(0x0000000c)

#define HW_ANADIG_PLL_528	(0x00000030)
#define HW_ANADIG_PLL_528_SET	(0x00000034)
#define HW_ANADIG_PLL_528_CLR	(0x00000038)
#define HW_ANADIG_PLL_528_TOG	(0x0000003c)

#endif



#ifdef CONFIG_CMD_CLOCK

/*!
 * This is to calculate divider based on reference clock and
 * targeted clock based on the equation for each PLL.
 *
 * @param pll		pll number
 * @param ref       reference clock freq in Hz
 * @param target    targeted clock in Hz
 *
 * @return          divider if successful; -1 otherwise.
 */
static int calc_pll_divider(enum pll_clocks pll, u32 ref, u32 target)
{
	int i, div;

	switch (pll) {
	case PLL_SYS:
		if (target < PLL1_FREQ_MIN || target > PLL1_FREQ_MAX) {
			printf("PLL1 frequency should be"
			"within [%d - %d] MHz\n", PLL1_FREQ_MIN / SZ_DEC_1M,
				PLL1_FREQ_MAX / SZ_DEC_1M);
			return -1;
		}
		for (i = 54, div = i; i < 109; i++) {
			if ((ref * (i >> 1)) > target)
				break;
			div = i;
		}
		break;
	case PLL_BUS:
		if (target < PLL2_FREQ_MIN || target > PLL2_FREQ_MAX) {
			printf("PLL2 frequency should be"
			"within [%d - %d] MHz\n", PLL2_FREQ_MIN / SZ_DEC_1M,
				PLL2_FREQ_MAX / SZ_DEC_1M);
			return -1;
		}
		for (i = 0, div = i; i < 2; i++) {
			if (ref * (20 + (i << 1)) > target)
				break;
			div = i;
		}
		break;
	default:
		printf("Changing this PLL not supported\n");
		return -1;
		break;
	}

	return div;
}


#define calc_div(target_clk, src_clk, limit) ({	\
		u32 tmp = 0;	\
		if ((src_clk % target_clk) <= 100)	\
			tmp = src_clk / target_clk;	\
		else	\
			tmp = (src_clk / target_clk) + 1;	\
		if (tmp > limit)	\
			tmp = limit;	\
		(tmp - 1);	\
	})

#define calc_pred_n_podf(target_clk, src_clk, p_pred, p_podf, pred_limit, podf_limit) {	\
		u32 div = calc_div(target_clk, src_clk,	\
				pred_limit * podf_limit);	\
		u32 tmp = 0, tmp_i = 0, tmp_j = 0;	\
		if ((div + 1) > (pred_limit * podf_limit))	{\
			tmp_i = pred_limit;	\
			tmp_j = podf_limit;	\
		}	\
		for (tmp_i = 1; tmp_i <= podf_limit; ++tmp_i) {	\
			for (tmp_j = 1; tmp_j <= pred_limit; ++tmp_j) {	\
				if ((div + 1) == (tmp_i * tmp_j)) {	\
					tmp = 1;	\
					break;	\
				}	\
			}	\
			if (1 == tmp)	\
				break;	\
		}	\
		*p_pred = tmp_j - 1;	\
		*p_podf = tmp_i - 1;	\
	}

static int config_pll_clk(enum pll_clocks pll, u32 divider)
{
	u32 ccsr = 0;
	u32 reg = 0;

	
	ccsr = __raw_readl(&imx_ccm->ccsr);
		
	switch (pll) {
	case PLL_SYS:
		/* Switch ARM to PLL2 clock */
		ccsr |= 0x4;
		writel(ccsr, &imx_ccm->ccsr);

		REG_CLR(ANATOP_BASE_ADDR, HW_ANADIG_PLL_SYS,
			BM_ANADIG_PLL_SYS_DIV_SELECT);
		REG_SET(ANATOP_BASE_ADDR, HW_ANADIG_PLL_SYS,
			BF_ANADIG_PLL_SYS_DIV_SELECT(divider));
		/* Enable CPU PLL1 */
		REG_SET(ANATOP_BASE_ADDR, HW_ANADIG_PLL_SYS,
			BM_ANADIG_PLL_SYS_ENABLE);
		/* Wait for PLL lock */
		while (REG_RD(ANATOP_BASE_ADDR, HW_ANADIG_PLL_SYS) &
			BM_ANADIG_PLL_SYS_LOCK)
			udelay(10);
		/* Clear bypass bit */
		REG_CLR(ANATOP_BASE_ADDR, HW_ANADIG_PLL_SYS,
			BM_ANADIG_PLL_SYS_BYPASS);

		/* Switch back */
		ccsr &= ~0x4;
		writel(ccsr, &imx_ccm->ccsr);
		
		break;
	case PLL_BUS:
		/* Switch to pll2 bypass clock */
		ccsr |= 0x2;
		writel(ccsr, &imx_ccm->ccsr);



		REG_CLR(ANATOP_BASE_ADDR, HW_ANADIG_PLL_528,
			BM_ANADIG_PLL_528_DIV_SELECT);
		REG_SET(ANATOP_BASE_ADDR, HW_ANADIG_PLL_528,
			divider);
		/* Enable BUS PLL2 */
		REG_SET(ANATOP_BASE_ADDR, HW_ANADIG_PLL_528,
			BM_ANADIG_PLL_528_ENABLE);
		/* Wait for PLL lock */
		while (REG_RD(ANATOP_BASE_ADDR, HW_ANADIG_PLL_528) &
			BM_ANADIG_PLL_528_LOCK)
			udelay(10);
		/* Clear bypass bit */
		REG_CLR(ANATOP_BASE_ADDR, HW_ANADIG_PLL_528,
			BM_ANADIG_PLL_528_BYPASS);

		/* Switch back */
		ccsr &= ~0x2;
		writel(ccsr, &imx_ccm->ccsr);
		break;
	default:
		return -1;
	}

	return 0;
}

static int config_core_clk(u32 ref, u32 freq)
{
	int div = calc_pll_divider(PLL_SYS, ref, freq);
	if (div < 0) {
		printf("Can't find pll parameters\n");
		return div;
	}

	return config_pll_clk(PLL_SYS, div);
}

static int config_periph_clk(u32 ref, u32 freq)
{

	u32 cbcdr = __raw_readl(&imx_ccm->cbcdr); 
	u32 cbcmr = __raw_readl(&imx_ccm->cbcmr); 
	u32 pll2_freq = decode_pll(PLL_BUS, CONFIG_MX6_HCLK_FREQ);
	u32 pll3_freq = decode_pll(PLL_USBOTG, CONFIG_MX6_HCLK_FREQ);



	if (freq >=  pll2_freq) {
		/* PLL2 */
		writel(cbcmr & ~MXC_CCM_CBCMR_PRE_PERIPH_CLK_SEL_MASK, &imx_ccm->cbcmr);						
		writel(cbcdr & ~MXC_CCM_CBCDR_PERIPH_CLK_SEL,
			&imx_ccm->cbcdr);
	} else if (freq < pll2_freq && freq >= pll3_freq) {
		/* PLL3 */
		writel(cbcmr & ~MXC_CCM_CBCMR_PERIPH_CLK2_SEL_MASK,
			&imx_ccm->cbcmr);
		writel(cbcdr | MXC_CCM_CBCDR_PERIPH_CLK_SEL,
						&imx_ccm->cbcdr);
	} else if (freq < pll3_freq && freq >= PLL2_PFD2_FREQ) {
		/* 400M PLL2 PFD */
		cbcmr = (cbcmr & ~MXC_CCM_CBCMR_PRE_PERIPH_CLK_SEL_MASK) |
			(1 << MXC_CCM_CBCMR_PRE_PERIPH_CLK_SEL_OFFSET);
		writel(cbcmr, &imx_ccm->cbcmr);
		writel(cbcdr & ~MXC_CCM_CBCDR_PERIPH_CLK_SEL,
			&imx_ccm->cbcdr);
	} else if (freq < PLL2_PFD2_FREQ && freq >= PLL2_PFD0_FREQ) {
		/* 352M PLL2 PFD */
		cbcmr = (cbcmr & ~MXC_CCM_CBCMR_PRE_PERIPH_CLK_SEL_MASK) |
			(2 << MXC_CCM_CBCMR_PRE_PERIPH_CLK_SEL_OFFSET);
		writel(cbcmr, &imx_ccm->cbcmr);
		writel(cbcdr & ~MXC_CCM_CBCDR_PERIPH_CLK_SEL,
			&imx_ccm->cbcdr);
	} else if (freq == PLL2_PFD2_DIV_FREQ) {
		/* 200M PLL2 PFD */
		cbcmr = (cbcmr & ~MXC_CCM_CBCMR_PRE_PERIPH_CLK_SEL_MASK) |
			(3 << MXC_CCM_CBCMR_PRE_PERIPH_CLK_SEL_OFFSET);
		writel(cbcmr, &imx_ccm->cbcmr);
		writel(cbcdr & ~MXC_CCM_CBCDR_PERIPH_CLK_SEL,
			&imx_ccm->cbcdr);
	} else {
		printf("Frequency requested not within range [%d-%d] MHz\n",
			PLL2_PFD2_DIV_FREQ / SZ_DEC_1M, pll2_freq / SZ_DEC_1M);
		return -1;
	}
	puts("\n");

	return 0;
}

static int config_ddr_clk(u32 ddr_clk)
{
	u32 clk_src = get_periph_clk();
	u32 i, podf;
	u32 cbcdr = 0;

	
	cbcdr = __raw_readl(&imx_ccm->cbcdr);




	if (ddr_clk > MAX_DDR_CLK) {
		printf("DDR clock should be less than"
			"%d MHz, assuming max value\n",
			(MAX_DDR_CLK / SZ_DEC_1M));
		ddr_clk = MAX_DDR_CLK;
	}

	for (i = 1; i < 9; i++)
		if ((clk_src / i) <= ddr_clk)
			break;

	podf = i - 1;

	cbcdr &= ~(MXC_CCM_CBCDR_MMDC_CH0_PODF_MASK |
			MXC_CCM_CBCDR_MMDC_CH1_PODF_MASK);
	cbcdr |= (podf << MXC_CCM_CBCDR_MMDC_CH0_PODF_OFFSET) |
			(podf << MXC_CCM_CBCDR_MMDC_CH1_PODF_OFFSET);

	writel(cbcdr, &imx_ccm->cbcdr);
	
	while ( __raw_readl(&imx_ccm->cdhipr)!= 0)
		;

	writel(0x0, &imx_ccm->cbcdr);

	return 0;
}

int config_ddr_400M(void)
{

	config_periph_clk(CONFIG_MX6_HCLK_FREQ, 480*SZ_DEC_1M);

	config_periph_clk(CONFIG_MX6_HCLK_FREQ, 396*SZ_DEC_1M);




	return 0;

}

int config_cpu_1G(void)
{

       config_core_clk(CONFIG_MX6_HCLK_FREQ, 1000*SZ_DEC_1M);

	return 0;

}

U_BOOT_CMD(
	cpu1g,	CONFIG_SYS_MAXARGS, 1, config_cpu_1G,
	"change cpu to 1G",
	""
);


#endif









/***************************************************/

U_BOOT_CMD(
	clocks,	CONFIG_SYS_MAXARGS, 1, do_mx6_showclocks,
	"display clocks",
	""
);


