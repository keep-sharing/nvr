/*
 * (C) Copyright 2002-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
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

/*
 * To match the U-Boot user interface on ARM platforms to the U-Boot
 * standard (as on PPC platforms), some messages with debug character
 * are removed from the default U-Boot build.
 *
 * Define DEBUG here if you want additional info as shown below
 * printed upon startup:
 *
 * U-Boot code: 00F00000 -> 00F3C774  BSS: -> 00FC3274
 * IRQ Stack: 00ebff7c
 * FIQ Stack: 00ebef7c
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <stdio_dev.h>
#include <timestamp.h>
#include <version.h>
#include <net.h>
#include <serial.h>
#include <nand.h>
#include <onenand_uboot.h>
#include <spi_flash.h>
#include <mmc.h>
#include <boot/customer.h>
#include <i2c.h>

#include <asm/arch/gpio.h>
#define mdelay(n)      udelay((n)*1000)

#ifdef CONFIG_BITBANGMII
#include <miiphy.h>
#endif

#ifdef CONFIG_DRIVER_SMC91111
#include "../drivers/net/smc91111.h"
#endif
#ifdef CONFIG_DRIVER_LAN91C96
#include "../drivers/net/lan91c96.h"
#endif

DECLARE_GLOBAL_DATA_PTR;

ulong monitor_flash_len;

#ifdef CONFIG_HAS_DATAFLASH
extern int  AT91F_DataflashInit(void);
extern void dataflash_print_info(void);
#endif

#ifndef CONFIG_IDENT_STRING
#define CONFIG_IDENT_STRING ""
#endif

#ifdef MS_UBOOT_VERSION
const char version_string[] = 
	U_BOOT_VERSION" (" U_BOOT_DATE " - " U_BOOT_TIME ")"CONFIG_IDENT_STRING"-"MS_UBOOT_VERSION;
#else
const char version_string[] = 
	U_BOOT_VERSION" (" U_BOOT_DATE " - " U_BOOT_TIME ")"CONFIG_IDENT_STRING;
#endif

#ifdef CONFIG_DRIVER_RTL8019
extern void rtl8019_get_enetaddr (uchar * addr);
#endif

#if defined(CONFIG_HARD_I2C) || \
    defined(CONFIG_SOFT_I2C)
#include <i2c.h>
#endif

#ifdef CONFIG_GENERIC_MMC
#ifdef CONFIG_HIMCI_V200
extern int mmc_flash_init(int dev_num);
#else
extern int mmc_flash_init(void);
#endif
#endif
#ifdef CONFIG_DDR_TRAINING_V300
extern int check_ddr_training(void);
#endif /* CONFIG_DDR_TRAINING_V300 */

/************************************************************************
 * Coloured LED functionality
 ************************************************************************
 * May be supplied by boards if desired
 */
void inline __coloured_LED_init (void) {}
void coloured_LED_init (void) __attribute__((weak, alias("__coloured_LED_init")));
void inline __red_LED_on (void) {}
void red_LED_on (void) __attribute__((weak, alias("__red_LED_on")));
void inline __red_LED_off(void) {}
void red_LED_off(void) __attribute__((weak, alias("__red_LED_off")));
void inline __green_LED_on(void) {}
void green_LED_on(void) __attribute__((weak, alias("__green_LED_on")));
void inline __green_LED_off(void) {}
void green_LED_off(void) __attribute__((weak, alias("__green_LED_off")));
void inline __yellow_LED_on(void) {}
void yellow_LED_on(void) __attribute__((weak, alias("__yellow_LED_on")));
void inline __yellow_LED_off(void) {}
void yellow_LED_off(void) __attribute__((weak, alias("__yellow_LED_off")));
void inline __blue_LED_on(void) {}
void blue_LED_on(void) __attribute__((weak, alias("__blue_LED_on")));
void inline __blue_LED_off(void) {}
void blue_LED_off(void) __attribute__((weak, alias("__blue_LED_off")));

