/******************************************************************************
 * Copyright by Recovision, Incorporated. All Rights Reserved.
 *---------------------------------------------------------------------------*/
 /**
 * @file	net_info.c
 * @brief  	get and set interface of network status
 * @author	lzm
 * @section	MODIFY history
 *     - 2013.12.17 : First Created
 */ 
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>   
#include <string.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <sys/ioctl.h>
#include <errno.h>
#include <net/if.h>
#include <net/route.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/rtnetlink.h>    //for rtnetlink

#include <netinet/if_ether.h>	//for ip check
#include <netinet/in.h>			//for ip check
#include <sys/socket.h>
#include <arpa/inet.h>
#include "network_info.h"
#include <sys/time.h>
#include "msdefs.h"

#ifndef SIOCETHTOOL   
#define SIOCETHTOOL     0x8946
#endif   
  
/* CMDs currently supported */  
#define ETHTOOL_GSET        0x00000001 /* Get settings. */   
#define ETHTOOL_SSET        0x00000002 /* Set settings. */  
 
#define MAX_NETIF	5 //网卡加pppoe个数
#define NETIF_INTERVAL	10

#define IF_DEBUG 0
#if !IF_DEBUG
#define DEBUG(msg, args...) ((void)0)
#else
#define DEBUG(msg, args...) printf("[%s, %d]:"msg, __FUNCTION__, __LINE__, ##args)
#endif

//#define GATEWAY_FILE	"/etc/resolv.conf"
#define MAC_BCAST_ADDR (__u8 *) "\xff\xff\xff\xff\xff\xff"

struct ethtool_cmd {
	__u32	cmd;
	__u32	supported;
	__u32	advertising;
	__u16	speed;
	__u8	duplex;
	__u8	port;
	__u8	phy_address;
	__u8	transceiver;
	__u8	autoneg;
	__u8	mdio_support;
	__u32	maxtxpkt;
	__u32	maxrxpkt;
	__u16	speed_hi;
	__u8	eth_tp_mdix;
	__u8	eth_tp_mdix_ctrl;
	__u32	lp_advertising;
	__u32	reserved[2];
};
struct net_traffic {
	char ifname[10]; ///< 网卡名称
	unsigned long long rx_last;  ///< 在rx_time时接收的数据量
	unsigned long long tx_last;  ///< 在tx_time时发送的数据量
//	long time;  ///< 上次更新的接收时间戳
	uint64_t time;
//	long tx_time;  ///< 上次更新的发送时间戳
};

static struct net_traffic g_traffic[MAX_NETIF];

struct arpMsg {
	struct ethhdr ethhdr;	/* Ethernet header */
	__u16 htype;			/* hardware type (must be ARPHRD_ETHER) */
	__u16 ptype;			/* protocol type (must be ETH_P_IP) */
	__u8  hlen; 			/* hardware address length (must be 6) */
	__u8  plen; 			/* protocol address length (must be 4) */
	__u16 operation; 		/* ARP opcode */
	__u8  sHaddr[6]; 		/* sender's hardware address */
	__u8  sInaddr[4]; 		/* sender's IP address */
	__u8  tHaddr[6]; 		/* target's hardware address */
	__u8  tInaddr[4]; 		/* target's IP address */
	//__u8  pad[18]; 		/* pad for min. Ethernet payload (60 bytes) */
};

struct server_config_t {
	__u32 server; 					/* Our IP, in network order */
	int ifindex; 					/* Index number of the interface to use */
	__u8 arp[6]; 					/* Our arp address */
};

