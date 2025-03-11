/*
 * ptz_op.c
 *
 *  Created on: 2012-11-22
 *      Author: chimmu
 */
#include "ptz_op.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "msstd.h"

struct ptz_srl *g_srl_info;
int g_fd = -1;
static int g_chn_cnt;

int ptz_srl_init(int num, const char *dev)
{
	if (open_comm(dev))
		return -1;
	int i;
//	printf("open tty01 done\n");
	g_srl_info = (struct ptz_srl *)ms_malloc(num * sizeof(struct ptz_srl));
	if (NULL == g_srl_info) {
		perror("ms_malloc");
		return -1;
	}
//	printf("111111111111111111111111111\n");
	memset(g_srl_info, 0, num * sizeof(struct ptz_srl));
	for (i = 0; i < num; i++)
		ptz_init_serial_info(&g_srl_info[i].srl_info);
//	printf("22222222222222222222\n");
	g_chn_cnt = num;
	return 0;
}

int ptz_srl_deinit(void)
{

	int i;
	if (g_srl_info) {
		for (i = 0; i < g_chn_cnt; i++) {
			if (g_srl_info[i].info) {
				ms_free(g_srl_info[i].info);
				g_srl_info[i].info = NULL;
			}
		}
		ms_free(g_srl_info);
		g_srl_info = NULL;
	}
	if (g_fd > 0)
    	close(g_fd);
	return 0;
}


int create_comm(PTZ_PROTOCOL_E nProtocolType, int chn, int addr)
{	
	int flag = 0;
    
	g_srl_info[chn].info = MsPtzInfoCreate(nProtocolType, addr);
//	printf("create info:chn: %d, %p\n", chn, g_srl_info[chn].info);
	if (g_srl_info[chn].info == NULL)
		flag = -1;
	return flag;
}

int open_comm(const char *path)
{
    if((g_fd = open(path, O_RDWR)) < 0){
		perror("ptz_op.c::open error:");
        return -1;
    }
    ptz_serialInfo info;
    ptz_init_serial_info(&info);
    ptz_set_serial_info(g_fd, &info);
    return 0;
}

int write_comm(int chn, unsigned char *buf, int len)
{
	if (!buf) {
		printf("write_comm error, buf is NULL\n");
		return -1;
	}
	if (write(g_fd, buf, len) < 0) {
		perror("write error:");
		return -1;
	}
	return 0;
}

int set_info_comm(int chn, PTZ_PROTOCOL_E proto, int addr)
{
    return MsPtzSetInfo(g_srl_info[chn].info, proto, addr);
}

int set_srl_comm(int chn)
{
	return ptz_set_serial_info(g_fd, &g_srl_info[chn].srl_info);
}

int set_speed_comm(int chn,  int pspd, int tspd, int zspd, int fspd)
{
    return MsPtzSetSpeed(g_srl_info[chn].info, pspd, tspd, zspd, fspd);
}

int read_comm(int chn, void* buf, int len)
{
    return read(g_fd, buf, len);
}

int info_del_comm(int chn)
{
	return MsPtzInfoDestroy(g_srl_info[chn].info);
}
