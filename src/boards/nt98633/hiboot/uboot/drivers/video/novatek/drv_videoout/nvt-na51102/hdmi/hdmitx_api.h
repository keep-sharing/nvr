#ifndef __HDMITX_API_H_
#define __HDMITX_API_H_
#include "hdmitx_drv.h"

int nvt_hdmitx_api_write_reg(PHDMITX_MODULE_INFO pmodule_info, unsigned char ucargc, char **pucargv);
//int nvt_hdmitx_api_write_pattern(PHDMITX_MODULE_INFO pmodule_info, unsigned char ucargc, char **pucargv);
int nvt_hdmitx_api_read_reg(PHDMITX_MODULE_INFO pmodule_info, unsigned char ucargc, char **pucargv);

#endif
