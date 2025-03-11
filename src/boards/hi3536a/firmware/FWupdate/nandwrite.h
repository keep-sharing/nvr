#ifndef __NANDWRITE_H__
#define __NANDWRITE_H__

#include <errno.h>
#include <fcntl.h> 
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>
#include <mtd/mtd-user.h>
#include <mtd/mtd-abi.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "update_cmd.h"
#include "chipinfo.h"
#include "crc32.h"

/**
* Partitions on device.
*/    
/* ------------------------------------------------------------------------- */
enum 
{
    PART_BLD = 0,
    PART_ENV,
    PART_JPG,
    PART_KER1,
    PART_KER2,
    PART_FS1,
    PART_FS2,
    PART_CFG,
    PART_OEM,
    PART_RSD,
    PART_MAX,
};

#define PART_MAX_WITH_RSV	32
#define FLDEV_CMD_LINE_SIZE	1024

#define ETH_INSTANCES		1
#define USE_WIFI			1
#define USE_USBETH			0
#define CMD_LINE_SIZE		512
#define MAC_SIZE			6
#define SN_SIZE				32

#define USE_DOUBLE_ENV		1

typedef struct
{
	char mtd_dev[32];
	char mtd_name[32];
	unsigned int fw_start;
	unsigned int fw_len;
	unsigned int fw_flag;
	unsigned int fw_load_addr;
	unsigned int fw_version;
	unsigned int fw_date;
	unsigned int fw_pad;	// bld and hal need to use pad

}IMG_PTB_INFO_T;

typedef struct 
{
	char mtd_dev[32];
	char mtd_name[32];

	unsigned int mtd_size;
}MTD_INFO_T;

typedef struct
{
	MTD_INFO_T mtd_info[PART_MAX];
	
}IMG_MTD_INFO_T, *MTD_Handle;

typedef struct part_info_s
{
    char         name[32];
    unsigned int size;
    unsigned int version;
    unsigned int date;
    
}part_info_t;

typedef struct mtd_table_s
{
    part_info_t part[PART_MAX];
    char bootargs[512]; 
    char bootcmd[512]; 

}mtd_table_t;

int get_env_mtd_table(mtd_table_t *table);
int set_env_mtd_table(mtd_table_t *table);
int set_env_cmdline(mtd_table_t *table);
int set_cmdline_memory(int memsize);

// set boot state 0:fail 1:success
int set_boot_state(int state);

/*switch filesys */
int set_switch_filesys(int fs_num);

// get real part name
int get_part_name(int part_index, char* part_name);

/* get patition mtd information */
int get_mtd_info(MTD_Handle mtd);

// get cmdline
int get_ptb_cmdline( char cmdline[FLDEV_CMD_LINE_SIZE]);

/* display ptb information */
int display_info(void);

// set  sn  cmdline bootdelay
int set_ptb_sn(char *sn_data);
int set_ptb_cmdline( char cmdline[FLDEV_CMD_LINE_SIZE]); 
int set_ptb_bootdelay(uint8_t delay);

// set auto_boot auto_dl
int set_ptb_autoboot(uint8_t auto_boot);
int set_ptb_autodownload(uint8_t auto_dl);

// set pri_addr pri_file
int set_ptb_pri_addr(uint32_t pri_addr);
int set_ptb_pri_file(char pri_file[SN_SIZE]);

// set tftpd server ip 
int set_ptb_tftpd_ip(char* tftpd);

// set eth mac ip netmask gateway 
#if (ETH_INSTANCES >= 1)
int set_ptb_eth_mac(int eth_id, char* eth_mac);
int set_ptb_eth_ip(int eth_id, char* eth_ip);
int set_ptb_eth_netmask(int eth_id, char* eth_nm);
int set_ptb_eth_gateway(int eth_id, char* eth_gw);
int set_ptb_uboot_version(char * ver);

#endif /* ETH_INSTANCES >= 1 */

// set wifi mac ip netmask gateway 
#if (USE_WIFI >= 1)
int set_ptb_wifi_mac(int wifi_id, char* wifi_mac);
int set_ptb_wifi_ip(int wifi_id, char* wifi_ip);
int set_ptb_wifi_netmask(int wifi_id, char* wifi_nm);
int set_ptb_wifi_gateway(int wifi_id, char* wifi_gw);
#endif /* USE_WIFI >= 1 */ 

// set usb eth mac ip netmask gateway 
#if (USE_USBETH >= 1)
int set_ptb_usbeth_mac(int usbeth_id, char* usbeth_mac);
int set_ptb_usbeth_ip(int usbeth_id, char* usbeth_ip);
int set_ptb_usbeth_netmask(int usbeth_id, char* usbeth_nm);
int set_ptb_usbeth_gateway(int usbeth_id, char* usbeth_gw);
#endif /* USE_USBETH >= 1 */

int spi_flash_write_file(char* img_name, IMG_PTB_INFO_T *part_info);

// update nand flash imager
int nand_flash_write_file(char* , IMG_PTB_INFO_T *);
int nand_flash_write_fs_file(char* img_name, IMG_PTB_INFO_T *part_info);

// clean memory
int clean_memory_and_sync(void);

int check_is_8064();

#endif /* __NANDWRITE_H__ */
