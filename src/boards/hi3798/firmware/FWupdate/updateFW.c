/**
 * @file updateFw.h
 *
 * Des:
 *	use for create/analysis update firmware
 * History:
 *	2014.8.16 - [Eric Zheng] created file
 *
 * Copyright (C) 2012-2014, Milesight, Inc.
 *
 */ 

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#include "updateFW.h"
#include "chipinfo.h"
#include "msoem.h"
#include "../../msgui/mssysconf.h"

//#define VERIFY_MD5

const char* img_file[UPFW_MAX] = 
{
	xstr(INPUT_BLD_IMG),
	xstr(INPUT_KER_IMG),
	xstr(INPUT_FS_IMG),
	xstr(INPUT_CMD_IMG),	
	xstr(INPUT_PARAM_IMG),
	xstr(INPUT_JPG_IMG),
	xstr(INPUT_BLD2_IMG),
	xstr(INPUT_TOOL_IMG),

};

const unsigned int ver_hex[UPFW_MAX] = 
{
	INPUT_BLD_VER,
	INPUT_KER_VER,
	INPUT_FS_VER,
	INPUT_CMD_VER,
    INPUT_PARAM_VER,
    INPUT_JPG_VER,
	INPUT_BLD2_VER,
	INPUT_TOOL_VER,

};

const unsigned int load_flag[UPFW_MAX] = 
{
	BLD_LOAD_FLAG,
	KER_LOAD_FLAG,
	FS_LOAD_FLAG,
	PART_NO_LOAD,
    PART_NO_LOAD,
    PART_NO_LOAD,
	BLD_LOAD_FLAG,
	PART_NO_LOAD,
};

const unsigned int load_addr[UPFW_MAX] = 
{
	BLD_LOAD_ADDR,
	KER_LOAD_ADDR,
	FS_LOAD_ADDR,
    NO_LOAD_ADDR,
    NO_LOAD_ADDR,
	NO_LOAD_ADDR,
	BLD_LOAD_ADDR,
	NO_LOAD_ADDR,
};


const char *img_name[UPFW_MAX] = 
{
	"bld", 
	"ker",
	"fs",
	"cmd",
	"param",
	"jpg",
	"bld2",
	"tool",
};

const char *out_upfw_name[UPFW_MAX] =
{
	ANALYSIS_BLD_NAME, 
	ANALYSIS_KER_NAME,
	ANALYSIS_FS_NAME,
	ANALYSIS_CMD_NAME,	
	ANALYSIS_PARAM_NAME,
	ANALYSIS_JPG_NAME,
	ANALYSIS_BLD2_NAME,
	ANALYSIS_TOOL_NAME,
};

/* get file size */
static unsigned long image_get_size(const char * pc_file_name);
/* get file md5 */
#ifdef VERIFY_MD5
static int image_get_md5(const char * pc_file_name, unsigned char md5_str[MD5_STR_LEN]);
/* check md5 if eq */
static int check_img_md5(unsigned char*, unsigned char*);
/* get file crc */
#else //VERIFY_CRC
static int image_get_crc(const char * pc_file_name, unsigned int *crcValue);
/* check crc32 if eq */
static int check_img_crc(unsigned int, unsigned int);
#endif
/* copy input datafile to output imager */
static int image_copy_data(int out_fd, const char* datafile, int pad);
/* get imager header info */
static int image_get_header_info(char* datafile, UPFW_header_handle pHeader);
/* write every image bin file*/
static int image_analysis_write(int ifd, const char* outfile, int ofst, int size);
	

/* get file size */
static 	unsigned long image_get_size(const char* pc_file_name)
{
	unsigned int filesize = -1;      
	struct stat statbuff; 
	
	if(stat(pc_file_name, &statbuff) < 0)
	{  
		return filesize;  
	}
	else
	{  
		filesize = statbuff.st_size;  
	}  
	return filesize;
}

/*pad file*/
static int image_pad_file(const char * pc_file_name)
{
	int in_fd;
	struct stat sbuf;
	int size;
	char pad[DEFAULT_PAGE_SIZE];
	int padsize = 0;

	if ((in_fd = open(pc_file_name, O_RDWR)) < 0)
	{
		PERR_RETURN(UPFW_FAILD, pc_file_name);
	}

	if (fstat(in_fd, &sbuf) < 0) 
	{
		PERR_CLOSE_RETURN(in_fd, UPFW_FAILD, "fstat pc_file_name");
	}

    if(sbuf.st_size%DEFAULT_PAGE_SIZE == 0)
    {
        close (in_fd);
        return UPFW_SUCCESS;
    }

    padsize = DEFAULT_PAGE_SIZE - sbuf.st_size%DEFAULT_PAGE_SIZE;

    lseek(in_fd, 0, SEEK_END);
    memset(pad, 0xff, sizeof(pad));
    size = write(in_fd, pad, padsize);
    if(size != padsize)
    {
        PERR_CLOSE_RETURN(in_fd, UPFW_FAILD, "write pc_file_name");
    }
    
	(void) close (in_fd);

	return UPFW_SUCCESS;

}

#ifdef VERIFY_MD5
/* get file md5 */
static int image_get_md5(const char * pc_file_name, unsigned char md5_str[MD5_STR_LEN])
{
	int dfd;
	int size = 0;
	MD5_CTX context;
	unsigned char read_buf[1024] = {0};
	unsigned char md5_des[16] = {0};
	
	if ((dfd = open(pc_file_name, O_RDONLY | O_BINARY)) < 0) 
	{
		PERR_RETURN(UPFW_FAILD, "open");
	}

	MD5Init (&context);
	while(1)
	{
		size = read(dfd, read_buf, sizeof(read_buf));
		if(size > 0) 
		{
			MD5Update (&context, read_buf, size);
		}
		else if( size == 0 )
		{
			break;
		}
		else
		{
			PERR_CLOSE_RETURN(dfd, UPFW_FAILD, "read datafile failed");
		}
	}
	MD5Final (md5_des, &context);
	
	sprintf((char*)md5_str,
		"%02x%02x%02x%02x"
		"%02x%02x%02x%02x"
		"%02x%02x%02x%02x"
		"%02x%02x%02x%02x",
		md5_des[0], md5_des[1], md5_des[2], md5_des[3],
		md5_des[4], md5_des[5], md5_des[6], md5_des[7],
		md5_des[8], md5_des[9], md5_des[10], md5_des[11],
		md5_des[12], md5_des[13], md5_des[14], md5_des[15]
	);
	md5_str[MD5_STR_LEN-1] = 0;
		
	(void) close (dfd);

	return UPFW_SUCCESS;
}

#else //crc
static int image_get_crc(const char * pc_file_name, unsigned int *crcValue)
{
    int dfd;
    int size = 0;
    unsigned char read_buf[1024] = {0};
    
    if ((dfd = open(pc_file_name, O_RDONLY | O_BINARY)) < 0) 
    {
        PERR_RETURN(UPFW_FAILD, "open");
    }

    while(1)
    {
        size = read(dfd, read_buf, sizeof(read_buf));
        if(size > 0) 
        {
            *crcValue = crc32(*crcValue, read_buf, size);
        }
        else if( size == 0 )
        {
            break;
        }
        else
        {
            PERR_CLOSE_RETURN(dfd, UPFW_FAILD, "read datafile failed");
        }
    }      
    (void) close (dfd);

    return UPFW_SUCCESS;
}
#endif

