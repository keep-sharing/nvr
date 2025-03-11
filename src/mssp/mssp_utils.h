#ifndef _COMMON_H_
#define _COMMON_H_
#include "pcap/pcap.h"

#define UPGRADE_PORT	9527

void switch_from_raw_packet(const u_char *packet, char values[][128]);
void get_random_md5str(char *dest, int dstsize);

#endif
