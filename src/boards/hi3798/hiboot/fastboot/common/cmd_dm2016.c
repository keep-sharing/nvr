/**
 * Eric.milesight 2014.1.1
 * This file is a command for read/write dm2016
 */
#include <common.h>
#include <command.h>
#include <config.h>
#include <i2c.h>
#include "hi_drv_i2c.h"
#ifdef CONFIG_CMD_DM2016

#define CFG_I2C_DM2016_ID               1
#define CFG_I2C_DM2016_MAX_SIZE			128
#define CFG_I2C_DM2016_ADDR_LEN			1
#define CFG_DM2016_PAGE_WRITE_BITS		3	// 8 byte per page
#define CFG_DM2016_PAGE_WRITE_DELAY_MS 	10


extern void dm2016_init  (void);
extern int  dm2016_read  (unsigned dev_addr, unsigned offset, uchar *buffer, unsigned cnt);
extern int  dm2016_write (unsigned dev_addr, unsigned offset, uchar *buffer, unsigned cnt);

int dm2016_read(unsigned dev_addr, unsigned offset, uchar *buffer, unsigned cnt)
{
    return HI_DRV_I2C_Read(CFG_I2C_DM2016_ID, dev_addr, offset, CFG_I2C_DM2016_ADDR_LEN, buffer, cnt);
}

int dm2016_write(unsigned dev_addr, unsigned offset, uchar *buffer, unsigned cnt)
{
    int i;
    int ret = 0;
    for (i = 0; i < cnt; i++)
    {
        ret = HI_DRV_I2C_Write(CFG_I2C_DM2016_ID, dev_addr, offset+i, CFG_I2C_DM2016_ADDR_LEN, buffer, 1);
        if (ret != 0)
        {
            printf("dm2016_write @index[%d] failed!\n", i);
            return -1;
        }
        buffer++;
        udelay(12000);
    }
    
    return 0;
}

void dm2016_init  (void)
{
    HI_DRV_I2C_Init();
}

int do_dm2016( cmd_tbl_t *cmdtp, int flag, int argc, char*argv[])
{
	const char *const fmt = "DM2016 @0x%lX %s: values %s  offset %04lx  count %ld ... ";
	ulong dev_addr = CFG_I2C_DM2016_ADDR;
	uchar raddr[CFG_I2C_DM2016_MAX_SIZE] = {0};

	if( argc == 5 ) {	
		dm2016_init ();
		if (strcmp (argv[1], "write") == 0) {
		    ulong off  = simple_strtoul (argv[3], NULL, 16);
		    ulong cnt  = simple_strtoul (argv[4], NULL, 16);
			int rcode;
			ulong addr = 0;
			if( argv[2][0] == '0' && (argv[2][1] == 'x' || argv[2][1] == 'X') ) {
				addr = simple_strtoul (argv[2], NULL, 16);
				printf (fmt, dev_addr, argv[1], (char *)addr, off, cnt);
				rcode = dm2016_write (dev_addr, off, (uchar *)addr, cnt);
			} else {
				addr = (ulong)argv[2];
				if( strlen(argv[2]) < cnt )
					cnt = strlen(argv[2]);
				printf (fmt, dev_addr, argv[1], (uchar *)addr, off, cnt);
				rcode = dm2016_write (dev_addr, off, (uchar *)argv[2], cnt);
			}
				
			if( rcode !=0 ) {
				puts ("failed\n");
			} else {
			    rcode = dm2016_read (dev_addr, off, (uchar *) raddr, cnt);
                *((uchar *) raddr+cnt) = 0;
				puts ("done\n");
				printf("dm2016 read value: %s \n", (uchar*)raddr);	
			}
			return rcode;
		}
	}
	else
	{
		dm2016_init ();
        if (strcmp (argv[1], "read") == 0) {
            int rcode;
            
            ulong off  = simple_strtoul (argv[2], NULL, 16);
		    ulong cnt  = simple_strtoul (argv[3], NULL, 16);
            
            printf (fmt, dev_addr, argv[1], raddr, off, cnt);
            rcode = dm2016_read (dev_addr, off, (uchar *) raddr, cnt);
            *((uchar *) raddr+cnt) = 0;
            puts ("done\n");
            printf("dm2016 read value: %s \n", (uchar*)raddr);
            
            return rcode;
        }
	}
	
	printf ("Usage:\n%s\n", cmdtp->usage);
	return 1;

}

//U_BOOT_CMD(
//	mstype,	5, 0, do_dm2016,
//	"mstype  - MSTYPE sub-system\n\t- read  offset cnt\n\t- write value/address offset cnt\n",
//	"mstype read offset cnt\n"
//	"mstype write value/address offset cnt\n"
//	"       - read/write `cnt' bytes at DM2016 offset `offset'\n"
//);

#endif /* CONFIG_CMD_DM2016 */