static int image_copy_data(int out_fd, const char* datafile, int pad)
{
	int in_fd;
	struct stat sbuf;
	unsigned char *ptr;
	int tail;
	int zero = 0;
	int offset = 0;
	int size;

	if ((in_fd = open(datafile, O_RDONLY | O_BINARY)) < 0)
	{
		PERR_RETURN(UPFW_FAILD, "open datafile");
	}

	if (fstat(in_fd, &sbuf) < 0) 
	{
		PERR_CLOSE_RETURN(in_fd, UPFW_FAILD, "fstat datafile");
	}

	ptr = mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, in_fd, 0);
	if (ptr == MAP_FAILED) 
	{
		PERR_CLOSE_RETURN(in_fd, UPFW_FAILD, "mmap datafile");
	}

	size = sbuf.st_size - offset;
	if (write(out_fd, ptr + offset, size) != size) 
	{
		(void) munmap((void *)ptr, sbuf.st_size);
		PERR_CLOSE_RETURN(in_fd, UPFW_FAILD, "write output imager");
	}

	if (pad && ((tail = size % 4) != 0)) 
	{
		if (write(out_fd, (char *)&zero, 4-tail) != 4-tail) 
		{
			(void) munmap((void *)ptr, sbuf.st_size);
			PERR_CLOSE_RETURN(in_fd, UPFW_FAILD, "write output imager");
		}
	}
	
	(void) munmap((void *)ptr, sbuf.st_size);
	(void) close (in_fd);

	return UPFW_SUCCESS;
	
}

int UPFW_list_intfo(char* datafile)
{
	UPFW_HEADER_T header;
	UPFW_header_handle headInfo = &header;	
	int i;

	if(datafile == NULL)
	{
		ERR_RETURN(UPFW_FAILD, "Please input datafile\n");
	}

	image_get_header_info(datafile, &header);
	
	debug("Image info: \n");
	for( i = 0; i < UPFW_MAX; i++)
	{
		if( strcmp((const char*)headInfo->img_head[i].fw_name, "") != 0 )
		{
			debug("%s: \n", headInfo->img_head[i].fw_name);
			if (headInfo->img_head[i].fw_being != 0 && 
				headInfo->img_head[i].fw_len != 0 )
			{
				debug("offset  = 0x%08X\n", headInfo->img_head[i].fw_being);
//				debug("size    = 0x%08X\n", headInfo->img_head[i].fw_len);
                debug("size    = %d K\n", headInfo->img_head[i].fw_len/1024);
				debug("addr    = 0x%08X\n", headInfo->img_head[i].fw_load_mem);
				debug("flag    = 0x%08X\n", headInfo->img_head[i].fw_load_flag);
				debug("date    = %d/%d/%d\n", 
					((headInfo->img_head[i].fw_date >> 16) & 0xffff),
					((headInfo->img_head[i].fw_date >> 8) & 0xff),
					((headInfo->img_head[i].fw_date) & 0xff)
					);
#ifdef VERIFY_MD5
				debug("md5     = %s\n", headInfo->img_head[i].fw_md5);
#else
				debug("crc32   = 0x%08X\n", headInfo->img_head[i].fw_crc);
#endif
			}
			debug("version = %d.%d.%d.%d\n",
				((headInfo->img_head[i].fw_version >> 24) & 0xff),
				((headInfo->img_head[i].fw_version >> 16) & 0xff),
				((headInfo->img_head[i].fw_version >> 8) & 0xff),
				(headInfo->img_head[i].fw_version & 0xff)
				);		
		}
	}

	return UPFW_SUCCESS;
}


/* check md5 if eq */
#ifdef VERIFY_MD5
static int check_img_md5(unsigned char old_md5[MD5_STR_LEN],
		unsigned char new_md5[MD5_STR_LEN])
{
	int i;
	for( i = 0; i < MD5_STR_LEN; i++)
	{
		if( old_md5[i] != new_md5[i] )
			return UPFW_FAILD;
	}
	
	return UPFW_SUCCESS;
}
#else //VERIFY_CRC
static int check_img_crc(unsigned int old_crc,
		unsigned int new_crc)
{
	if( old_crc != new_crc)
	{
		return UPFW_FAILD;
	}
	
	return UPFW_SUCCESS;
}
#endif

#ifdef BUILD_NANDWRITE
static int check_is_oem()
{
    if(access("/mnt/nand2/oem.db", F_OK))
        return 0;
    else
        return 1;
}

static int check_is_special_oem()
{
    if(access(UP_SPL_OEM_CFG, F_OK))
        return 0;
    else
        return 1;
}
static int get_current_fs_index(mtd_table_t *mtdtable)
{
    if (strstr(mtdtable->bootargs, "fs2"))
        return PART_FS2;
    else
        return PART_FS1;
}
#endif

int UPFW_creat_img()
{
	int i;
	int fd_out = -1;
	int img_offset = 0;
	UPFW_HEADER_T upfw_header;

	// get header base infomation
	img_offset =  IMG_HEADER_SIZE;
	memset(&upfw_header, 0, IMG_HEADER_SIZE);
	for( i = 0; i < UPFW_MAX && img_name[i] != NULL; i++)
	{
		// check if exist or make in
		if(img_file[i] != NULL && strcmp(img_file[i], "") != 0)
		{
		    if(image_pad_file(img_file[i]) != UPFW_SUCCESS)
		    {
		    	return UPFW_FAILD;
		    }

			upfw_header.img_head[i].fw_being = img_offset;
			upfw_header.img_head[i].fw_len = image_get_size(img_file[i]);
			img_offset += upfw_header.img_head[i].fw_len;
			upfw_header.img_head[i].fw_load_flag = load_flag[i];
			upfw_header.img_head[i].fw_load_mem= load_addr[i];
			upfw_header.img_head[i].fw_date = UPFW_VER_DATE;
#ifdef VERIFY_MD5
			image_get_md5(img_file[i], upfw_header.img_head[i].fw_md5);
#else
			image_get_crc(img_file[i], &upfw_header.img_head[i].fw_crc);
#endif
		}
		upfw_header.img_head[i].fw_version = ver_hex[i];	
		strcpy((char*)upfw_header.img_head[i].fw_name, img_name[i]);
	}

	//
    time_t t_time;
    time(&t_time);
    struct tm *tm_now;
    tm_now = localtime(&t_time);
    strftime(upfw_header.img_date, sizeof(upfw_header.img_date), "%Y-%m-%d %H:%M:%S", tm_now);

    snprintf(upfw_header.img_version, sizeof(upfw_header.img_version), MS_HI_NVR_VERSION);

	// create new image 
	fd_out = open(OUTPUT_UPFW_NAME, O_RDWR | O_CREAT | O_TRUNC | O_BINARY, 0666);
	if( fd_out < 0)
	{
		PERR_RETURN(UPFW_FAILD, "open output imager "OUTPUT_UPFW_NAME);
	}

	/*
	 * Must be -w then:
	 *
	 * write dummy header, to be fixed later
	 */
	if (write(fd_out, &upfw_header,  IMG_HEADER_SIZE ) != IMG_HEADER_SIZE)
	{
		PERR_CLOSE_RETURN(fd_out, UPFW_FAILD, "write output imager");
	}	

	// write data file
	debug("Copy data to image!!!\n");
	for( i = 0; i < UPFW_MAX && img_file[i] != NULL; i++)
	{
		// check if exist or make in
		if( strcmp(img_file[i], "") != 0)
		{
			if(image_copy_data(fd_out, img_file[i], 0) != UPFW_SUCCESS)
			{
			    fprintf(stderr, "   Copy %s: %s failed!\n", img_name[i], img_file[i]);
				ERR_CLOSE_RETURN(fd_out, UPFW_FAILD, "image_copy_data faild.\n");
			}
			/* We're a bit of paranoid */
			(void) fsync (fd_out);	
		}
	}

	if (close(fd_out))
	{
		ERR_RETURN(UPFW_FAILD, "close output imager\n");
	}

	return UPFW_SUCCESS;	
	
}

