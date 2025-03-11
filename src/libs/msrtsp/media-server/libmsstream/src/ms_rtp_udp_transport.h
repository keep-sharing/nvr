#ifndef _MS_RTP_UDP_TRANSPORT_H_
#define _MS_RTP_UDP_TRANSPORT_H_

//include file
#include "sys/sock.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct MSRtpUdpTransport {
    socket_t m_socket[2];
    socklen_t m_addrlen[2];
    struct sockaddr_storage m_addr[2];
} MSRTPUDPTRANSPORT_S;

/////////start////////
////////function
int ms_rtp_udp_transport_create(MSRTPUDPTRANSPORT_S **param);
int ms_rtp_udp_transport_destroy(MSRTPUDPTRANSPORT_S *param);
int ms_rtp_udp_transport_init(MSRTPUDPTRANSPORT_S *param, const char *ip, unsigned short port[2]);
int ms_rtp_udp_transport_send(MSRTPUDPTRANSPORT_S *param, int rtcp, const void *data, size_t bytes);
int ms_rtp_udp_transport_set_multicast(MSRTPUDPTRANSPORT_S *param, const char *multiaddr);





/////////end////////

#ifdef __cplusplus
}
#endif

#endif /* !ms_rtp_udp_transport_h */
