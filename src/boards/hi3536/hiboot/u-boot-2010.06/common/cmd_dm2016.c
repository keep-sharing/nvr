/**
 * Eric.milesight 2014.1.1
 * This file is a command for read/write dm2016
 */
#include <common.h>
#include <command.h>
#include <config.h>
#include <i2c.h>


#ifdef CONFIG_CMD_DM2016

extern void dm2016_init  (void);
extern int  dm2016_read  (unsigned dev_addr, unsigned offset, uchar *buffer, unsigned cnt);
extern int  dm2016_write (unsigned dev_addr, unsigned offset, uchar *buffer, unsigned cnt);

int dm2016_read(unsigned dev_addr, unsigned offset, uchar *buffer, unsigned cnt)
{
	unsigned end = offset + cnt;
	unsigned blk_off;
	int rcode = 0;

	/* Read data until done or would cross a page boundary.
	 * We must write the address again when changing pages
	 * because the next page may be in a different device.
	 */
	while (offset < end) {
		unsigned alen, len;
		
#if !defined(CFG_I2C_FRAM)
		unsigned maxlen;
#endif /*CFG_I2C_FRAM*/

#if CFG_I2C_DM2016_ADDR_LEN == 1 && !defined(CONFIG_SPI_X)
		uchar addr[2];

		blk_off = offset & 0xFF;	/* block offset */

		addr[0] = offset >> 8;		/* block number */
		addr[1] = blk_off;		/* block offset */
		alen	= 2;
#else /* CFG_I2C_EEPROM_ADDR_LEN, CONFIG_SPI_X */
		uchar addr[3];

		blk_off = offset & 0xFF;	/* block offset */

		addr[0] = offset >> 16; 	/* block number */
		addr[1] = offset >>  8; 	/* upper address octet */
		addr[2] = blk_off;		/* lower address octet */
		alen	= 3;
#endif	/* CFG_I2C_EEPROM_ADDR_LEN, CONFIG_SPI_X */

		addr[0] |= dev_addr;		/* insert device address */

		len = end - offset;

		/*
		 * For a FRAM device there is no limit on the number of the
		 * bytes that can be ccessed with the single read or write
		 * operation.
		 */
#if !defined(CFG_I2C_FRAM)
		maxlen = 0x100 - blk_off;
		if (maxlen > I2C_RXTX_LEN)
			maxlen = I2C_RXTX_LEN;
		if (len > maxlen)
			len = maxlen;
#endif /* CFG_I2C_FRAM */

#ifdef CONFIG_SPI
		spi_read (addr, alen, buffer, len);
#else	/* CONFIG_SPI */

		if (i2c_read (addr[0], offset, alen-1, buffer, len) != 0)
			rcode++;
#endif	/* CONFIG_SPI */

		buffer += len;
		offset += len;
	}

	return rcode;

}

int dm2016_write(unsigned dev_addr, unsigned offset, uchar *buffer, unsigned cnt)
{
	unsigned end = offset + cnt;
	unsigned blk_off;
	int rcode = 0;
	
	/* Write data until done or would cross a write page boundary.
	 * We must write the address again when changing pages
	 * because the address counter only increments within a page.
	 */

	/*
	 * Max offset is 127
	 */
	if( end >= CFG_I2C_DM2016_MAX_SIZE ) {
		printf("The offset is too large, offset + cnt  < %d \n", CFG_I2C_DM2016_MAX_SIZE);
		return -1;
	}

	while (offset < end) {
		unsigned alen, len;

#if CFG_I2C_DM2016_ADDR_LEN == 1 && !defined(CONFIG_SPI_X)
		uchar addr[2];

		blk_off = offset & 0xFF;	/* block offset */

		addr[0] = offset >> 8;	/* block number */
		addr[1] = blk_off;		/* block offset */
		alen	= 2;
#else
		uchar addr[3];

		blk_off = offset & 0xFF;	/* block offset */

		addr[0] = offset >> 16; 	/* block number */
		addr[1] = offset >>  8; 	/* upper address octet */
		addr[2] = blk_off;		/* lower address octet */
		alen	= 3;
#endif	/* CFG_I2C_EEPROM_ADDR_LEN, CONFIG_SPI_X */

		addr[0] |= dev_addr;	/* insert device address */

		len = 1;				/* data length */

#ifdef CONFIG_SPI
		spi_write (addr, alen, buffer, len);
#else /*CONFIG_SPI*/
		if ( i2c_write (addr[0], offset, alen-1, buffer, len ) != 0)
			rcode ++;

#endif /*CONFIG_SPI*/
		buffer += len;
		offset += len;

#if defined(CFG_DM2016_PAGE_WRITE_DELAY_MS)
		udelay(CFG_DM2016_PAGE_WRITE_DELAY_MS * 1000);
#endif /*CFG_EEPROM_PAGE_WRITE_DELAY_MS*/

	}

	return rcode;
}

