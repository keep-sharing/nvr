#ifndef NET_INFO_H
#define NET_INFO_H

#define  LINK_DOWN 		0
#define  LINK_UP		1

#define SPEED_10        10   
#define SPEED_100       100   
#define SPEED_1000      1000   
#define SPEED_2500      2500   
#define SPEED_10000     10000 
#define DUPLEX_HALF		0
#define DUPLEX_FULL		1

#define IP_EMPTY		0
#define IP_CONFLICT		1	

#ifdef __cplusplus
extern "C" {
#endif
struct net_info{
	int status; //状态，启不启用
	int speed; //工作模式, 10M/s 100M/s...
	int duplex; //全双工，半双工
};

struct net_conf {
	char ip[24];
	char mac[24];
	char gateway[24];
	char netmask[24];
	char broadcast[24];
	int state;
	int mtu;
};
int if_get_netinfo(const char *net_device, struct net_info *net);			//get network info

int if_getnetwork(const char *net_device);				//reserve

int if_set_network(const char *net_device,char mode);			//set network status

int if_get_speed(const char *ifname, int *rx_speed, int *tx_speed);

int if_get_netconf(const char *dev, struct net_conf *conf);

int if_get_mac_addr(char *mac0, char *mac1);

int if_ip_check(const char *net_device,const char *check_ip);
int ms_get_mac(char *device_name, unsigned char *mac);
int ms_get_ifaddr(const char *ifname, char *addr, int length);
int ms_strcmp_two_ip_same_net(const char *ip1, const char *net1, const char *ip2, const char *net2);

#ifdef __cplusplus
}
#endif
#endif
