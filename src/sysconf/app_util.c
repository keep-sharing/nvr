#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <ctype.h>

//# for network info
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <paths.h>

#include "msstd.h"
#include "app_util.h"


#define eprintf(x...)	printf("err: " x);
//#define dprintf(x...)	printf(x);
#define dprintf(x...)

char chrtbl[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
static int chrtbl_size = sizeof(chrtbl) / sizeof(chrtbl[0]);

/*----------------------------------------------------------------------------
 Network Info
-----------------------------------------------------------------------------*/
static int gateway_info(char *dev, char *gateway, int set)
{
	FILE *fp;
	unsigned char buf[128], gate[16];
	unsigned char *find;

	//# get gateway
	sprintf((char *)buf, "route -n | grep 'UG' | grep %s | awk '{print $2}'", dev);//sprintf(buf, "route -n | grep 'UG[ \t]' | grep %s | awk '{print $2}'", dev);

	fp = ms_vpopen((const char *)buf, "r");
	if(NULL == fp) {
		eprintf("ms_vpopen error (%s)\n", buf);
		return -1;
	}

	if(!fgets((char *)gate, 16, fp))	{
		strcpy((char *)gate, "0.0.0.0");
	}
	else {
		find = (unsigned char *)strchr((const char *)gate,'\n');	//# remove '\n'
		if(find) *find='\0';
	}
	ms_vpclose(fp);

	if(set)	//# set gateway
	{
		printf("set gateway\n");
		if(strcmp((char *)gate, "0.0.0.0")) {
			sprintf((char *)buf, "route del default gw %s %s", gate, dev);
			ms_system((char *)buf);
		}
		sprintf((char *)buf, "route add default gw %s %s", gateway, dev);
		ms_system((char *)buf);
		printf("buf=%s\n", buf);
	}
	else
	{
		strcpy(gateway, (char *)gate);
	}

	return 0;
}

int get_net_info(const char *ifname, dvr_net_info_t *inet)
{
	int ret, fd;
	char dev[8];
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	/* I want to get an IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;

	/* I want IP address attached to "eth0" */
	sprintf(dev, "%s", ifname);
	strncpy(ifr.ifr_name, dev, IFNAMSIZ-1);

	//# check up/down
	ioctl(fd, SIOCGIFFLAGS, &ifr);
	inet->state = ifr.ifr_flags & IFF_UP;

	#if 1
	if(!inet->state) {	//# down
		close(fd);
		strcpy(inet->ip, "0.0.0.0");
		strcpy(inet->mask, "255.255.255.0");
		strcpy(inet->gate, "0.0.0.0");
		return 0;
	}
	#endif

	ret = ioctl(fd, SIOCGIFADDR, &ifr);
	if(ret<0)	strcpy(inet->ip, "0.0.0.0");
	else		sprintf(inet->ip, "%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

	ret = ioctl(fd, SIOCGIFNETMASK, &ifr);
	if(ret<0)	strcpy(inet->mask, "255.255.255.0");
	else		sprintf(inet->mask, "%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

	close(fd);

	gateway_info(dev, inet->gate, 0);

	return 0;
}

int get_dhcp_client_pid(const char *ifname)
{
	FILE *fp;
	char cmd[128] = {0};
	char pid[128] = {0};
	
	snprintf(cmd, sizeof(cmd), "ps | grep dhcpcd | grep %s | grep -v grep | awk '{print $1}'", ifname);
	if ((fp = ms_vpopen(cmd, "r")) == NULL) {
		return -1;
	}
	
	fgets(pid, sizeof(pid), fp);
	ms_vpclose(fp);
	
	return atoi(pid);
}
	
int set_net_info(const char *ifname, dvr_net_info_t *inet)
{
	int ret = -1, fd;
	char dev[8], cmd[128];
	struct ifreq ifr;
  	struct sockaddr_in sin;

  	sprintf(dev, "%s", ifname);

  	fd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name, dev, IFNAMSIZ);

	//# check up/down
	ioctl(fd, SIOCGIFFLAGS, &ifr);
	inet->state = ifr.ifr_flags & IFF_UP;
	if(!inet->state) {	//# down
		ifr.ifr_flags |= IFF_UP;
		ioctl(fd, SIOCSIFFLAGS, &ifr);
		sleep(1);
	}

  	if(inet->type == NET_DHCP)
  	{	
  		int pid = get_dhcp_client_pid(ifname);
  		if (pid > 0) 
		{
  			snprintf(cmd, sizeof(cmd), "kill %d;rm -rf /etc/dhcpc/dhcpcd-%s.pid;ifconfig %s up", pid, dev, dev);
  			system(cmd);
  		}
		sprintf(cmd, "dhcpcd %s", dev);
  		ret = ms_system(cmd);
  	}
  	else if(inet->type == NET_STATIC)
  	{
  		int pid = get_dhcp_client_pid(ifname);
  		if (pid > 0) 
		{
  			snprintf(cmd, sizeof(cmd), "kill %d;rm -rf /etc/dhcpc/dhcpcd-%s.pid;ifconfig %s up", pid, dev, dev);
  			system(cmd);
  		}
		//memset(&sin, 0, sizeof(struct sockaddr));
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = inet_addr(inet->ip);
		memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));
		ioctl(fd, SIOCSIFADDR, &ifr);

		sin.sin_addr.s_addr = inet_addr(inet->mask);
		memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));
		ret = ioctl(fd, SIOCSIFNETMASK, &ifr);
		if(ret < 0)
			dprintf("netmask: Invalid argument\n");
		printf("gateway_info\n");
		gateway_info(dev, inet->gate, 1);
	}

	close(fd);

	return ret;
}

int check_ip(const char *ipaddr)
{
	if (ipaddr == NULL || ipaddr[0] == 0) return -1;
	
	int i = 0;
	int len = strlen(ipaddr);
	int pcnt = 0;
	int num = 0;
	int flag = 0;
	
	for (i = 0; i < len; i++) {
		if (isdigit(ipaddr[i])) {
			num = 10 * num + (ipaddr[i] - '0');
			flag = 0;

			if (num > 255) return -1;
		} else if (ipaddr[i] == '.') {
			pcnt++;

			if (!flag) {
				num = 0;
				flag = 1;
			} else {
				return -1;
			}
		} else {
			return -1;
		}
	}
	
	return (pcnt == 3) ? 0 : -1;
}

int get_timezone(char *timezone, int size)
{
	FILE *fp;
	
	if ((fp = fopen("/etc/TZ", "r")) == NULL) return -1;
	
	fgets(timezone, size, fp);
	fclose(fp);
	
	if (timezone[strlen(timezone) - 1] == '\n') {
		timezone[strlen(timezone) - 1] = 0;
	}
	
	return 0;
}

static UInt_64 now_usec(void)
{
	struct timeval tv = {0};  
  
	gettimeofday(&tv, NULL);  
	return (UInt_64)(tv.tv_sec * 1000000) + tv.tv_usec;
}
  
int generate_rand_str(char *str, int size)  
{
	if (size <= 0) return -1;
	
	int i, mode;  
  
	srand(now_usec());  
	mode = chrtbl_size - 1;  
  
	for (i = 0; i < size - 1; i++) {  
		str[i] = chrtbl[rand() % mode];  
	}
      
	str[size - 1] = 0;  
	
	return 0;
}

