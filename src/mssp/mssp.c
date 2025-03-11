#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <arpa/inet.h>

#include "msg.h"
#include "mssp_msg.h"
#include "request.h"
#include "mssp.h"
#include "msdb.h"
#include "msdefs.h"
#include "mssp_utils.h"
#include "msstd.h"
#include "openssl/md5.h"
#include "list.h"
#include "mssocket.h"
#include "msoem.h"

#define DEBUG(msg, ...) //printf("[%s, %s, %d]"msg"\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define UPTIME_PATH	"/tmp/uptime.txt"

u_char g_mac[6];

static LIST_HEAD(g_clilist);

struct clilist
{
	char mac[10];
	char nonce[32];
	struct list_head list;
};

static struct device_info global_device = {0};
static struct device_info global_oem_device = {0};

static int global_failover = 0;
static struct network global_network = {0};
static struct network_more global_more = {0};
static struct time global_time = {0};
static struct failover_list global_failover_list = {0};
static struct db_user global_user = {0};

int get_network_card(char card[], int len)
{
	struct network net = {0};

	read_network(SQLITE_FILE_NAME, &net);
	if (net.mode != NETMODE_MULTI) 
	{
		snprintf(card, len, "bond0");
	} 
	else 
	{
		if (net.lan1_enable)
			snprintf(card, len, DEVICE_NAME_ETH0);
		else
			snprintf(card, len, DEVICE_NAME_ETH1);
	}
	
	return 0;
}

static int get_mac_address(u_char *pMacAddress)
{
    struct ifreq tmp;
    int socket_mac = 0;
    char card[16] = {0};
    socket_mac = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_mac == -1)
	{
        msprintf("Create socket fail.");
        return 0;
    }
    memset(&tmp, 0, sizeof(tmp));
    get_network_card(card, sizeof(card));

    strncpy(tmp.ifr_name, card, sizeof(tmp.ifr_name)-1);
    if((ioctl(socket_mac, SIOCGIFHWADDR, &tmp)) < 0)
	{
		close(socket_mac);
        msprintf("mac ioctl error.");
        return 0;
    }
    memcpy(pMacAddress, tmp.ifr_hwaddr.sa_data, sizeof(u_char)*6);
    close(socket_mac);
    return 1;
}

int get_mac(char *device_name, u_char *pMacAddress)
{
	struct ifreq tmp;
	int sock_mac;
	sock_mac = socket(AF_INET, SOCK_STREAM, 0);
	if(!device_name || !pMacAddress || sock_mac == -1){
	    perror("create socket fail\n");
	    return -1;
	}
	memset(&tmp,0,sizeof(tmp));
	strncpy(tmp.ifr_name,device_name,sizeof(tmp.ifr_name)-1 );
	if( (ioctl( sock_mac, SIOCGIFHWADDR, &tmp)) < 0 ){
	    close(sock_mac);
	    msdebug(DEBUG_ERR, "mac ioctl error(%s).", device_name);
	    return -1;
	}
	memcpy(pMacAddress, tmp.ifr_hwaddr.sa_data, sizeof(unsigned char)*6);
	close(sock_mac);
	return 0;
}