static int image_get_header_info(char* datafile, UPFW_header_handle pHeader)
{
	int fd_out;
	struct stat sbuf;

	if(datafile == NULL)
	{
		ERR_RETURN(UPFW_FAILD, "Please input datafile\n");
	}

	fd_out = open(datafile, O_RDONLY | O_BINARY);
	if (fd_out < 0) 
	{
		PERR_RETURN(UPFW_FAILD, "open output imager");
	}

	/*
	 * list header information of existing image
	 */
	if (fstat(fd_out, &sbuf) < 0) 
	{
		PERR_CLOSE_RETURN(fd_out, UPFW_FAILD, "fstat output imager");
	}

	if ((unsigned)sbuf.st_size < IMG_HEADER_SIZE) 
	{
		fprintf (stderr, "Bad size: \"%s\" is no valid image\n", OUTPUT_UPFW_NAME);
		close(fd_out);
		return UPFW_FAILD;
	}

	if (read(fd_out, pHeader,  IMG_HEADER_SIZE ) != IMG_HEADER_SIZE)
	{
		PERR_CLOSE_RETURN(fd_out, UPFW_FAILD, "read output imager");
	}	

	return UPFW_SUCCESS;	
}

#ifdef BUILD_NANDWRITE
typedef struct hiI2C_DATA_COMPAT_S
{
    unsigned int    	I2cNum;
    unsigned char     I2cDevAddr;
    unsigned int    	I2cRegAddr;
    unsigned int     	I2cRegCount;
    unsigned char    	*pData;
    unsigned int     	DataLen;
} I2C_DATA_COMPAT_S;

#define CMD_I2C_READ _IOW(0x53, 0x1,unsigned int)

#define READ(_a)     ( *((unsigned long*)(_a)) )
#define WRITE(_a, _v)   (*(volatile unsigned long *)(_a) = (_v))

#define ALLOC_SIZE 1024
static int g_fd = -1;
static void *map_base = NULL;

#define MDIO_BASE 0xF9841000
#define MDIO_DATA (map_base + 0X03C4)
#define MDIO_ADDR (map_base + 0X03C0)
#define MDIO_STAT (map_base + 0X03D0)
#define MII_PHY   0x04

#define test_mdio_ready(stat) ({ \
				unsigned long _bits_desc = 0x140001; \
				unsigned long _shift = (_bits_desc) >> 16; \
				unsigned long _mask = ((_bits_desc & 0x3F) < 32) ? (((1 << (_bits_desc & 0x3F)) - 1) << (_shift)) : 0xffffffff; \
				(READ(stat) & _mask) >> (_shift); })

#define test_mdio_read_data_done(stat) ({ \
			unsigned long _bits_desc = 0x1; \
			unsigned long _shift = (_bits_desc) >> 16; \
			unsigned long _mask = ((_bits_desc & 0x3F) < 32) ? (((1 << (_bits_desc & 0x3F)) - 1) << (_shift)) : 0xffffffff; \
            (READ(stat) & _mask) >> (_shift); })

static int wait_mdio_ready()
{
	int timeout_us = 1000;

	while (--timeout_us && !test_mdio_ready(MDIO_ADDR) == 0)
		usleep(10);

	return timeout_us;
}

static void mdio_start_phyread(unsigned char phy,unsigned char reg)
{
	WRITE(MDIO_ADDR, 0);
	WRITE(MDIO_ADDR, (phy<<8)|(reg<<0)|(0x2<<16)|(0x01<<20));
}

static int mdio_read(int phyaddr, int phyreg)
{
	int data = 0;
	int timeout = 100;
	
	if (!wait_mdio_ready())
		return -1;
	mdio_start_phyread(phyaddr, phyreg);
	while (!wait_mdio_ready() && timeout-- > 0)
	{
        usleep(5*1000);
	}
	if (timeout <= 0 || !test_mdio_read_data_done(MDIO_STAT)==0)
	{
        data = 0;
    	/* it should return Error(-1), but miiphy_read() will
        	 * print error info, it's annoying
        	 */
    	return data;
	}
		
	data = READ(MDIO_DATA)>>16;
	return data;
}

int get_poe_id(void)
{
	unsigned char data = 0;
	struct hiI2C_DATA_COMPAT_S i2c_info ;
	memset(&i2c_info,0,sizeof(struct hiI2C_DATA_COMPAT_S));
	i2c_info.I2cNum = 2;
	i2c_info.I2cDevAddr = 0x40;
	i2c_info.I2cRegAddr = 0x43;
	i2c_info.I2cRegCount = 1;
	i2c_info.DataLen = 1;
	i2c_info.pData = &data;
	int fd = open("/dev/hi_i2c",O_RDWR);
	if(fd<0)
		printf("open file fail");
	ioctl(fd,CMD_I2C_READ,&i2c_info);
	close(fd);
	return *i2c_info.pData;
}

/****************************************************************************************************/

static int phy_map()
{
    if (g_fd < 0)
    {
        g_fd = open("/dev/mem", O_RDWR | O_SYNC);  
        if (g_fd < 0) 
        {  
            printf("Fail to open /dev/mem fd=%08x\n", g_fd);
            return -1;
        }
    }
    map_base = mmap(0, ALLOC_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, g_fd, MDIO_BASE);
    if (!map_base)
    {
        printf("mmap phyaddr %#x failed ! \n", MDIO_BASE);
        close(g_fd);
        g_fd = -1;
        return -1;
    }

    return 0;
}

static void phy_umap()
{
    if (map_base)
        munmap(map_base, ALLOC_SIZE);

    if (g_fd > 0)
        close(g_fd);

    map_base = NULL;
    g_fd = -1;
}

static int rtl8211_read()
{
	if (phy_map() == -1)
    {
        return -1;
    }
    int data = mdio_read(0x4,0x2);
    phy_umap();
    return data;
}

#endif

