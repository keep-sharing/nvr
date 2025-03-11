#ifndef _MSSP_MSG_H_
#define _MSSP_MSG_H_

#define MSSP_GET_NETWORK_INFO	1
#define MSSP_SET_NETWORK_INFO	2
#define MSSP_UPGRADE_NVR		3
#define MSSP_CHECK_PERMISSION	4
#define MSSP_RESET_NVR			5
#define MSSP_REBOOT				6
#define MSSP_SET_SYSTEM_TIME	7
#define MSSP_GET_SYSTEM_TIME 	8

#define RERR_NONE					0
#define RERR_OPER_FAILED			31
#define RERR_INVALID_USER_OR_PASSWD	32
#define RERR_NO_PERMISSION			33
#define RERR_NETWORK				34

#define DATA_SIZE	(256 - 20)

struct mssp_header
{
	char src[32];
	char dst[32];
	int req;
	int size;
};

struct network_info
{
    char ip[16];
    int port;
    char netmask[16];
    char gateway[16];
    char dns[32];
    char devicetype[16];
    char sn[65]; // MAX_LEN_SNCODE
    char software[24];
    char hostname[32];
//    char username[32];
//    char password[32];
    char mac[32];
    char model[32];
	char oem_model[32];
    char uptime[32];
    int http_port;
	int upsec;
	char uptone[32];
};

struct account
{
	char mac[6];
    char username[32];
    char password[64];
};

struct exter_info
{
	char nvr_ip[16];
	char ipc_ip[16];
	int ipc_num;
	int cam_num;
	int type;
	int port;
	char netmask[16];
	char gateway[16];
	char dns[32];	
};

struct system_time
{
    int ntp_enable;
    int dst_enable;
    char time_zone[32];
    char time_zone_name[32];
    char ntp_server[64];
    int 	keep_time_flag;
    long time;
	char sDateTime[128];
};

#endif