int mssp_get_ifaddr(const char *ifname, char *addr, int length)
{
	int skfd;
	struct ifreq ifr;
	struct sockaddr_in *saddr;

	if ( (skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
	{
		msdebug(DEBUG_ERR, "socket error");
		return -1;
	}

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(skfd, SIOCGIFADDR, &ifr) < 0)
	{
		msdebug(DEBUG_ERR, "mssp_get_ifaddr: ioctl SIOCGIFADDR");
		close(skfd);
		return -1;
	}
	close(skfd);

	saddr = (struct sockaddr_in *) &ifr.ifr_addr;
	snprintf(addr, length, "%s", inet_ntoa(saddr->sin_addr));
	
	return 0;
}

static int get_network_ex(struct network_info *net, char *device_name)
{
	if(!net) return 0;

	struct network network = {0};
	struct network_more more = {0};

	memcpy(&network, &global_network, sizeof(struct network));
	snprintf(net->hostname, sizeof(net->hostname), "%s", network.host_name);
	if (network.mode != NETMODE_MULTI)
	{
		snprintf(net->dns, sizeof(net->dns), network.bond0_primary_dns);
		snprintf(net->netmask, sizeof(net->netmask), network.bond0_netmask);
		snprintf(net->gateway, sizeof(net->gateway), network.bond0_gateway);
		snprintf(net->ip, sizeof(net->ip), network.bond0_ip_address);
	}
	else 
	{
		if (strcmp(DEVICE_NAME_ETH0, device_name) == 0)
		{
			snprintf(net->dns, sizeof(net->dns), network.lan1_primary_dns);
			snprintf(net->netmask, sizeof(net->netmask), network.lan1_netmask);
			snprintf(net->gateway, sizeof(net->gateway), network.lan1_gateway);
			snprintf(net->ip, sizeof(net->ip), network.lan1_ip_address);
		}
		else
		{
			snprintf(net->dns, sizeof(net->dns), network.lan2_primary_dns);
			snprintf(net->netmask, sizeof(net->netmask), network.lan2_netmask);
			snprintf(net->gateway, sizeof(net->gateway), network.lan2_gateway);
			snprintf(net->ip, sizeof(net->ip), network.lan2_ip_address);
		}
	}
	
	memcpy(&more, &global_more, sizeof(struct network_more));
	net->port = more.http_port;
	net->http_port = more.http_port;
	
	return 1;
}

static int get_uptime()
{
	char sResult[128] = {0};
	FILE *fp = fopen("/proc/uptime", "rb");
	if(fp){
		fgets(sResult, sizeof(sResult), fp);
		sscanf(sResult, "%127s ", sResult);
		fclose(fp);
	}
	
	return atoi(sResult); 
}

static int get_network_info_ex(struct network_info *net_info,char *device_name)
{
	if(!net_info) return -1;

	struct time t = {0};
	get_network_ex(net_info, device_name);
	snprintf(net_info->devicetype, sizeof(net_info->devicetype), "N");
	
	snprintf(net_info->software, sizeof(net_info->software), "%s", global_device.softver);
	snprintf(net_info->sn, sizeof(net_info->sn), "%s", global_device.sncode);	
	if(global_device.oem_type != OEM_TYPE_STANDARD)
	{
		snprintf(net_info->oem_model, sizeof(net_info->oem_model), "%s", global_oem_device.model);
	}
	else
	{
		snprintf(net_info->oem_model, sizeof(net_info->oem_model), "%s", global_device.model);
	}
	snprintf(net_info->model, sizeof(net_info->model), "%s", global_device.model);
	
	get_mac(device_name, (u_char*)net_info->mac);
	FILE *fp = fopen(UPTIME_PATH, "r");
	if (fp)
	{
		fgets(net_info->uptime, sizeof(net_info->uptime), fp);
		net_info->uptime[strlen(net_info->uptime) - 1] = '\0';
		fclose(fp);
	}
	net_info->upsec = get_uptime();
	memcpy(&t, &global_time, sizeof(struct time));
	
	snprintf(net_info->uptone, sizeof(net_info->uptone), "%s", t.time_zone);
	
	return 0;
}

static void print_password(char buf[], int size)
{
	int i = 0;

	for (; i < size; i++) 
	{
		printf("%x", buf[i]);
		if (i % 16 == 0)
		  putchar('\n');
	}
	putchar('\n');
}

int check_permission(char *mac, char *username, char *password)
{
	int i = 0; 
    int find = 0;
    struct clilist *pos, *n;
    struct db_user user = {0};
	DEBUG("username: %s", username);
    if(!username || !password)
    	return RERR_INVALID_USER_OR_PASSWD;
    if (read_user_by_name(SQLITE_FILE_NAME, &user, username))
    	return RERR_INVALID_USER_OR_PASSWD;
    if (strcmp(user.username, "admin") && strcmp(user.username, "Admin"))
    	return RERR_NO_PERMISSION;

    list_for_each_entry_safe(pos, n, &g_clilist, list) 
	{
    	if (!memcmp(mac, pos->mac, 6)) 
		{
    		find = 1;
    		break;
    	}
    }
	
    DEBUG("pos.mac is: %02x:%02x:%02x:%02x:%02x:%02x: %s", pos->mac[0], pos->mac[1], pos->mac[2], pos->mac[3], pos->mac[4], pos->mac[5], pos->nonce);
    DEBUG("mac is: %02x:%02x:%02x:%02x:%02x:%02x %s", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], password);

	if (!find) 
	{
    	DEBUG("can't find mac");
    	return RERR_INVALID_USER_OR_PASSWD;
    }
    unsigned char buf[64] = {0}, out[64] = {0};
	memcpy(buf, user.password, 32);
	memcpy(buf + 32, pos->nonce, 32);
	MD5(buf, sizeof(buf), out);

	for (i = 0; i < 64; i++) 
	{
		if (out[i] == 0)
		  out[i] = 0xff;
		else if (out[i] == ';')
		  out[i] = 0xff;
	}
	if (memcmp(out, password, sizeof(out)))
	{
		DEBUG("invalid password");
		print_password((char *)out, sizeof(buf));
		print_password(password, 64);
		return RERR_INVALID_USER_OR_PASSWD;
	}
	
    return RERR_NONE;
}