/************************************************************************
 * Init Utilities							*
 ************************************************************************
 * Some of this code should be moved into the core functions,
 * or dropped completely,
 * but let's get it working (again) first...
 */

#if defined(CONFIG_ARM_DCC) && !defined(CONFIG_BAUDRATE)
#define CONFIG_BAUDRATE 115200
#endif
static int init_baudrate (void)
{
	char tmp[64];	/* long enough for environment variables */
	int i = getenv_r ("baudrate", tmp, sizeof (tmp));
	gd->bd->bi_baudrate = gd->baudrate = (i > 0)
			? (int) simple_strtoul (tmp, NULL, 10)
			: CONFIG_BAUDRATE;

	return (0);
}

static int display_banner (void)
{
	printf ("\n\n%s\n\n", version_string);
	debug ("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	       _armboot_start, _bss_start, _bss_end);
#ifdef CONFIG_MODEM_SUPPORT
	debug ("Modem Support enabled\n");
#endif
#ifdef CONFIG_USE_IRQ
	debug ("IRQ Stack: %08lx\n", IRQ_STACK_START);
	debug ("FIQ Stack: %08lx\n", FIQ_STACK_START);
#endif

	return (0);
}

/*
 * WARNING: this code looks "cleaner" than the PowerPC version, but
 * has the disadvantage that you either get nothing, or everything.
 * On PowerPC, you might see "DRAM: " before the system hangs - which
 * gives a simple yet clear indication which part of the
 * initialization if failing.
 */
#if 0
static int display_dram_config (void)
{
	int i;

#ifdef DEBUG
	puts ("RAM Configuration:\n");

	for(i=0; i<CONFIG_NR_DRAM_BANKS; i++) {
		printf ("Bank #%d: %08lx ", i, gd->bd->bi_dram[i].start);
		print_size (gd->bd->bi_dram[i].size, "\n");
	}
#else
	ulong size = 0;

	for (i=0; i<CONFIG_NR_DRAM_BANKS; i++) {
		size += gd->bd->bi_dram[i].size;
	}
	puts("DRAM:  ");
	print_size(size, "\n");
#endif

	return (0);
}
#endif
#ifndef CONFIG_SYS_NO_FLASH
static void display_flash_config (ulong size)
{
	puts ("Flash: ");
	print_size (size, "\n");
}
#endif /* CONFIG_SYS_NO_FLASH */

#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
static int init_func_i2c (void)
{
	puts ("I2C:   ");
	i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	puts ("ready\n");
	return (0);
}
#endif

#if defined(CONFIG_CMD_PCI) || defined (CONFIG_PCI)
#include <pci.h>
static int arm_pci_init(void)
{
	pci_init();
	return 0;
}
#endif /* CONFIG_CMD_PCI || CONFIG_PCI */

#include <asm/io.h>

#include <net.h>


static void string_to_mac(unsigned char *mac, char* s)
{
	int i;
	char *e;

	for (i = 0; i < 6; ++i) {
		mac[i] = s ? simple_strtoul(s, &e, 16) : 0;
		if (s)
			s = (*e) ? e+1 : e;
	}
}


static unsigned long rstate = 1;
/* trivial congruential random generators. from glibc. */
static void srandom(unsigned long seed)
{
	rstate = seed ? seed : 1;  /* dont allow a 0 seed */
}

static unsigned long random(void)
{
	unsigned int next = rstate;
	int result;

	next *= 1103515245;
	next += 12345;
	result = (unsigned int) (next / 65536) % 2048;

	next *= 1103515245;
	next += 12345;
	result <<= 10;
	result ^= (unsigned int) (next / 65536) % 1024;

	next *= 1103515245;
	next += 12345;
	result <<= 10;
	result ^= (unsigned int) (next / 65536) % 1024;

	rstate = next;

	return result;
}


static void random_ether_addr(unsigned char *mac)
{
	unsigned long ethaddr_low, ethaddr_high;

	srandom(get_timer(0));
	/*
	 * setting the 2nd LSB in the most significant byte of
	 * the address makes it a locally administered ethernet
	 * address
	 */
	ethaddr_high = (random() & 0xfeff) | 0x0200;
	ethaddr_low = random();

	mac[5] = ethaddr_high >> 8;
	mac[4] = ethaddr_high & 0xff;
	mac[3] = ethaddr_low >> 24;
	mac[2] = 0x16;
	mac[1] = 0xc3;
	mac[0] = 0x1c;

	//mac[0] &= 0xc3;    /* clear multicast bit */
	//mac[0] = 0x1c;    /* set local assignment bit (IEEE802) */

}

static int set_random_mac_address(unsigned char *mac, char *ethaddr)
{
	random_ether_addr(mac);

	sprintf((char *)ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	setenv("ethaddr", (char *)ethaddr);


    if(mac[5] == 0xff)
        mac[5] = mac[5] - 1;
    else
        mac[5] = mac[5] + 1;

	sprintf((char *)ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    setenv("ethaddr2", (char *)ethaddr);

    //write_ethaddr_to_mac(gmac_port);
    saveenv();
    
	return 0;
}


static void write_ethaddr_to_mac(void)
{
	char *s;
	unsigned char mac[6];
	char ethaddr[32];

	s = getenv("ethaddr");

    if(!s)
    {
        set_random_mac_address(&mac[0], ethaddr);
    }
    else
    {
	    string_to_mac(&mac[0], s);
    }

    if (!is_valid_ether_addr((u8 *)mac))
    {
		printf("The ethaddr is not valid! \n");
        return;
    }
    
#if 0
    unsigned long reg, iobase_gmac;
    //unsigned long 
    
    reg = mac[4] | (mac[5] << 8);

    iobase_gmac = 0x11020000 + 0 * 0x4000;
    writel(reg, iobase_gmac + 0x00000040);

    reg = mac[0] | (mac[1] << 8) | (mac[2] << 16) | (mac[3] << 24);
    writel(reg, iobase_gmac + 0x00000044);
#endif
    
    // write eth2 mac address
    s = getenv("ethaddr2");

    if(!s)
    {
        set_random_mac_address(&mac[0], ethaddr);
    }
    else
    {
	    string_to_mac(&mac[0], s);
    }

#if 0
    reg = mac[4] | (mac[5] << 8);

    iobase_gmac = 0x11020000 + 1 * 0x4000;
    writel(reg, iobase_gmac + 0x00000040);

    reg = mac[0] | (mac[1] << 8) | (mac[2] << 16) | (mac[3] << 24);
    writel(reg, iobase_gmac + 0x00000044);
#endif

    //return 0;
}

static void set_gpio_defaut(void)
{
    gpio_init();
    gpio_write(BUZZ_CTL, 0); // buzzer on
    udelay(200000);
    gpio_write(BUZZ_CTL, 1); // buzzer off

    gpio_write(POWER_GP0, 0); // power off
    udelay(10000);
    gpio_write(POWER_GP0, 1); // power on
    udelay(10000);
    gpio_write(POWER_GP0, 0); // power off
    
}

/**********************************
GPIO9_2 & GPIO0_6 = [1..0]
00:1004
01:5008
10:1008
11:待定
**********************************/
static void phy_setting(void)
{
    int type;

    type = ((gpio_read(PYH_SELECT_2) << 1) | gpio_read(PYH_SELECT));

    if (MS_DDR_VERSION) //5008/1004 16bit 512M ddr
    {
        switch (type) {
            case 0x00://1004 100Mbps rmii
                gpio_mux(77, 0x3); //RMII0_CLK
                gpio_mux(5, 0x5); //RMII1_CLK
                //gpio_mux(8, 0x0);
                setenv("mdio_intf", "rmii,rmii");
                setenv("phy_addr", "3,5");
                setenv("phy_speed", "0,100");
                break;
            case 0x01:// 5008 1000Mbps  rgmii
                setenv("mdio_intf", "rgmii,rgmii");
                setenv("phy_addr", "1,2");
                setenv("phy_speed", "0,1000");
                break;
            case 0x02:// 1008 1000Mbps  rgmii
                gpio_mux(77, 0x1); //RGMII0_CLK
                gpio_mux(5, 0x5); //RMII1_CLK
                setenv("mdio_intf", "rgmii,rmii");
                setenv("phy_addr", "4,5");
                setenv("phy_speed", "0,100");
                break;
            default:
                printf("unknow phy type!");
                break;
        }
    }
    else //8bit x 2 512M ddr , old 1004
    {
        setenv("mdio_intf", "rmii,rmii");
        setenv("phy_addr", "3,5");
        setenv("phy_speed", "0,100");
    }
}

static void check_mem_size(void)
{
	char *sBoot = NULL;
	char *sMem = NULL;
	
	sBoot = getenv("bootargs");
	if(sBoot)
	{
		sMem = strstr(sBoot, "mem=256M");
		if (!sMem)
            sMem = strstr(sBoot, "mem=316M");
            
		if(sMem != NULL)
		{
			sMem[4] = '3';
			sMem[5] = '0';
			sMem[6] = '0';
			printf("Set mem 256M/316M -> 300M .... \n");
		}
	}
}

static void show_logo(void)
{
    char *cmd = NULL;
    char *end = NULL;
    
    cmd = getenv("bootcmd");
    end = strstr(cmd, "nboot");
	
    if(end != NULL)
    {   
        memset(cmd, ' ', end-cmd);
    }    
    setenv("jpeg_addr", "0x83000000");    
    setenv("vobuf", "0x83800000");
    run_command("nand read $(jpeg_addr) 0 0x80000", 0);
    run_command("decjpg", 0);
    run_command("cfgvobg 0 0", 0);
    run_command("startvo 0 36 12", 0);
    run_command("startvl 0 $(vobuf) 1920 0 0 1920 1080", 0);

}

extern int mstype_write(const char *value, uchar off, uchar cnt);
extern int mstype_read(uchar off, uchar cnt, char *output);

static void check_poe_num(void)
{
    char rdata[64] = {0};
    
    mstype_read(0, 32, rdata);
    *((uchar *) rdata+32) = 0;
    
    if (rdata[15] == 'P')
    {
        if (rdata[16] == '6')
        {
            setenv("poeNum", "24");
        }
        else if (rdata[16] == '1')
        {
            setenv("poeNum", "4");
        }
        else
        {
            setenv("poeNum", "16");
        }
    }
    else
    {
        setenv("poeNum", "0");
    }
}

static void poe_init_dpsp(uchar addr)
{
	uchar disable_class = 0x0;
	uchar poe_disable = 0xf0;
	uchar switch_at = 0xff;
	uchar poe_auto = 0xff;

	i2c_write(addr,0x14, 1, &disable_class, 1);
	i2c_write(addr,0x19, 1, &poe_disable, 1);
	i2c_write(addr,0x44,1, &switch_at, 1);
	i2c_write(addr,0x12, 1, &poe_auto, 1);
}

static void poe_init_clhp(uchar addr)
{
	uchar cut = 0xe2;
	uchar limt = 0xc0;
	uchar hpstat = 0x1;
	uchar poe_hight = 0x24;

	i2c_write(addr,0x81, 1, &poe_hight, 1);
	i2c_write(addr,0x82, 1, &poe_hight, 1);
	i2c_write(addr,0x83, 1, &poe_hight, 1);
	i2c_write(addr,0x84, 1, &poe_hight, 1);

	i2c_write(addr,0x47, 1, &cut, 1);
	i2c_write(addr,0x48, 1, &limt, 1);
	i2c_write(addr,0x49, 1, &hpstat, 1);

	i2c_write(addr,0x4c, 1, &cut, 1);
	i2c_write(addr,0x4d, 1, &limt, 1);
	i2c_write(addr,0x4e, 1, &hpstat, 1);

	i2c_write(addr,0x51, 1, &cut, 1);
	i2c_write(addr,0x52, 1, &limt, 1);
	i2c_write(addr,0x53, 1, &hpstat, 1);

	i2c_write(addr,0x56, 1, &cut, 1);
	i2c_write(addr,0x57, 1, &limt, 1);
	i2c_write(addr,0x58, 1, &hpstat, 1);
}

static int  poe_init(void)
{
	char *poe_num;
	uchar test = 0;
	uchar addr = 0xc0;
	uchar addr1 = 0xa0;
	uchar addr2 = 0xa1;	
	uchar addr3 = 0xa2;	
	uchar addr4 = 0xa4;	
	uchar addr5 = 0xb1;	
	uchar addr6 = 0xb2;	

	poe_num = getenv("poeNum");
	if(NULL != strstr(poe_num,"16"))
	{	
		i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
		poe_read(0xa0,0x43, 1, &test, 1);   //Chip type
		printf("test_type = %0x\n",test);
		if(test != 0x44)
		{
      		i2c_write(0x30,0xaa, 1, &addr, 1);
      		i2c_write(0xc0,0x11, 1, &addr1, 1);	
			i2c_write(0xc0,0x11, 1, &addr2, 1);
			i2c_write(0xc0,0x11, 1, &addr3, 1);
			i2c_write(0xc0,0x11, 1, &addr4, 1);	
		}
		mdelay(1000);

		poe_init_dpsp(addr1);
		poe_init_dpsp(addr2);
		poe_init_dpsp(addr3);
		poe_init_dpsp(addr4);

		if(test == 0x44)
		{
			poe_init_clhp(addr1);
			poe_init_clhp(addr2);
			poe_init_clhp(addr3);
			poe_init_clhp(addr4);
		}
	
	}
	else if(NULL != strstr(poe_num,"24"))
	{	
		i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
		poe_read(0xa0,0x43, 1, &test, 1);   //Chip type
		printf("poe:24 test_type = %0x\n",test);
		if(test != 0x44)
		{
      		i2c_write(0x30,0xaa, 1, &addr, 1);
      		i2c_write(0xc0,0x11, 1, &addr1, 1);	
			ndelay(100000);
			i2c_write(0xc0,0x11, 1, &addr2, 1);
			ndelay(100000);
			i2c_write(0xc0,0x11, 1, &addr3, 1);
			ndelay(100000);
			i2c_write(0xc0,0x11, 1, &addr4, 1);	
			ndelay(100000);
			i2c_write(0xc0,0x11, 1, &addr5, 1);
			ndelay(100000);
			i2c_write(0xc0,0x11, 1, &addr6, 1);	
		}
		mdelay(1000);

		poe_init_dpsp(addr1);
		poe_init_dpsp(addr2);
		poe_init_dpsp(addr3);
		poe_init_dpsp(addr4);
		poe_init_dpsp(addr5);
		poe_init_dpsp(addr6);

		if(test == 0x44)
		{
			poe_init_clhp(addr1);
			poe_init_clhp(addr2);
			poe_init_clhp(addr3);
			poe_init_clhp(addr4);
			poe_init_clhp(addr5);
			poe_init_clhp(addr6);
		}
	}
	return 0;
}

/*
 * Breathe some life into the board...
 *
 * Initialize a serial port as console, and carry out some hardware
 * tests.
 *
 * The first part of initialization is running from Flash memory;
 * its main purpose is to initialize the RAM so that we
 * can relocate the monitor code to RAM.
 */

/*
 * All attempts to come up with a "common" initialization sequence
 * that works for all boards and architectures failed: some of the
 * requirements are just _too_ different. To get rid of the resulting
 * mess of board dependent #ifdef'ed code we now make the whole
 * initialization sequence configurable to the user.
 *
 * The requirements for any new initalization function is simple: it
 * receives a pointer to the "global data" structure as it's only
 * argument, and returns an integer return code, where 0 means
 * "continue" and != 0 means "fatal error, hang the system".
 */
typedef int (init_fnc_t) (void);

int print_cpuinfo (void);

init_fnc_t *init_sequence[] = {
#if defined(CONFIG_ARCH_CPU_INIT)
	arch_cpu_init,		/* basic arch cpu dependent setup */
#endif
	timer_init,		/* initialize timer before usb init */
	board_init,		/* basic board dependent setup */
#if defined(CONFIG_USE_IRQ)
	interrupt_init,		/* set up exceptions */
#endif
//	timer_init,		/* initialize timer */
#ifdef CONFIG_FSL_ESDHC
	get_clocks,
#endif
	env_init,		/* initialize environment */
	init_baudrate,		/* initialze baudrate settings */
	serial_init,		/* serial communications setup */
	console_init_f,		/* stage 1 init of console */
	display_banner,		/* say that we are here */
#if defined(CONFIG_DISPLAY_CPUINFO)
	print_cpuinfo,		/* display cpu info (and speed) */
#endif
#if defined(CONFIG_DISPLAY_BOARDINFO)
	checkboard,		/* display board info */
#endif
#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
	init_func_i2c,
#endif
	dram_init,		/* configure available RAM banks */
#if defined(CONFIG_CMD_PCI) || defined (CONFIG_PCI)
	arm_pci_init,
#endif
/*	display_dram_config */
	NULL,
};


/*****************************************************************************
*   Prototype    : ms_set_flashtype_env
*   Description  : add flash_type into env
*   Input        : char *flash_type  
*   Output       : None
*   Return Value : void
*   Calls        : 
*   Called By    : 
*
*   History:
* 
*       1.  Date         : 2022/4/22
*           Author       : goli
*           Modification : Created function
*
*****************************************************************************/
void ms_set_flashtype_env(char *flash_type)
{
	char *env_flashtype = getenv("flashtype");
	
	if (env_flashtype) {
		if (strcmp(env_flashtype, flash_type)) {
			setenv("flashtype", flash_type);
		}
	}
	else {
		setenv("flashtype", flash_type);
	}
}


void start_armboot (void)
{
	init_fnc_t **init_fnc_ptr;
	char *s;
#ifdef CONFIG_HAS_SLAVE
	char *e;
#endif
#if defined(CONFIG_VFD) || defined(CONFIG_LCD)
	unsigned long addr;
#endif

	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t*)(_armboot_start - CONFIG_SYS_MALLOC_LEN - sizeof(gd_t));
	/* compiler optimization barrier needed for GCC >= 3.4 */
	__asm__ __volatile__("": : :"memory");

	memset ((void*)gd, 0, sizeof (gd_t));
	gd->bd = (bd_t*)((char*)gd - sizeof(bd_t));
	memset (gd->bd, 0, sizeof (bd_t));

	gd->flags |= GD_FLG_RELOC;

	monitor_flash_len = _bss_start - _armboot_start;

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr)() != 0) {
			hang ();
		}
	}

	/* armboot_start is defined in the board-specific linker script */
	mem_malloc_init (_armboot_start - CONFIG_SYS_MALLOC_LEN,
			CONFIG_SYS_MALLOC_LEN);