/************获取网络流量统计***************/
static int if_get_traffic(const char *net_device, unsigned long long *traffic0, unsigned long long *traffic1)
{
	if(NULL == net_device)
	{
		DEBUG("%s:%d Invalid argument!\n", __FILE__, __LINE__);
		return -1;
	}

    int nDevLen = strlen(net_device);
    if (nDevLen < 1 || nDevLen > 100)
    {
        DEBUG("dev length too long\n");
        return -1;
    }
    int fd = open("/proc/net/dev", O_RDONLY | O_EXCL);
    if (-1 == fd)
    {
        DEBUG("/proc/net/dev not exists!\n");
        return -1;
    }

	char buf[1024*2];
	char *ptr = NULL;
//	lseek(fd, 0, SEEK_SET);
	int nBytes = read(fd, buf, sizeof(buf)-1);
	if (-1 == nBytes)
	{
		perror("read error");
		close(fd);
        return -1;
    }
  	buf[nBytes] = '\0';
    //返回第一次指向net_device位置的指针
    char *pDev = strstr(buf, net_device);
    if (NULL == pDev)
    {
        DEBUG("can not find dev '%s'\n", net_device);
	    close(fd);
        return -1;
    }

    char *p, *tmp = NULL;
//    char ifconfig_value[20] = {0};
    int i = 0;
    unsigned long long rx2_tx10[2] = {0};
    /*去除空格，制表符，换行符等不需要的字段*/
    for (p = strtok_r(pDev, " \t\r\n", &ptr); p; p = strtok_r(NULL, " \t\r\n", &ptr))
    {
        i++;
 //       ifconfig_value = (char*)malloc(20);
 //       strcpy(ifconfig_value, p);
        /*得到的字符串中的第二个字段是接收流量*/
        if(i == 2)
        {
            rx2_tx10[0] = strtoull(p, &tmp, 0);//atol(p);//atol(ifconfig_value);
        }
        /*得到的字符串中的第十个字段是发送流量*/
        if(i == 10)
        {
            rx2_tx10[1] = strtoull(p, &tmp, 0);//atol(p);//atol(ifconfig_value);
            break;
        }
 //       free(ifconfig_value);
    }
//	printf("ALL RX bytes:%ld\n",rx2_tx10[0]);
//	printf("ALL TX bytes:%ld\n",rx2_tx10[1]);

	*traffic0 = rx2_tx10[0];
	*traffic1 = rx2_tx10[1];

	close(fd);
    return 0;
}

int if_get_speed(const char *ifname, int *rx_speed, int *tx_speed)
{
	int i = 0, flag = 0;
	unsigned long long traffic[2] = {0};
//	long elapse = 0;// tmp = 0;
	struct timeval tm = {0};
	uint64_t elapse, cur_time;

	if (if_get_traffic(ifname, &traffic[0], &traffic[1])) {
		DEBUG("get traffic for %s error\n", ifname);
		return -1;
	}
	for (i = 0; i < MAX_NETIF; i++) {
		if (!strcmp(ifname, g_traffic[i].ifname)) {
//			DEBUG("find net if: i: %d\n", i);
			break;
		}
	}
	if (i == MAX_NETIF) {
		i = 0;
		flag = 1;
		while (g_traffic[i].ifname[0]) i++;
		if (i == MAX_NETIF) {
			DEBUG("meet max netif, bad ifname: %s\n", ifname);
			return -1;
		}
	}
//	cur_time = time(NULL);
	gettimeofday(&tm, NULL);
	cur_time = (uint64_t)tm.tv_sec * 1000000 + tm.tv_usec;
	if (flag) {
		DEBUG("first time\n");  
		*rx_speed = 0;
		*tx_speed = 0;
		g_traffic[i].rx_last = traffic[0];
		g_traffic[i].tx_last = traffic[1];
		g_traffic[i].time = cur_time;
//		g_traffic[i].tx_time = cur_time;
		strncpy(g_traffic[i].ifname, ifname, sizeof(g_traffic[i].ifname));
		return 0;
	}
//	DEBUG("rx_cur: %lu, rx_last: %lu, tx_cur: %lu, tx_last: %lu\n",
//			traffic[0], g_traffic[i].rx_last, traffic[1], g_traffic[i].tx_last);
	if ((cur_time - g_traffic[i].time) / 1000000 >= NETIF_INTERVAL) {
		DEBUG("time interval too long\n");
		*tx_speed = 0;
		*rx_speed = 0;
	} else {
		elapse = cur_time - g_traffic[i].time;
//		elapse = (tmp > 0) ? tmp : 1;
		if (g_traffic[i].rx_last <= traffic[0])
			*rx_speed = (uint64_t)(traffic[0] - g_traffic[i].rx_last) * 1000000 / elapse;
		else {
			DEBUG("rx_last < traffic[0]: %u, %u\n", g_traffic[i].rx_last, traffic[0]);
			*rx_speed = 0;
		}

		if (g_traffic[i].tx_last <= traffic[1])
			*tx_speed = (uint64_t)(traffic[1] - g_traffic[i].tx_last) * 1000000 / elapse;
		else {
			DEBUG("tx_last < traffic[1]: %u, %u\n", g_traffic[i].tx_last, traffic[1]);
			*tx_speed = 0;
		}
	}
	g_traffic[i].rx_last = traffic[0];
	g_traffic[i].tx_last = traffic[1];
	g_traffic[i].time = cur_time;;

//	DEBUG("elapse time: %ld", elapse);
	return 0;
}