static int set_network_info_ex(struct network_info *net_info, char *device)
{
    if(!net_info) return RERR_OPER_FAILED;
    struct network net;
	struct network old;
    struct req_set_sysconf req;

    DEBUG("set_network_info");
    memcpy(&net, &global_network, sizeof(struct network));
	memcpy(&old, &net, sizeof(struct network));
	if (strcmp(device, "bond0") == 0)
	{
		strncpy(net.bond0_ip_address, net_info->ip, sizeof(net.bond0_ip_address));
		strncpy(net.bond0_gateway, net_info->gateway, sizeof(net.bond0_gateway));
		strncpy(net.bond0_primary_dns, net_info->dns, sizeof(net.bond0_primary_dns));
		strncpy(net.host_name, net_info->hostname, sizeof(net.host_name));
		strncpy(net.bond0_netmask, net_info->netmask, sizeof(net.bond0_netmask));
		net.bond0_enable = 1;
	}
	else
	{
		if (strcmp(device, "eth0") == 0)
		{
			net.lan1_type = 0; 		
			strncpy(net.lan1_ip_address, net_info->ip, sizeof(net.lan1_ip_address));
			strncpy(net.lan1_gateway, net_info->gateway, sizeof(net.lan1_gateway));
			strncpy(net.lan1_primary_dns, net_info->dns, sizeof(net.lan1_primary_dns));
			strncpy(net.host_name, net_info->hostname, sizeof(net.host_name));
			strncpy(net.lan1_netmask, net_info->netmask, sizeof(net.lan1_netmask));
		}
		else 
		{
			net.lan2_type = 0; 		
			strncpy(net.lan2_ip_address, net_info->ip, sizeof(net.lan2_ip_address));
			strncpy(net.lan2_gateway, net_info->gateway, sizeof(net.lan2_gateway));
			strncpy(net.lan2_primary_dns, net_info->dns, sizeof(net.lan2_primary_dns));
			strncpy(net.host_name, net_info->hostname, sizeof(net.host_name));
			strncpy(net.lan2_netmask, net_info->netmask, sizeof(net.lan2_netmask));
		}
	}
	
    if (write_network(SQLITE_FILE_NAME, &net))
	{
		DEBUG("write_network error");
	}
	memcpy(&global_network, &net, sizeof(struct network));
	DEBUG("write_network done, netmask: %s", net.lan1_netmask);

    if (global_more.http_port != net_info->port) {
        global_more.http_port = net_info->port;
        write_port(SQLITE_FILE_NAME, &global_more);
        memset(&req, 0, sizeof(struct req_set_sysconf));
        snprintf(req.arg, sizeof(req.arg), "more");
        ms_sock_send_msg(SOCKET_TYPE_CORE, REQUEST_FLAG_SET_NETWORK, &req, sizeof(struct req_set_sysconf), 0);
    }
	
    ms_system("sysconf -s network hostname");
    ms_sock_send_msg(SOCKET_TYPE_CORE, REQUEST_FLAG_GET_SYSINFO, 0, 0, 0);
	snprintf(old.host_name, sizeof(old.host_name), "%s", net.host_name);
	if (!memcmp(&old, &net, sizeof(struct network)))
	{
		return -1;
	}
	
    return 0;
}