int UPFW_verify(char* datafile, mtd_table_t *mtdtable)
{
	int fd_out = 0;
	UPFW_HEADER_T headInfo;;	
	struct stat sbuf;
	unsigned char read_buf[1024] = {0};
	int i, cnt, read_size;
    unsigned int new_crc;
    
	if(datafile == NULL)
	{
		ERR_RETURN(UPFW_FAILD, "Please input datafile\n");
	}

	fd_out = open(datafile, O_RDONLY | O_BINARY);
	if (fd_out < 0) 
	{
		PERR_RETURN(UPFW_FAILD, "open output imager");
	}

	/*
	 * list header information of existing image
	 */
	if (fstat(fd_out, &sbuf) < 0) 
	{
		PERR_CLOSE_RETURN(fd_out, UPFW_FAILD, "fstat output imager");
	}

	if ((unsigned)sbuf.st_size < IMG_HEADER_SIZE) 
	{
		fprintf (stderr, "Bad size: \"%s\" is no valid image\n", OUTPUT_UPFW_NAME);
		close(fd_out);
		return UPFW_FAILD;
	}

	memset(&headInfo, 0, IMG_HEADER_SIZE);
	if (read(fd_out, &headInfo,  IMG_HEADER_SIZE ) != IMG_HEADER_SIZE)
	{
		PERR_CLOSE_RETURN(fd_out, UPFW_FAILD, "read output imager");
	}

	if (mtdtable)
	{
#if 0//remove these code to UPFW_version_check();

		// for update
    	int curNo = 0;
        int cur_fsPart = 0;
        int fwNo =0;
        int fw_subNo = 0;
    	int fwoem_No = 0 /*, curoem_No = 0*/;
        int data = 0;
    	unsigned char poe_id = 0;
    	char cmd[64] = {0};
    	
	    cur_fsPart = get_current_fs_index(mtdtable);
        if ((((headInfo.img_head[UPFW_BLD].fw_version >> 24) & 0xff) != ((mtdtable->part[PART_BLD].version >> 24) & 0xff))
            && (((headInfo.img_head[UPFW_BLD2].fw_version >> 24) & 0xff) != ((mtdtable->part[PART_BLD].version >> 24) & 0xff)))
            {
                debug("platform dismatch !\n");
                return UPFW_FAILD;
            }        
		data = rtl8211_read();
		poe_id = get_poe_id();
		fwNo = (headInfo.img_head[UPFW_FS].fw_version >> 16) & 0xff;
		fw_subNo = headInfo.img_head[UPFW_FS].fw_version & 0xff;
		fwoem_No = (headInfo.img_head[UPFW_FS].fw_version >> 8) & 0xff;
		curNo = (mtdtable->part[cur_fsPart].version >> 16) & 0xff;
		if(fwNo <= 7 && curNo <= 7 && check_is_oem()){
			//for special update
			if(fwoem_No == 0 && check_is_special_oem())
			{
				debug("OEM special do not support update 7.0.x \n");
	       		return UPFW_FAILD;
			}	
		}
		
        if (fwNo >= 8 && curNo < 8 && check_is_oem())
        {
        	if(!check_is_special_oem())
			{
	            debug("OEM do not support update 7.x to 8.x \n");
	            return UPFW_FAILD;
        	}
        }

        if (fwNo < 8 && curNo >= 8 && check_is_oem())
        {
			debug("OEM do not support update 8.x to 7.x \n");
        	return UPFW_FAILD;
        }

		if (fwNo >= 9 && curNo < 9 && check_is_oem())
        {
            debug("OEM do not support update 8.x to 9.x \n");
            return UPFW_FAILD;
        }
        else if(fwNo < 8 && fw_subNo < 9 && data!=0x4d)
        {
        	debug("OEM do not support update < 7.x.9 \n");
        	return UPFW_FAILD;
        }
		else if(fwNo < 9 && curNo >=9)
		{
			debug("do not support back to < 9.0.1 form 9.0.x. \n");
			return UPFW_FAILD;
		}

		if(fwNo < 8 && fw_subNo < 10 && (poe_id == 0x44 || poe_id == 0xe3))
		{
			debug("do not support update < 8.x.10 0x%x\n", poe_id);
			return UPFW_FAILD;//not update
		}
		
		if (fwNo >= 8 && curNo < 8)
        {
            snprintf(cmd, sizeof(cmd), "touch %s", "/mnt/nand/updb.cfg");
			system(cmd);
        }
#endif
	}      
    
	for( i = 0; i < UPFW_MAX; i++)
	{
		if( strcmp((const char*)headInfo.img_head[i].fw_name, "") != 0 &&
			headInfo.img_head[i].fw_being != 0 &&
			headInfo.img_head[i].fw_len != 0)
		{
			debug("%s: \n", headInfo.img_head[i].fw_name);
			if (headInfo.img_head[i].fw_being != 0 && 
				headInfo.img_head[i].fw_len != 0 )
			{
				debug("offset  = 0x%08X\n", headInfo.img_head[i].fw_being);
				debug("size    = 0x%08X (%d K)\n", headInfo.img_head[i].fw_len, 
							headInfo.img_head[i].fw_len/1024);
				debug("addr    = 0x%08X\n", headInfo.img_head[i].fw_load_mem);
				debug("flag    = 0x%08X\n", headInfo.img_head[i].fw_load_flag);
				debug("date    = %d/%d/%d\n", 
					((headInfo.img_head[i].fw_date >> 16) & 0xffff),
					((headInfo.img_head[i].fw_date >> 8) & 0xff),
					((headInfo.img_head[i].fw_date) & 0xff)
					);
				debug("crc32   = 0x%08X\n", headInfo.img_head[i].fw_crc);
			}
			debug("version = %d.%d.%d.%d\n",
					((headInfo.img_head[i].fw_version >> 24) & 0xff),
					((headInfo.img_head[i].fw_version >> 16) & 0xff),
					((headInfo.img_head[i].fw_version >> 8) & 0xff),
					(headInfo.img_head[i].fw_version & 0xff)
					);
					
			read_size = 0;
			new_crc = 0;
			if(lseek( fd_out, headInfo.img_head[i].fw_being, SEEK_SET) == -1)
			{
				PERR_CLOSE_RETURN(fd_out, UPFW_FAILD, "lseek output imager");
			}
			while(read_size < headInfo.img_head[i].fw_len)
			{
				if(read_size + sizeof(read_buf) <=  headInfo.img_head[i].fw_len)
				{
					cnt = read(fd_out, read_buf, sizeof(read_buf));
					if(cnt != sizeof(read_buf)) 
					{
						PERR_CLOSE_RETURN(fd_out, UPFW_FAILD, "read output imager");
					}
				}
				else
				{
					cnt = read(fd_out, read_buf, headInfo.img_head[i].fw_len - read_size);
					if(cnt != headInfo.img_head[i].fw_len - read_size) 
					{
						PERR_CLOSE_RETURN(fd_out, UPFW_FAILD, "read output imager");
					}
				}
				new_crc = crc32(new_crc, read_buf, cnt);
				read_size += cnt;
			}
			if(check_img_crc(headInfo.img_head[i].fw_crc, new_crc) != UPFW_SUCCESS)
			{
				fprintf (stderr, "check_img_crc %s failed\n", headInfo.img_head[i].fw_name);
				close(fd_out);
				return UPFW_FAILD;
			}
		}
	}

	close (fd_out);

	return UPFW_SUCCESS;

}