#ifndef CONFIG_SYS_NO_FLASH
	/* configure available FLASH banks */
	display_flash_config (flash_init ());
#endif /* CONFIG_SYS_NO_FLASH */

#ifdef CONFIG_VFD
#	ifndef PAGE_SIZE
#	  define PAGE_SIZE 4096
#	endif
	/*
	 * reserve memory for VFD display (always full pages)
	 */
	/* bss_end is defined in the board-specific linker script */
	addr = (_bss_end + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);
	vfd_setmem (addr);
	gd->fb_base = addr;
#endif /* CONFIG_VFD */

#ifdef CONFIG_LCD
	/* board init may have inited fb_base */
	if (!gd->fb_base) {
#		ifndef PAGE_SIZE
#		  define PAGE_SIZE 4096
#		endif
		/*
		 * reserve memory for LCD display (always full pages)
		 */
		/* bss_end is defined in the board-specific linker script */
		addr = (_bss_end + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);
		lcd_setmem (addr);
		gd->fb_base = addr;
	}
#endif /* CONFIG_LCD */

    set_gpio_defaut();
    //gpio_write(POWER_GP1, 1);

#ifdef CONFIG_CMD_SF
#if (defined CONFIG_ARCH_HI3559 || defined CONFIG_ARCH_HI3556)
    if(get_boot_media() == BOOT_MEDIA_SPIFLASH) {
#endif
	spi_flash_probe(0, 0, 0, 0);
#if (defined CONFIG_ARCH_HI3559 || defined CONFIG_ARCH_HI3556)
    }