static int modify_network_info_ex(const char *mac, const u_char *packet, char *device)
{
    char values[32][128] = {{0}};
    int ret = RERR_NONE;
    if(!packet)
		return RERR_OPER_FAILED;

    switch_from_raw_packet(packet, values);
    if ((ret = check_permission((char *)mac, values[8], values[9])) != RERR_NONE) 
	{
    	DEBUG("ret is: %d",ret);
    	return ret;
    }
	
    struct network_info net_info;
	memset(&net_info, 0, sizeof(net_info));
    snprintf(net_info.hostname, sizeof(net_info.hostname), "%s", values[0]);
    snprintf(net_info.ip, sizeof(net_info.ip), "%s", values[1]);
    net_info.port = atoi(values[2]);
    snprintf(net_info.netmask, sizeof(net_info.netmask), "%s", values[3]);
    snprintf(net_info.gateway, sizeof(net_info.gateway), "%s", values[4]);
    snprintf(net_info.dns, sizeof(net_info.dns), "%s", values[5]);
    snprintf(net_info.devicetype, sizeof(net_info.devicetype), "%s", values[6]);
    snprintf(net_info.software, sizeof(net_info.software), "%s", values[7]);
	
    return set_network_info_ex(&net_info, device);
}

static int mssp_check_permission(struct mssp_header *phdr, void *args, void *buf)
{
	char values[32][128];
	memset(values, 0, sizeof(values));
	switch_from_raw_packet((u_char*)buf, values);
	int ret = check_permission(phdr->src ,values[0], values[1]);
	phdr->size = 0;
	memcpy(phdr->dst, g_mac, 6);

	return req_sendmsg(phdr, ret, NULL, args);
}

static int mssp_set_network(struct mssp_header *phdr, void *args, void *buff)
{
	search_device *device_info = (search_device*)args;
	int ret = RERR_NONE;

	ret = modify_network_info_ex(phdr->src, (u_char *)buff, device_info->device_name);
	memcpy(phdr->dst, device_info->mac, 6);
	phdr->size = 0;
	int result = 0;
	if (ret == 0 || ret == -1)
		result = req_sendmsg(phdr, 0, NULL, (void *)args);
	else
		result = req_sendmsg(phdr, ret, NULL, (void *)args);
	if(!ret)
		ms_sock_send_msg(SOCKET_TYPE_CORE, REQUEST_FLAG_SET_NETWORK_RESTART, &result, sizeof(result), 0);
	return result;
}

static int mssp_get_max_bandwidth()
{
    char sBuf[32] = {0};
    FILE *fp = fopen(MS_MAX_BANDWIDTH, "rb");
    if(!fp)
        return 0;
    fgets(sBuf, sizeof(sBuf), fp);
    fclose(fp);
    if(sBuf[0] == '\0')
        return 0;

    return atoi(sBuf);
}

