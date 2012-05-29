/*
 * Copyright (C) 2011 Samsung Electronics
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <netdev.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <asm/arch/sromc.h>
#include <usb/s3c_udc.h>
#include <pmic.h>
#include <max8997_pmic.h>

DECLARE_GLOBAL_DATA_PTR;
struct exynos4_gpio_part1 *gpio1;
struct exynos4_gpio_part2 *gpio2;

static int galaxys2_usb_init(void);

#ifndef CONFIG_VIDEO
//causes the camera LED to blink shortly
static void blink_led(unsigned times) {
	struct pmic *p = get_pmic();
	if (pmic_probe(p)) {
		printf("failed to get pmic\n");
		return;
	}

	while (times --> 0) {	
		pmic_set_output(p, MAX8997_REG_LEN_CNTL,
			MAX8997_LED0_FLASH_MASK, LDO_OFF);
		
		pmic_set_output(p, MAX8997_REG_BOOST_CNTL,
			MAX8997_LED_BOOST_ENABLE_MASK, LDO_OFF);

		pmic_set_output(p, MAX8997_REG_LEN_CNTL,
			0, 0);
		
		pmic_reg_write(p, MAX8997_REG_FLASH1_CUR,
			(0xff) << MAX8997_LED_FLASH_SHIFT);
		
		pmic_set_output(p, MAX8997_REG_BOOST_CNTL,
			MAX8997_LED_BOOST_ENABLE_MASK, LDO_ON);
		
		pmic_set_output(p, MAX8997_REG_LEN_CNTL,
			MAX8997_LED0_FLASH_MASK, LDO_ON);
		
		mdelay(100);
	}
}
#endif

static void max8997_reg_update(struct pmic *p, u32 reg, int val, int mask) {
	u32 tmp;
	pmic_reg_read(p, reg, &tmp);
	tmp &= ~mask;
	tmp |= (val & mask);
	pmic_reg_write(p, reg, tmp);
}

static void microsd_power_enable(void) {
	struct pmic *p = get_pmic();
	if (pmic_probe(p)) {
		printf("failed to get pmic\n");
		return;
	}
	
	//set 2.8V
	max8997_reg_update(p, MAX8997_REG_LDO17CTRL, 40, 0x3f);
	pmic_set_output(p, MAX8997_REG_LDO17CTRL, MAX8997_MASK_LDO, LDO_ON);
	mdelay(50);
}

static void max8997_init (void) {
	struct pmic *p;
	int i;

	if (pmic_init()) {
		printf("failed to init pmic\n");
		return;
	}
	
	p = get_pmic();
	if (pmic_probe(p)) {
		printf("failed to get pmic\n");
		return;
	}
	
	/* For the safety, set max voltage before setting up */
	for (i = 0; i < 8; i++) {
		//these regulators are 650..2225mv by 25mv steps
		max8997_reg_update(p, MAX8997_REG_BUCK1DVS1 + i, 28, 0x3f);
		max8997_reg_update(p, MAX8997_REG_BUCK2DVS1 + i, 20, 0x3f);
		max8997_reg_update(p, MAX8997_REG_BUCK5DVS1 + i, 22, 0x3f);
	}
	pmic_reg_write(p, MAX8997_REG_BUCKRAMP, 0xf9);
}

static void init_gpio_keys() {
	/* Home key */
	s5p_gpio_cfg_pin(&gpio2->x3, 5, GPIO_INPUT);
	s5p_gpio_set_pull(&gpio2->x3, 5, GPIO_PULL_NONE);
	s5p_gpio_set_drv(&gpio2->x3, 5, GPIO_DRV_1X);
	
	/* Volume up key */
	s5p_gpio_cfg_pin(&gpio2->x2, 0, GPIO_INPUT);
	s5p_gpio_set_pull(&gpio2->x2, 0, GPIO_PULL_NONE);
	s5p_gpio_set_drv(&gpio2->x2, 0, GPIO_DRV_1X);
}