static int get_gateway(const char *dev, long *gate)
{
	FILE *fp = NULL;
	char buf[256] = {0}, iface[16] = {0};
	long dst = 0, gateway = 0;

	if ((fp = fopen("/proc/net/route", "r")) == NULL) {
		DEBUG("fopen error\n");
		perror("fopen");
		return -1;
	}
	fgets(buf, sizeof(buf), fp);
	while (fgets(buf, sizeof(buf), fp)) {

		if (sscanf(buf, "%15s %lX %lX", iface, &dst, &gateway) != 3)
			continue;
		if (dst)
			continue;
//		DEBUG("iface: %s, dst: %ld, gateway: %ld\n", iface,dst, gateway);
		if (!strcmp(dev, iface)) {
//			DEBUG("gateway is : %ld\n", gateway);
			*gate = gateway;
			break;
		}
	}
	fclose(fp);
	return 0;
}

int if_get_netconf(const char *dev, struct net_conf *conf)
{
	if (!dev || !conf) {
		DEBUG("invalid params\n");
		return -1;
	}
	int sockfd, ret = -1;
	struct ifreq ifreq;
	memset(&ifreq, 0, sizeof(ifreq));
	struct sockaddr_in *sin = NULL;
	unsigned char *pmac = NULL, *pgate = NULL;
	long gateway = 0;

	strncpy(ifreq.ifr_ifrn.ifrn_name, dev, sizeof(ifreq.ifr_ifrn.ifrn_name));
	do {
		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			break;
		}
		if (ioctl(sockfd, SIOCGIFMTU, &ifreq)) {
			perror("[SIOCGIFMTU] ioctl");
			DEBUG("get mtu error\n");
		}
		conf->mtu = ifreq.ifr_ifru.ifru_mtu;

		if (ioctl(sockfd, SIOCGIFFLAGS, &ifreq)) {
			perror("[SIOCGIFFLAGS] ioctl");
			DEBUG("get running status error\n");
			memset(&ifreq, 0, sizeof(ifreq));
		}
		conf->state = (ifreq.ifr_ifru.ifru_flags & IFF_UP) ? LINK_UP : LINK_DOWN;
		DEBUG("conf state: %d\n", conf->state);

		if (ioctl(sockfd, SIOCGIFADDR, &ifreq)) {
			DEBUG("get ip address error\n");
			perror("[SIOCGIFADDR] ioctl");
		}
		sin = (struct sockaddr_in *)&ifreq.ifr_ifru.ifru_addr;
		inet_ntop(AF_INET, (void*)&(sin->sin_addr), conf->ip, 16);

		if (ioctl(sockfd, SIOCGIFHWADDR, &ifreq)) {
			DEBUG("get mac address error\n");
			perror("[SIOCGIFHWADDR] ioctl");
		}
		pmac = (unsigned char *)&ifreq.ifr_ifru.ifru_hwaddr.sa_data[0];
		snprintf(conf->mac, sizeof(conf->mac), "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
				pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5]);
		DEBUG("%s mac : %s\n", dev, conf->mac);

		if (ioctl(sockfd, SIOCGIFNETMASK, &ifreq)) {
			DEBUG("get netmask error\n");
			perror("[SIOCGIFNETMASK] ioctl");
		}
		sin = (struct sockaddr_in *)&ifreq.ifr_ifru.ifru_netmask;
		inet_ntop(AF_INET, &(sin->sin_addr), conf->netmask, 16);

		if (ioctl(sockfd, SIOCGIFBRDADDR, &ifreq)) {
			DEBUG("get broadcast addr error\n");
			perror("[SIOCGIFBRDADDR] ioctl");
		}
		sin = (struct sockaddr_in *)&ifreq.ifr_ifru.ifru_broadaddr;
		inet_ntop(AF_INET, &(sin->sin_addr), conf->broadcast, 16);

		if (!get_gateway(dev, &gateway)) {
			pgate = (unsigned char *)&gateway;
			snprintf(conf->gateway, sizeof(conf->gateway), "%d.%d.%d.%d", pgate[0], pgate[1], pgate[2], pgate[3]);
			DEBUG("gateway: %s\n", conf->gateway);
		}
		close(sockfd);
		ret = 0;
	} while (0);

	return ret;
}

