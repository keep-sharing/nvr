/*
 * ***************************************************************
 * Filename:        poe.c
 * Created at:      2018.8.25
 * Description:     poe API
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "util.h"
#include "poe.h"
#include "switch.h"

#define RED             "\033[0;31m"
#define GREEN           "\033[0;32m"
//#define CLRCOLOR      "\033[0;39m"

pthread_mutex_t g_ctlMtx;

#define POELOG(format, ...) \
    do{\
        printf(GREEN format CLRCOLOR "@func:%s file:%s line:%d\n", \
               ##__VA_ARGS__, __func__, __FILE__, __LINE__); \
    }while(0);

#define POEERR(format, ...) \
    do{\
        printf(RED format CLRCOLOR "@func:%s file:%s line:%d\n", \
               ##__VA_ARGS__, __func__, __FILE__, __LINE__); \
    }while(0);


typedef struct ms_poe_s {
    int     fd;
    float     power[MAX_PORT_NUM];
} MS_POE_S;

static MS_POE_S g_ms_poe;

int poe_get_net_port(char *pMac)
{
    struct  net_port_s stNetPort;
    int ret = 0;

    memset(&stNetPort, 0, sizeof(struct net_port_s));
    string_to_mac(stNetPort.mac, pMac);
    pthread_mutex_lock(&g_ctlMtx);
    ret = ioctl(g_ms_poe.fd, POE_GET_NET_PORT, &stNetPort);
    pthread_mutex_unlock(&g_ctlMtx);
    if (ret) {
        return POE_FAILURE;
    }

    return stNetPort.port;
}

POE_BOOL poe_is_net_link_up(int port)
{
    struct  net_link_s stNetLink;
    int ret = 0;

    if (port > MAX_PORT_NUM) {
        POEERR("port id[%d] over max port id[%d] \n", port, MAX_PORT_NUM);
        return POE_FALSE;
    }
    stNetLink.port = port;
    stNetLink.link = 0;
    pthread_mutex_lock(&g_ctlMtx);
    ret = ioctl(g_ms_poe.fd, POE_IS_NET_LINK, &stNetLink);
    pthread_mutex_unlock(&g_ctlMtx);
    if (ret) {
        POEERR("get port id[%d] net link state failed with %#x \n", port, ret);
        return POE_FALSE;
    }

    return stNetLink.link;
}

int poe_set_net_enable(int port, POE_BOOL bEnable)
{
    struct  net_ctrl_s stNetCtrl;
    int ret = 0;

    if (port > MAX_PORT_NUM) {
        POEERR("port id[%d] over max port id[%d] \n", port, MAX_PORT_NUM);
        return POE_FAILURE;
    }

    stNetCtrl.enable = bEnable;
    stNetCtrl.port = port;
    pthread_mutex_lock(&g_ctlMtx);
    ret = ioctl(g_ms_poe.fd, POE_SET_NET_ENABLE, &stNetCtrl);
    pthread_mutex_unlock(&g_ctlMtx);
    if (ret) {
        POEERR("set port[%d] enable[%d] failed with %#x \n", port, bEnable, ret);
        return POE_FAILURE;
    }

    return POE_SUCCESS;
}

POE_BOOL poe_is_power_link_up(int port)
{
    struct  power_link_s stPowerLink;
    int ret = 0;

    if (port > MAX_PORT_NUM) {
        POEERR("port id[%d] over max port id[%d] \n", port, MAX_PORT_NUM);
        return POE_FALSE;
    }

    stPowerLink.port = port;
    stPowerLink.link = 0;
    pthread_mutex_lock(&g_ctlMtx);
    ret = ioctl(g_ms_poe.fd, POE_IS_POWER_LINK, &stPowerLink);
    pthread_mutex_unlock(&g_ctlMtx);
    if (ret) {
        POEERR("get port id[%d] power link state failed with %#x \n", port, ret);
        return POE_FALSE;
    }

    return stPowerLink.link;
}

float poe_get_power_rate(int port)
{
    struct  power_rate_s stPowerRate;
    int ret = 0;
    float p = 0.0;

    if (port > MAX_PORT_NUM) {
        POEERR("port id[%d] over max port id[%d] \n", port, MAX_PORT_NUM);
        return 0.0;
    }

    stPowerRate.port = port;
    pthread_mutex_lock(&g_ctlMtx);
    ret = ioctl(g_ms_poe.fd, POE_GET_POWER_RATE, &stPowerRate);
    pthread_mutex_unlock(&g_ctlMtx);
    if (ret) {
		
        POEERR("get port id[%d] power rate failed with %#x \n", port, ret);
        return 0.0;
    }
    
    p = stPowerRate.mA * stPowerRate.mV / 1000000.0;
    if (g_ms_poe.power[port-1] + 10.0 < p) {
        p = g_ms_poe.power[port-1] + 10.0;
    }
    
    g_ms_poe.power[port-1] = p;
    return p;
}

int poe_set_power_enable(int port, POE_BOOL bEnable)
{
    struct power_ctrl_s stPowerCtrl;
    int ret;
    if (poe_is_power_link_up(port) == bEnable) {
        return POE_SUCCESS;
    } else {
        stPowerCtrl.enable = bEnable;
        stPowerCtrl.port = port;
        pthread_mutex_lock(&g_ctlMtx);
        ret = ioctl(g_ms_poe.fd, POE_SET_POWER_ENABLE, &stPowerCtrl);
        pthread_mutex_unlock(&g_ctlMtx);
        if (ret) {
            POEERR("set port id[%d] power failed with %#x \n", port, ret);
            return POE_FAILURE;
        }
    }

    return POE_SUCCESS;
}

int poe_init(char *devName, int poeNum)
{
    memset(&g_ms_poe, 0, sizeof(MS_POE_S));

    pthread_mutex_init(&g_ctlMtx, NULL);

    POELOG("init %s POE ...\n", devName);
    if (!strcmp(devName, "MS-N1004-UPC") ||
            !strcmp(devName, "MS-N1004-UPT") ||
            !strcmp(devName, "MS-N1009-UPT") ||
            !strcmp(devName, "MS-N1008-UPC")) {
        g_ms_poe.fd = open("/dev/switch", O_RDWR | O_CLOEXEC, 0);
    } else if (!strcmp(devName, "MS-N5008-UPC") ||
                !strcmp(devName, "MS-N5008-PE")) {
        g_ms_poe.fd = open("/dev/ip179h", O_RDWR | O_CLOEXEC, 0);
    } else if (!strcmp(devName, "MS-N5016-PE") ||
               !strcmp(devName, "MS-N5016-NPE") ||
               !strcmp(devName, "MS-N7016-PG")) {
        g_ms_poe.fd = open("/dev/ip181x_switch", O_RDWR | O_CLOEXEC, 0);
    } else if (!strcmp(devName, "MS-N5016-UPT") ||
              !strcmp(devName, "MS-N5008-UPT") ||
              !strcmp(devName, "MS-N7016-UPH")) {
        g_ms_poe.fd = open("/dev/ip1818a_switch", O_RDWR | O_CLOEXEC, 0);
    } else if (!strcmp(devName, "MS-N7032-UPH") ||
               !strcmp(devName, "MS-N7048-UPH") ||
               !strcmp(devName, "MS-N7048-PG")) {
        if (poeNum == 24) {
            g_ms_poe.fd = open("/dev/ip1829", O_RDWR | O_CLOEXEC, 0);
        } else { //16 port
            g_ms_poe.fd = open("/dev/ip1818a_switch", O_RDWR | O_CLOEXEC, 0);
        }
    } else if (!strcmp(devName, "MS-N1008-UNPC")) {
        g_ms_poe.fd = open("/dev/rtl8309n_switch", O_RDWR | O_CLOEXEC, 0);
    } else if (!strcmp(devName, "MS-N5008-NPE")) {
        g_ms_poe.fd = open("/dev/ip181x_switch_special", O_RDWR | O_CLOEXEC, 0);
    } else {
        POEERR(" open switch %s failed! \n", devName);
        return POE_FAILURE;
    }

    if (g_ms_poe.fd <= 0) {
        POEERR(" open switch %s failed! \n", devName);
        return POE_FAILURE;
    }

    return POE_SUCCESS;
}

int poe_uninit()
{
    if (g_ms_poe.fd != 0) {
        close(g_ms_poe.fd);
        g_ms_poe.fd = 0;
    }

    pthread_mutex_destroy(&g_ctlMtx);

    return POE_SUCCESS;
}

