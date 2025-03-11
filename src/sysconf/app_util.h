#if !defined (_APP_UTIL_H_)
#define _APP_UTIL_H_

#include <stdio.h>

//# for network detect ----------------
#define DEV_ETH0		0
#define DEV_ETH1		1
#define NET_STATIC		0
#define NET_DHCP		1
#define SIZE_NET_STR	16

typedef unsigned long long UInt_64;

typedef struct {
	char type;					//# 0: static, 1: dhcp
	char state;					//# 0: link down, 1: link up
	char ip[SIZE_NET_STR];		//# ip address
	char mask[SIZE_NET_STR];	//# netmask
	char gate[SIZE_NET_STR];	//# gateway
} dvr_net_info_t;

int get_net_info(const char *ifname, dvr_net_info_t *inet);
int set_net_info(const char *ifname, dvr_net_info_t *inet);

int check_ip(const char *ipaddr);

//mail timezone
int get_timezone(char *timezone, int size);

int generate_rand_str(char *str, int size);

int get_dhcp_client_pid(const char *ifname);

#endif	//_APP_UTIL_H_
