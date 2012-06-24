/*
 * Copyright (C) 2011 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG GALAXYS2 (EXYNOS4210) board.
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

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_SAMSUNG			1	/* SAMSUNG core */
#define CONFIG_S5P			1	/* S5P Family */
#define CONFIG_EXYNOS4210		1	/* which is a EXYNOS4210 SoC */
#define CONFIG_GALAXYS2			1	/* working with GALAXYS2*/

#include <asm/arch/cpu.h>		/* get chip and board defs */

#define CONFIG_ARCH_CPU_INIT
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO_LATE

/* Keep L2 Cache Disabled */
#define CONFIG_SYS_L2CACHE_OFF		1
#define CONFIG_SYS_DCACHE_OFF		1

#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define CONFIG_SYS_TEXT_BASE		0x40008000

/* input clock of PLL: GALAXYS2 has 24MHz input clock */
#define CONFIG_SYS_CLK_FREQ		24000000

#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SKIP_LOWLEVEL_INIT

#define CONFIG_VIDEO
#define CONFIG_CFB_CONSOLE
#define CONFIG_VGA_AS_SINGLE_DEVICE

#define CONFIG_STD_DEVICES_SETTINGS "stdin=serial\0" \
									"stdout=serial,lcd\0" \
									"stderr=serial,lcd\0"

//#define CONFIG_MACH_TYPE		MACH_TYPE_GALAXYS2
#define CONFIG_MACH_TYPE		2838

/* Power Down Modes */
#define S5P_CHECK_SLEEP			0x00000BAD
#define S5P_CHECK_DIDLE			0xBAD00000
#define S5P_CHECK_LPA			0xABAD0000

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (1 << 20))

/* select serial console configuration */
#define CONFIG_SERIAL_MULTI		1
#define CONFIG_SERIAL2			1	/* use SERIAL 2 */
#define CONFIG_BAUDRATE			115200
#define EXYNOS4_DEFAULT_UART_OFFSET	0x020000

/* SD/MMC configuration */
#define CONFIG_GENERIC_MMC		1
#define CONFIG_MMC			1
#define CONFIG_S5P_MMC			1

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Command definition*/
#include <config_cmd_default.h>

#define CONFIG_CMD_MMC
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS

#define CONFIG_DOS_PARTITION		1
#define CONFIG_EFI_PARTITION		1

#define CONFIG_BOOTDELAY		1
#define CONFIG_ZERO_BOOTDELAY_CHECK
#define CONFIG_BOOTCOMMAND	"run galaxy_boot"

#define CONFIG_EXTRA_ENV_SETTINGS \
	\
	"kernel_name=/boot/vmlinux.uimg\0"\
	"script_img=/boot/boot.scr.uimg\0"\
	\
	"run_disk_boot_script=" \
		"if fatload $devtype} ${devnum}:${script_part} " \
			"${loadaddr} ${script_img}; then " \
			"source ${loadaddr}; " \
		"elif ext2load ${devtype} ${devnum}:${script_part} " \
				"${loadaddr} ${script_img}; then " \
			"source ${loadaddr}; " \
		"fi\0" \
	\
	"real_boot="\
		"setenv bootargs "\
			"${dev_extras} root=/dev/${devname}${rootpart} rootwait ro ;"\
		"echo Load Address:${loadaddr};" \
		"echo Cmdline:${bootargs}; " \
		"if fatload ${devtype} ${devnum}:${kernel_part} " \
			"${loadaddr} ${kernel_name}; then " \
			"bootm ${loadaddr}; " \
		"elif ext2load ${devtype} ${devnum}:${kernel_part} " \
		            "${loadaddr} ${kernel_name}; then " \
			"bootm ${loadaddr};" \
		"fi\0" \
	\
	"mmc_boot=mmc rescan; " \
		"setenv devtype mmc; " \
		"setenv devname mmcblk${devnum}p; " \
		"mmc dev ${devnum}; " \
		"run run_disk_boot_script; " \
		"run real_boot\0" \
	\
	"boot_custom_emmc=setenv devnum 0; " \
		"echo Booting from EMMC; "\
		"setenv script_part 0xb; " \
		"setenv kernel_part 0xb; " \
		"setenv rootpart 0xb; " \
		"run mmc_boot\0" \
	\
	"boot_android=" \
		"setenv android_cmd loglevel=4 console=ram sec_debug.enable=0 " \
			"sec_debug.enable_user=0 sec_log=0x100000@0x4d900000 " \
			"s3cfb.bootloaderfb=0x5ec00000 ld9040.get_lcdtype=0x2 " \
			"consoleblank=0 lpj=3981312 vmalloc=144m ;" \
		"if test $sgs2_bootmode_val -eq 1; then "\
			"setenv android_cmd ${android_cmd} bootmode=2; " \
		"fi ;" \
		"mmc dev 0; " \
		"mmc read ${loadaddr} 0x16000 0x4000; " \
		"setenv bootargs ${android_cmd}; "\
		"echo Command Line: ${bootargs}; " \
		"bootm ${loadaddr}\0" \
	\
	"galaxy_boot=" \
		/*"setenv verify n; "*/ \
		"setenv loadaddr 0x4EE08000; " \
		"setenv dev_extras console=tty0 --no-log lpj=3981312; " \
		"mmc rescan; " \
		"sgs2_get_bootmode; " \
		"echo [SGS2:bootmode] $sgs2_bootmode_val; " \
		"if test $sgs2_bootmode_val -lt 2; then " \
			"echo Regular boot; " \
			"run boot_android; " \
		"else; " \
			"echo Custom boot from emmc; "\
			"run boot_custom_emmc; " \
		"fi; " \
		"echo Failed to boot\0"

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser	*/
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_PROMPT		"GALAXYS2 # "
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size*/
#define CONFIG_SYS_PBSIZE		384	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
//#define CONFIG_DEFAULT_CONSOLE		"console=ttySAC2,115200n8\0"
#define CONFIG_DEFAULT_CONSOLE		"console=tty0\0"

