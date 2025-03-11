#ifndef __UPDATEFW_CMD_H__
#define __UPDATEFW_CMD_H__


#define UPFW_ERR_LOG			"/mnt/nand/upfw.log"
#define UPFW_ERR_ERRNO_FMT		"%d\n"
#define UPFW_ERR_ERRSTR_FMT		"%s\n"

/**
  *	Log format:	
  *		first line: errno
  *		second line: description
  *
  */

typedef enum
{
	UPFW_SUCCESS		= 0,	// update success.
	UPFW_FAILD			= -1,	// update faild. For unknow error.
	UPFW_VERIFY_FAILD	= -2,	// verify update firmware faild.
	UPFW_ANALYSIS_FAILD = -3,	// analysis update firmware faild.	
	UPFW_UPDATE_PTB_ERR	= -4,	// update ptb faild
	UFPW_UPDATE_BLD_ERR	= -5,	// update bld faild
	UPFW_UPDATE_HAL_ERR = -6,	// update hal faild
	UPFW_UPDATE_DSP_ERR = -7,	// update dsp faild
	UPFW_UPDATE_KER_ERR = -8,	// update kernel faild
	UPFW_UPDATE_FS_ERR	= -9,	// update fs faild
	UPFW_UPDATE_CMDLINE = -10,	// update cmdline faild
	UPFW_KER1_MISMATCH	= -11,	// kernel 1 version mismatch
	UPFW_KER2_MISMATCH	= -12,	// kernel 2 version mismatch
	UPFW_DDR_MISMATCH	= -13,	// ddr memory mismatch
	UPFW_UPDATE_VER_LIMIT= -14,	// software version limit
}UPFW_ERRNO_E;

#undef PERR_RETURN
#ifndef PERR_RETURN
#define PERR_RETURN(errno, errstr) \
	do { \
		perror(errstr); \
		return (errno); \
	}while(0)
#endif /* PERR_RETURN */

#undef PERR_CLOSE_RETURN
#ifndef PERR_CLOSE_RETURN
#define PERR_CLOSE_RETURN(fd, errno, errstr) \
	do { \
		perror(errstr); \
		close(fd); \
		return (errno); \
	}while(0)
#endif /* PERR_CLOSE_RETURN */

#undef ERR_RETURN
#ifndef ERR_RETURN
#define ERR_RETURN(errno, errstr) \
	do { \
		fprintf(stderr, errstr); \
		return (errno); \
	}while(0)
#endif /* ERR_RETURN */

#undef ERR_CLOSE_RETURN
#ifndef ERR_CLOSE_RETURN
#define ERR_CLOSE_RETURN(fd, errno, errstr) \
	do { \
		fprintf(stderr, errstr); \
		close(fd); \
		return (errno); \
	}while(0)
#endif /* ERR_CLOSE_RETURN */

#define DEF_UPDATE_CMD		"/opt/app/bin/update"
#define NEW_UPDATE_CMD		"/dev/shm/update"
/* here is update usage, often we only use 
		[ update -a {image} ]
		[ update -u {image}]
		 image default = "/dev/shm/updateFW.bin"
*/

/****
  *	  Usage:
  *		update [OPTION] upfw_file 
  *		Writes to the specified MTD device.
  *	  
  *		 --help 	  display this help and exit
  *		 --version	  output version information and exit
  *		 --show_info  Show Milesight AMBOOT information 
  *	  -u --upfw 	  Update Milesight upgrade images
  *	  -a --analysis   Analysis Milesight upgrade images
  *	  -l --list 	  List upgrade image information
  *	  -N --sn [sn]	  Update Milesight AMBOOT SN 
  *	  -C --cmd [str]  Update Milesight AMBOOT CMD line 
  *	  -D --delay [digit]	  Update Milesight AMBOOT bootdelay 
  *		 --auto_boot [digit]  Update Milesight AMBOOT load linux from nandflash or no 
  *		 --auto_dl [digit]	  Update Milesight AMBOOT load linux from network or no 
  *		 --pri_addr [hex]	  Update Milesight AMBOOT pri_addr 
  *		 --pri_file [str]	  Update Milesight updated file name(32bytes) 
  *		 --serverip    [ip]   Update Milesight AMBOOT tftpd server ip 
  *	  -E --eth0_mac [MAC]	  Update Milesight AMBOOT eth0 MAC 
  *		 --eth0_ip [ip] 	  Update Milesight AMBOOT eth0 lan ip 
  *		 --eth0_mask [ip]	  Update Milesight AMBOOT eth0 lan mask 
  *		 --eth0_gw [ip] 	  Update Milesight AMBOOT eth0 lan gw 
  *	  -W --wifi0_mac [MAC]	  Update Milesight AMBOOT wifi0 MAC 
  *		 --wifi0_ip [ip]	  Update Milesight AMBOOT wifi0 ip 
  *		 --wifi0_mask [ip]	  Update Milesight AMBOOT wifi0 mask
  *		 --wifi0_gw [ip]	  Update Milesight AMBOOT wifi0 gw 
  */

#endif /* __UPDATEFW_CMD_H__  */