static int mssp_get_network(struct mssp_header *phdr, void *args, void *buf)
{
	search_device *device_info = (search_device*)args;

	int failover_mode = 0;
	int activeStatus = 0;
	struct network_info net;
	struct db_user user = {0};
	struct failover_list failover;
    
	char data[256];
	char nonce[64] = {0}, nonce2[65] = {0};
	char curDevIPaddr[16] = {0};
	
	memset(data, 0, sizeof(data));
	u_char packet[DATA_SIZE];
	memset(packet, 0, sizeof(packet));
	memset(&net, 0, sizeof(net));
	memset(&failover, 0, sizeof(struct failover_list));
	
	int ret = get_network_info_ex(&net, device_info->device_name);
	if (ret)
	{
		ret = RERR_OPER_FAILED;
	}
	else 
	{
		int find = 0;
		struct clilist *pos, *n, *tmp = NULL;
		list_for_each_entry_safe(pos, n, &g_clilist, list) 
		{
			if (!memcmp(pos->mac, phdr->src, 6))
			{
				get_random_md5str(pos->nonce, sizeof(pos->nonce));
				find = 1;
				memcpy(nonce, pos->nonce, sizeof(pos->nonce));
				break;
			}
		}
		if (!find) 
		{
			tmp = ms_calloc(1, sizeof(*tmp));
			if (!tmp)
			{
				DEBUG("out of memory");
				return RERR_OPER_FAILED;
			}
			memcpy(tmp->mac, phdr->src, 6);
			get_random_md5str(tmp->nonce, sizeof(tmp->nonce));
			list_add(&tmp->list, &g_clilist);
			memcpy(nonce, tmp->nonce, sizeof(pos->nonce));
		}
		ret = RERR_NONE;
	}
	
	memcpy(nonce2, nonce, sizeof(nonce));
	mssp_get_ifaddr(device_info->device_name, curDevIPaddr, sizeof(curDevIPaddr));
	if(strcmp(net.ip, curDevIPaddr))
	{
		snprintf(net.ip, sizeof(net.ip), "%s", curDevIPaddr);
	}
	memcpy(&user, &global_user, sizeof(struct db_user));

    //failover
    failover_mode = global_failover; 
    int band = mssp_get_max_bandwidth();
    band >>= 10;
	
    memcpy(&failover, &global_failover_list, sizeof(struct failover_list));
	if(user.password_ex[0] != '\0')
	{
		activeStatus = 1;
	}

    //17 filover_mode, 18 slave ipaddr 19 slave password 20 max camera 21 band 22 device_name
	snprintf(data, sizeof(data), "%s;%s;%d;%s;%s;%s;%s;%s;%s;%s;%s;%s;%d;%s;%d;%s;%d;%d;%s;%s;%d;%d;%s;",
			net.hostname, net.ip, net.http_port, net.netmask, net.gateway,
			net.dns,net.sn, net.devicetype, net.software, net.model, net.uptime, nonce2, net.http_port, net.oem_model,
			net.upsec, net.uptone,activeStatus, failover_mode, failover.ipaddr, failover.password, global_device.max_cameras, band, device_info->device_name);

	memcpy(phdr->dst, net.mac, 6);
	phdr->size = strlen(data);
	if (phdr->size > 235) {
        //最大长度 PACKET_SIZE - 21 = 235，直接改长度会有前后兼容问题， 超出时暂时把nonce置空
        nonce2[0] = '\0';
        snprintf(data, sizeof(data), "%s;%s;%d;%s;%s;%s;%s;%s;%s;%s;%s;%s;%d;%s;%d;%s;%d;%d;%s;%s;%d;%d;%s;",
			net.hostname, net.ip, net.http_port, net.netmask, net.gateway,
			net.dns,net.sn, net.devicetype, net.software, net.model, net.uptime, nonce2, net.http_port, net.oem_model,
			net.upsec, net.uptone,activeStatus, failover_mode, failover.ipaddr, failover.password, global_device.max_cameras, band, device_info->device_name);
        phdr->size = strlen(data);
    }
	DEBUG("send back msg: %s, size: %d",data, phdr->size);
	req_sendmsg(phdr, ret, data, (void*)device_info);
	
	return 0;
}

static int mssp_reset_nvr(struct mssp_header *phdr, void *args, void *buf)
{
	DEBUG("reset nvr");
	int ret = RERR_NONE;
	char data[256];
	char values[32][128];
	memset(data, 0, sizeof(data));
	memset(values, 0, sizeof(values));

	memcpy(phdr->dst, g_mac, 6);

	switch_from_raw_packet((u_char *) buf, values);
	ret = check_permission(phdr->src, values[0], values[1]);
	if (ret != RERR_NONE)
	{
		phdr->size = 0;
		req_sendmsg(phdr, ret, NULL, args);
		return 0;
	}
	ms_sock_send_msg(SOCKET_TYPE_CORE, REQUEST_FLAG_RESET, 0, 0, 0);
	
	phdr->size = 0;
	req_sendmsg(phdr, ret, NULL, args);
	
	return 0;
}

static int mssp_reboot_nvr(struct mssp_header *phdr, void *args, void *buf)
{
	DEBUG("reboot nvr");
	int ret = RERR_NONE;
	char data[256];
	char values[32][128];
	memset(data, 0, sizeof(data));
	memset(values, 0, sizeof(values));	
	memcpy(phdr->dst, g_mac, 6);
	//printf("[david debug] g_mac %02x:%02x:%02x:%02x:%02x:%02x src %02x:%02x:%02x:%02x:%02x:%02x dst %02x:%02x:%02x:%02x:%02x:%02x\n",
	//	g_mac[0], g_mac[1], g_mac[2], g_mac[3], g_mac[4], g_mac[5], phdr->src[0], phdr->src[1], phdr->src[2], phdr->src[3], phdr->src[4], phdr->src[5],
	//	phdr->dst[0], phdr->dst[1], phdr->dst[2], phdr->dst[3], phdr->dst[4], phdr->dst[5]);
	switch_from_raw_packet((u_char *) buf, values);
	ret = check_permission(phdr->src, values[0], values[1]);
	if (ret != RERR_NONE)
	{
		DEBUG("error: %d", ret);
		phdr->size = 0;
		req_sendmsg(phdr, ret, NULL, args);
		return 0;
	}

	phdr->size = 0;
	req_sendmsg(phdr, ret, NULL, args);
	ret = ms_system("reboot");

	return 0;
}

