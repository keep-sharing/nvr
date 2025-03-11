修改vlan.c文件
**** ../src/lib/libnl/lib/route/link/vlan.c ****

#include <linux/if_vlan.h>
//找到这里，新增以下3行代码
#ifndef VLAN_FLAG_REORDER_HDR
#define VLAN_FLAG_REORDER_HDR 0x01
#endif