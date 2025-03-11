/*
 * ptz_op.h
 *
 *  Created on: 2012-11-22
 *      Author: chimmu
 */

#ifndef PTZ_OP_H_
#define PTZ_OP_H_

#include "MsPtz.h"

#define SRLCHN_MAX	16

typedef struct ms_ptz_info ptz_info;
typedef MS_PTZ_PROTOCOL PTZ_PROTOCOL_E;

struct ptz_srl{
	ptz_info *info;
	ptz_serialInfo srl_info;//for setting serial info
};

#ifdef __cplusplus
extern "C"
{
#endif

int create_comm(PTZ_PROTOCOL_E nProtocolType, int chn, int addr);
int write_comm(int chn, unsigned char *buf,int len);
//int preset_comm(prefn, int fd, ptz_info* buf, int preNo);
int set_speed_comm(int chn,  int pspd, int tspd, int zspd, int fspd);
int set_info_comm(int chn, PTZ_PROTOCOL_E proto, int addr);
int set_srl_comm(int chn);
int open_comm(const char *path);
int read_comm(int chn, void *buf, int len);
int info_del_comm(int chn);
int ptz_srl_deinit(void);
int ptz_srl_init(int num, const char *dev);

#ifdef __cplusplus
}
#endif

#endif /* PTZ_OP_H_ */