static int mssp_set_system_time(struct mssp_header *phdr, void *args, void *buf)
{
	int ret = RERR_NONE;
	int cnt = 0;
	char data[256];
	char values[32][128];
	memset(data, 0, sizeof(data));
	memset(values, 0, sizeof(values));
	memcpy(phdr->dst, g_mac, 6);
	DEBUG("set system time  nvr");

	switch_from_raw_packet((u_char *) buf, values);
	char *username = values[(cnt++)];
	char *password = values[(cnt++)];
	ret = check_permission(phdr->src, username, password);
	if (ret != RERR_NONE) 
	{
		phdr->size = 0;
		req_sendmsg(phdr, ret, NULL, args);
		return 0;
	}

    struct system_time systime = {0};
	
    systime.ntp_enable = atoi(values[cnt++]);
    systime.dst_enable = atoi(values[cnt++]);
    snprintf(systime.time_zone,sizeof(systime.time_zone),"%s",values[cnt++]);
    snprintf(systime.time_zone_name, sizeof(systime.time_zone_name), "%s", values[cnt++]);
    snprintf(systime.ntp_server, sizeof(systime.ntp_server), "%s", values[cnt++]);
    systime.keep_time_flag = atoi(values[cnt++]);
    systime.time = atol(values[cnt++]);
	snprintf(systime.sDateTime,sizeof(systime.sDateTime),"%s",values[cnt++]);
    DEBUG("ntp_enalbe: %d, dst_enable: %d, time_zone: %s, time_zone_name: %s, ntp_server: %s, keep_time_flag:%d, time; %ld\n",
    		systime.ntp_enable,systime.dst_enable,systime.time_zone,systime.time_zone_name,systime.ntp_server,systime.keep_time_flag,systime.time);

	struct time t = {0};
	struct req_set_sysconf sys;
	
	memset(&sys, 0, sizeof(sys));
	memset(&t, 0, sizeof(struct time));
	
	t.ntp_enable = systime.ntp_enable;
    t.dst_enable = systime.dst_enable;
	
    snprintf(t.time_zone, sizeof(t.time_zone), "%s", systime.time_zone);
	snprintf(t.time_zone_name, sizeof(t.time_zone_name), "%s", systime.time_zone_name);
	snprintf(t.ntp_server, sizeof(t.ntp_server), "%s", systime.ntp_server);
	
	write_time(SQLITE_FILE_NAME, &t);
	if(systime.ntp_enable== 0 && systime.keep_time_flag == 0)
	{
		snprintf(sys.arg, sizeof(sys.arg), "\"%s\"", systime.sDateTime);
		DEBUG("sys arg = %s\n",sys.arg);			
	}
	memcpy(&global_time, &t, sizeof(struct time));
	
	ret = ms_sock_send_msg(SOCKET_TYPE_CORE, REQUEST_FLAG_SET_SYSTIME, &sys, sizeof(sys), 0);
	phdr->size = 0;
	req_sendmsg(phdr, ret, NULL, args);
	
	DEBUG("set system time  nvr  end!");
	return 0;
}

static int mssp_get_system_time(struct mssp_header *phdr, void *args, void *buf)
{
	DEBUG("get system time  nvr");
	int ret = RERR_NONE;
	char data[256];
	char values[32][128];
	memset(data, 0, sizeof(data));
	memset(values, 0, sizeof(values));
	memcpy(phdr->dst, g_mac, 6);

	switch_from_raw_packet((u_char *) buf, values);
	ret = check_permission(phdr->src, values[0], values[1]);
	if (ret != RERR_NONE) 
	{
		phdr->size = 0;
		req_sendmsg(phdr, ret, NULL, args);
		return 0;
	}

	struct system_time systemTime = {0};
	struct time t = {0};
	
	memcpy(&t, &global_time, sizeof(struct time));
	systemTime.ntp_enable = t.ntp_enable;;
    systemTime.dst_enable = t.dst_enable;;
    snprintf(systemTime.time_zone, sizeof(systemTime.time_zone), "%s", t.time_zone);
	snprintf(systemTime.time_zone_name, sizeof(systemTime.time_zone_name), "%s", t.time_zone_name);
	snprintf(systemTime.ntp_server, sizeof(systemTime.ntp_server), "%s", t.ntp_server);
	systemTime.time = time(NULL);

	time_t tmNow = 0;
	time(&tmNow);
	struct tm *tm_Now;
	tm_Now = localtime(&tmNow);
	strftime(systemTime.sDateTime, sizeof(systemTime.sDateTime), "%Y-%m-%d %H:%M:%S", tm_Now);
	
	snprintf(data, sizeof(data), "%d;%d;%s;%s;%s;%d;%ld;%s;",
			systemTime.ntp_enable, systemTime.dst_enable, systemTime.time_zone, systemTime.time_zone_name, 
			systemTime.ntp_server, systemTime.keep_time_flag,systemTime.time, systemTime.sDateTime);
	phdr->size = strlen(data);
	DEBUG("send back msg: %s, size: %d",data, phdr->size);

	req_sendmsg(phdr, ret, data, args);
	
	return 0;
}

