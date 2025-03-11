#ifndef __SWITCH__
#define __SWITCH__

#endif

struct net_port_s
{
	unsigned int port;
    unsigned char mac[6];
};

struct  net_link_s
{
	unsigned int port;
	unsigned int link;
	
};

struct net_ctrl_s
{
	unsigned int port;
	unsigned int enable;
};

struct power_link_s
{
	unsigned int port;
	unsigned int link;
};

struct power_rate_s
{   
	unsigned int port;
    unsigned int mA;
    unsigned int mV;
};

struct power_ctrl_s
{
	unsigned int port;
	unsigned int enable;
};


#define SWITCH_MAGIC 'S'
#define POE_GET_NET_PORT	_IOWR(SWITCH_MAGIC, 1, struct  net_port_s)
#define POE_IS_NET_LINK   _IOWR(SWITCH_MAGIC, 2, struct  net_link_s)
#define POE_SET_NET_ENABLE _IOWR(SWITCH_MAGIC, 3, struct  net_ctrl_s)

#define POE_IS_POWER_LINK  _IOWR(SWITCH_MAGIC, 4, struct power_link_s)
#define POE_GET_POWER_RATE _IOWR(SWITCH_MAGIC, 5, struct power_rate_s)
#define POE_SET_POWER_ENABLE _IOWR(SWITCH_MAGIC, 6, struct power_ctrl_s)