int do_get_sgs2_bootmode(cmd_tbl_t *cmdtp, int flag,
	int argc, char * const argv[])
{
	int home = !s5p_gpio_get_value(&gpio2->x3, 5);
	int volup = !s5p_gpio_get_value(&gpio2->x2, 0);
	int rc = (home << 1) | volup;
	setenv("sgs2_bootmode_val", simple_itoa(rc));
	return 0;
}

U_BOOT_CMD(sgs2_get_bootmode, CONFIG_SYS_MAXARGS, 1, do_get_sgs2_bootmode,
	"Get Galaxy S2 boot mode\n"
	"Bit 0 -> recovery\n"
	"Bit 1 -> custom kernel",
	"sgs2_bootmode\n"
);

int board_init(void)
{
	gpio1 = (struct exynos4_gpio_part1 *) EXYNOS4_GPIO_PART1_BASE;
	gpio2 = (struct exynos4_gpio_part2 *) EXYNOS4_GPIO_PART2_BASE;

	gd->bd->bi_boot_params = (PHYS_SDRAM_1 + 0x100UL);

	max8997_init();

#ifndef CONFIG_VIDEO
	blink_led(1);
#endif
	
	if (galaxys2_usb_init()) {
		printf("failed to initialize usb\n");
	}

	init_gpio_keys();
	
	return 0;
}

int dram_init(void)
{
	gd->ram_size	= get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_2, PHYS_SDRAM_2_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_3, PHYS_SDRAM_3_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_4, PHYS_SDRAM_4_SIZE);
	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = get_ram_size((long *)PHYS_SDRAM_1, \
							PHYS_SDRAM_1_SIZE);
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = get_ram_size((long *)PHYS_SDRAM_2, \
							PHYS_SDRAM_2_SIZE);
	gd->bd->bi_dram[2].start = PHYS_SDRAM_3;
	gd->bd->bi_dram[2].size = get_ram_size((long *)PHYS_SDRAM_3, \
							PHYS_SDRAM_3_SIZE);
	gd->bd->bi_dram[3].start = PHYS_SDRAM_4;
	gd->bd->bi_dram[3].size = get_ram_size((long *)PHYS_SDRAM_4, \
							PHYS_SDRAM_4_SIZE);
}

int board_eth_init(bd_t *bis)
{
	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	printf("\nBoard: GALAXYS2\n");
	return 0;
}
#endif

#ifdef CONFIG_GENERIC_MMC
static int galaxys2_init_emmc(void) {
	int i;

	//massmem enable
	s5p_gpio_direction_output(&gpio2->l1, 1, 0);

	/*
	 * eMMC GPIO:
	 * SDR 8-bit@48MHz at MMC0
	 * GPK0[0]	SD_0_CLK(2)
	 * GPK0[1]	SD_0_CMD(2)
	 * GPK0[2]	SD_0_CDn	-> Not used
	 * GPK0[3:6]	SD_0_DATA[0:3](2)
	 * GPK1[3:6]	SD_0_DATA[0:3](3)
	 *
	 * DDR 4-bit@26MHz at MMC4
	 * GPK0[0]	SD_4_CLK(3)
	 * GPK0[1]	SD_4_CMD(3)
	 * GPK0[2]	SD_4_CDn	-> Not used
	 * GPK0[3:6]	SD_4_DATA[0:3](3)
	 * GPK1[3:6]	SD_4_DATA[4:7](4)
	 */
	for (i = 0; i < 7; i++) {
		if (i == 2)
			continue;
		/* GPK0[0:6] special function 2 */
		s5p_gpio_cfg_pin(&gpio2->k0, i, 0x2);
		/* GPK0[0:6] pull disable */
		s5p_gpio_set_pull(&gpio2->k0, i, GPIO_PULL_NONE);
		/* GPK0[0:6] drv 4x */
		s5p_gpio_set_drv(&gpio2->k0, i, GPIO_DRV_4X);
	}

	for (i = 3; i < 7; i++) {
		/* GPK1[3:6] special function 3 */
		s5p_gpio_cfg_pin(&gpio2->k1, i, 0x3);
		/* GPK1[3:6] pull disable */
		s5p_gpio_set_pull(&gpio2->k1, i, GPIO_PULL_NONE);
		/* GPK1[3:6] drv 4x */
		s5p_gpio_set_drv(&gpio2->k1, i, GPIO_DRV_4X);
	}

	/*
	 * mmc0	 : eMMC (8-bit buswidth)
	 */
	return s5p_mmc_init(0, 8);
}

