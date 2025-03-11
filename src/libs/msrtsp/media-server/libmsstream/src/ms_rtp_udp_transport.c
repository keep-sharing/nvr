#include <net/if.h>
#include "ms_rtp_udp_transport.h"
#include "sockpair.h"
#include "ctypedef.h"
#include "port/ip-route.h"
#include "ms_stream_common.h"

int ms_rtp_udp_transport_create(MSRTPUDPTRANSPORT_S **param)
{
    if (!param) {
        rtsp_server_debug(RTSP_WARN, "ms_rtp_udp_transport_create is failed.");
        return -1;
    }
    *param = malloc(sizeof(MSRTPUDPTRANSPORT_S));

    (*param)->m_socket[0] = socket_invalid;
    (*param)->m_socket[1] = socket_invalid;

    return 0;
}

int ms_rtp_udp_transport_destroy(MSRTPUDPTRANSPORT_S *param)
{
    if (!param) {
        rtsp_server_debug(RTSP_WARN, "ms_rtp_udp_transport_destroy is failed.");
        return -1;
    }

    int i = 0;
    for (i = 0; i < 2; i++) {
        if (socket_invalid != param->m_socket[i]) {
            socket_close(param->m_socket[i]);
        }
        param->m_socket[i] = socket_invalid;
    }

    free(param);
    return 0;
}

int ms_rtp_udp_transport_init(MSRTPUDPTRANSPORT_S *param, const char *ip, unsigned short port[2])
{
    if (!param) {
        rtsp_server_debug(RTSP_WARN, "ms_rtp_udp_transport_init is failed.");
        return -1;
    }

    char local[SOCKET_ADDRLEN];
    int r1 = socket_addr_from(&param->m_addr[0], &param->m_addrlen[0], ip, port[0]);
    int r2 = socket_addr_from(&param->m_addr[1], &param->m_addrlen[1], ip, port[1]);
    if (0 != r1 || 0 != r2) {
        return 0 != r1 ? r1 : r2;
    }

    r1 = ip_route_get(ip, local);
    return sockpair_create(0 == r1 ? local : NULL, param->m_socket, port);
}

int ms_rtp_udp_transport_send(MSRTPUDPTRANSPORT_S *param, int rtcp, const void *data, size_t bytes)
{
    if (!param) {
        rtsp_server_debug(RTSP_DEBUG, "ms_rtp_udp_transport_send is failed.");
        return -1;
    }

    int i = rtcp ? 1 : 0;
    return socket_sendto(param->m_socket[i], data, bytes, 0, (struct sockaddr *)&param->m_addr[i], param->m_addrlen[i]);
}

static int ms_rtp_udp_transport_get_local_addr(char *localAddr, int length)
{
    struct ifreq ifr;
    int skfd;
    struct sockaddr_in *saddr;

    if ( (skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        return -1;
    }

    strncpy(ifr.ifr_name, "eth0", 16);
    if (ioctl(skfd, SIOCGIFADDR, &ifr) < 0) {
        close(skfd);
        return -1;
    }
    close(skfd);

    saddr = (struct sockaddr_in *) &ifr.ifr_addr;
    snprintf(localAddr, length, "%s", inet_ntoa(saddr->sin_addr));
    return 0;
}


int ms_rtp_udp_transport_set_multicast(MSRTPUDPTRANSPORT_S *param, const char *multiaddr)
{
    int ttl = 255;
    int i;
    int ret = 0;
    unsigned char loop = 1;
    char interface[32] = {0};
    struct in_addr addr;
    struct ip_mreq mreq;
    int r1, r2, r3, r4;


    if (ms_rtp_udp_transport_get_local_addr(interface, sizeof(interface))) {
        return -1;
    }
    
    for (i = 0; i < 2; i++) {
        //set loop
        r1 = setsockopt(param->m_socket[i], IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));

        //set IF
        addr.s_addr = inet_addr(interface);
        r2 = setsockopt(param->m_socket[i], IPPROTO_IP, IP_MULTICAST_IF, &addr, sizeof(addr));

        //set member
        mreq.imr_interface.s_addr = inet_addr(interface);
        mreq.imr_multiaddr.s_addr = inet_addr(multiaddr);
        r3 = setsockopt(param->m_socket[i], IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));

        //set ttl
        r4 = setsockopt(param->m_socket[i], IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

        if (r1 || r2 || r3 || r4) {
            ret = -1;
            break;
        }
    }

    return ret;
}
