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
#include <asm/io.h>
#include <environment.h>
#include <ethcfg.h>
#include "hi_drv_gpio.h"
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

//zbing add
#ifdef MS_UBOOT_VERSION
const char version_string[] = 
	U_BOOT_VERSION" (" U_BOOT_DATE " - " U_BOOT_TIME ")"CONFIG_IDENT_STRING"-"MS_UBOOT_VERSION;
#else
const char version_string[] = 
	U_BOOT_VERSION" (" U_BOOT_DATE " - " U_BOOT_TIME ")"CONFIG_IDENT_STRING;
#endif

#if 0
#ifndef CONFIG_SUPPORT_CA_RELEASE
const char version_string[] =
	U_BOOT_VERSION" (" U_BOOT_DATE " - " U_BOOT_TIME ")"CONFIG_IDENT_STRING;
#else
const char version_string[] = "";
#endif
#endif
//end

#ifdef CONFIG_DRIVER_RTL8019
extern void rtl8019_get_enetaddr (uchar * addr);
#endif

#if defined(CONFIG_HARD_I2C) || \
    defined(CONFIG_SOFT_I2C)
#include <i2c.h>
#endif

#ifdef CONFIG_CMD_NAND
extern int nand_rr_param_init(void);
#endif

#ifdef CONFIG_GENERIC_MMC
extern int mmc_flash_init(void);
#endif

#ifdef CONFIG_DDR_TRAINING
extern int check_ddr_training(void);
#endif /* CONFIG_DDR_TRAINING */

#ifdef CONFIG_PRODUCT_WITH_BOOT
extern int fastapp_entry(int argc, char *argv[]);
extern int product_init(void);
#endif

extern int reserve_mem_init(void);
extern int show_reserve_mem(void);
extern void init_reg2(void);
/*
 * Phy address should pass to kernel, even if u-boot is not support net driver
 * The value priority: (The left side config will replace the right side config)
 * Environment > compile config > uboot default value
 *     Environment           - setenv "phyaddr"
 *     Compile config        - HISFV_PHY_U
 *     u-boot default value  - HISFV_DEFAULT_PHY_U
 */
unsigned char U_PHY_ADDR = HISFV_DEFAULT_PHY_U;
unsigned char D_PHY_ADDR = HISFV_DEFAULT_PHY_D;


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
	return (0);
}

#ifndef CONFIG_SYS_NO_FLASH
static void display_flash_config (ulong size)
{
	puts ("Flash: ");
	print_size (size, "\n");
}
#endif /* CONFIG_SYS_NO_FLASH */

#ifdef CONFIG_MS_RANDOM_ETHADDR

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

	return 0;
}