static int read_interface(const char *interface, int *ifindex, __u32 *addr, __u8 *arp)		//used for ip_check
{
    int fd;
    struct ifreq ifr;
    struct sockaddr_in *our_ip;

    memset(&ifr, 0, sizeof(struct ifreq));
    if((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0)
    {
        ifr.ifr_addr.sa_family = AF_INET;
        strcpy(ifr.ifr_name, interface);

        /*this is not execute*/
        if (addr)
        {
            if (ioctl(fd, SIOCGIFADDR, &ifr) == 0)
            {
                our_ip = (struct sockaddr_in *) &ifr.ifr_addr;
                *addr = our_ip->sin_addr.s_addr;
                //DEBUG("%s (our ip) = %s\n", ifr.ifr_name, inet_ntoa(our_ip->sin_addr));
            }
            else
            {
                DEBUG("SIOCGIFADDR failed00: %s\n",strerror(errno));
				close(fd);
                return -1;
            }
        }

        if (ioctl(fd, SIOCGIFINDEX, &ifr) == 0)
        {
            //DEBUG("adapter index %d\n", ifr.ifr_ifindex);
            *ifindex = ifr.ifr_ifindex;
        }
        else
        {
            DEBUG("SIOCGIFINDEX failed11: %s\n", strerror(errno));
			close(fd);
            return -1;
        }

        if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0)
        {
            memcpy(arp, ifr.ifr_hwaddr.sa_data, 6);
            //DEBUG("adapter hardware address %02x:%02x:%02x:%02x:%02x:%02x\n",
            //arp[0], arp[1], arp[2], arp[3], arp[4], arp[5]);
        }
        else
        {
            DEBUG("SIOCGIFHWADDR failed: %s\n", strerror(errno));
			close(fd);
            return -1;
        }
    }
    else
    {
        DEBUG("socket failed: %s\n", strerror(errno));
		close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

static int arpping(__u32 target_ip, __u32 source_ip, __u8 *mac, const char *interface)		//used for ip_check
{

    int timeout = 2;
    int optval = 1;
    int sockfd; 
    int rv = 1; 			/* return value */
    struct sockaddr addr; 	/* for interface name */
    struct arpMsg arp;
    fd_set fdset;
    struct timeval tm;
    time_t prevTime;
	
	if(target_ip == source_ip)
	{
		//DEBUG("The ip is used by youself !\n");
		return 0;
	}

    if ((sockfd = socket (PF_PACKET, SOCK_PACKET, htons(ETH_P_ARP))) == -1)
    {
        DEBUG("Could not open raw socket !\n");
        return -1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) == -1)
    {
        DEBUG("Could not setsocketopt on raw socket !\n");
        close(sockfd);
        return -1;
    }

    /* send arp request */
    memset(&arp, 0, sizeof(arp));
    memcpy(arp.ethhdr.h_dest, MAC_BCAST_ADDR, 6);	/* MAC DA */
    memcpy(arp.ethhdr.h_source, mac, 6); 	/* MAC SA */
    arp.ethhdr.h_proto = htons(ETH_P_ARP); 	/* protocol type (Ethernet) */
    arp.htype = htons(ARPHRD_ETHER);		/* hardware type */
    arp.ptype = htons(ETH_P_IP);			/* protocol type (ARP message) */
    arp.hlen = 6;	/* hardware address length */
    arp.plen = 4;	/* protocol address length */
    arp.operation = htons(ARPOP_REQUEST); 	/* ARP op code */
    *((u_int *) arp.sInaddr) = source_ip; 	/* source IP address */
    memcpy(arp.sHaddr, mac, 6); 			/* source hardware address */
    *((u_int *) arp.tInaddr) = target_ip; 	/* target IP address */

    memset(&addr, 0, sizeof(addr));
    strcpy(addr.sa_data, interface);
    if (sendto(sockfd, &arp, sizeof(arp), 0, &addr, sizeof(addr)) < 0)
        rv = 0;

    /* wait arp reply, and check it */
    tm.tv_usec = 0;
    time(&prevTime);
    while (timeout > 0)
    {
        FD_ZERO(&fdset);
        FD_SET(sockfd, &fdset);
        tm.tv_sec = timeout;
        if (select(sockfd + 1, &fdset, (fd_set *) NULL, (fd_set *) NULL, &tm) < 0)
        {
            DEBUG("Error on ARPING request: %s\n", strerror(errno));
            if (errno != EINTR)
                rv = 0;
        }
        else if (FD_ISSET(sockfd, &fdset))
        {
            if (recv(sockfd, &arp, sizeof(arp), 0) < 0 )
                rv = 0;
            if (arp.operation == htons(ARPOP_REPLY) &&
                bcmp(arp.tHaddr, mac, 6) == 0 &&
                *((u_int *) arp.sInaddr) == target_ip)
            {
                //DEBUG("Valid arp reply receved for this address\n");
                rv = 0;
                break;
            }
        }
        timeout -= time(NULL) - prevTime;
        time(&prevTime);
    }
    close(sockfd);
   // DEBUG("%salid arp replies for this address\n", rv ? "No v" : "V");
    return rv;
}
static __inline__ __u32 ethtool_cmd_speed(const struct ethtool_cmd *ep)
{
	return (ep->speed_hi << 16) | ep->speed;
}
/************获取连接信息***************/
static int if_get_linkinfo(const char *net_device, int info[3])
{
	int skfd;
//	static int ret[3];
	struct ifreq ifr;
	char dev[32] = {0};
	uint32_t speed;
//	DEBUG("get link info\n");
	skfd = socket( AF_INET, SOCK_DGRAM, 0 );
	if(skfd < 0 )
	{
		perror("create socket error"); 
		return -1;
	}
//	DEBUG("strncpy .......\n");
	/* 获取网络连接状态 */
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_ifrn.ifrn_name, net_device);
//	DEBUG("get ifflags**************\n");
	if(ioctl(skfd, SIOCGIFFLAGS, &ifr) <0 )  
	{  
		DEBUG("get %s running state error!\n", net_device);
		close(skfd);
		return -1;
	}  

    if(ifr.ifr_ifru.ifru_flags & IFF_RUNNING)
    {  
		info[0] = LINK_UP;
    }  
    else  
    {  
		info[0] = LINK_DOWN;
    }

    snprintf(dev, sizeof(dev), "%s", net_device);
    /**
     * 不支持bond0和ppp0
     */
    if (net_device[0] == 'p' || net_device[0] == 'b') {
//    	close(skfd);
//    	return 0;
//    	snprintf(net_device, sizeof(net_device), "eth0");
//    	snprintf(dev, sizeof(dev), DEVICE_NAME_ETH0);
    }
  
	//printf("%s is %s !\n", net_device,ret? "Up":"Down");

//    DEBUG("ethtool start...................\n");
	/* 获取网络连接速率 */
	struct ethtool_cmd ep;  
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_ifrn.ifrn_name, dev);
    ep.cmd = ETHTOOL_GSET;
    ifr.ifr_ifru.ifru_data = (void *)&ep;  		//caddr_t 是void类型