#endif
#endif

/* it is not needed in A7 in A17-A7  */
#ifdef CONFIG_CMD_NAND
#if (defined CONFIG_ARCH_HI3559 || defined CONFIG_ARCH_HI3556)
    if(get_boot_media() == BOOT_MEDIA_NAND) {
#endif
	char flash_type[64] = {0};
	nand_init(flash_type);		/* go init the NAND */
#if (defined CONFIG_ARCH_HI3559 || defined CONFIG_ARCH_HI3556)
    }
#endif
#endif

#if defined(CONFIG_CMD_ONENAND)
	onenand_init();
#endif

#ifdef CONFIG_HAS_DATAFLASH
	AT91F_DataflashInit();
	dataflash_print_info();
#endif

#if defined(CONFIG_ARCH_HI3536) \
	|| defined(CONFIG_HI3518EV200) \
	|| defined(CONFIG_HI3516CV300) \
	|| defined(CONFIG_ARCH_HI3519) \
	|| defined(CONFIG_ARCH_HI3519V101) \
	|| defined(CONFIG_ARCH_HI3516AV200) \
	|| defined(CONFIG_ARCH_HI3559) || defined(CONFIG_ARCH_HI3556)
#ifdef CONFIG_GENERIC_MMC
#if (defined CONFIG_ARCH_HI3559 || defined CONFIG_ARCH_HI3556)
    if(get_boot_media() == BOOT_MEDIA_EMMC) {
#endif
        puts("MMC:   ");
        mmc_initialize(0);
        mmc_flash_init(0);
#if (defined CONFIG_ARCH_HI3559 || defined CONFIG_ARCH_HI3556)
    }