/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x6000000)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0xEE00000)

#define CONFIG_SYS_HZ			1000

/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/* Stack sizes */
#define CONFIG_STACKSIZE		(256 << 10)	/* 256KB */

/* GALAXYS2 has 4 bank of DRAM */
#define CONFIG_NR_DRAM_BANKS	4
#define SDRAM_BANK_SIZE		(256UL << 20UL)	/* 256 MB */
#define PHYS_SDRAM_1		CONFIG_SYS_SDRAM_BASE
#define PHYS_SDRAM_1_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_2		(CONFIG_SYS_SDRAM_BASE + SDRAM_BANK_SIZE)
#define PHYS_SDRAM_2_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_3		(CONFIG_SYS_SDRAM_BASE + (2 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_3_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_4		(CONFIG_SYS_SDRAM_BASE + (3 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_4_SIZE	SDRAM_BANK_SIZE

/* FLASH and environment organization */
#define CONFIG_SYS_NO_FLASH		1
#undef CONFIG_CMD_IMLS
#define CONFIG_IDENT_STRING		" for GALAXYS2"

#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ		(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ		(4*1024)	/* FIQ stack */
#endif

#define CONFIG_CLK_1000_400_200

/* MIU (Memory Interleaving Unit) */
#define CONFIG_MIU_2BIT_21_7_INTERLEAVED

#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE			(16 << 10)	/* 16 KB */

#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_LOAD_ADDR - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_CACHELINE_SIZE       32

/* U-boot copy size from boot Media to DRAM.*/
#define COPY_BL2_SIZE		0x80000
#define BL2_START_OFFSET	((CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)/512)
#define BL2_SIZE_BLOC_COUNT	(COPY_BL2_SIZE/512)

/* Enable devicetree support */
#define CONFIG_OF_LIBFDT

#define CONFIG_SOFT_I2C_GPIO_SCL exynos4_gpio_part1_get_nr(b, 7)
#define CONFIG_SOFT_I2C_GPIO_SDA exynos4_gpio_part1_get_nr(b, 6)

#define CONFIG_SOFT_I2C
#define CONFIG_SOFT_I2C_READ_REPEATED_START
#define CONFIG_SYS_I2C_SPEED	50000
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_SYS_MAX_I2C_BUS	7

#define CONFIG_PMIC
#define CONFIG_PMIC_I2C
#define CONFIG_PMIC_MAX8997

#define CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_S3C_UDC_OTG
#define CONFIG_USB_GADGET_DUALSPEED

#define CONFIG_PWM 1 //for the S5P timer

#endif	/* __CONFIG_H */