//    DEBUG("ioctl ethtool#############\n");
	if(ioctl(skfd, SIOCETHTOOL, &ifr) != 0)  
	{  
		DEBUG("ethtool IOCTL error!\n");
		printf("NIC:%s", net_device);
		perror("[SIOCETHTOOL] ioctl");
		printf("\n");
		close(skfd);
		return -1;
	}
//	DEBUG("ioctl end...........\n");

	speed = ethtool_cmd_speed(&ep);
	info[1] = speed;//ep.speed;
//	printf("ifname:%s Speed : %dMb/s, %d\n",dev,info[1]);
	info[2] = ep.duplex;
//	printf("Duplex: %s\n",info[2]? "Full" : "Half");
//	DEBUG("call end\n");
	close(skfd);
	return 0;
}

/************获取网卡数量***************/
int if_get_netcount(void)
{
	int nCount = 0;
	FILE* fd = fopen("/proc/net/dev", "r");

	if (!fd)
	{
		fprintf(stderr, "Open /proc/net/dev failed!errno:%d\n", errno);
		return nCount;
	}

	char szLine[512];
	fgets(szLine, sizeof(szLine), fd);    /* eat line */
	fgets(szLine, sizeof(szLine), fd);
	
	while(fgets(szLine, sizeof(szLine), fd))
	{
		char szName[128] = {0};
		sscanf(szLine, "%127s", szName);
		int nLen = strlen(szName);
		if (nLen <= 0)continue;
		if (szName[nLen - 1] == ':') szName[nLen - 1] = 0;
		if (strcmp(szName, "lo") == 0)continue;
		nCount++;
	}

	fclose(fd);
	fd = NULL;
	return nCount;
}

