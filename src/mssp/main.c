#include <pcap/pcap.h>
#include <stdio.h>  
#include <time.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <string.h>
#include <sys/ioctl.h>
#include "mssp.h"
#include "request.h"
#include "mssp_msg.h"
#include "msstd.h"
#include "msdefs.h"
#include "msdb.h"

#define PACKET_SIZE     256
#define SEARCH_PACKET   0x01
#define SEARCH_BACK_PACKET   0x02
#define MODIFY_PACKET   0x03
#define MODIFY_BACK_PACKET   0x04
#define IPADDR_SIZE 16

extern  u_char g_mac[6];
static int global_exit = 0;
static struct timeval last_time;
// unsigned int global_debug_mask = DEBUG_LEVEL_DEFAULT;
// hook_print global_debug_hook = 0;

static struct search_device global_search_device[MAX_DEVICE_NUM];
//static int global_search_mode = 0;

static void signal_handler(int signal)
{
	printf("mssp recv signal %d\n", signal);
	int i = 0;
	switch(signal)
	{
		case SIGINT:
		case SIGTERM:
		case SIGKILL:
		case SIGPIPE:
			global_exit = 1;
			for(i = 0; i < MAX_DEVICE_NUM; i++){
				if(global_search_device[i].task_pcap){
					pcap_breakloop(global_search_device[i].task_pcap);
					usleep(100*1000);
				}
			}
			break;
		default:
			break;
	}	
}

static void set_mssp_signal()
{
	signal(SIGINT,  signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGKILL, signal_handler);
	signal(SIGPIPE, signal_handler);
}

static void fill_packet(struct mssp_header *phdr, int ret, u_char *data, u_char *packet)
{
    int realLen = 0;
	memcpy(packet, phdr->src, 6);
	memcpy(packet+6, phdr->dst, 6);
    packet[12] = 0x80;
    packet[13] = 0x08;

    packet[14] = (u_char)'R';
    packet[15] = (u_char)'E';
    packet[16] = (u_char)'C';
    packet[17] = (u_char)'O';
    packet[18] = 0x00;
    packet[19] = (u_char)phdr->req;

    packet[20] = (u_char)ret;
    if (phdr->size > 0)
    {
        
        if ((phdr->size + 21) <= PACKET_SIZE) {
            realLen = phdr->size;
        } else {
            printf("### len:%d over limit %d . ###\n", phdr->size, (PACKET_SIZE-21));
            realLen = PACKET_SIZE - 21;
        }
		memcpy(packet + 21, data, realLen);
	}

	return ;
}

static int mssp_send(struct mssp_header *phdr, int ret, void *data, void *args)
{
	u_char packet[PACKET_SIZE] = {0};
	search_device *pcap_device = (search_device *)args;
	fill_packet(phdr, ret, (u_char *)data, packet);

	if (pcap_sendpacket((pcap_t *)pcap_device->task_pcap, packet, sizeof(packet))) 
	{
		pcap_perror((pcap_t *)pcap_device->task_pcap, "send packet");
		return -1;
	}
	return 0;
}
  
static void packet_dispatch(u_char *arg, const struct pcap_pkthdr *pkthdr, const u_char *packet)
{
	int timeuse = 0;
	search_device *device_info = (search_device*)arg;
	struct mssp_header hdr;
	static u_char cast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	if(device_info == NULL)
	{
        printf("error:device_info is null\n");
		memcpy(&last_time, &pkthdr->ts, sizeof(struct timeval));
        return;
	}
	else
	{
		timeuse = 1000000 *(pkthdr->ts.tv_sec - last_time.tv_sec) + pkthdr->ts.tv_usec - last_time.tv_usec;
        memcpy(&last_time, &pkthdr->ts, sizeof(struct timeval));
        /* less than 100ms means someone attach*/
        if(timeuse < 100000)
		{
            //pcap_breakloop(device_info->task_pcap);
        }
	}

	if(device_info->mac[0])
	{
		if(memcmp(device_info->mac, packet, 6) && memcmp(cast, packet, 6))
		{
			//printf("===[david debug] packet_dispatch:%s ===\n", device_info->device_name);
			return ;
		}
	}

	if(pkthdr->caplen > 20)
	{
		if(packet[14] == (u_char)'R' || packet[15] == (u_char)'E' 
			|| packet[16] == (u_char)'C' || packet[17] == (u_char)'O')
		{
			int req = packet[18] + packet[19];
			memset(&hdr, 0, sizeof(hdr));
			hdr.req = req;
			memcpy(hdr.src, packet + 6, 6);
			hdr.size = pkthdr->caplen - 20;
			req_dispatch(&hdr, (void*)device_info, (void*)(packet + 21));
		}
	}
}

