#ifndef _SYS_CFG_H_
#define _SYS_CFG_H_

#include "msdefs.h"
#include "msdb.h"
#include "email.h"

#define MS_NGINX_CONF 		"/opt/app/nginx/conf/nginx.conf"
#define MS_NGINX_CONF_HEAD	"\
#user  nobody;\r\n\
worker_processes 1;\r\n\
events {\r\n\
	worker_connections  1024;\r\n\
	multi_accept on;\r\n\
} \r\n\
http {\r\n\
	include       mime.types;\r\n\
	default_type  application/octet-stream;\r\n\
	sendfile      on;\r\n\
	sendfile_max_chunk	512k;\r\n\
	gzip	on;\r\n\
	gzip_min_length	1k;\r\n\
	gzip_buffers	8	32k;\r\n\
	gzip_comp_level 4;\r\n\
	client_header_buffer_size 50m;\r\n\
	client_body_buffer_size	50m;\r\n\
	client_max_body_size 100m;\r\n\
	large_client_header_buffers	4	10m;\r\n\
	access_log /dev/null;\r\n\
"
#define MS_NGINX_CONF_BADY	"\
		client_body_buffer_size 50m;\r\n\
		client_max_body_size	1000m;\r\n\
		location / {\r\n\
"
#define MS_NGINX_CONF_LOCAL "\
			proxy_http_version 1.1;\r\n\
			proxy_set_header Connection \"\";\r\n\
			add_header Cache-Control no-cache;\r\n\
		}\r\n\
	}\r\n\
###########################################\r\n\
\r\n\
"
/*
fastcgi_buffer_size 	50m;
fastcgi_busy_buffers_size	   50m;
fastcgi_temp_file_write_size    50m;

*/
typedef struct
{
    char enable;                // 0:link down, 1: link up
    char type;                  // 0:static, 1: dhcp
    char ipaddr[MAX_LEN_16];
    char netmask[MAX_LEN_16];
    char gateway[MAX_LEN_16];
    char dns1[MAX_LEN_16];
    char dns2[MAX_LEN_16];
    char macaddr[32]; // qh add
    int mtu;//mtu,?1500

	
	char ip6addr[MAX_LEN_64];//hrz.milesight
    char ip6netmask[MAX_LEN_16];
    char ip6gateway[MAX_LEN_64];
    int ip6type;//0=static 1=Router Advertisement, 2=dhcp
    
	
} NETWORKINFO,*PNETWORKINFO;     //NETWORK INFO

typedef struct 
{
	NETWORKINFO bondif;// 0 - eth0, 1 - eth1
	int primary;//default 0
	int miimon;// default :100
} BONDCFG;

typedef struct 
{
	char hostname[128];
	NETWORKINFO netif1;// 1 NETMODE_MULTI use
	NETWORKINFO netif2;// 2  NETMODE_MULTI use
	BONDCFG bondcfg;//NETMODE_LOADBALANCE  NETMODE_BACKUP use
	NETMODE mode;
} NETWORKCFG;

typedef struct 
{	
	int access;//0 - enable, 1 - disable 
	int sshport;
	int httpport;
	int rtpstart;
	int rtpend;
	int rtspport;
	int httpsport;
}ACCESSCFG;

typedef struct 
{
	char sendername[MAX_USER_LEN*2];
	char sender[128];
	char username[MAX_USER_LEN*2];
	char passwd[MAX_PWD_LEN*2];
	char smtpserver[MAX_USER_LEN*2];
	int port;
	int enabletls;
	int enableattach;
	int snapshot_interval;
}MAILCFG;

typedef struct 
{
	int enable;
	int version;
	int port;
	char rcommunity[32];
	char wcommunity[32];
	char trapdest[32];
	int trapport;
}SNMPCFG;

int ms_set_hostname(const char *hostname);

int ms_set_network(struct network *networkDb);

int ms_set_network_apply(struct network *networkDb);

int ms_set_ethname(void);

/* *********************************************************
   set time , ntp timstr=NULL;
   manually tmstr: YYYY-MM-DD hh:mm:ss
***********************************************************/
int ms_set_time(struct time *tmcfg, const char *tmstr);

/* pppoe */
int ms_set_PPPOE(struct pppoe *pppoecfg, int netmode);

/* ddns */
int ms_set_DDNS(struct ddns *ddnscfg);

/* set ssh , http, rtsp port*/
int ms_set_remote_access(struct network_more *accessDb);

/* set mail conf */
int ms_set_mail(struct email *emailDb);

/* test mail */
int ms_send_test_mail(struct email *emailDb);

/* SNMP */
int ms_set_SNMP(SNMPCFG *snmpcfg);
int ms_set_net_snmp(struct snmp *snmp);


//////////////////////////////////////////////////////////
//bruce.milesight add
void ms_sys_init();
int ms_start_ssh(int ssh_port, int enable);

int ms_sys_get_chipinfo(char* chip_info, int size);
int write_keep_config(const char *localfile, const char *pDbFile);
int write_keep_user_config(const char *localfile, const char *pDbFile);
int write_keep_encrypted_config(const char *localfile, const char *pDbFile);
int write_keep_params_config(const char *localfile, const char *pDbFile);
int read_keep_config(const char *localfile, const char *pDbFile);
int read_keep_user_config(const char *localfile, const char *pDbFile);
int read_keep_encrypted_config(const char *localfile, const char *pDbFile);
int read_keep_params_config(const char *localfile, const char *pDbFile);



#endif /* end SYS_CFG_H */