static int galaxys2_init_microsd(void) {
	int i;

	/* T-flash detect */
	s5p_gpio_cfg_pin(&gpio2->x3, 4, 0xf);
	s5p_gpio_set_pull(&gpio2->x3, 4, GPIO_PULL_UP);

	if (!s5p_gpio_get_value(&gpio2->x3, 4)) {
		printf("enabling uSD\n");
		microsd_power_enable();

		/*
		 * MMC2 SD card GPIO:
		 *
		 * GPK2[0]	SD_2_CLK(2)
		 * GPK2[1]	SD_2_CMD(2)
		 * GPK2[2]	SD_2_CDn -> Not used
		 * GPK2[3:6]	SD_2_DATA[0:3](2)
		 */
		for (i = 0; i < 7; i++) {
			if (i == 2)
				continue;
			/* GPK2[0:6] special function 2 */
			s5p_gpio_cfg_pin(&gpio2->k2, i, GPIO_FUNC(0x2));

			if (i < 2) {
				/* GPK2[0:1] pull disable */
				s5p_gpio_set_pull(&gpio2->k2, i, GPIO_PULL_NONE);
			}
			else {
				/* GPK2[2:6] pull up */
				s5p_gpio_set_pull(&gpio2->k2, i, GPIO_PULL_UP);
			}
			
			/* GPK2[0:6] drv 4x */
			s5p_gpio_set_drv(&gpio2->k2, i, GPIO_DRV_4X);
		}

		return s5p_mmc_init(2, 4);
	}
	return 0;
}

int board_mmc_init(bd_t *bis)
{
	if (galaxys2_init_emmc()) {
		printf("failed to initialize emmc\n");
	}

	if (galaxys2_init_microsd()) {
		printf("failed to initialize uSD\n");
	}

	return 0;
}
#endif

#ifdef CONFIG_USB_GADGET
static int galaxys2_phy_control(int on)
{
	int ret = 0;
	struct pmic *p = get_pmic();

	if (pmic_probe(p))
		return -1;

	if (on) {
		ret |= pmic_set_output(p, MAX8997_REG_LDO8CTRL,
				      MAX8997_MASK_LDO, LDO_ON);
		ret |= pmic_set_output(p, MAX8997_REG_LDO3CTRL,
				      MAX8997_MASK_LDO, LDO_ON);
	} else {
		ret |= pmic_set_output(p, MAX8997_REG_LDO3CTRL,
				      MAX8997_MASK_LDO, LDO_OFF);
		ret |= pmic_set_output(p, MAX8997_REG_LDO8CTRL,
				      MAX8997_MASK_LDO, LDO_OFF);
	}

	if (ret) {
		puts("MAX8997 LDO setting error!\n");
		return -1;
	}

	return 0;
}

struct s3c_plat_otg_data galaxys2_otg_data = {
	.phy_control = galaxys2_phy_control,
	.regs_phy = EXYNOS4_USBPHY_BASE,
	.regs_otg = EXYNOS4_USBOTG_BASE,
	.usb_phy_ctrl = EXYNOS4_USBPHY_CONTROL,
	.usb_flags = PHY0_SLEEP,
};

static int galaxys2_usb_init(void) {
	return s3c_udc_probe(&galaxys2_otg_data);
}
#else
static int galaxys2_usb_init(void) {
	return 0;
}
#endif

#ifdef CONFIG_VIDEO
#include <video_fb.h>

/* when we are chainloading from the proprietary bootloader,
 * the framebuffer is already set up at this fixed location */
#define I9100_FB 0x5ec00000

GraphicDevice gdev;

void *video_hw_init(void) {
	memset((void*)I9100_FB, 0, 480 * 800 * 4);
	gdev.frameAdrs = I9100_FB;
	gdev.winSizeX = 480;
	gdev.winSizeY = 800;
	gdev.gdfBytesPP = 4;
	gdev.gdfIndex = GDF_32BIT_X888RGB;
	return &gdev;
}
#endif