static struct request global_request[] = 
{
	{.req_from = MSSP_GET_NETWORK_INFO, .notify = mssp_get_network},
	{.req_from = MSSP_SET_NETWORK_INFO, .notify = mssp_set_network},
	{.req_from = MSSP_CHECK_PERMISSION, .notify = mssp_check_permission},
	{.req_from = MSSP_RESET_NVR, .notify = mssp_reset_nvr},
	{.req_from = MSSP_REBOOT, .notify = mssp_reboot_nvr},
	{.req_from = MSSP_SET_SYSTEM_TIME, .notify = mssp_set_system_time},
	{.req_from = MSSP_GET_SYSTEM_TIME, .notify = mssp_get_system_time},
};

static void mssp_db_data_update()
{
	global_failover = get_param_int(SQLITE_FILE_NAME, PARAM_FAILOVER_MODE, 0);
	
	db_get_device_oem(SQLITE_FILE_NAME, &global_oem_device);
	read_network(SQLITE_FILE_NAME, &global_network);
	read_port(SQLITE_FILE_NAME, &global_more);
	read_time(SQLITE_FILE_NAME, &global_time);
	read_failover(SQLITE_FILE_NAME, &global_failover_list, 0);
	read_user(SQLITE_FILE_NAME, &global_user, 0);
	
	return ;
}

static void cb_recv(void *arg, struct packet_header *phdr, void *data)
{
    int *flag = (int *)arg;
    char sncode[MAX_LEN_SNCODE] = {0};
    DEBUG("received msg: %d", phdr->type);
    
    switch(phdr->type)
    {
        case RESPONSE_FLAG_GET_SYSINFO: 
            memcpy(&global_device, data, sizeof(global_device)); 
            *flag = 1;
            break;

        case REQUEST_FLAG_UPDATE_SNCODE:
            get_param_value(SQLITE_FILE_NAME, PARAM_DEVICE_SN, sncode, sizeof(sncode), global_device.sncode);
            //DEBUG("sncode changed from %s to %s", global_device.sncode, sncode);
            snprintf(global_device.sncode, sizeof(global_device.sncode), "%s", sncode);
            break;
            
        default:
            break;
    }

    mssp_db_data_update();
    return;
}

void mssp_init()
{
	int try_cnt = 0;
	static int flag = 0;
	get_mac_address(g_mac);
	ms_sock_init(cb_recv, &flag, SOCKET_TYPE_MSSP, SOCK_MSSERVER_PATH);
	
	while (!flag && try_cnt < 10)
	{
		try_cnt++;
		ms_sock_send_msg(SOCKET_TYPE_CORE, REQUEST_FLAG_GET_SYSINFO, 0, 0, 0);
		usleep(300*1000);
	}
	
	if(!flag)
	{
		db_get_device(SQLITE_FILE_NAME, &global_device);
		snprintf(global_device.softver, sizeof(global_device.softver), "ERROR");// tips

		mssp_db_data_update();
	}
	
	req_register(global_request, sizeof(global_request)/sizeof(struct request));

	return ;
}

void mssp_uninit()
{
	struct clilist *pos, *n;
	ms_sock_uninit();
	list_for_each_entry_safe(pos, n, &g_clilist, list)
	{
		list_del(&pos->list);
		free(pos);
	}
	req_unregister(global_request, sizeof(global_request) / sizeof(struct request));

	return;
}