/****************获取网络信息接口******************/
int if_get_netinfo(const char *net_device, struct net_info *net_data)
{
	int netinfo[3] = {0};
//	static struct net_info net_data;

	if (NULL == net_device) {
		DEBUG("Invalid network device!\n");
		return -1;
	}
	//DEBUG("enter if_get_netinfo, dev: %s\n", net_device);
	if (if_get_linkinfo(net_device, netinfo)) {
		DEBUG("get_linkinfo error\n");
		return -1;
	}
	if (NULL == netinfo) {
		DEBUG("Call function of 'get_linkinfo' error!\n");
		return -1;
	}
	net_data->status = netinfo[0];
	net_data->speed = netinfo[1];
	net_data->duplex = netinfo[2];

	return 0;
}

/****************设置网络状态接口******************/
int if_set_network(const char *net_device,char mode)
{
	if(NULL == net_device)
	{
		DEBUG("%s:%d Invalid network device!\n", __FILE__, __LINE__);
		return -1;
	}

	int ret;
	char str[20];
	char *set_mode = mode? "up":"down";

	ret = sprintf(str,"ifconfig %s %s",net_device,set_mode);
	system(str);
	DEBUG("ifconfig %s %s\n",net_device,set_mode);
	return ret;
}

static int get_mac(char *device_name, unsigned char *pMacAddress)
{
	struct ifreq tmp;
	int sock_mac;

	sock_mac = socket(AF_INET, SOCK_STREAM, 0);
	if( sock_mac == -1){
	    perror("create socket fail\n");
	    return -1;
	}
	memset(&tmp,0,sizeof(tmp));
	strncpy(tmp.ifr_name,device_name,sizeof(tmp.ifr_name)-1 );
	if( (ioctl( sock_mac, SIOCGIFHWADDR, &tmp)) < 0 ){
	    close(sock_mac);
	    printf("mac ioctl error(%s).\n", device_name);
	    return -1;
	}
	memcpy(pMacAddress, tmp.ifr_hwaddr.sa_data, sizeof(unsigned char)*6);
	close(sock_mac);
	return 0;
}