static int image_analysis_write(int ifd, const char* outfile, int ofst, int size)
{
	int ofd;
	int w_cnt, r_cnt, read_size = 0;
	char read_buf[1024];

	if(outfile == NULL)
	{
		ERR_RETURN(UPFW_FAILD, "Please check outfile\n");
	}	

	ofd = open(outfile, O_CREAT | O_RDWR | O_BINARY, 0666);
	if (ofd < 0)
	{
		PERR_RETURN(UPFW_FAILD, "open output imager");
	}
	
	if(lseek( ifd, ofst, SEEK_SET) == -1)
	{
		PERR_CLOSE_RETURN(ofd, UPFW_FAILD, "lseek input imager");
	}
	
	while(read_size < size)
	{
		if(read_size + sizeof(read_buf) <= size)
		{
			r_cnt = read(ifd, read_buf, sizeof(read_buf));
		}
		else
		{
			r_cnt = read(ifd, read_buf, size - read_size);
		}
		if(r_cnt < 0)
		{
			PERR_CLOSE_RETURN(ofd, UPFW_FAILD, "read input imager");
		}
		else if(r_cnt == 0)
		{
			break;
		}
		else
		{
			w_cnt = write(ofd, read_buf, r_cnt);
			if(w_cnt != r_cnt)
			{
				PERR_CLOSE_RETURN(ofd, UPFW_FAILD, "write output imager");
			}
			read_size += r_cnt;
		}
	}

	close(ofd);

	return UPFW_SUCCESS;
}

int UPFW_analysis_img( char* datafile , int b_analysis_all)
{
	int fd_out = 0;
	UPFW_HEADER_T headInfo;
	struct stat sbuf;
	int ret = UPFW_SUCCESS, i;
	char out_name[128];
	int bcmd = 0;

	if(datafile == NULL)
	{
		ERR_RETURN(UPFW_FAILD, "Please input datafile\n");
	}
    printf("==============name %s=\n",datafile);
	fd_out = open(datafile, O_RDONLY | O_BINARY);
	if (fd_out < 0) 
	{
		PERR_RETURN(UPFW_FAILD, "open output imager");
	}

	/*
	 * list header information of existing image
	 */
	if (fstat(fd_out, &sbuf) < 0) 
	{
		PERR_CLOSE_RETURN(fd_out, UPFW_FAILD, "fstat output imager");
	}

	if ((unsigned)sbuf.st_size < IMG_HEADER_SIZE) 
	{
		fprintf (stderr, "Bad size: \"%s\" is no valid image\n", OUTPUT_UPFW_NAME);
		ret = UPFW_FAILD;
		goto analysis_exit;
	}

	memset(&headInfo, 0, IMG_HEADER_SIZE);
	if (read(fd_out, &headInfo,  IMG_HEADER_SIZE ) != IMG_HEADER_SIZE)
	{
		perror("read output imager");
		ret = UPFW_FAILD;
		goto analysis_exit;
	}

	for( i = 0; i < UPFW_MAX; i++)
	{
		memset(out_name, 0, sizeof(out_name));
		if(headInfo.img_head[i].fw_being != 0 && 
			headInfo.img_head[i].fw_len != 0 )
		{
			if(!b_analysis_all)
			{
				if(strcmp((const char*)headInfo.img_head[i].fw_name, 
						img_name[UPFW_CMD]) == 0)
				{
					strcpy(out_name, out_upfw_name[UPFW_CMD]);
					bcmd = 1;
				}
			}
			else
			{
				if( i <= UPFW_CMD &&  out_upfw_name[i] != NULL)
					strcpy(out_name, out_upfw_name[i]);
			}
			
			if( strlen(out_name) != 0 )
			{
				debug("Create file: %s \n", out_name);
				ret = image_analysis_write( fd_out, out_name,
						headInfo.img_head[i].fw_being,
						headInfo.img_head[i].fw_len
					);
				if( ret != UPFW_SUCCESS)
				{
					fprintf (stderr, "write %s faild\n", out_name);
					goto analysis_exit;
				}
				
				if( bcmd )
				{
					char cmd[256];
					sprintf(cmd, "chmod 777 %s", out_name);
					system(cmd);
					bcmd = 0;					
				}
			}
		}
	}

analysis_exit:

	close (fd_out);
	return ret;
}


#ifdef BUILD_NANDWRITE

static unsigned char b_exist_dm2016(void )
{
	int fd = -1;
	unsigned char b_exist = 0;
	
	fd = ms_dm2016_open();
	if (fd < 0)
	{
		b_exist = 0;
	}
	else
	{
		if (ms_dm2016_eeprom_read(fd, 0, 1, &b_exist) < 0)
			b_exist = 0;
		else
			b_exist = 1;
		ms_dm2016_close(fd);
	}

	return b_exist;
}

static void  get_old_fs_version(int *version)
{
	 mtd_table_t table;
	 int i;
	 
	 get_env_mtd_table(&table);
	 if (strstr(table.bootargs, "ubi.mtd=fs1") != NULL)
		i = PART_FS1;
	 else
		i = PART_FS2;

	  version[0] = (table.part[i].version >> 24) & 0xff;
	  version[1] = (table.part[i].version  >> 16) & 0xff;
	  version[2] = (table.part[i].version  >> 8) & 0xff;
	  version[3] = (table.part[i].version  ) & 0xff;
}

int env_add_msauthcheck()
{
	int version[4] = {0};
	unsigned char dm2016_detected = 0;
	
	get_old_fs_version(version);
	if (version[1] == 8 && version[3] >= 2)
	{
		dm2016_detected = b_exist_dm2016();
		if (!dm2016_detected)
		{
			hs_setenv(MSAUTHCHECK, MSAUTHCHECK_VALUE);
		}
	}
	
	return 0;
}

static int check_is_5016(void)
{
	FILE * pfd = NULL;
	char buff[64];
	int result = 0;

	pfd = popen("mschip_update", "r");
	if(pfd != NULL)
	{
		if(fgets(buff, sizeof(buff), pfd))
		{
			if(strstr(buff, "MSN5016"))
				result = 1;
			else
				result = 0;
		}
	}
	pclose(pfd);
	return result;
}

static int check_is_5016_1GB(void)
{
	int ver_ddr;
	mtd_table_t mtd_table;
	
	if(!check_is_5016())
		return 0;
	
	get_env_mtd_table(&mtd_table);
	ver_ddr = (mtd_table.part[PART_BLD].version >> 8) & 0xff;
	if(ver_ddr == 1)
		return 1;
	else
		return 0;
}