static void write_ethaddr_to_mac(void)
{
	char *s;
	unsigned char mac[6];
	char ethaddr[32];
	
	s = getenv("ethaddr");
    if(s)
    {	    
        string_to_mac(&mac[0], s);
        if (!is_valid_ether_addr((u8 *)mac))
        {
            set_random_mac_address(&mac[0], ethaddr);
    		printf("using random MAC0 address :%s \n", ethaddr);
            setenv("ethaddr", (char *)ethaddr);
        }
    }
    else
    {
        set_random_mac_address(&mac[0], ethaddr);
        printf("using random MAC0 address :%s \n", ethaddr);
        setenv("ethaddr", (char *)ethaddr);
    }
    
    // write eth2 mac address
	s = getenv("ethaddr2");
    if(s)
    {	    
        string_to_mac(&mac[0], s);
        if (!is_valid_ether_addr((u8 *)mac))
        {
            set_random_mac_address(&mac[0], ethaddr);
    		printf("using random MAC1 address :%s \n", ethaddr);
            setenv("ethaddr2", (char *)ethaddr);
        }
    }
    else
    {
        set_random_mac_address(&mac[0], ethaddr);
        printf("using random MAC1 address :%s \n", ethaddr);
        setenv("ethaddr2", (char *)ethaddr);
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

    //return 0;
}

#endif


static void set_gpio_defaut(void)
{
	char buff[64];
	HI_U32 u32Val;

    HI_DRV_GPIO_Init();

//  set mcu boot status
    HI_DRV_GPIO_SetDirBit(POWEROFF_CTL, OUTPUT);
    HI_DRV_GPIO_WriteBit(POWEROFF_CTL, 0);
    udelay(10000);    
    HI_DRV_GPIO_WriteBit(POWEROFF_CTL, 1);    
	udelay(10000);
    HI_DRV_GPIO_WriteBit(POWEROFF_CTL, 0);
//  sata reset    
    HI_DRV_GPIO_SetDirBit(SATA_RESET, OUTPUT);
    HI_DRV_GPIO_WriteBit(SATA_RESET, 0);
    udelay(10000);    
    HI_DRV_GPIO_WriteBit(SATA_RESET, 1);
//set state led
    HI_DRV_GPIO_SetDirBit(STATE_LED, OUTPUT);
    HI_DRV_GPIO_WriteBit(STATE_LED, 1);    
// buzz reset
    HI_DRV_GPIO_SetDirBit(BUZZ_CTL, OUTPUT);
    HI_DRV_GPIO_WriteBit(BUZZ_CTL, BUZZ_ON);
    udelay(200000);    
    HI_DRV_GPIO_WriteBit(BUZZ_CTL, BUZZ_OFF);

	HI_DRV_GPIO_SetDirBit(MS_GPIO(12,3), INPUT);
	writel(0x00001170, 0xf8a2116c);//gpio 12_3 set pull up
	HI_DRV_GPIO_ReadBit(MS_GPIO(12,3), &u32Val);//gpio 12_3
	if(MS_DDR_VERSION == 1 && u32Val == 1)	//1GB 1009
	{
		printf("set 1009!\n");
		//ioshare  RMII1 settings 1009 	1009:1GB
	    writel(0x00000134, 0xf8a21078);
	    writel(0x00000124, 0xf8a2107c);
	    writel(0x00000124, 0xf8a21080);
	    writel(0x00000174, 0xf8a21084);
	    writel(0x00000174, 0xf8a21088);
	    writel(0x00000174, 0xf8a2108C);
	    writel(0x00000174, 0xf8a21090);
	    writel(0x00000174, 0xf8a21094);
	    writel(0x00000164, 0xf8a21098);
	    writel(0x00000170, 0xf8a2109C);
	    writel(0x00000174, 0xf8a210a0);
	    writel(0x00000170, 0xf8a210a4);
	    writel(0x00000170, 0xf8a210a8);
	    writel(0x00000170, 0xf8a210aC);
	    writel(0x00000170, 0xf8a210b0);
	    writel(0x000001f0, 0xf8a210b4);
	}
	else
	{
		printf("set 5008 5016!\n");
		//ioshare  RGMII1 settings  5016/5008    5008:2GB 
	    writel(0x00000130, 0xf8a21078);
	    writel(0x00000130, 0xf8a2107c);
	    writel(0x00000131, 0xf8a21080);
	    writel(0x00000161, 0xf8a21084);
	    writel(0x00000161, 0xf8a21088);
	    writel(0x00000161, 0xf8a2108C);
	    writel(0x00000161, 0xf8a21090);
	    writel(0x00000161, 0xf8a21094);
	    writel(0x00000170, 0xf8a21098);
	    writel(0x00000161, 0xf8a2109C);
	    writel(0x00000171, 0xf8a210a0);
	    writel(0x00000171, 0xf8a210a4);
	    writel(0x00000171, 0xf8a210a8);
	    writel(0x00000171, 0xf8a210aC);
	    writel(0x00000171, 0xf8a210b0);
	    writel(0x000001f1, 0xf8a210b4);
		
		writel(0x0000003c, 0xf984300c);	//网口0的 MAC_IF 状态控制寄存器 force PHY link 驱动中无法修改该值
	}
}

/************************hdmi to vga begin***********************************/
#define VGA_I2C_ID   2

static int HDMI_WriteI2C_Byte(unsigned char devAddr, unsigned char reg, unsigned char value)
{
    int ret = -1;
    
    ret = HI_DRV_I2C_Write(VGA_I2C_ID, devAddr, reg, sizeof(reg), &value, sizeof(value));
    if (ret != 0) {
        printf( "%s: write addr[0x%x] reg[0x%x] value[0x%x] with %#x\n", \
                __func__, devAddr, reg, value, ret);
    }
    
    return ret;
}

static unsigned char HDMI_ReadI2C_Byte(unsigned char devAddr, unsigned char reg)
{
    int ret = -1;
    unsigned char value = 0;
    
    ret = HI_DRV_I2C_Read(VGA_I2C_ID, devAddr, reg, sizeof(reg), &value, sizeof(value));
    if (ret != 0) {
        printf( "%s: read addr[0x%x] reg[0x%x] with %#x\n", \
                __func__, devAddr, reg, ret);
        
    }
    
    return value;

}


static void hdmi_to_vga(void)
{
    HI_DRV_I2C_Init();
    if (MS_DDR_VERSION == 2)
    {   
        // HW reset
       if(!(HDMI_ReadI2C_Byte(0x78,0x00) == 0x86 && HDMI_ReadI2C_Byte(0x78,0x01) == 0x12))
    	{
			return 0;
    	}
        HI_DRV_GPIO_SetDirBit(VGA_RESET, OUTPUT);
        HI_DRV_GPIO_WriteBit(VGA_RESET, 0);        
        udelay(20000);    
        HI_DRV_GPIO_WriteBit(VGA_RESET, 1);
        udelay(20000);    
//        I2CADR=0x78;
        HDMI_WriteI2C_Byte(0x78, 0x5c,0x1d);//set HDMI TX HPD software
        HDMI_WriteI2C_Byte(0x78, 0x09,0x2b);
        HDMI_WriteI2C_Byte(0x78, 0x0a,0xdf);
        HDMI_WriteI2C_Byte(0x78, 0x0b,0xf9);
        HDMI_WriteI2C_Byte(0x78, 0x0d,0x5b);
        HDMI_WriteI2C_Byte(0x78, 0xa8,0x08);
        HDMI_WriteI2C_Byte(0x78, 0xac,0x00);
        HDMI_WriteI2C_Byte(0x78, 0xad,0x00);
        HDMI_WriteI2C_Byte(0x78, 0xb4,0x80);
        HDMI_WriteI2C_Byte(0x78, 0x52,0x1a);
        HDMI_WriteI2C_Byte(0x78, 0xb8,0xb0);
        HDMI_WriteI2C_Byte(0x78, 0xbb,0xec);
        HDMI_WriteI2C_Byte(0x78, 0x4f,0x00);
        HDMI_WriteI2C_Byte(0x78, 0x4d,0x10);
        /********************VGA_OUTPUT********************/
//        I2CADR=0x78;
        HDMI_WriteI2C_Byte(0x78, 0x8d,0xf0);
        HDMI_WriteI2C_Byte(0x78, 0x91,0xf0);
        HDMI_WriteI2C_Byte(0x78, 0x43,0x00); //调节RGB的色偏
//        I2CADR=0x7c;
        HDMI_WriteI2C_Byte(0x7c, 0x55,0xea);
        HDMI_WriteI2C_Byte(0x7c, 0x56,0x54);
//        I2CADR=0x78;
        /********************END********************/    
        HDMI_WriteI2C_Byte(0x78, 0x95,0xa7);//clk swing set
        HDMI_WriteI2C_Byte(0x78, 0x96,0xd8);//tap0 en
        HDMI_WriteI2C_Byte(0x78, 0x97,0x38);//hdmitx en sel rg
        HDMI_WriteI2C_Byte(0x78, 0x99,0xda);// 0xd0 tap0 d0 swing set
        HDMI_WriteI2C_Byte(0x78, 0x9b,0xc2);//LDO select
        HDMI_WriteI2C_Byte(0x78, 0x9c,0x5a);//0x4f tap0 d1 swing
        HDMI_WriteI2C_Byte(0x78, 0x9d,0x5a);//0x4e tap0 d2 swing
        HDMI_WriteI2C_Byte(0x78, 0x9e,0x40);//tap1 d1/d2 swing
    }
    HI_DRV_I2C_Exit();
}

/************************hdmi to vga end***********************************/


extern void dm2016_init(void);
extern int dm2016_read(unsigned dev_addr,unsigned offset,uchar * buffer,unsigned cnt);
extern int mstype_write(const char *value, uchar off, uchar cnt);
extern int mstype_read(uchar off, uchar cnt, char *output);

static void check_poe_num(void)
{
    char rdata[63] = {0};
    //
    dm2016_init();
    //dm2016_read(CFG_I2C_DM2016_ADDR, 0, (uchar *)rdata, 32);
    mstype_read(0, 32, rdata);
    *((uchar *) rdata+32) = 0;
    
    if (rdata[15] == 'P')
    {
        if (rdata[3] == '1')
        {
            setenv("poeNum", "4");
        }
        else if (rdata[16] == '4')
        {
            setenv("poeNum", "16");
        }
        else ///if (rdata[3] == '5')
        {
            setenv("poeNum", "8");
        }
    }
    else
    {
        setenv("poeNum", "0");
    }
}


static void poe_init(void)
{
	char *poe_num;
	char test = 0;
	uchar disable_class = 0x0;
	char poe_disable = 0xf0;
	char switch_at = 0xf;
	uchar poe_auto = 0xff;
	uchar poe_hight = 0x24;
	uchar cut = 0xe2;
	uchar limt = 0xc0;
	uchar hpstat = 0x1;
	poe_num = getenv("poeNum");
	if(NULL != strstr(poe_num,"4"))
	{
		HI_DRV_I2C_Read(2, 0x40,0x43, 1, &test, 1);   //Chip type
		printf("test_type = %0x\n",test);
		if(test != 0x44)
			{
				char pse_addr1 = 0;	
				HI_DRV_I2C_Read(2, 0x40,0x11, 1, &pse_addr1, 1);
						
				if((pse_addr1 != 0xa0))
				{
					char addr = 0x50;
				  	char addr1 = 0xa0;
		      		HI_DRV_I2C_Write(2, 0x60,0xaa, 1, &addr, 1);
		      		HI_DRV_I2C_Write(2, 0xa0,0x11, 1, &addr1, 1);
				}

			}
		HI_DRV_I2C_Write(2, 0x40,0x14, 1, &disable_class, 1);
		HI_DRV_I2C_Write(2, 0x40,0x19, 1, &poe_disable, 1);
		HI_DRV_I2C_Write(2, 0x40,0x44, 1, &switch_at, 1);
		HI_DRV_I2C_Write(2, 0x40,0x12, 1, &poe_auto, 1);
		
		if(test == 0x44)
		{
			HI_DRV_I2C_Write(2, 0x40,0x81, 1, &poe_hight, 1);
			HI_DRV_I2C_Write(2, 0x40,0x82, 1, &poe_hight, 1);
			HI_DRV_I2C_Write(2, 0x40,0x83, 1, &poe_hight, 1);
			HI_DRV_I2C_Write(2, 0x40,0x84, 1, &poe_hight, 1);
			
			HI_DRV_I2C_Write(2, 0x40,0x47, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x40,0x48, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x40,0x49, 1, &hpstat, 1);

			HI_DRV_I2C_Write(2, 0x40,0x4c, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x40,0x4d, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x40,0x4e, 1, &hpstat, 1);

			HI_DRV_I2C_Write(2, 0x40,0x51, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x40,0x52, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x40,0x53, 1, &hpstat, 1);

			HI_DRV_I2C_Write(2, 0x40,0x56, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x40,0x57, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x40,0x58, 1, &hpstat, 1);
		}
	
	}
	else if(NULL != strstr(poe_num,"8"))
	{
		
		HI_DRV_I2C_Read(2, 0x40,0x43, 1, &test, 1);   //Chip type
		printf("test_type = %0x\n",test);
		if(test != 0x44)
		{
			char pse_addr1 = 0;
			char pse_addr2 = 0;
						
			HI_DRV_I2C_Read(2, 0x40,0x11, 1, &pse_addr1, 1);
			HI_DRV_I2C_Read(2, 0x42,0x11, 1, &pse_addr2, 1);
			if((pse_addr1 != 0xa0)||(pse_addr2 != 0xa1))
			{
				char addr = 0x50;
				char addr1 = 0xa0;
				char addr2 = 0xa1;
				HI_DRV_I2C_Write(2, 0x60,0xaa, 1, &addr, 1);
				HI_DRV_I2C_Write(2, 0xa0,0x11, 1, &addr1, 1);
				HI_DRV_I2C_Write(2, 0xa0,0x11, 1, &addr2, 1);				
			}
		}
		
		mdelay(1000);
		HI_DRV_I2C_Write(2, 0x40,0x14, 1, &disable_class, 1);
		HI_DRV_I2C_Write(2, 0x42,0x14, 1, &disable_class, 1);
		HI_DRV_I2C_Write(2, 0x40,0x19, 1, &poe_disable, 1);
		HI_DRV_I2C_Write(2, 0x42,0x19, 1, &poe_disable, 1);
		HI_DRV_I2C_Write(2, 0x40,0x44, 1, &switch_at, 1);
		HI_DRV_I2C_Write(2, 0x42,0x44, 1, &switch_at, 1);
		HI_DRV_I2C_Write(2, 0x40,0x12, 1, &poe_auto, 1);
		HI_DRV_I2C_Write(2, 0x42,0x12, 1, &poe_auto, 1);
		
		if(test == 0x44)
		{
			HI_DRV_I2C_Write(2, 0x40,0x81, 1, &poe_hight, 1);
			HI_DRV_I2C_Write(2, 0x40,0x82, 1, &poe_hight, 1);
			HI_DRV_I2C_Write(2, 0x40,0x83, 1, &poe_hight, 1);
			HI_DRV_I2C_Write(2, 0x40,0x84, 1, &poe_hight, 1);
			
			HI_DRV_I2C_Write(2, 0x40,0x47, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x40,0x48, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x40,0x49, 1, &hpstat, 1);

			HI_DRV_I2C_Write(2, 0x40,0x4c, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x40,0x4d, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x40,0x4e, 1, &hpstat, 1);

			HI_DRV_I2C_Write(2, 0x40,0x51, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x40,0x52, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x40,0x53, 1, &hpstat, 1);

			HI_DRV_I2C_Write(2, 0x40,0x56, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x40,0x57, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x40,0x58, 1, &hpstat, 1);



			HI_DRV_I2C_Write(2, 0x42,0x81, 1, &poe_hight, 1);
			HI_DRV_I2C_Write(2, 0x42,0x82, 1, &poe_hight, 1);
			HI_DRV_I2C_Write(2, 0x42,0x83, 1, &poe_hight, 1);
			HI_DRV_I2C_Write(2, 0x42,0x84, 1, &poe_hight, 1);

			HI_DRV_I2C_Write(2, 0x42,0x47, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x42,0x48, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x42,0x49, 1, &hpstat, 1);
			
			HI_DRV_I2C_Write(2, 0x42,0x4c, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x42,0x4d, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x42,0x4e, 1, &hpstat, 1);
			
			HI_DRV_I2C_Write(2, 0x42,0x51, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x42,0x52, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x42,0x53, 1, &hpstat, 1);
			
			HI_DRV_I2C_Write(2, 0x42,0x56, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x42,0x57, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x42,0x58, 1, &hpstat, 1);
		}
	}
	else if(NULL != strstr(poe_num,"16"))
	{
		
		HI_DRV_I2C_Read(2, 0x40,0x43, 1, &test, 1);   //Chip type
		printf("test_type = %0x\n",test);
		if(test != 0x44)
		{
			char pse_addr1 = 0;
			char pse_addr2 = 0;
			char pse_addr3 = 0;
			char pse_addr4 = 0;
			
			HI_DRV_I2C_Read(2, 0x40,0x11, 1, &pse_addr1, 1);
			HI_DRV_I2C_Read(2, 0x42,0x11, 1, &pse_addr2, 1);
			HI_DRV_I2C_Read(2, 0x44,0x11, 1, &pse_addr3, 1);
			HI_DRV_I2C_Read(2, 0x48,0x11, 1, &pse_addr4, 1);
			
			if((pse_addr1 != 0xa0)||(pse_addr2 != 0xa1)||(pse_addr3 != 0xa2)||(pse_addr4 != 0xa4))
			{
				char addr = 0x50;
				char addr1 = 0xa0;
				char addr2 = 0xa1;
				char addr3 = 0xa2;
				char addr4 = 0xa4;
				HI_DRV_I2C_Write(2, 0x60,0xaa, 1, &addr, 1);
				HI_DRV_I2C_Write(2, 0xa0,0x11, 1, &addr1, 1);
				HI_DRV_I2C_Write(2, 0xa0,0x11, 1, &addr2, 1);
				HI_DRV_I2C_Write(2, 0xa0,0x11, 1, &addr3, 1);
				HI_DRV_I2C_Write(2, 0xa0,0x11, 1, &addr4, 1);
			}
		}
		
		mdelay(1000);
		HI_DRV_I2C_Write(2, 0x40,0x14, 1, &disable_class, 1);
		HI_DRV_I2C_Write(2, 0x42,0x14, 1, &disable_class, 1);
		HI_DRV_I2C_Write(2, 0x44,0x14, 1, &disable_class, 1);
		HI_DRV_I2C_Write(2, 0x48,0x14, 1, &disable_class, 1);
		
		HI_DRV_I2C_Write(2, 0x40,0x19, 1, &poe_disable, 1);
		HI_DRV_I2C_Write(2, 0x42,0x19, 1, &poe_disable, 1);
		HI_DRV_I2C_Write(2, 0x44,0x19, 1, &poe_disable, 1);
		HI_DRV_I2C_Write(2, 0x48,0x19, 1, &poe_disable, 1);
		
		HI_DRV_I2C_Write(2, 0x40,0x44, 1, &switch_at, 1);
		HI_DRV_I2C_Write(2, 0x42,0x44, 1, &switch_at, 1);
		HI_DRV_I2C_Write(2, 0x44,0x44, 1, &switch_at, 1);
		HI_DRV_I2C_Write(2, 0x48,0x44, 1, &switch_at, 1);
		
		HI_DRV_I2C_Write(2, 0x40,0x12, 1, &poe_auto, 1);
		HI_DRV_I2C_Write(2, 0x42,0x12, 1, &poe_auto, 1);
		HI_DRV_I2C_Write(2, 0x44,0x12, 1, &poe_auto, 1);
		HI_DRV_I2C_Write(2, 0x48,0x12, 1, &poe_auto, 1);
		
		if(test == 0x44)
		{
		    // 40
			HI_DRV_I2C_Write(2, 0x40,0x81, 1, &poe_hight, 1);
			HI_DRV_I2C_Write(2, 0x40,0x82, 1, &poe_hight, 1);
			HI_DRV_I2C_Write(2, 0x40,0x83, 1, &poe_hight, 1);
			HI_DRV_I2C_Write(2, 0x40,0x84, 1, &poe_hight, 1);
			
			HI_DRV_I2C_Write(2, 0x40,0x47, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x40,0x48, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x40,0x49, 1, &hpstat, 1);

			HI_DRV_I2C_Write(2, 0x40,0x4c, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x40,0x4d, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x40,0x4e, 1, &hpstat, 1);

			HI_DRV_I2C_Write(2, 0x40,0x51, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x40,0x52, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x40,0x53, 1, &hpstat, 1);

			HI_DRV_I2C_Write(2, 0x40,0x56, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x40,0x57, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x40,0x58, 1, &hpstat, 1);

		    // 42
			HI_DRV_I2C_Write(2, 0x42,0x81, 1, &poe_hight, 1);
			HI_DRV_I2C_Write(2, 0x42,0x82, 1, &poe_hight, 1);
			HI_DRV_I2C_Write(2, 0x42,0x83, 1, &poe_hight, 1);
			HI_DRV_I2C_Write(2, 0x42,0x84, 1, &poe_hight, 1);

			HI_DRV_I2C_Write(2, 0x42,0x47, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x42,0x48, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x42,0x49, 1, &hpstat, 1);
			
			HI_DRV_I2C_Write(2, 0x42,0x4c, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x42,0x4d, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x42,0x4e, 1, &hpstat, 1);
			
			HI_DRV_I2C_Write(2, 0x42,0x51, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x42,0x52, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x42,0x53, 1, &hpstat, 1);
			
			HI_DRV_I2C_Write(2, 0x42,0x56, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x42,0x57, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x42,0x58, 1, &hpstat, 1);

			// 44
			HI_DRV_I2C_Write(2, 0x44,0x81, 1, &poe_hight, 1);
			HI_DRV_I2C_Write(2, 0x44,0x82, 1, &poe_hight, 1);
			HI_DRV_I2C_Write(2, 0x44,0x83, 1, &poe_hight, 1);
			HI_DRV_I2C_Write(2, 0x44,0x84, 1, &poe_hight, 1);

			HI_DRV_I2C_Write(2, 0x44,0x47, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x44,0x48, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x44,0x49, 1, &hpstat, 1);
			
			HI_DRV_I2C_Write(2, 0x44,0x4c, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x44,0x4d, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x44,0x4e, 1, &hpstat, 1);
			
			HI_DRV_I2C_Write(2, 0x44,0x51, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x44,0x52, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x44,0x53, 1, &hpstat, 1);
			
			HI_DRV_I2C_Write(2, 0x44,0x56, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x44,0x57, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x44,0x58, 1, &hpstat, 1);

			// 48
			HI_DRV_I2C_Write(2, 0x48,0x81, 1, &poe_hight, 1);
			HI_DRV_I2C_Write(2, 0x48,0x82, 1, &poe_hight, 1);
			HI_DRV_I2C_Write(2, 0x48,0x83, 1, &poe_hight, 1);
			HI_DRV_I2C_Write(2, 0x48,0x84, 1, &poe_hight, 1);

			HI_DRV_I2C_Write(2, 0x48,0x47, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x48,0x48, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x48,0x49, 1, &hpstat, 1);
			
			HI_DRV_I2C_Write(2, 0x48,0x4c, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x48,0x4d, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x48,0x4e, 1, &hpstat, 1);
			
			HI_DRV_I2C_Write(2, 0x48,0x51, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x48,0x52, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x48,0x53, 1, &hpstat, 1);
			
			HI_DRV_I2C_Write(2, 0x48,0x56, 1, &cut, 1);
			HI_DRV_I2C_Write(2, 0x48,0x57, 1, &limt, 1);
			HI_DRV_I2C_Write(2, 0x48,0x58, 1, &hpstat, 1);
		}
	}
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
	board_init,		/* basic board dependent setup */
#if defined(CONFIG_USE_IRQ)
	interrupt_init,		/* set up exceptions */
#endif
	timer_init,		/* initialize timer */
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
	dram_init,		/* configure available RAM banks */
	arch_usb_init,
	NULL,
};

static void show_boot_env(void)
{
	int env_media;

	printf("\nBoot Env on %s\n", env_get_media(&env_media));
	printf("    Env Offset:          0x%08X\n", CONFIG_ENV_OFFSET);
#ifdef CONFIG_ENV_OFFSET_REDUND
	printf("    Backup Env Offset:   0x%08X\n", CONFIG_ENV_OFFSET_REDUND);
#endif /* CONFIG_ENV_BACKUP */
	printf("    Env Size:            0x%08X\n", CONFIG_ENV_SIZE);

	/* check if env addr is alignment block size */
#if defined(CONFIG_GENERIC_SF)
	if ((env_media == BOOT_MEDIA_SPIFLASH) &&
	    ((get_spiflash_info()->erasesize - 1) & CONFIG_ENV_OFFSET))
		printf("*** Warning - Env offset is NOT aligned to SPI Flash "
		       "block size, environment value is read only.\n\n");
#endif /* CONFIG_GENERIC_SF */

#if defined(CONFIG_GENERIC_NAND)
	if (((env_media == BOOT_MEDIA_NAND) || (env_media == BOOT_MEDIA_SPI_NAND)) &&
	    ((get_nand_info()->erasesize - 1) & CONFIG_ENV_OFFSET))
		printf("*** Warning - Env offset is NOT aligned to NAND Flash "
		       "block size, environment value is read only.\n\n");
#endif /* CONFIG_GENERIC_SF */
}

static void bootargs_prepare(void)
{
#ifdef CONFIG_BOOTARGS_512M
	{
		char *bootargs = getenv("bootargs_512M");
		if (!bootargs)
			setenv("bootargs_512M", CONFIG_BOOTARGS_512M);
	}
#endif /* CONFIG_BOOTARGS_512M */

#ifdef CONFIG_BOOTARGS_1G
	{
		char *bootargs = getenv("bootargs_1G");
		if (!bootargs)
			setenv("bootargs_1G", CONFIG_BOOTARGS_1G);
	}
#endif /* CONFIG_BOOTARGS_1G */

#ifdef CONFIG_BOOTARGS_2G
	{
		char *bootargs = getenv("bootargs_2G");
		if (!bootargs)
			setenv("bootargs_2G", CONFIG_BOOTARGS_2G);
	}
#endif /* CONFIG_BOOTARGS_2G */

//zbing add   5008 1G hardware ;
    char *bootargs = getenv("bootargs");
    if (bootargs)
    {
        char *p = strstr(bootargs, "mem=");
        if (p)
        {
            int mem = 0;
            int ddr = 0;
            int y = 0;
            int x = 1;
            for (; *p != 'M'; p++);

            for(p--; *p != '='; p--)
            {
                mem += x*(*p - '0'); 
                x *= 10;
            }
            y = ddr = get_ddr_size()/1024/1024;
            if (mem > ddr)
            {
                for (x /= 10, p++; *p != 'M'; p++)
                {
                    *p = y/x + '0';
                    y = y%x;
                    x /= 10;
                }
            }
            p = strstr(bootargs, "mmz=ddr,0,0,150M");
            if(p != NULL)
            {
                p[12] = '1';
                p[13] = '0';
                p[14] = '0';
                printf("Set mmz 150M -> 100M .... \n");
                run_command("saveenv", 0);
            }
            p = strstr(bootargs, "mmz=ddr,0,0,200M");
            if(p != NULL)
            {
                p[12] = '1';
                p[13] = '0';
                p[14] = '0';
                printf("Set mmz 200M -> 100M .... \n");
                run_command("saveenv", 0);
            }
        }
    }
}
//end

#ifndef CONFIG_BOOT_SIMULATE
#define step(x)
#endif

void start_armboot (void)
{
	init_fnc_t **init_fnc_ptr;
	char *s;
#if defined(CONFIG_VFD) || defined(CONFIG_LCD)
	unsigned long addr;
#endif
	step(81);

#ifdef CONFIG_ARCH_S5
	/* Initilize reg2 */
	init_reg2();
#endif

	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t*)(_armboot_start - CONFIG_BOOTHEAD_GAP
		- CONFIG_SYS_MALLOC_LEN - sizeof(gd_t));
	/* compiler optimization barrier needed for GCC >= 3.4 */
	__asm__ __volatile__("": : :"memory");

	step(82);

	memset ((void*)gd, 0, sizeof (gd_t));
	gd->bd = (bd_t*)((char*)gd - sizeof(bd_t));
	memset (gd->bd, 0, sizeof (bd_t));
	insert_ddr_layout((unsigned int)gd->bd,
		(unsigned int)((char *)gd + sizeof(gd_t)), "global data");

	gd->flags |= GD_FLG_RELOC;

	monitor_flash_len = _bss_start - _armboot_start;

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr)() != 0) {
			hang ();
		}
	}

	/* armboot_start is defined in the board-specific linker script */
	mem_malloc_init (_armboot_start - CONFIG_BOOTHEAD_GAP
		- CONFIG_SYS_MALLOC_LEN,
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
    hdmi_to_vga();

#if defined(CONFIG_CMD_NAND)
	nand_init();        /* go init the NAND */
#endif

#if defined(CONFIG_CMD_ONENAND)
	onenand_init();
#endif

#ifdef CONFIG_HAS_DATAFLASH
	AT91F_DataflashInit();
	dataflash_print_info();
#endif

#if defined(CONFIG_CMD_SF)
	puts("\n");
	spi_flash_probe(0, 0, 0, 0);
#endif

#ifdef CONFIG_GENERIC_MMC
	mmc_flash_init();
#endif

#if defined(CONFIG_CMD_NAND)
	nand_rr_param_init();
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

#ifdef 	CONFIG_MS_RANDOM_ETHADDR        
    //    autoset_mac_addr();
    write_ethaddr_to_mac();
#endif

	/* IP Address */
	gd->bd->bi_ip_addr = getenv_IPaddr ("ipaddr");

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

#ifdef HISFV_PHY_U
	U_PHY_ADDR = HISFV_PHY_U;
#endif /* HISFV_PHY_U */
	U_PHY_ADDR = get_eth_phyaddr(0, U_PHY_ADDR);

#ifdef HISFV_PHY_D
	D_PHY_ADDR = HISFV_PHY_D;
#endif /* HISFV_PHY_D */
	D_PHY_ADDR = get_eth_phyaddr(1, D_PHY_ADDR);

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
#endif /* CONFIG_DRIVER_SMC91111 || CONFIG_DRIVER_LAN91C96 */

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

#if defined(CONFIG_SHOW_MEMORY_LAYOUT)
	show_ddr_layout();
#endif /* CONFIG_SHOW_MEMORY_LAYOUT */

	reserve_mem_init();

	show_boot_env();

#ifdef CONFIG_PRODUCT_WITH_BOOT
	product_init();
#endif

#if defined(CONFIG_BOOTROM_SUPPORT) || defined(CONFIG_BOOTROM_CA_SUPPORT)
#ifndef CONFIG_SUPPORT_CA_RELEASE
	extern void download_boot(const int (*handle)(void));
	download_boot(NULL);
#endif
#endif

#ifdef CONFIG_DDR_TRAINING
	check_ddr_training();
#endif /* CONFIG_DDR_TRAINING */

	bootargs_prepare();

	printf("\n");

#ifdef CONFIG_PRODUCT_WITH_BOOT
	fastapp_entry(0, NULL);
#endif

#if defined(CONFIG_SHOW_RESERVE_MEM_LAYOUT)
	show_reserve_mem();
#endif

	HI_DRV_GPIO_WriteBit(STATE_LED, 0);
	
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

#if 0
/*
 * this is usb keyboard test code.
 */
wait_keybard_input(int c, int timeout)
{
	int jx = 0, ix = 0;
	int chr = -1;
	extern struct stdio_dev* stdio_get_by_name(char* name);
	struct stdio_dev* std_dev = NULL;

	usb_stop();
	printf("start usb ...\n");
	usb_init();
	drv_usb_kbd_init();
	std_dev =  stdio_get_by_name("usbkbd");
	if (std_dev) {
		while (ix < 20000) {
			if (jx++ > 100) {
				if (!(ix % 100))
					printf(" \rWait for keyboard CTRL press %d ...",
					(ix/100));
				jx = 0;
				ix++;
			}
			if (std_dev->tstc()) {
				chr = std_dev->getc();
				printf(" \b%c", chr);
				if (chr == 0)
					break;
			}
			udelay(10);
		}
		printf("\n");
	}
}

#endif