static int search_listen(void *arg)
{
	char rule[64] = {0};
	char errBuf[PCAP_ERRBUF_SIZE];	
	struct bpf_program filter = {0};
	
	search_device *device_info = (search_device*)arg;
	if(global_exit || device_info->device_enable != 1)
		return -1;

	device_info->task_pcap = pcap_open_live(device_info->device_name, 65535, 0, 0, errBuf);
	if(!device_info->task_pcap)
	{
		printf("error:pcap_open_live():%s\n", errBuf);
		return 0;
	}
	
	snprintf(rule, sizeof(rule), "ether proto 0x8008 and len==%d", PACKET_SIZE);
	if(pcap_compile(device_info->task_pcap, &filter, rule, 1, 0) == -1)
	{
		printf("error: %s pcap_compile():%s\n", device_info->device_name, pcap_geterr(device_info->task_pcap));
		pcap_close(device_info->task_pcap);
		device_info->task_pcap = NULL;
		sleep(1);
		return -1;
	}
	
	if(pcap_setfilter(device_info->task_pcap, &filter) == -1)
	{
		printf("error: %s pcap_setfilter():%s\n", device_info->device_name, pcap_geterr(device_info->task_pcap));
		pcap_freecode(&filter);
		pcap_close(device_info->task_pcap);
		device_info->task_pcap = NULL;
		sleep(1);
		return -1;
	}
	
	pcap_loop(device_info->task_pcap, -1, packet_dispatch, (u_char *)arg);

	pcap_freecode(&filter);
	pcap_close(device_info->task_pcap);
	device_info->task_pcap = NULL;

	sleep(1);
	
	return 0;
}

static void *search_thread(void *arg)
{
	char thread_name[64] = {0};
	search_device *device_info = (search_device*)arg;
	snprintf(thread_name, sizeof(thread_name), "search_%s", device_info->device_name);
	
	if(strcmp(device_info->device_name, "eth1") == 0) 
		sleep(5);
	ms_task_set_name(thread_name);
	usleep(500*1000);
	search_listen((void*)arg);
	ms_task_quit(NULL);
}

int main(int argc, char **argv)
{
	int iCnt = 0;
	int max_poe_num = 0;
	struct network net = {0};
	
	set_mssp_signal();
	mssp_init();

	req_sendmsg_init(mssp_send);

	max_poe_num = get_param_int(SQLITE_FILE_NAME, PARAM_POE_NUM, 0);
	read_network(SQLITE_FILE_NAME, &net);
	memset(global_search_device, 0, sizeof(struct search_device)*MAX_DEVICE_NUM);
	
	if(net.mode != NETMODE_MULTI)
	{
		global_search_device[0].device_mode = net.mode ;
		global_search_device[0].device_enable = net.bond0_enable;
		snprintf(global_search_device[0].device_name, sizeof(global_search_device[0].device_name), "%s", DEVICE_NAME_BOND);
		get_mac(global_search_device[0].device_name, global_search_device[0].mac);
	}
	else
	{
		for(iCnt = 0; iCnt < 2; iCnt++)
		{
			if(max_poe_num && iCnt == 1)
				break;
			global_search_device[iCnt].device_mode = net.mode ;
			if(iCnt == 0)
				global_search_device[iCnt].device_enable = net.lan1_enable;
			if(iCnt == 1)
				global_search_device[iCnt].device_enable = net.lan2_enable;
			snprintf(global_search_device[iCnt].device_name, sizeof(global_search_device[iCnt].device_name), "eth%d", iCnt);
			get_mac(global_search_device[iCnt].device_name, global_search_device[iCnt].mac);
		}
	}

	for(iCnt = 0; iCnt < MAX_DEVICE_NUM; iCnt++)
	{
		printf("mssp iCnt:%d, device_enable:%d\n", iCnt, global_search_device[iCnt].device_enable);
		if(global_search_device[iCnt].device_enable)
		{
			if(ms_task_create(&global_search_device[iCnt].task_handle, THREAD_STACK_512K_SIZE, search_thread, (void*)&global_search_device[iCnt]) != TASK_OK)
			{
				msdebug(DEBUG_ERR, "create search task failed.");
				return -1;
			}
		}
	}
	do
	{
		usleep(500*1000);
	}while(!global_exit);
	
	mssp_uninit();
	return 0;
}