#endif
#endif
#if ((defined(CONFIG_ARCH_HI3559) || defined(CONFIG_ARCH_HI3556))&& defined(CONFIG_AUTO_SD_UPDATE))
	extern int auto_update_flag;
	extern int bare_chip_program;
	extern int is_auto_update(void);
	extern int is_bare_program(void);

	/* auto update flag */
	if(is_auto_update())
		auto_update_flag = 1;
	else
		auto_update_flag = 0;

	/* bare chip program flag */
	if(is_bare_program())
		bare_chip_program = 1;
	else
		bare_chip_program = 0;
	printf("bare_chip_program=%0d,auto_update_flag=%0d",bare_chip_program,auto_update_flag);
    if(bare_chip_program==0 && auto_update_flag==0)
        printf("\n");

    /* init SD card */
	if(auto_update_flag || bare_chip_program) {
        /*
         * because bootrom code init mmc and sd card,
         * wait some ms here before reinit!
         */
#define mdelay(n) ({unsigned long msec = (n); while (msec--) udelay(1000); })
        if(bare_chip_program == 1)
            mdelay(1000);
        mmc_initialize(2);
		mmc_flash_init(2);
	}
#endif

#if (defined(CONFIG_HI3516CV300) || defined(CONFIG_ARCH_HI3519)\
		|| defined(CONFIG_ARCH_HI3519V101) || defined(CONFIG_ARCH_HI3516AV200))\
	&& defined(CONFIG_AUTO_SD_UPDATE)
	mmc_initialize(2);
	mmc_flash_init(2);
