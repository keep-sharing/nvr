#ifndef __SDREPORT_H__
#define __SDREPORT_H__

// define ioctl cmd
#define IOSTIMG 's'
#define IOSTSTAT _IOR(IOSTIMG, 1, int)
#define IOSTERR _IOR(IOSTIMG, 2, int)
#define IOSTFDNAME _IOR(IOSTIMG, 3, int)
#define IOSTALLSTAT _IOR(IOSTIMG, 4, int)
#define IOSTRWERR _IOR(IOSTIMG, 5, int)
#define IOSTHOTPLUG _IOR(IOSTIMG, 6, int)

#define DISK_NAME_LEN			32
#define MAX_SDINFO_SZ			16
#define MAX_SD_ERR_CNT          50
#define DISK_DRV_PATH  "/dev/msdisk"
#define DISK_DRV_NAME  "msdisk"

enum {
	SD_S_CLEAR		  = (0),
	SD_S_REMOVE_DEV = (1 << 0),
	SD_S_ADD_DEV	  = (1 << 1),
	SD_S_LINK_UP	  = (1 << 2),
	SD_S_LINK_DOWN	  = (1 << 3),
	SD_S_HOTPLUG	  = (1 << 4), // for add or remove disk flag
	SD_S_ERROR		  = (1 << 5),
};

struct sdinfo{
	unsigned int port;
	unsigned int serror;
	unsigned int stat;
	unsigned char isusb;
	char sdname[DISK_NAME_LEN];
	//signed int hub;// save extend for future
};


typedef struct sdinfoall{
	struct sdinfo siall[MAX_SDINFO_SZ];
	int num;
}sdinfoall;

typedef struct sd_err_info{
	unsigned int err_mask;
	unsigned char cmd_cmd;// real command flag(enum)
	unsigned char res_cmd;
	unsigned char res_feature;
	unsigned long long pos; // err addr:as badblock pos
}sd_err_info;

typedef struct sd_rwerr{
	int fd;
	struct sd_err_info info[MAX_SD_ERR_CNT];
	int count;// piece of size 512
}sd_rwerr;


#endif