#ifndef CONFIG_SPI
int dm2016_probe (unsigned dev_addr, unsigned offset)
{
	unsigned char chip;

	/* Probe the chip address
	 */
#if CFG_I2C_DM2016_ADDR_LEN == 1 && !defined(CONFIG_SPI_X)
	chip = offset >> 8;		/* block number */
#else
	chip = offset >> 16;		/* block number */
#endif	/* CFG_I2C_EEPROM_ADDR_LEN, CONFIG_SPI_X */

	chip |= dev_addr;		/* insert device address */

	return (i2c_probe (chip));
}
#endif


/*-----------------------------------------------------------------------
 * Set default values
 */
#ifndef	CONFIG_SYS_I2C_SPEED
#define	CONFIG_SYS_I2C_SPEED	50000
#endif

#ifndef	CONFIG_SYS_I2C_SLAVE
#define	CONFIG_SYS_I2C_SLAVE	0xFE
#endif

void dm2016_init  (void)
{

#if defined(CONFIG_SPI)
	spi_init_f ();
#endif

#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
	i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
#endif

}
/*-----------------------------------------------------------------------
 */


int do_dm2016( cmd_tbl_t *cmdtp, int flag, int argc, char*argv[])
{
	const char *const fmt1 = "DM2016 @0x%lX %s: address %08lx  offset %04lx  count %ld ... ";
	const char *const fmt2 = "DM2016 @0x%lX %s: values %s  offset %04lx  count %ld ... ";
	ulong dev_addr = CFG_I2C_DM2016_ADDR;
	uchar raddr[CFG_I2C_DM2016_MAX_SIZE] = {0};

	if( argc == 5 ) {	
#ifndef CONFIG_SPI
		dm2016_init ();
#endif /* !CONFIG_SPI */	
		if (strcmp (argv[1], "write") == 0) {
		    ulong off  = simple_strtoul (argv[3], NULL, 16);
		    ulong cnt  = simple_strtoul (argv[4], NULL, 16);
			int rcode;
			ulong addr = 0;
			if( argv[2][0] == '0' && (argv[2][1] == 'x' || argv[2][1] == 'X') ) {
				addr = simple_strtoul (argv[2], NULL, 16);
				printf (fmt1, dev_addr, argv[1], addr, off, cnt);
				rcode = dm2016_write (dev_addr, off, (uchar *)addr, cnt);
			} else {
				addr = (ulong)argv[2];
				if( strlen(argv[2]) < cnt )
					cnt = strlen(argv[2]);
				printf (fmt2, dev_addr, argv[1], (uchar *)addr, off, cnt);
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
	else if( argc == 4 )
	{
#ifndef CONFIG_SPI
		dm2016_init ();
#endif /* !CONFIG_SPI */
        if (strcmp (argv[1], "read") == 0) {
            int rcode;
            
            ulong off  = simple_strtoul (argv[2], NULL, 16);
		    ulong cnt  = simple_strtoul (argv[3], NULL, 16);
            
            printf (fmt1, dev_addr, argv[1], raddr, off, cnt);
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

/*
U_BOOT_CMD(
	mstype,	5, 0, do_dm2016,
	"mstype  - MSTYPE sub-system\n\t- read  offset cnt\n\t- write value/address offset cnt\n",
	"mstype read offset cnt\n"
	"mstype write value/address offset cnt\n"
	"       - read/write `cnt' bytes at DM2016 offset `offset'\n"
);
*/

#endif /* CONFIG_CMD_DM2016 */