int UPFW_version_check(char* datafile)
{
    //zbing cut from UPFW_verify 2018.07.25
    int fd = -1;
	UPFW_HEADER_T headInfo;
	unsigned char dm2016_detected = 0;
    int curNo = 0;
    int cur_fsPart = 0;
    int fwNo =0;
    int fw_subNo = 0;
    int fwoem_No = 0 /*, curoem_No = 0*/;
    int data = 0;
    unsigned char poe_id = 0;
    char cmd[64] = {0};
    mtd_table_t mtdtable;
    
    //** read version to judge if update **//
	fd = open(datafile, O_RDONLY | O_BINARY);
	if (fd < 0)
		PERR_RETURN(UPFW_FAILD, "open output imager");
	
	memset(&headInfo, 0, IMG_HEADER_SIZE);
	if (read(fd, &headInfo, IMG_HEADER_SIZE) != IMG_HEADER_SIZE)
	{
		close(fd);
		PERR_RETURN(UPFW_FAILD, "read output imager");
	}
    close(fd);

	if (get_env_mtd_table(&mtdtable) < 0)
	{
		ERR_RETURN(UPFW_UPDATE_PTB_ERR, "flprog_get_part_table faild!\n");
	}
    cur_fsPart = get_current_fs_index(&mtdtable);
    fwNo = (headInfo.img_head[UPFW_FS].fw_version >> 16) & 0xff;
    fw_subNo = headInfo.img_head[UPFW_FS].fw_version & 0xff;
    fwoem_No = (headInfo.img_head[UPFW_FS].fw_version >> 8) & 0xff;
    curNo = (mtdtable.part[cur_fsPart].version >> 16) & 0xff;
	debug("version = %d.%d.%d\n", fwNo, fwoem_No, fw_subNo);


    // platform check
    if ((((headInfo.img_head[UPFW_BLD].fw_version >> 24) & 0xff) != ((mtdtable.part[PART_BLD].version >> 24) & 0xff))
        && (((headInfo.img_head[UPFW_BLD2].fw_version >> 24) & 0xff) != ((mtdtable.part[PART_BLD].version >> 24) & 0xff)))
        {
            debug("platform dismatch !\n");
            return UPFW_FAILD;
        }

    //mschip check
	dm2016_detected = b_exist_dm2016(); //goli modify
  	if (!dm2016_detected && ((fw_subNo < 2 && fwNo == 8) || fwNo < 8))
	{
		debug("Can't find dm2016, version must >= XX.8.0.2.");
		PERR_RETURN(UPFW_FAILD, "Can't find dm2016, version must >= XX.8.0.2.");
	}
	
    data = rtl8211_read();
    poe_id = get_poe_id(); 
    
    //version check
    if(fwNo <= 7 && curNo <= 7 && check_is_oem()){
        //for special update
        if(fwoem_No == 0 && check_is_special_oem())
        {
            debug("OEM special do not support update 7.0.x \n");
            return UPFW_FAILD;
        }   
    }
    else if (fwNo >= 8 && curNo < 8 && check_is_oem())
    {
        if(!check_is_special_oem())
        {
            debug("OEM do not support update 7.x to 8.x \n");
            return UPFW_FAILD;
        }
    }
    else if(fwNo < 8 && curNo >= 8 && check_is_oem())
    {
        debug("OEM do not support update 8.x to 7.x \n");
        return UPFW_FAILD;
    }
    else if (fwNo >= 9 && curNo < 9 && check_is_oem())
    {
        if(!check_is_special_oem())
        {
            debug("OEM do not support update 8.x to 9.x \n");
            return UPFW_FAILD;
        }
    }
    else if(fwNo < 8 && fw_subNo < 9 && data!=0x4d)
    {
        debug("OEM do not support update < 7.x.9 \n");
        return UPFW_FAILD;
    }
    else if(fwNo < 9 && curNo >=9)
    {
        debug("do not support back to < 9.0.1 form 9.0.x. \n");
        return UPFW_FAILD;
    }
    else if(fwNo < 8 && fw_subNo < 10 && (poe_id == 0x44 || poe_id == 0xe3))
    {
        debug("do not support update < 8.x.10 0x%x\n", poe_id);
        return UPFW_FAILD;//not update
    }
	else if(check_is_5016_1GB() && fwNo == 9 && fw_subNo < 10)
	{
		debug("new 5016 do not support update < 9.x.10 \n");
        return UPFW_FAILD;
	}
    else if (fwNo >= 8 && curNo < 8)
    {
        snprintf(cmd, sizeof(cmd), "touch %s", "/mnt/nand/updb.cfg");
        system(cmd);
    }
    
	return UPFW_SUCCESS;
    //zbing cut from UPFW_verify 2018.07.25
}

