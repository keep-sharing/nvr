#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <sys/mman.h>
#include <signal.h>
#include <fcntl.h>//F_OK
#include <sys/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h> 
#include <sys/ioctl.h>
#include <sys/wait.h>

#include "msdefs.h"

//#define TOOL_NAME "update"
//#define LOG_PATH "/mnt/nand/upfw.log"

/*********************add for rtl8211************************************/

typedef unsigned long U32;
typedef unsigned short U16;
#define HI_FAILURE (-1)
#define HI_SUCCESS (0)

#define READ(_a)     ( *((U32*)(_a)) )
#define WRITE(_a, _v)   (*(volatile U32 *)(_a) = (_v))

#define ALLOC_SIZE 1024



#if defined _HI3798_

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

//static void *map_base = NULL;
#define MDIO_BASE 0xF9841000
//#define MDIO_DATA (map_base + 0X03C4)
//#define MDIO_ADDR (map_base + 0X03C0)
//#define MDIO_STAT (map_base + 0X03D0)
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
#if 0
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
#endif
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

#endif

static int is_glibc(char *toolNmae)
{
	int ifd = -1;
	int ofd = -1;
	int len = 0;
	char buf[1024];
	char out[32];
	
	struct stat sbuf; 
	
    ifd = open(toolNmae, O_RDONLY);
    if (ifd < 0)
    {
        printf("open [%s] failed\n", toolNmae);
        return 0;
    }
        
	if (fstat(ifd , &sbuf) < 0)
	{
	    printf("fsate failed\n");
        close(ifd);
        return 0;
	}
    if (sbuf.st_size < 102405)
    {
	    printf("file size[%ld] less than 102400\n", sbuf.st_size);
        close(ifd);
        return 0;
    }
    lseek(ifd, 102400, SEEK_SET);
    if (read(ifd, buf, 5) < 0)
    {
	    printf("read failed\n");
        close(ifd);
        return 0;
    }

    if (!strstr(buf, "GLIBC"))
    {
	    printf("[%s]is not GLIBC\n", buf);
        close(ifd);
        return 0;
    }
    
	snprintf(out, sizeof(out), "%s/%s", IMAGE_PATH, TOOL_GLIBC);
    ofd = open(out, O_RDWR|O_CREAT, 0777);
    if (ofd < 0)
    {
        printf("open [%s] failed\n", out);
        close(ifd);
        return 0;
    }

    do{
        len = read(ifd, buf, sizeof(buf));
        if (len > 0) 
            write(ofd, buf, len);
        else
            break;
    }while(1);
    
    close(ifd);
    close(ofd);

    return 1;
}

/****************************************************************************************************/

#if defined _HI3536_ || defined _HI3536C_

#define MDIO_BASE 0x11020000
#define MDIO_DATA 0X0014
#define MDIO_ADDR 0X0010
#define MII_BUSY  0x00000001
#define MII_WRITE 0x00000002
#define MII_PHY   0x04


#define I2C_16BIT_REG  0x0709
#define I2C_16BIT_DATA 0x070A

#if 0
static int g_fd = -1;

static int mdio_write(int phyaddr, int phyreg, U16 phydata)
{
    U16 value = 0;
	value = (((phyaddr << 11) & (0x0000F800)) |
		    ((phyreg << 6) & (0x000007C0))) |
		    MII_WRITE;
	value |= MII_BUSY;
	do {} while (((READ(map_base + MDIO_ADDR)) & MII_BUSY) == 1);
	/* Set the MII address register to write */
	WRITE(map_base + MDIO_DATA, phydata);
	WRITE(map_base + MDIO_ADDR, value);

	/* Wait until any existing MII operation is complete */
	do {} while (((READ(map_base + MDIO_ADDR)) & MII_BUSY) == 1);
    return 0;
}

static int mdio_read(int phyaddr, int phyreg)
{   
    int data = -1;
    U16 value = 0;
    
	value = (((phyaddr << 11) & (0x0000F800)) |
		    ((phyreg << 6) & (0x000007C0)));
	value |= MII_BUSY;
	do {} while (((READ(map_base + MDIO_ADDR)) & MII_BUSY) == 1);
	/* Set the MII address register to write */
	WRITE(map_base + MDIO_ADDR, value);
	/* Wait until any existing MII operation is complete */
	do {} while (((READ(map_base + MDIO_ADDR)) & MII_BUSY) == 1);

    data = READ(map_base + MDIO_DATA);
    
    return data;
}

int i2c_read(int addr,int offset, int len, unsigned char * data)
{
	int res;
	unsigned int cur_addr;
	unsigned char *buf = data;
	int fd;
	if(data == NULL){
		return -1;
	}

	fd = open("/dev/i2c-0", O_RDWR);
	if(fd < 0){
		printf("Unable to open i2c bus 2!!!\n");
		return fd;
	}
	ioctl(fd, I2C_SLAVE, addr);
	ioctl(fd, I2C_16BIT_REG, 0);
	ioctl(fd, I2C_16BIT_DATA, 0);

	for(cur_addr = offset; cur_addr < offset + len; cur_addr ++){
		buf[cur_addr-offset] = cur_addr & 0xff;
		res = read(fd, buf+cur_addr-offset, 1);
		if(res < 0) {
			printf("read addr 0x%x error. \n", cur_addr);
			return res;
		}
	}

	return 0;
}
#endif 