#endif

#else
#ifdef CONFIG_GENERIC_MMC
	puts("MMC:   ");
	mmc_initialize(gd->bd);
	mmc_flash_init();
#endif
#endif

	/* initialize environment */
	env_relocate ();

#ifdef CONFIG_VFD
	/* must do this after the framebuffer is allocated */
	drv_vfd_init();
#endif /* CONFIG_VFD */

#ifdef CONFIG_SERIAL_MULTI
	serial_initialize();
#endif

	/* IP Address */
	gd->bd->bi_ip_addr = getenv_IPaddr ("ipaddr");

	ms_set_flashtype_env(flash_type);	//goli add to setenv "flashtype", 2022.04.22
	
	stdio_init ();	/* get the devices list going. */

	jumptable_init ();

#if defined(CONFIG_API)
	/* Initialize API */
	api_init ();
#endif

	console_init_r ();	/* fully init console as a device */

#if defined(CONFIG_ARCH_MISC_INIT)
	/* miscellaneous arch dependent initialisations */
	arch_misc_init ();
#endif
#if defined(CONFIG_MISC_INIT_R)
	/* miscellaneous platform dependent initialisations */
	misc_init_r ();
#endif

	/* enable exceptions */
	enable_interrupts ();

	/* Perform network card initialisation if necessary */