int if_get_mac_addr(char *mac0, char *mac1)
{
	//bruce.milesight debug notyet
	unsigned char mac[8] = {0};
	if(get_mac(DEVICE_NAME_ETH0, mac))
		snprintf(mac0, 18, "1C:C3:16:0A:19:C8");
	else
		snprintf(mac0, 18, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	if(get_mac(DEVICE_NAME_ETH1, mac))
		snprintf(mac1, 18, "1C:C3:16:0A:19:C9");
	else
		snprintf(mac1, 18, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	//end debug

    return 0;
}

/****************检测IP是否被使用******************/
int if_ip_check(const char *net_device,const char *check_ip)
{
	struct server_config_t server_config;

	if (NULL == net_device) {
		DEBUG("Invalid network device!\n");
		return -1;
	}
	if(check_ip == NULL)
	{
		DEBUG("Invalid IP\n");
		return -1;
	}
    if (read_interface(net_device, &server_config.ifindex, &server_config.server, server_config.arp) < 0)
    {
        return -1;
    }
	int ret = arpping(inet_addr(check_ip), server_config.server, server_config.arp, net_device);
	if (ret == 0)
    {
         DEBUG("[ip_check] net:%s IP:%s conflict !\n", net_device, check_ip);
		 return IP_CONFLICT;
    }
    else if(ret == 1)
    {
       	DEBUG("[ip_check] net:%s IP:%s can use .\n", net_device, check_ip);
		return IP_EMPTY;
    }
	else
	{
		DEBUG("[ip_check] Call arpping() error!\n");
		return -1;
	}
}

int ms_get_mac(char *device_name, unsigned char *mac)
{
	struct ifreq tmp;
	int sock_mac;
	sock_mac = socket(AF_INET, SOCK_STREAM, 0);
	if( sock_mac == -1)
	{
	    perror("create socket fail\n");
	    return -1;
	}
	memset(&tmp,0,sizeof(tmp));
	strncpy(tmp.ifr_name,device_name,sizeof(tmp.ifr_name)-1);
	if((ioctl( sock_mac, SIOCGIFHWADDR, &tmp)) < 0)
	{
	    close(sock_mac);
	    //msdebug(DEBUG_ERR, "mac ioctl error(%s).", device_name);
	    return -1;
	}
	memcpy(mac, tmp.ifr_hwaddr.sa_data, sizeof(unsigned char)*6);
	close(sock_mac);
	return 0;
}

int ms_get_ifaddr(const char *ifname, char *addr, int length)
{
	struct ifreq ifr;
	int skfd;
	struct sockaddr_in *saddr;

	if ( (skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		msdebug(DEBUG_ERR, "socket error");
		return -1;
	}

	strncpy(ifr.ifr_name, ifname, 16);
	if (ioctl(skfd, SIOCGIFADDR, &ifr) < 0)
	{
		close(skfd);
		return -1;
	}
	close(skfd);

	saddr = (struct sockaddr_in *) &ifr.ifr_addr;
	snprintf(addr, length, "%s", inet_ntoa(saddr->sin_addr));
	return 0;
}

int ms_strcmp_two_ip_same_net(const char *ip1, const char *net1, const char *ip2, const char *net2)
{
	unsigned long ul_ip1 = inet_addr(ip1);
	unsigned long ul_ip2 = inet_addr(ip2);
	unsigned long ul_mask1 = inet_addr(net1);
	unsigned long ul_mask2 = inet_addr(net2);
	if((ul_ip1 & ul_mask1) == (ul_ip2 & ul_mask2))
		return 0;
	return -1;
}