#endif

#if 0
static int phy_map()
{
    if (g_fd < 0)
    {
        g_fd = open("/dev/mem", O_RDWR | O_SYNC);  
        if (g_fd < 0) 
        {  
            printf("Fail to open /dev/mem fd=%08x\n", g_fd);
            return HI_FAILURE;
        }
    }
    map_base = mmap(0, ALLOC_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, g_fd, MDIO_BASE);
    if (!map_base)
    {
        printf("mmap phyaddr %#x failed ! \n", MDIO_BASE);
        close(g_fd);
        g_fd = -1;
        return HI_FAILURE;
    }

    return HI_SUCCESS;
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
		if (phy_map() == HI_FAILURE)
    {
        return -1;
    }
    int data = mdio_read(0x4,0x2);
    phy_umap();
    return data;
}


static int is_oem()
{
    if(access("/mnt/nand2/oem.db", F_OK))
        return 0;
    else
        return 1;    
}

static int get_main_ver_no(char *version)
{
    int no;
    
    sscanf(version, "%d.*", &no);
//    printf("======no = %d==\n\n", no);
    
    return no;
}

static int get_sub_ver_no(char *version)
{
	int no;
    char *sub;
	sub = strstr(strstr(version,".")+1,".");
    no = atoi(sub + 1);
	//printf("======no = %d==\n\n", no);
    
    return no;
}

static int get_fireware_ver(char *version)
{
    FILE *pfd = NULL;
    char buff[512];
	char cmd[128] = {0};
	
	snprintf(cmd, sizeof(cmd), "%s/%s -l | grep version | sed -n 3p | cut -d . -f 2-4 ", MS_EXE_PATH, TOOL_NAME);
	
    pfd = popen(cmd, "r");
    if (pfd != NULL)
    {
        if (fgets(buff, sizeof(buff), pfd) != NULL)
        {
            if (sscanf(buff, "%s", version) != 1)
            {
                printf("sscanf failed \n");
				pclose(pfd);
                return -1;
            }
			pclose(pfd);
            return 0;
        }
		pclose(pfd);
    }
	
    return -1;
}

static int is_update()
{
    int curNo = 0;
    int fwNo =0;
    int fw_subNo = 0;
    //int cur_subNo = 0;
    int data = 0;
	char version[32] = {0};
	unsigned char poe_id = 0;
	if (get_fireware_ver(version) != -1)
	{
	    fwNo = get_main_ver_no(version);
	    fw_subNo = get_sub_ver_no(version);
        curNo = get_main_ver_no(MS_HI_NVR_VERSION);
#if defined _HI3798_
		data = rtl8211_read();
		poe_id = get_poe_id();
#endif

#if defined _HI3536_
		data = rtl8211_read();
		i2c_read(0x42,0x43, 1, &poe_id);
		//printf("poe_id = %0x\n",poe_id);
#endif

        if (fwNo >= 8 && curNo < 8 && is_oem())
        {
            return 0;//not update
        }
        else if (fwNo < 8 && curNo >= 8 && is_oem())
        {
            return 0;//not update
        }
		
#if defined _HI3798_
        else if(fwNo < 8 && fw_subNo < 9 && data!=0x4d)
        {
        	return 0;//not update
        }
#endif

#if defined _HI3536_
	else if(fwNo < 8 && fw_subNo < 10 && data!=0x4d)
	{
		return 0;//not update
	}
#endif

	else if(fwNo < 8 && fw_subNo < 10 && (poe_id == 0x44 || poe_id == 0xe3))
	{
		return 0;//not update
	}

    else
    {
        return 1;
    }
	}
	return 1;
}
#endif

int main(int argc, char **argv)
{
	char cmd[512] = {0};
	char file_path[256] = {0};
	int oem_type = 0;
	
	if(argc == 2)
	{
		oem_type = atoi(argv[1]);
	}
	snprintf(file_path, sizeof(file_path), "%s/%s", IMAGE_PATH, IMAGE_NAME);
	if(oem_type) //for oem
	{
		snprintf(cmd, sizeof(cmd), "%s/%s -e %s ", MS_EXE_PATH, TOOL_NAME, file_path);
	}
	else
	{
		snprintf(cmd, sizeof(cmd), "%s/%s -a %s ", MS_EXE_PATH, TOOL_NAME, file_path);
	}
	system(cmd);


    /*if (!is_update())
    {
        return 0;
    }*/
	//update the file
	snprintf(cmd, sizeof(cmd), "%s/%s", IMAGE_PATH, TOOL_NAME);
	if(WEXITSTATUS(system(cmd)) != 127)
	{
		snprintf(cmd, sizeof(cmd), "%s/%s -u %s ", IMAGE_PATH, TOOL_NAME, file_path);
	}
	else
	{
		//snprintf(cmd, sizeof(cmd), "%s/%s -u %s > /dev/null", MS_EXE_PATH, TOOL_NAME, file_path);
		if (is_glibc(cmd))
		{
            printf("glibc version ...\n");
            snprintf(cmd, sizeof(cmd), "%s/%s -u %s ", IMAGE_PATH, TOOL_GLIBC, file_path);
		}
		else
		{
            printf("local version ...\n");
            snprintf(cmd, sizeof(cmd), "%s/%s -u %s ", MS_EXE_PATH, TOOL_NAME, file_path);
		}
	}
	
	system(cmd);
	return 0;
}