#ifdef CONFIG_DRIVER_TI_EMAC
	/* XXX: this needs to be moved to board init */
extern void davinci_eth_set_mac_addr (const u_int8_t *addr);
	if (getenv ("ethaddr")) {
		uchar enetaddr[6];
		eth_getenv_enetaddr("ethaddr", enetaddr);
		davinci_eth_set_mac_addr(enetaddr);
	}
#endif

#if defined(CONFIG_DRIVER_SMC91111) || defined (CONFIG_DRIVER_LAN91C96)
	/* XXX: this needs to be moved to board init */
	if (getenv ("ethaddr")) {
		uchar enetaddr[6];
		eth_getenv_enetaddr("ethaddr", enetaddr);
		smc_set_mac_addr(enetaddr);
	}
#endif /* CONFIG_DRIVER_SMC 91111 || CONFIG_DRIVER_LAN91C96 */

    write_ethaddr_to_mac();
	/* Initialize from environment */
	if ((s = getenv ("loadaddr")) != NULL) {
		load_addr = simple_strtoul (s, NULL, 16);
	}
#if defined(CONFIG_CMD_NET)
	if ((s = getenv ("bootfile")) != NULL) {
		copy_filename (BootFile, s, sizeof (BootFile));
	}
#endif

#ifdef BOARD_LATE_INIT
	board_late_init ();
