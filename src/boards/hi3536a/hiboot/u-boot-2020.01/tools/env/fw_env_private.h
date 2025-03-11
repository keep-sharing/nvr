/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002-2008
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* Pull in the current config to define the default environment */
#include <linux/kconfig.h>

#ifndef __ASSEMBLY__
#define __ASSEMBLY__ /* get only #defines from config.h */
#include <config.h>
#undef	__ASSEMBLY__
#else
#include <config.h>
#endif

/*
 * To build the utility with the static configuration
 * comment out the next line.
 * See included "fw_env.config" sample file
 * for notes on configuration.
 */
 #ifdef CONFIG_MS_NVR
//#define CONFIG_FILE     "/etc/fw_env.config"

#ifndef CONFIG_FILE
#define HAVE_REDUND /* For systems with 2 env sectors */
#define DEVICE1_NAME      "/dev/mmcblk0p2"
#define DEVICE2_NAME      "/dev/mmcblk0p2"
#define DEVICE1_OFFSET    0x0000
#define ENV1_SIZE         0x40000
#define DEVICE1_ESIZE     ENV1_SIZE
#define DEVICE1_ENVSECTORS     1
#define DEVICE2_OFFSET    ENV1_SIZE
#define ENV2_SIZE         ENV1_SIZE
#define DEVICE2_ESIZE     ENV1_SIZE
#define DEVICE2_ENVSECTORS     1
#endif
#else
#define CONFIG_FILE     "/etc/fw_env.config"

#ifndef CONFIG_FILE
#define HAVE_REDUND /* For systems with 2 env sectors */
#define DEVICE1_NAME      "/dev/mtd1"
#define DEVICE2_NAME      "/dev/mtd2"
#define DEVICE1_OFFSET    0x0000
#define ENV1_SIZE         0x4000
#define DEVICE1_ESIZE     0x4000
#define DEVICE1_ENVSECTORS     2
#define DEVICE2_OFFSET    0x0000
#define ENV2_SIZE         0x4000
#define DEVICE2_ESIZE     0x4000
#define DEVICE2_ENVSECTORS     2
#endif
#endif /* CONFIG_MS_NVR */

#ifndef CONFIG_BAUDRATE
#define CONFIG_BAUDRATE		115200
#endif

#ifndef CONFIG_BOOTDELAY
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#ifndef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND						\
	"bootp; "							\
	"setenv bootargs root=/dev/nfs nfsroot=${serverip}:${rootpath} "\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off; "\
	"bootm"
#endif
