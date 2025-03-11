#ifndef __FWINFO_H__
#define __FWINFO_H__

#define MD5_STR_LEN			33

typedef enum
{
    UPFW_BLD = 0, //ddr 2G uboot
    UPFW_KER,
    UPFW_FS,
    UPFW_CMD,
    UPFW_JPG,
    UPFW_BLD2, // ddr 4G uboot.
    UPFW_TOOL,
    UPFW_MAX,

}UPFW_INDEX_E;

typedef struct
{
    unsigned int  fw_being;					// image begin addr
    unsigned int  fw_len;					// image lengths
    unsigned int  fw_version;				// image version
    unsigned int  fw_load_mem;				// image load memory
    unsigned int  fw_load_flag;				// image load flag
    unsigned int  fw_date;					// image build date
    unsigned char fw_name[16];				// image name
    unsigned char fw_md5[MD5_STR_LEN];		// image md5 check number use hex
    unsigned int  fw_crc;                   // image crc32

}IMG_HEADER_T;

#define IMG_HEADER_SIZE		2048
#define HEADER_PAD_SIZE		( IMG_HEADER_SIZE - sizeof(IMG_HEADER_T)*UPFW_MAX - 32 - 64)

typedef struct __upfw_head_t__
{
    IMG_HEADER_T img_head[UPFW_MAX];
    char img_date[32];
    char img_version[64];
    char img_rsv[HEADER_PAD_SIZE];

}UPFW_HEADER_T, *UPFW_header_handle;

#endif /* __FWINFO_H__ */