#endif

#ifdef CONFIG_BITBANGMII
	bb_miiphy_init();
#endif
#if defined(CONFIG_CMD_NET)
#if defined(CONFIG_NET_MULTI)
	puts ("Net:   ");
#endif
	eth_initialize(gd->bd);
#if defined(CONFIG_RESET_PHY_R)
	debug ("Reset Ethernet PHY\n");
	reset_phy();
#endif
#endif

#if defined(CONFIG_BOOTROM_SUPPORT) && (!defined(CONFIG_ARCH_HI3559) && !defined(CONFIG_ARCH_HI3556))
	extern void download_boot(const int (*handle)(void));
	download_boot(NULL);
#endif
#ifdef CONFIG_HAS_SLAVE
	e = getenv("slave_autostart");
	if (e && (*e == '1'))
		slave_start();
#endif

#ifdef CONFIG_DDR_TRAINING_V300
	check_ddr_training();
#endif /* CONFIG_DDR_TRAINING_V300 */
	product_control();

#ifdef CONFIG_SNAPSHOT_BOOT
	/* example */
	/* set_mtdparts_info("mtdparts=hinand:1M(boot),3M(kernel),64M(rootfs),2M(param),16M(hibernate)"); */

	extern void comp_save_snapshot_image(void);
	comp_save_snapshot_image();
#endif
    phy_setting();
    show_logo();
	gpio_write(STAT_GREEN_LED, 0);
	check_mem_size();
	check_poe_num();
	poe_init();
	/* main_loop() can return to retry autoboot, if so just run it again. */
	for (;;) {
		main_loop ();
	}

	/* NOTREACHED - no way out of command loop except booting */
}

void hang (void)
{
	puts ("### ERROR ### Please RESET the board ###\n");
	for (;;);
}