int UPFW_run_update( char* datafile )
{
	UPFW_HEADER_T header_info;
	mtd_table_t mtd_table;
	IMG_MTD_INFO_T mtd;
	IMG_PTB_INFO_T ptb_info[UPFW_MAX];
	int i;
	int cureent_fs = 1;
	char *cmdline;
	char firmware_flag[UPFW_MAX] = {0};

	// get image header information
	if(image_get_header_info(datafile, &header_info) != UPFW_SUCCESS)
	{
		ERR_RETURN(UPFW_FAILD, "image_get_header_info faild!\n");
	}
	
	// get ptb header information
	debug("Get old ptb information!\n");
//	display_info();
	if (get_env_mtd_table(&mtd_table) < 0)
	{
		ERR_RETURN(UPFW_UPDATE_PTB_ERR, "flprog_get_part_table faild!\n");
	}

	debug("Verify image %s \n", datafile);
	if(UPFW_verify(datafile, &mtd_table) != 0)
	{
		ERR_RETURN(UPFW_VERIFY_FAILD, "Verify update firmware faild!\n");
	}

	//check ddr memory type legality add by zbing 2015.8.10
	// get mtd information
	debug("Get mtd device information!\n");
	if(get_mtd_info(&mtd) != UPFW_SUCCESS)
	{
		ERR_RETURN(UPFW_UPDATE_PTB_ERR, "get_mtd_info faild!\n");
	}

	cmdline = strstr(mtd_table.bootargs, "=fs");
	if( cmdline != NULL )
    {
        if(*(cmdline+3) == '2')
        {
            cureent_fs = 2; // current run filesys is fs1
            *(cmdline+3) = '1';
            debug("  current is fs2 \n");
        }
        else if(*(cmdline+3) == '1')
        {
            cureent_fs = 1; // current run filesys is fs1
            *(cmdline+3) = '2';
            debug("  current is fs1 \n");
        }
        else
        {
            ERR_RETURN(UPFW_UPDATE_PTB_ERR, "unknow fs partition!\n");
        }
    }
    else
    {
//        cureent_fs = 1; // current run filesys is fs1
//        *(cmdline+3) = '2';
//        debug("debug  current is fs1 \n");
		fprintf(stderr, "cmdline: %s \n", mtd_table.bootargs);
        ERR_RETURN(UPFW_UPDATE_PTB_ERR, "bootargs not find fs, maybe NFS!\n");
    }

	cmdline = strstr(mtd_table.bootcmd, "ker");
	if( cmdline != NULL )
	{
		if(*(cmdline+3) == '2')
		{
			*(cmdline+3) = '1' ;
			debug("  current is ker2 \n");
		}
		else if(*(cmdline+3) == '1' )
		{
			*(cmdline+3) = '2' ;
			debug("  current is ker1 \n");
		}
		else
		{
            ERR_RETURN(UPFW_UPDATE_PTB_ERR, "  unknow bootcmd!\n");
		}
	}
	else
	{
        ERR_RETURN(UPFW_UPDATE_PTB_ERR, " bootcmd is null!\n");
	}	
	

	memset(&ptb_info, 0, sizeof(ptb_info));
	
	// check amboot version legality
	// 2GB
	if((header_info.img_head[UPFW_BLD].fw_len > 0) 
		&& (header_info.img_head[UPFW_BLD].fw_being > 0)
		&& (((header_info.img_head[UPFW_BLD].fw_version >> 8) & 0xff) == 
	    	((mtd_table.part[PART_BLD].version >> 8) & 0xff))
#ifndef BLD_UPDATE_ALWAYS	    	
		&& (header_info.img_head[UPFW_BLD].fw_version > 
			mtd_table.part[PART_BLD].version)
#endif /*BLD_UPDATE_ALWAYS*/			
	){	
		firmware_flag[UPFW_BLD] = 1;
		ptb_info[UPFW_BLD].fw_pad = 1;
		// update base infomation( part name and mtd device)
		strcpy(ptb_info[UPFW_BLD].mtd_name, mtd_table.part[PART_BLD].name);
		strcpy(ptb_info[UPFW_BLD].mtd_dev, mtd.mtd_info[PART_BLD].mtd_dev);
	}
	
#ifdef INPUT_BLD2_IMG	
	// 4GB
	if((header_info.img_head[UPFW_BLD2].fw_len > 0) 
		&& (header_info.img_head[UPFW_BLD2].fw_being > 0)
		&& (((header_info.img_head[UPFW_BLD2].fw_version >> 8) & 0xff) == 
	    	((mtd_table.part[PART_BLD].version >> 8) & 0xff))	    	
		&& (header_info.img_head[UPFW_BLD2].fw_version > 
			mtd_table.part[PART_BLD].version)
	){	
		firmware_flag[UPFW_BLD2] = 1;
		ptb_info[UPFW_BLD2].fw_pad = 1;
		// update base infomation( part name and mtd device)
		strcpy(ptb_info[UPFW_BLD2].mtd_name, mtd_table.part[PART_BLD].name);
		strcpy(ptb_info[UPFW_BLD2].mtd_dev, mtd.mtd_info[PART_BLD].mtd_dev);
	}
#endif /*INPUT_BLD2_IMG*/	

#ifdef INPUT_PARAM_IMG	
		if((header_info.img_head[UPFW_PARAM].fw_len > 0) 
			&& (header_info.img_head[UPFW_PARAM].fw_being > 0)
			&& (((header_info.img_head[UPFW_PARAM].fw_version >> 8) & 0xff) == 
				((mtd_table.part[PART_PARAM].version >> 8) & 0xff))    	
			&& (header_info.img_head[UPFW_PARAM].fw_version != 
				mtd_table.part[PART_PARAM].version)
		){	
			firmware_flag[UPFW_PARAM] = 1;
			ptb_info[UPFW_PARAM].fw_pad = 1;
			// update base infomation( part name and mtd device)
			strcpy(ptb_info[UPFW_PARAM].mtd_name, mtd_table.part[PART_PARAM].name);
			strcpy(ptb_info[UPFW_PARAM].mtd_dev, mtd.mtd_info[PART_PARAM].mtd_dev);
		}
#endif /*INPUT_PARAM_IMG*/	

#ifdef INPUT_JPG_IMG	
		if((header_info.img_head[UPFW_JPG].fw_len > 0) 
			&& (header_info.img_head[UPFW_JPG].fw_being > 0)
			&& (((header_info.img_head[UPFW_JPG].fw_version >> 8) & 0xff) == 
				((mtd_table.part[PART_JPG].version >> 8) & 0xff))	    	
			&& (header_info.img_head[UPFW_JPG].fw_version != 
				mtd_table.part[PART_JPG].version)
		){	
			firmware_flag[UPFW_JPG] = 1;
			ptb_info[UPFW_JPG].fw_pad = 1;
			// update base infomation( part name and mtd device)
			strcpy(ptb_info[UPFW_JPG].mtd_name, mtd_table.part[PART_JPG].name);
			strcpy(ptb_info[UPFW_JPG].mtd_dev, mtd.mtd_info[PART_JPG].mtd_dev);
		}
#endif /*INPUT_JPG_IMG*/	

	// check kernel and filesys version legality
	if(header_info.img_head[UPFW_FS].fw_len > 0  
		&& header_info.img_head[UPFW_FS].fw_being > 0)
	{
		firmware_flag[UPFW_FS] = 1;
		ptb_info[UPFW_FS].fw_pad = 1;
		if(cureent_fs == 1)	// current fs is fs1, then update fs2
		{	
			// update base infomation( part name and mtd device)
			strcpy(ptb_info[UPFW_FS].mtd_name, mtd.mtd_info[PART_FS2].mtd_name);
			strcpy(ptb_info[UPFW_FS].mtd_dev, mtd.mtd_info[PART_FS2].mtd_dev);
		}
		else
		{		
			// update base infomation( part name and mtd device)
			strcpy(ptb_info[UPFW_FS].mtd_name, mtd.mtd_info[PART_FS1].mtd_name);
			strcpy(ptb_info[UPFW_FS].mtd_dev, mtd.mtd_info[PART_FS1].mtd_dev);
		}
	}
	else
	{
		ERR_RETURN(UPFW_UPDATE_FS_ERR, "No filesys image!\n");
	}

#if 1
	// check kernel if has kernel image
	if( header_info.img_head[UPFW_KER].fw_len > 0 &&
		header_info.img_head[UPFW_KER].fw_being > 0 )
	{
		firmware_flag[UPFW_KER] = 1;
		ptb_info[UPFW_KER].fw_pad = 1;
		if(cureent_fs == 1)	//  need update kernel 2
		{
			// update base infomation( part name and mtd device)
			strcpy(ptb_info[UPFW_KER].mtd_name, mtd.mtd_info[PART_KER2].mtd_name);
			strcpy(ptb_info[UPFW_KER].mtd_dev, mtd.mtd_info[PART_KER2].mtd_dev);
		}
		else	//  need update kernel 1
		{
			// update base infomation( part name and mtd device)
			strcpy(ptb_info[UPFW_KER].mtd_name, mtd.mtd_info[PART_KER1].mtd_name);
			strcpy(ptb_info[UPFW_KER].mtd_dev, mtd.mtd_info[PART_KER1].mtd_dev);
		}
	}
	else
	{
		ERR_RETURN(UPFW_UPDATE_KER_ERR, "No filesys image!\n");
	}
	env_add_msauthcheck(); //goli add to add msauthcheck, when old_version is 8.0.2
#else
	if(firmware_flag[UPFW_FS] == 1) // has fs
	{
		// check kernel if has kernel image
		if( header_info.img_head[UPFW_KER].fw_len > 0 &&
			header_info.img_head[UPFW_KER].fw_being > 0 )
		{
			if(cureent_fs == 1)	// current fs is fs1, then check kernel2 version
			{
				if(header_info.img_head[UPFW_KER].fw_version ==
						mtd_table.part[PART_KER2].version
				) {
					firmware_flag[UPFW_KER] = 0;// the same version, skip update
				}
				else
				{
					firmware_flag[UPFW_KER] = 1;// need update kernel 2
					ptb_info[UPFW_KER].fw_pad = 1;
					// update base infomation( part name and mtd device)
					strcpy(ptb_info[UPFW_KER].mtd_name, 
						mtd.mtd_info[PART_KER2].mtd_name);
					strcpy(ptb_info[UPFW_KER].mtd_dev, 
						mtd.mtd_info[PART_KER2].mtd_dev);
				}
			}
			else	// check kernel 1 if need update
			{
				if(header_info.img_head[UPFW_KER].fw_version ==
                    mtd_table.part[PART_KER1].version
				) {
					firmware_flag[UPFW_KER] = 0;// the same version, skip update
				}
				else
				{
					firmware_flag[UPFW_KER] = 1;//  need update kernel 1
					ptb_info[UPFW_KER].fw_pad = 1;
					// update base infomation( part name and mtd device)
					strcpy(ptb_info[UPFW_KER].mtd_name, 
						mtd.mtd_info[PART_KER1].mtd_name);
					strcpy(ptb_info[UPFW_KER].mtd_dev, 
						mtd.mtd_info[PART_KER1].mtd_dev);
				}
			}
		}
		else // if not has kernel 
		{	
			// check if kernel version is meet the requirements
			if(cureent_fs == 1)	// current fs is fs1, then check kernel2 version
			{
                if(header_info.img_head[UPFW_KER].fw_version != 
                    mtd_table.part[PART_KER2].version)
                {
                    fprintf(stderr, "kernel 2 version: %d.%d.%d.%d isn't "
                        "match the require version: %d.%d.%d.%d!\n",
                        ((header_info.img_head[UPFW_KER].fw_version >> 24) & 0xff),
                        ((header_info.img_head[UPFW_KER].fw_version >> 16) & 0xff),
                        ((header_info.img_head[UPFW_KER].fw_version >> 8) & 0xff),
                        (header_info.img_head[UPFW_KER].fw_version & 0xff),
                        ((mtd_table.part[PART_KER2].version  >> 24) & 0xff),
                        ((mtd_table.part[PART_KER2].version  >> 16) & 0xff),
                        ((mtd_table.part[PART_KER2].version  >> 8) & 0xff),
                        (mtd_table.part[PART_KER2].version  & 0xff)
                        );
                    return UPFW_KER2_MISMATCH;  
                }
			}
			else	// current fs is fs2, then check kernel1 version
			{
                if(header_info.img_head[UPFW_KER].fw_version != 
                    mtd_table.part[PART_KER1].version)
                {
                    fprintf(stderr, "kernel 1 version: %d.%d.%d.%d isn't "
                        "match the require version: %d.%d.%d.%d!\n",
                        ((header_info.img_head[UPFW_KER].fw_version >> 24) & 0xff),
                        ((header_info.img_head[UPFW_KER].fw_version >> 16) & 0xff),
                        ((header_info.img_head[UPFW_KER].fw_version >> 8) & 0xff),
                        (header_info.img_head[UPFW_KER].fw_version & 0xff),
                        ((mtd_table.part[PART_KER1].version  >> 24) & 0xff),
                        ((mtd_table.part[PART_KER1].version  >> 16) & 0xff),
                        ((mtd_table.part[PART_KER1].version  >> 8) & 0xff),
                        (mtd_table.part[PART_KER1].version  & 0xff)
                        );
                    return UPFW_KER1_MISMATCH;  
                }
			}
		}
	}
#endif

	for( i = UPFW_BLD; i < UPFW_MAX ; i++)
	{
	    //printf("====[%d]firmware_flag=%d======\n\n",i,firmware_flag[i]);
		if( firmware_flag[i] == 1)	// need to update
		{
			ptb_info[i].fw_start= header_info.img_head[i].fw_being;
			ptb_info[i].fw_len= header_info.img_head[i].fw_len;
			ptb_info[i].fw_flag= header_info.img_head[i].fw_load_flag;
			ptb_info[i].fw_load_addr= header_info.img_head[i].fw_load_mem;
			ptb_info[i].fw_version = header_info.img_head[i].fw_version;
			ptb_info[i].fw_date = header_info.img_head[i].fw_date;

			debug("Update %s partition:\n", ptb_info[i].mtd_name);
			debug("dev 	    = %s\n", ptb_info[i].mtd_dev);
			debug("offset   = 0x%08X\n", ptb_info[i].fw_start);
			debug("size	    = 0x%08X (%d K)\n", ptb_info[i].fw_len, 
											ptb_info[i].fw_len/1024);
			debug("addr	    = 0x%08X\n", ptb_info[i].fw_load_addr);
			debug("flag	    = 0x%08X\n", ptb_info[i].fw_flag);
			debug("date	    = %d/%d/%d\n", 
				((ptb_info[i].fw_date >> 16) & 0xffff),
				((ptb_info[i].fw_date >> 8) & 0xff),
				((ptb_info[i].fw_date) & 0xff)
				);
			debug("version = %d.%d.%d.%d\n",
					((ptb_info[i].fw_version >> 24) & 0xff),
					((ptb_info[i].fw_version >> 16) & 0xff),
					((ptb_info[i].fw_version >> 8) & 0xff),
					(ptb_info[i].fw_version & 0xff)
					);

			clean_memory_and_sync();
			
            if(i == UPFW_BLD || i == UPFW_BLD2)
            {
                if(spi_flash_write_file( datafile, &ptb_info[i]) != UPFW_SUCCESS)
                {
                    return UFPW_UPDATE_BLD_ERR;
                }
            
            }
  			else if(nand_flash_write_file( datafile, &ptb_info[i]) != UPFW_SUCCESS)
  			{
				switch(i)
				{
				case UPFW_KER:
					return UPFW_UPDATE_KER_ERR;
				case UPFW_FS:
					return UPFW_UPDATE_FS_ERR;
				default:
					ERR_RETURN(UPFW_FAILD, 
						"nand_flash_write_file unkown parttition faild!\n");
				}
  			}
			clean_memory_and_sync();
		}
	}

	if (set_env_cmdline(&mtd_table) < 0)
	{
		ERR_RETURN(UPFW_UPDATE_PTB_ERR, "set_env_cmdline faild!\n");
	}	
	// get ptb header information
//	debug("Get new ptb information!\n");
//	display_info();
	
    debug("Update Success:\n");
	return UPFW_SUCCESS;
}
#endif

int UPFW_log(int err_not, char errstr[])
{
	int fd;
	int ret;
	char buf[256] = {0};
	
	fd = open(UPFW_ERR_LOG, O_CREAT | O_RDWR | O_TRUNC, 0666);
	if( fd < 0 )
	{
		PERR_RETURN(UPFW_FAILD, "open "UPFW_ERR_LOG);
	}

	// write errno first
	sprintf(buf, UPFW_ERR_ERRNO_FMT, err_not);
	ret = write(fd, buf, strlen(buf));
	if(ret <= 0)
	{
		PERR_CLOSE_RETURN(fd, UPFW_FAILD, "write "UPFW_ERR_LOG);
	}

	fprintf(stderr, "%s", errstr);
	snprintf(buf, sizeof(buf), "%s", errstr);

	ret = write(fd, buf, strlen(buf));
	if(ret < 0)
	{
		PERR_CLOSE_RETURN(fd, UPFW_FAILD, "write upfw.log");
	}

	close(fd);
	
	return ret;
}



