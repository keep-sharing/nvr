#ifndef __MSSP__H__
#define __MSSP__H__

#include <pcap/pcap.h>
#include "msstd.h"

typedef struct search_device
{
	TASK_HANDLE task_handle;
	int device_enable;
	int device_mode;
	char device_name[10];
	unsigned char mac[6];
	pcap_t *task_pcap;
}search_device;

void mssp_init();
void mssp_uninit();
int get_network_card(char card[], int len);
int get_mac(char *device_name, u_char *pMacAddress);

#endif
