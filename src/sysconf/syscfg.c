#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>


#include <pthread.h>
#include "syscfg.h"
#include "app_util.h"
#include "ccsqlite.h"
#include <sqlite3.h>

#include "datatran.h"
#include "emaildb.h"
#include "msoem.h"
#include "msstd.h"

#include <dirent.h>
/* --------------------------------------------------------------------------------------------------- */
#define DB_INIT_VERSION 1000
#define NETIF_CFG_FILE "/etc/network/interfaces"
#define BONDIFNAME "bond0"

#define NETWORK_CFG_PATH (MS_ETC_DIR "/network.conf")
#define HOSTNAME_CFG_PATH ("/etc/HOSTNAME")
#define HOSTS_CFG_PATH ("/etc/hosts")

#define NETWORK_CFG_FMT "\
MODE=%d\n\
BONDIFNAME=%s\n\
BONDIFTYPE=%d\n\
BONDIFADDR=%s\n\
BONDIFMASK=%s\n\
BONDIFGATEWAY=%s\n\
BONDIFMTU=%d\n\
BONDIFMIIMON=100\n\
PRIMARY=%d\n\
IF0=%s\n\
IF1=%s\n"

#define DEF_LANG_TEXT "{\"defaultLan\":\"%s\",\"list\":["
//#define DEF_LANG_TEXT "{\"defaultLan\":\"%s\",\"list\":[{\"value\":\"en-US\",\"name\":\"English\",\"isDefault\":true},{\"value\":\"zh-CN\",\"name\":\"中文\",\"isDefault\":false},{\"value\":\"zh-TW\",\"name\":\"繁體中文\",\"isDefault\":false},{\"value\":\"hu-HU\",\"name\":\"Magyar\",\"isDefault\":false},{\"value\":\"ru-RU\",\"name\":\"Russian\",\"isDefault\":false},{\"value\":\"fr-FR\",\"name\":\"French\",\"isDefault\":false},{\"value\":\"pl-PL\",\"name\":\"Polski\",\"isDefault\":false},{\"value\":\"dk-DK\",\"name\":\"Danish\",\"isDefault\":false},{\"value\":\"it-IT\",\"name\":\"Italian\",\"isDefault\":false},{\"value\":\"ge-GE\",\"name\":\"German\",\"isDefault\":false}]}"

#define POE_DEFAULT_IPADDR  "192.168.20.1"
#define POE_DEFAULT_GW      "192.168.20.1"

#define PPPOECFG_PATH       "/opt/app/pppoe/pppoe.conf"
#define PPPOE_RESOLV_PATH   "/etc/resolv.conf"
#define PPPOE_PAP_PATH      "/opt/app/pppoe/pap-secrets"
#define PPPOE_CHAP_PATH     "/etc/ppp/chap-secrets"     //must /etc/ppp
#define PPPOE_SCRIPT_PATH   "/opt/app/pppoe/"


////////////////////////////////////////////////////////////////////////////////////////////////////////
//function

static int write_netif_cfg(FILE *fp, const char *ifname, NETWORKINFO *netif)
{
    if (netif == NULL) {
        fprintf(fp, "auto %s\n", ifname);
        fprintf(fp, "iface %s inet static\naddress 0.0.0.0\nnetmask 255.255.255.0\ngateway 0.0.0.0\nmtu 1500\n\n", ifname);
    } else {
        printf("iface %s inet static\naddress %s\nnetmask %s\ngateway %s\nmtu %d\n\n",
               ifname, netif->ipaddr,  netif->netmask, netif->gateway,  netif->mtu);
        if (netif->enable == 0) {
            return -1;
        }

        fprintf(fp, "auto %s\n", ifname);


        if (netif->type == 0) { //ipv4
            if (strcmp(netif->gateway, "") == 0) {
                fprintf(fp, "iface %s inet static\naddress %s\nnetmask %s\nmtu %d\n\n",
                        ifname, netif->ipaddr,  netif->netmask,  netif->mtu);
            } else {
                printf("iface %s inet static\naddress %s\nnetmask %s\ngateway %s\nmtu %d\n\n",
                       ifname, netif->ipaddr,  netif->netmask, netif->gateway,  netif->mtu);
                fprintf(fp, "iface %s inet static\naddress %s\nnetmask %s\ngateway %s\nmtu %d\n\n",
                        ifname, netif->ipaddr,  netif->netmask,  netif->gateway, netif->mtu);
            }
        } else if (netif->type == 1) {
            //【Settings-Network：多址模式下，设置两网卡都是DHCP获取，两网卡网线都接在同一交换机上，能获取到ip，但是NVR无法正常启动。】https://www.tapd.cn/21417271/bugtrace/bugs/view?bug_id=1121417271001055414
            //fprintf(fp, "iface %s inet dhcp\nmtu %d\n\n", ifname, netif->mtu);
        }

        if (netif->ip6type == 0) { //hrz.milesight ipv6 dhcp
            //if (netif->ip6addr[0] && netif->ip6netmask[0] && netif->ip6gateway[0])//hrz.milesight
            if (netif->ip6addr[0] && netif->ip6netmask[0]) { //gateway can be null
                printf("iface %s inet6 static\naddress %s\nnetmask %s\ngateway %s\nmtu %d\n\n",
                       ifname, netif->ip6addr,  netif->ip6netmask, netif->ip6gateway,  netif->mtu);
                if (netif->ip6gateway[0]) {
                    fprintf(fp, "iface %s inet6 static\naddress %s\nnetmask %s\ngateway %s\nmtu %d\n\n",
                            ifname, netif->ip6addr,  netif->ip6netmask, netif->ip6gateway, netif->mtu);
                } else {
                    fprintf(fp, "iface %s inet6 static\naddress %s\nnetmask %s\nmtu %d\n\n",
                            ifname, netif->ip6addr,  netif->ip6netmask, netif->mtu);
                }
            }
        } else if (netif->ip6type == 1) { //Router Advertisement
            if (netif->ip6addr[0] && netif->ip6netmask[0]) { //gateway can be null
                printf("iface %s inet6 static\naddress %s\nnetmask %s\ngateway %s\nmtu %d\n\n",
                       ifname, netif->ip6addr,  netif->ip6netmask, netif->ip6gateway,  netif->mtu);
                if (netif->ip6gateway[0]) {
                    fprintf(fp, "iface %s inet6 static\naddress %s\nnetmask %s\ngateway %s\nmtu %d\n\n",
                            ifname, netif->ip6addr,  netif->ip6netmask, netif->ip6gateway, netif->mtu);
                } else {
                    fprintf(fp, "iface %s inet6 static\naddress %s\nnetmask %s\nmtu %d\n\n",
                            ifname, netif->ip6addr,  netif->ip6netmask, netif->mtu);
                }
            }
        } else { //dhcp
            //fprintf(fp, "iface %s inet6 dhcp\nmtu %d\n\n", ifname, netif->mtu);
        }//end

    }

    return 0;
}

static void apply_netif_change(NETWORKINFO *src, dvr_net_info_t *dst, const char *ifname)
{
    char cmd[128] = {0};
    int pid;
    printf("get %s info\n", ifname);
    get_net_info(ifname, dst);
    dst->type = src->type;
    snprintf(dst->ip, sizeof(dst->ip), "%s", src->ipaddr);
    snprintf(dst->mask, sizeof(dst->mask), "%s", src->netmask);
    snprintf(dst->gate, sizeof(dst->gate), "%s", src->gateway);

    if (src->enable == 1) {
        if (check_ip(src->dns1) == 0) {
            snprintf(cmd, sizeof(cmd), "echo nameserver %s >> %s/resolv.conf;", src->dns1, MS_ETC_DIR);
        }
        if (check_ip(src->dns2) == 0) {
            snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), "echo nameserver %s >> %s/resolv.conf;", src->dns2, MS_ETC_DIR);
        }
        ms_system(cmd);
    }

    if (dst->state == 0) {
        if (src->enable == 1) {
            printf("ifconfig mtu\n");
            snprintf(cmd, sizeof(cmd), "ifconfig %s mtu %d up > /dev/null;", ifname, src->mtu);
            ms_system(cmd);
            printf("cmd1=%s\n", cmd);
            set_net_info(ifname, dst);
        }
    } else {
        if (src->enable == 0) {
            pid = get_dhcp_client_pid(ifname);
            if (pid > 0) {
                snprintf(cmd, sizeof(cmd), "kill %d\n", pid);
                ms_system(cmd);
            }
            snprintf(cmd, sizeof(cmd), "ifconfig %s down > /dev/null", ifname);
            printf("cmd2=%s\n", cmd);
            ms_system(cmd);
        } else {
            set_net_info(ifname, dst);
            snprintf(cmd, sizeof(cmd), "ifconfig %s mtu %d", ifname, src->mtu);
            printf("cmd3=%s\n", cmd);
            ms_system(cmd);
        }
    }
}
static int write_resolv_cfg(NETWORKINFO *src)
{
    char cmd[128] = {0};
    if (src == NULL) {
        return -1;
    }
    if (src->enable == 1) {
        if (check_ip(src->dns1) == 0) {
            snprintf(cmd, sizeof(cmd), "echo nameserver %s >> %s/resolv.conf;", src->dns1, MS_ETC_DIR);
        }
        if (check_ip(src->dns2) == 0) {
            snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), "echo nameserver %s >> %s/resolv.conf;", src->dns2, MS_ETC_DIR);
        }
        ms_system(cmd);
    }

    return 0;
}

static int write_network_cfg(NETWORKCFG *netcfg)
{
    FILE *fp = NULL;
    if (netcfg == NULL) {
        return -1;
    }

    if ((fp = fopen(NETWORK_CFG_PATH, "w")) == NULL) {
        return -1;
    }

    lockf(fileno(fp), F_LOCK, 0);
    fprintf(fp, NETWORK_CFG_FMT, (netcfg->mode - 1), BONDIFNAME, netcfg->bondcfg.bondif.type,
            netcfg->bondcfg.bondif.ipaddr, netcfg->bondcfg.bondif.netmask,
            netcfg->bondcfg.bondif.gateway, netcfg->bondcfg.bondif.mtu, netcfg->bondcfg.primary, DEVICE_NAME_ETH0,
            DEVICE_NAME_ETH1);
    fflush(fp);
    lockf(fileno(fp), F_ULOCK, 0);

    fclose(fp);

    return 0;
}

static int network_set_dhcpv6(const char *ifname)
{
    char cmd[128] = {0};

    snprintf(cmd, sizeof(cmd), "dhcp6c -f %s -p /tmp/dhcp6c_%s.pid &", ifname, ifname);
    ms_system(cmd);

    //printf("[debug] cmd:%s\n", cmd);
    return 0;
}

static int network_show_mac_addr(char *eth)
{
    struct ifreq req;
    int sock = 0;
    char mac[32];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("error sock");
        return 2;
    }

    strcpy(req.ifr_name, eth);
    if (ioctl(sock, SIOCGIFHWADDR, &req) < 0) {
        perror("error ioctl");
        return 3;
    }

    int i = 0;
    for (i = 0; i < 6; i++) {
        sprintf(mac + 3 * i, "%02X:", (unsigned char)req.ifr_hwaddr.sa_data[i]);
    }
    mac[strlen(mac) - 1] = 0;
    printf("[%s] MAC: %s\n", eth, mac);
    return 0;
}


int ms_set_hostname(const char *hostname)
{
    char cmd[128] = {0};

    snprintf(cmd, sizeof(cmd), "echo \"%s\" > %s", hostname, HOSTNAME_CFG_PATH);
    ms_system(cmd);

    snprintf(cmd, sizeof(cmd), "echo 127.0.0.1 hdvr.com localhost > %s;echo 127.0.0.1 %s >> %s", HOSTS_CFG_PATH, hostname,
             HOSTS_CFG_PATH);
    ms_system(cmd);

    snprintf(cmd, sizeof(cmd), "hostname -F %s", HOSTNAME_CFG_PATH);
    return ms_system(cmd);
}

/*dhcp update dns ipaddr network gateway*/
static void update_dhcp_config(struct network *networkDb)
{
    int changed = 0;
    int obtained = 0;
    char cmd[256] = {0};
    if (networkDb->mode == NETMODE_MULTI) {
        if (networkDb->lan1_enable && networkDb->lan1_type == NET_DHCP) {
            changed = 1;
            net_get_dhcp_dns(DEVICE_NAME_ETH0, networkDb->lan1_primary_dns, sizeof(networkDb->lan1_primary_dns));
            if (net_get_ifaddr(DEVICE_NAME_ETH0, networkDb->lan1_ip_address, sizeof(networkDb->lan1_ip_address)) == 0) {
                obtained = 1;
            }

            if (net_get_netmask(DEVICE_NAME_ETH0, networkDb->lan1_netmask, sizeof(networkDb->lan1_netmask)) == 0) {
                obtained = 1;
            }

            if (net_get_gateway(DEVICE_NAME_ETH0, networkDb->lan1_gateway, sizeof(networkDb->lan1_gateway)) == 0) {
                obtained = 1;
            }
            if (!obtained) {
                snprintf(networkDb->lan1_ip_address, sizeof(networkDb->lan1_ip_address), "192.168.5.200");
                snprintf(networkDb->lan1_netmask, sizeof(networkDb->lan1_netmask), "255.255.255.0");
                snprintf(networkDb->lan1_gateway, sizeof(networkDb->lan1_gateway), "192.168.5.1");
                snprintf(networkDb->lan1_primary_dns, sizeof(networkDb->lan1_primary_dns), "8.8.8.8");
                snprintf(cmd, sizeof(cmd), "ifconfig %s %s netmask %s", DEVICE_NAME_ETH0, "192.168.5.200" ,"255.255.255.0");
                ms_system(cmd);
                snprintf(cmd, sizeof(cmd), "route add default gw %s %s", "192.168.5.1", DEVICE_NAME_ETH0);
                ms_system(cmd);
            }
        }

        if (networkDb->lan2_enable && networkDb->lan2_type == NET_DHCP) {
            changed = 1;
            net_get_dhcp_dns(DEVICE_NAME_ETH1, networkDb->lan2_primary_dns, sizeof(networkDb->lan2_primary_dns));
            if (net_get_ifaddr(DEVICE_NAME_ETH1, networkDb->lan2_ip_address, sizeof(networkDb->lan2_ip_address)) == 0) {
                obtained = 1;
            }

            if (net_get_netmask(DEVICE_NAME_ETH1, networkDb->lan2_netmask, sizeof(networkDb->lan2_netmask)) == 0) {
                obtained = 1;
            }

            if (net_get_gateway(DEVICE_NAME_ETH1, networkDb->lan2_gateway, sizeof(networkDb->lan2_gateway)) == 0) {
                obtained = 1;
            }
            if (!obtained) {
                snprintf(networkDb->lan2_ip_address, sizeof(networkDb->lan2_ip_address), "192.168.10.200");
                snprintf(networkDb->lan2_netmask, sizeof(networkDb->lan2_netmask), "255.255.255.0");
                snprintf(networkDb->lan2_gateway, sizeof(networkDb->lan2_gateway), "192.168.10.1");
                snprintf(networkDb->lan2_primary_dns, sizeof(networkDb->lan2_primary_dns), "8.8.8.8");
                snprintf(cmd, sizeof(cmd), "ifconfig %s %s netmask %s", DEVICE_NAME_ETH1, "192.168.10.200" ,"255.255.255.0");
                ms_system(cmd);
                snprintf(cmd, sizeof(cmd), "route add default gw %s %s", "192.168.10.1", DEVICE_NAME_ETH1);
                ms_system(cmd); 
            }
        }
    } else {
        if (networkDb->bond0_type == NET_DHCP) {
            changed = 1;
            net_get_dhcp_dns(DEVICE_NAME_BOND, networkDb->bond0_primary_dns, sizeof(networkDb->bond0_primary_dns));
            if (net_get_ifaddr(DEVICE_NAME_BOND, networkDb->bond0_ip_address, sizeof(networkDb->bond0_ip_address)) == 0) {
                obtained = 1;
            }

            if (net_get_netmask(DEVICE_NAME_BOND, networkDb->bond0_netmask, sizeof(networkDb->bond0_netmask)) == 0) {
                obtained = 1;
            }

            if (net_get_gateway(DEVICE_NAME_BOND, networkDb->bond0_gateway, sizeof(networkDb->bond0_gateway)) == 0) {
                obtained = 1;
            }
            if (!obtained) {
                snprintf(networkDb->bond0_ip_address, sizeof(networkDb->bond0_ip_address), "192.168.5.200");
                snprintf(networkDb->bond0_netmask, sizeof(networkDb->bond0_netmask), "255.255.255.0");
                snprintf(networkDb->bond0_gateway, sizeof(networkDb->bond0_gateway), "192.168.5.1");
                snprintf(networkDb->bond0_primary_dns, sizeof(networkDb->bond0_primary_dns), "8.8.8.8");
                snprintf(cmd, sizeof(cmd), "ifconfig %s %s netmask %s", DEVICE_NAME_BOND, "192.168.5.200" ,"255.255.255.0");
                ms_system(cmd);
                snprintf(cmd, sizeof(cmd), "route add default gw %s %s", "192.168.5.1", DEVICE_NAME_BOND);
                ms_system(cmd);  
            }
        }
    }

    if (changed) {
        write_network(SQLITE_FILE_NAME, networkDb);
    }

    return;
}

int ms_set_network(struct network *networkDb)
{
    char cmd[256] = {0};
    FILE *fp;

    NETWORKCFG tmpcfg;
    memset(&tmpcfg, 0, sizeof(tmpcfg));
    netDbToInternal(networkDb, &tmpcfg);

    snprintf(cmd, sizeof(cmd), "rm -rf %s/resolv.conf", MS_ETC_DIR);
    ms_system(cmd);
    ms_system("ifdown -af");
    ms_system("killall -9 dhcpcd");
    ms_system("killall -9 dhcp6c");//hrz.milesight
    //ms_system("rm -f /tmp/dhcp6c.pid");
    ms_system("rm -f /tmp/dhcp6c_eth0.pid");
    ms_system("rm -f /tmp/dhcp6c_eth1.pid");
    ms_system("rm -f /tmp/dhcp6c_bond0.pid");
    ms_system("/etc/init.d/bond stop");
    ms_system("rm -rf /etc/network/interfaces");

    if (tmpcfg.mode != NETMODE_MULTI) {
        printf("tmpcfg.mode=%d bondif\n", tmpcfg.mode);
        printf("tmpcfg.tmpcfg.netif1.gateway=%s bondif\n", tmpcfg.netif1.gateway);
        printf("tmpcfg.tmpcfg.netif2.gateway=%s bondif\n", tmpcfg.netif2.gateway);
        if ((fp = fopen(NETIF_CFG_FILE, "wb")) == NULL) {
            perror("open failed.");
            printf("open network interface configure file failed.\n");
            return -1;
        }
        lockf(fileno(fp), F_LOCK, 0);
        fprintf(fp, "#The loopback interface\nauto lo\niface lo inet loopback\n\n");
        write_netif_cfg(fp, DEVICE_NAME_ETH0, NULL);
        write_netif_cfg(fp, DEVICE_NAME_ETH1, NULL);
        fflush(fp);
        lockf(fileno(fp), F_ULOCK, 0);
        fclose(fp);
        write_resolv_cfg(&tmpcfg.bondcfg.bondif);
        //apply_netif_change(&tmpcfg.bondcfg.bondif, &inetbond, BONDIFNAME);
    } else {
        printf("tmpcfg.mode=%d multi\n", tmpcfg.mode);
        printf("tmpcfg.tmpcfg.netif1.gateway=%s bondif\n", tmpcfg.netif1.gateway);
        printf("tmpcfg.tmpcfg.netif2.gateway=%s bondif\n", tmpcfg.netif2.gateway);
        if ((fp = fopen(NETIF_CFG_FILE, "wb")) == NULL) {
            perror("open failed.");
            printf("open network interface configure file failed.\n");
            return -1;
        }
        lockf(fileno(fp), F_LOCK, 0);
        //fprintf(fp, "#The loopback interface\nauto lo\niface lo inet loopback\n\n");
        fprintf(fp, "#The loopback interface\nauto lo\niface lo inet loopback\niface lo inet6 loopback\n\n");
        write_netif_cfg(fp, DEVICE_NAME_ETH0, &tmpcfg.netif1);
        write_netif_cfg(fp, DEVICE_NAME_ETH1, &tmpcfg.netif2);
        fflush(fp);
        lockf(fileno(fp), F_ULOCK, 0);
        fclose(fp);
        write_resolv_cfg(&tmpcfg.netif1);
        write_resolv_cfg(&tmpcfg.netif2);
        //apply_netif_change(&tmpcfg.netif1, &inet1, DEVICE_NAME_ETH0);
        //apply_netif_change(&tmpcfg.netif2, &inet2, DEVICE_NAME_ETH1);
        if (tmpcfg.netif1.type == NET_DHCP) {
            ms_system("rm -f /var/run/dhcpcd-eth0.pid");
            snprintf(cmd, sizeof(cmd), "dhcpcd %s -t 15", DEVICE_NAME_ETH0);
            ms_system(cmd);
        }
        if (tmpcfg.netif2.type == NET_DHCP) {
            ms_system("rm -f /var/run/dhcpcd-eth1.pid");
            snprintf(cmd, sizeof(cmd), "dhcpcd %s -t 15", DEVICE_NAME_ETH1);
            ms_system(cmd);
        }

        if (tmpcfg.netif1.enable == 1) {
            if (tmpcfg.netif1.ip6type == 1) {
                //dhcp6c -P fe80::1ec3:16ff:fe63:b172/64 eth1
                ;//Router Advertisement
            } else if (tmpcfg.netif1.ip6type == 2) {
                //dhcp6c
                network_set_dhcpv6(DEVICE_NAME_ETH0);

            }
        }

        if (tmpcfg.netif2.enable == 1) {
            if (tmpcfg.netif2.ip6type == 1) {
                ;//Router Advertisement
            } else if (tmpcfg.netif2.ip6type == 2) {
                //dhcp6c
                network_set_dhcpv6(DEVICE_NAME_ETH1);
            }
        }
    }

    snprintf(cmd, sizeof(cmd), "ln -sf %s/resolv.conf /etc/resolv.conf", MS_ETC_DIR);
    ms_system(cmd);
    write_network_cfg(&tmpcfg);
    //ms_set_hostname(tmpcfg.hostname);
    if (tmpcfg.mode != NETMODE_MULTI) {
        ms_system("rm /var/run/dhcpcd-bond0.pid");
        ms_system("/etc/init.d/bond");

        //hrz.milesight here add bond0 ipv6 set(static, ra, dhcpv6)
        if (tmpcfg.bondcfg.bondif.ip6type == 0) {
            //static
            if (tmpcfg.bondcfg.bondif.ip6addr[0] && tmpcfg.bondcfg.bondif.ip6netmask[0]) {
                snprintf(cmd, sizeof(cmd), "ifconfig %s add %s/%s", DEVICE_NAME_BOND, tmpcfg.bondcfg.bondif.ip6addr,
                         tmpcfg.bondcfg.bondif.ip6netmask);
                ms_system(cmd);
                if (tmpcfg.bondcfg.bondif.ip6gateway[0]) {
                    //route -A inet6 add default gw 2001:f80:754::1 eth1
                    snprintf(cmd, sizeof(cmd), "route -A inet6 add default gw %s %s", tmpcfg.bondcfg.bondif.ip6gateway, DEVICE_NAME_BOND);
                    ms_system(cmd);
                }
            }
        } else if (tmpcfg.bondcfg.bondif.ip6type == 1) {
            //RA
            if (tmpcfg.bondcfg.bondif.ip6addr[0] && tmpcfg.bondcfg.bondif.ip6netmask[0]) {
                snprintf(cmd, sizeof(cmd), "ifconfig %s add %s/%s", DEVICE_NAME_BOND, tmpcfg.bondcfg.bondif.ip6addr,
                         tmpcfg.bondcfg.bondif.ip6netmask);
                ms_system(cmd);
                if (tmpcfg.bondcfg.bondif.ip6gateway[0]) {
                    //route -A inet6 add default gw 2001:f80:754::1 eth1
                    snprintf(cmd, sizeof(cmd), "route -A inet6 add default gw %s %s", tmpcfg.bondcfg.bondif.ip6gateway, DEVICE_NAME_BOND);
                    ms_system(cmd);
                }
            }
        } else {
            //DHCPv6
            network_set_dhcpv6(DEVICE_NAME_BOND);
        }

    }

    update_dhcp_config(networkDb);
    return 0;
}

int ms_set_ethname(void)
{
    printf("-------------reName before--------------\n");
    network_show_mac_addr("eth0");
    network_show_mac_addr("eth1");
    ms_system("ifdown -af");
    ms_system("ip link set eth0 name eth2");
    ms_system("ip link set eth1 name eth0");
    ms_system("ip link set eth2 name eth1");
    printf("-------------reName after--------------\n");
    network_show_mac_addr("eth0");
    network_show_mac_addr("eth1");

    return 0;
}

int ms_set_network_apply(struct network *networkDb)
{
    char cmd[256] = {0};
    FILE *fp;
    dvr_net_info_t inet1, inet2;

    NETWORKCFG tmpcfg;
    memset(&tmpcfg, 0, sizeof(tmpcfg));
    netDbToInternal(networkDb, &tmpcfg);

    snprintf(cmd, sizeof(cmd), "rm -rf %s/resolv.conf", MS_ETC_DIR);
    ms_system(cmd);
    ms_system("rm -rf /etc/network/interfaces");

    if (tmpcfg.mode != NETMODE_MULTI) {
        printf("tmpcfg.mode=%d bondif\n", tmpcfg.mode);
        printf("tmpcfg.tmpcfg.netif1.gateway=%s bondif\n", tmpcfg.netif1.gateway);
        printf("tmpcfg.tmpcfg.netif2.gateway=%s bondif\n", tmpcfg.netif2.gateway);
        if ((fp = fopen(NETIF_CFG_FILE, "w")) == NULL) {
            printf("open network interface configure file failed.\n");
            return -1;
        }
        lockf(fileno(fp), F_LOCK, 0);
        fprintf(fp, "#The loopback interface\nauto lo\niface lo inet loopback\n\n");
        write_netif_cfg(fp, DEVICE_NAME_ETH0, NULL);
        write_netif_cfg(fp, DEVICE_NAME_ETH1, NULL);
        fflush(fp);
        lockf(fileno(fp), F_ULOCK, 0);
        fclose(fp);
        write_resolv_cfg(&tmpcfg.bondcfg.bondif);
        //apply_netif_change(&tmpcfg.bondcfg.bondif, &inetbond, BONDIFNAME);

    } else {
        printf("tmpcfg.mode=%d multi\n", tmpcfg.mode);
        printf("tmpcfg.tmpcfg.netif1.gateway=%s bondif\n", tmpcfg.netif1.gateway);
        printf("tmpcfg.tmpcfg.netif2.gateway=%s bondif\n", tmpcfg.netif2.gateway);
        if ((fp = fopen(NETIF_CFG_FILE, "w")) == NULL) {
            printf("open network interface configure file failed.\n");
            return -1;
        }
        lockf(fileno(fp), F_LOCK, 0);
        fprintf(fp, "#The loopback interface\nauto lo\niface lo inet loopback\n\n");
        write_netif_cfg(fp, DEVICE_NAME_ETH0, &tmpcfg.netif1);
        write_netif_cfg(fp, DEVICE_NAME_ETH1, &tmpcfg.netif2);
        fflush(fp);
        lockf(fileno(fp), F_ULOCK, 0);
        fclose(fp);
        write_resolv_cfg(&tmpcfg.netif1);
        write_resolv_cfg(&tmpcfg.netif2);
        //apply_netif_change(&tmpcfg.netif1, &inet1, DEVICE_NAME_ETH0);
        //apply_netif_change(&tmpcfg.netif2, &inet2, DEVICE_NAME_ETH1);
    }

    snprintf(cmd, sizeof(cmd), "ln -sf %s/resolv.conf /etc/resolv.conf", MS_ETC_DIR);
    ms_system(cmd);
    write_network_cfg(&tmpcfg);

    if (tmpcfg.mode != NETMODE_MULTI) {
        //apply_netif_change(&tmpcfg.bondcfg.bondif, &inetbond, BONDIFNAME);
        ms_system("/etc/init.d/bond");//bruce.milesight modify
    } else {
        apply_netif_change(&tmpcfg.netif1, &inet1, DEVICE_NAME_ETH0);
        apply_netif_change(&tmpcfg.netif2, &inet2, DEVICE_NAME_ETH1);
    }

    return 0;
}

/* -------------------------------------------------------------------------------------------------- */
#define TIME_ZONE_CMD "ln -sf /usr/share/zoneinfo/%s /etc/localtime"
#define TIMEZONE_DB_PATH "/usr/share/zoneinfo/zonedb"

#define NTP_CONF_PATH "/etc/ntp.conf"
#define NTP_CONF "server 127.127.1.0\n\
fudge 127.127.1.0 stratum 1\n\
restrict default kod notrap nomodify nopeer noquery limited\n\
restrict -6 default kod notrap nomodify nopeer noquery limited\n\
restrict 127.0.0.1\n"

#define NTP_SH_PATH "/etc/init.d/ntp"

#define NTP_SH "#!/bin/sh\n\
# Start up file for ntp\n\
case $1 in\n\
        start)          killall -9 ntpd\n\
                        ntpdate -s -c 2 -i 5 -h %s -d &\n\
                        date;\n\
                        /etc/ntp.sh &\n\
                        ntpd \n\
                        sleep 1;\n\
                        ntpq -p > /dev/null;;\n\
        stop)           echo \"ntp stop\" ;;\n\
        restart)        ntpdate -s -c 2 -i 5 -h %s -d &\n\
                        date;\n\
                        /etc/ntp.sh &;;\n\
        enable)         ln -sf /etc/init.d/ntp /etc/init.d/S50ntp;;\n\
        disable)        rm -f /etc/init.d/S50ntp;;\n\
        *) cat <<EOF;;\n\
Syntax: /etc/init.d/ntp [command]\n\
\n\
Available commands:\n\
        start   Start the service\n\
        stop    Stop the service\n\
        restart Restart the service\n\
        enable  Enable service autostart\n\
        disable Disable service autostart\n\
EOF\n\
esac\n"

#if defined (_HI3536_) || defined (_HI3536C_)

typedef struct _flist {
    struct _flist *prev;
    struct _flist *next;
} FListType;

#define file_list_size sizeof(FListType)

static int  readFile(FListType *head, char *mand, int len, char *sDstTimeZone)
{
    FListType *list = head;
    char Rule[8] = {0};
    char Iran[8] = {0};
    char yearstart[8] = {0};
    char yearend[8] = {0};
    char mouth[8] = {0};
    int  day = 0;
    char noUse1[8] = {0};
    char noUse2[8] = {0};
    char flag;
    char defaut_tz1[32] = "Asia/Tehran1";
    char defaut_tz2[32] = "Asia/Tehran2";


    while (list) {

        sscanf((char *)list + file_list_size, "%7s\t%7s\t%7s\t%7s\t-\t%7s\t%d\t%7s\t%7s\t%c",
               Rule, Iran, yearstart, yearend, mouth, &day, noUse1, noUse2, &flag);
        if (!strcmp(yearend, "only")) {
            snprintf(yearend, sizeof(yearend), "%s", yearstart);
        }

        if (!strcmp(mand, "1970")) {
            snprintf(mand, len, "%s", yearstart);
        }

        if (((strcmp(yearstart, mand) < 0) && (strcmp(yearend, mand) > 0))
            || !strcmp(yearstart, mand) || !strcmp(yearend, mand)) {

            switch (day) {
                case 21:
                    memcpy(sDstTimeZone, defaut_tz1, strlen(defaut_tz1) + 1);

                    break;

                default:
                    memcpy(sDstTimeZone, defaut_tz2, strlen(defaut_tz2) + 1);
                    break;
            }

            return -1;
        }

        list = list->next;
    }

    return 0;
}


//add for ssh config
static FListType *LoadFile(char *fileName)
{
    FILE *fp = NULL;
    char buf[128];
    FListType *head = NULL, *list, *next;

    //openfile
    fp = fopen(fileName, "r");
    if (NULL == fp) {
        printf("Open %s failed!\n", fileName);
        return NULL;
    }

    //get by column
    while (NULL != fgets(buf, sizeof(buf), fp)) {
        //'/0'
        list = (FListType *)ms_malloc(file_list_size + strlen(buf) + 1);
        if (NULL == list) {
            break;
        }
        //copy
        memcpy(list + 1, buf, strlen(buf));

        if (NULL == head) {
            head = list;
        } else {
            next = head;
            while (next->next) {
                next = next->next;
            }
            next->next = list;
            list->prev = next;
        }
    }

    fclose(fp);
    return head;
}

static void free_list(FListType *head)
{
    FListType *tmp1 = head;
    FListType *tmp2 = NULL;
    if (!head) {
        return ;
    }
    while (tmp1->next) {
        tmp2 = tmp1;
        tmp1 = tmp1->next;
        ms_free(tmp2);
    }
    ms_free(tmp1);
    return;
}

static void on_update_timezone_3(char *sDstTimeZone)
{
    struct tm *tmnow;
    FListType *head = NULL;
    char years[8] = {0};
    time_t tnow;
    int len = 0;
    head = LoadFile("/usr/share/zoneinfo/Iran");
    tzset();
    time(&tnow);
    tmnow = localtime(&tnow);
    len = sizeof(years);
    sprintf(years, "%04d", tmnow->tm_year + 1900);
    readFile(head, years, len, sDstTimeZone);
    free_list(head);
}
#endif /* (_HI3536_) */


int ms_set_time(struct time *tmcfg, const char *tmstr)
{
    char cmd[128] = {0};
    char realtz[128] = {0};
    FILE *fp;

    snprintf(realtz, sizeof(realtz), "%s", tmcfg->time_zone);
    if (tmcfg->dst_enable == 1) { // enable daylight saving time
        db_get_tz_filename(TIMEZONE_DB_PATH, tmcfg->time_zone, tmcfg->time_zone_name, sizeof(tmcfg->time_zone), realtz,
                           sizeof(realtz));
    }
#if defined (_HI3536_) || defined (_HI3536C_)
    if (strstr(realtz, "Tehran")) {
        on_update_timezone_3(realtz);
    }
#endif
    if (strcmp(realtz, "UTC-6:30")) {
        snprintf(cmd, sizeof(cmd), TIME_ZONE_CMD, realtz);
    } else {
        snprintf(cmd, sizeof(cmd), TIME_ZONE_CMD, "Asia/Rangoon");
    }

    ms_system("rm -rf /etc/localtime");
    ms_system(cmd);
    tzset();
    fp = fopen(NTP_SH_PATH, "wb");
    if (fp) {
        lockf(fileno(fp), F_LOCK, 0);
        fprintf(fp, NTP_SH, tmcfg->ntp_server, tmcfg->ntp_server);
        fflush(fp);
        lockf(fileno(fp), F_ULOCK, 0);
        fclose(fp);
        snprintf(cmd, sizeof(cmd), "chmod +x %s", NTP_SH_PATH);
        ms_system(cmd);
    }
    if (tmcfg->ntp_enable == 0) {
        ms_system("killall -9 ntpd");
        ms_system("ntpd &");
        if (tmstr == NULL) {
            printf("no available time string.\n");
            return -1;
        }
        snprintf(cmd, sizeof(cmd), "date -s \"%s\" > /dev/null", tmstr);
        ms_system(cmd);

        ms_system("/etc/init.d/ntp disable > /dev/null");
        ms_system("hwclock -wu");
    } else {
        ms_system("/etc/init.d/ntp enable > /dev/null");
        //snprintf(cmd, sizeof(cmd), "echo \"%s\" > %s", NTP_CONF, NTP_CONF_PATH);
        fp = fopen(NTP_CONF_PATH, "wb");
        if (fp) {
            fprintf(fp, NTP_CONF);
            fflush(fp);
            fclose(fp);
        }
        ms_system(cmd);
        ms_system("/etc/init.d/ntp start > /dev/null");
    }
    tzset();

    return 0;
}

/* -------------------------------------------------------------------------------------------------- */

#define PPPOE_CFG_PATH (MS_ETC_DIR "/pppoe.conf")

#define PPPOE_CFG_FMT "\
NIC=%s\n\
USERNAME=%s\n\
PASSWORD=%s\n\
HOLDOFF=30\n\
MAXFAIL=10\n"

int disable_change_gw(char *lan1_gateway, char *lan2_gateway)
{
    FILE *fp;
    int ret = -1, i = 0;
    char cmd[128] = {0}, sBuf[64] = {0};
    char ethFile[32] = {0}, gateway[MAX_LEN_16] = {0}, eth[32] = {0};
    for (i = 0; i <= 1; i++) {
        if (i) {
            snprintf(gateway, sizeof(gateway), "%s", lan1_gateway);
            snprintf(ethFile, sizeof(ethFile), "/tmp/route_eth0");
            snprintf(eth, sizeof(gateway), DEVICE_NAME_ETH0);
        } else {
            snprintf(gateway, sizeof(gateway), "%s", lan2_gateway);
            snprintf(ethFile, sizeof(ethFile), "/tmp/route_eth1");
            snprintf(eth, sizeof(eth), DEVICE_NAME_ETH1);
        }
        snprintf(cmd, sizeof(cmd), "route -n | grep \'%s\' | wc -l > %s", gateway, ethFile);
        ms_system(cmd);
        fp = fopen(ethFile, "r");
        if (fp != NULL) {
            fgets(sBuf, sizeof(sBuf), fp);
            fclose(fp);
            remove(ethFile);
        }
        ret = atoi(sBuf);
        if (!ret) {
            snprintf(cmd, sizeof(cmd), "route add default gw %s %s > /dev/null", gateway, eth);
            ms_system(cmd);
        }
        memset(sBuf, 0, sizeof(sBuf));
    }
    return 0;
}

int enable_change_gw(char *lan1_gateway, char *lan2_gateway)
{
    char cmd[256] = {0}, sBuf[64] = {0};
    FILE *fp;
    char ppp0File[32] = "/tmp/route_ppp0";
    snprintf(cmd, sizeof(cmd), "route del default gw %s eth0 > /dev/null", lan1_gateway);
    ms_system(cmd);
    snprintf(cmd, sizeof(cmd), "route del default gw %s eth1 > /dev/null", lan2_gateway);
    ms_system(cmd);

    snprintf(cmd, sizeof(cmd),
             "route -n | grep ppp0 | grep -oE '((1[0-9]{2}|25[0-5]|2[0-4][0-9]|[1-9]?[0-9])\\.){3}(1[0-9]{2}|25[0-5]|2[0-4][0-9]|[1-9]?[0-9])' | head -n1 > %s",
             ppp0File);
    ms_system(cmd);
    fp = fopen(ppp0File, "r");
    if (fp != NULL) {
        fgets(sBuf, sizeof(sBuf), fp);
        fclose(fp);
        remove(ppp0File);
        snprintf(cmd, sizeof(cmd), "route add default gw %s ppp0 > /dev/null", sBuf);
        ms_system(cmd);
    }
    return 0;
}

static int pppoe_change_gw(int enable)
{
    struct network net = {0};
    read_network(SQLITE_FILE_NAME, &net);
    if (enable) {
        enable_change_gw(net.lan1_gateway, net.lan2_gateway);
    } else {
        disable_change_gw(net.lan1_gateway, net.lan2_gateway);
    }
    return 0;
}

int ms_set_PPPOE(struct pppoe *pppoecfg, int netmode)
{
    char cmd[128] = {0};
    FILE *fp;

    snprintf(cmd, sizeof(cmd), "%s%s %s > /dev/null", PPPOE_SCRIPT_PATH, "pppoe-stop", PPPOECFG_PATH);
    ms_system(cmd);

    usleep(40 * 1000);

    if (pppoecfg->enable) {

        if ((fp = fopen(PPPOE_RESOLV_PATH, "w")) == NULL) {
            msprintf("open %s failed.\n", PPPOE_RESOLV_PATH);
            return -1;
        } else {
            fprintf(fp, "# MADE-BY-RP-PPPOE\nnameserver 8.8.8.8");
            fclose(fp);
        }
        if (!access(PPPOE_PAP_PATH, R_OK)) {
            snprintf(cmd, sizeof(cmd), "cp %s %s%s", PPPOE_PAP_PATH, PPPOE_PAP_PATH, "-bak");
            ms_system(cmd);
        }
        if ((fp = fopen(PPPOE_PAP_PATH, "w")) == NULL) {
            msprintf("open %s failed.\n", PPPOE_PAP_PATH);
            return -1;
        } else {
            fprintf(fp, "%s    *    %s\n", pppoecfg->username, pppoecfg->password);
            fclose(fp);
        }
        if (!access(PPPOE_CHAP_PATH, R_OK)) {
            snprintf(cmd, sizeof(cmd), "cp %s %s%s", PPPOE_CHAP_PATH, PPPOE_CHAP_PATH, "-bak");
            ms_system(cmd);
        }
        if ((fp = fopen(PPPOE_CHAP_PATH, "w")) == NULL) {
            msprintf("open %s failed.\n", PPPOE_CHAP_PATH);
            return -1;
        } else {
            fprintf(fp, "%s    *    %s\n", pppoecfg->username, pppoecfg->password);
            fclose(fp);
        }
        if (netmode) {
            snprintf(cmd, sizeof(cmd), "%s%s %s %s %s", PPPOE_SCRIPT_PATH, "pppoe-start", DEVICE_NAME_BOND, pppoecfg->username,
                     PPPOECFG_PATH);
        } else {
            snprintf(cmd, sizeof(cmd), "%s%s %s %s %s", PPPOE_SCRIPT_PATH, "pppoe-start", DEVICE_NAME_ETH0, pppoecfg->username,
                     PPPOECFG_PATH);
        }
        ms_system(cmd);
    }
    pppoe_change_gw(pppoecfg->enable);
    return 0;
}

/* -------------------------------------------------------------------------------------------------- */

#define DDNS_CFG_PATH (MS_ETC_DIR "/inadyn.conf")

#define DDNS_CFG_FMT "\
username %s\n\
password %s\n\
dyndns_system default@%s\n\
alias %s\n\
update_period_sec %d\n"

#define DDNS_CFG_FMT_FREEDNS "\
username %s\n\
password %s\n\
dyndns_system default@%s\n\
alias %s,%s\n\
update_period_sec %d\n"

#define DDNS_CFG_FMT_CUSTOMIZE "\
username %s\n\
password %s\n\
dyndns_system custom@http_svr_basic_auth\n\
dyndns_server_name %s\n\
dyndns_server_url %s\n\
alias ddns.ddnsdemo.org\n\
update_period_sec %d\n"

#define DDNS_CFG_FMT_DYNDNS "\
username %s\n\
password %s\n\
dyndns_system default@%s\n\
alias %s\n\
dyndns_server_name %s\n\
update_period_sec %d\n"
int ms_set_DDNS(struct ddns *ddnscfg)
{
    FILE *fp;

    if ((fp = fopen(DDNS_CFG_PATH, "wb")) == NULL) {
        printf("open %s failed.\n", DDNS_CFG_PATH);
        return -1;
    }
    lockf(fileno(fp), F_LOCK, 0);
    if (strcmp(ddnscfg->domain, "customize") == 0) {
        char *szIPAddr = NULL, *szIPAddrEnd = NULL, *szPortEnd = NULL;
        char domain[64] = {0};
        char sPort[8] = {0};
        if ((szIPAddr = strstr(ddnscfg->host_name, "http://")) != NULL) {
            szIPAddr = szIPAddr + 7;
            if ((szIPAddrEnd = strstr(szIPAddr, ":")) != NULL) {
                memcpy(domain, szIPAddr, (int)(szIPAddrEnd - szIPAddr));
                domain[(int)(szIPAddrEnd - szIPAddr)] = '\0';
                if ((szPortEnd = strstr(szIPAddrEnd, "/")) != NULL) {
                    memcpy(sPort, szIPAddrEnd + 1, (int)(szPortEnd - szIPAddrEnd) - 1);
                    sPort[(int)(szPortEnd - szIPAddrEnd) - 1] = '\0';
                }
            } else if ((szIPAddrEnd = strstr(szIPAddr, "/")) != NULL) {
                memcpy(domain, szIPAddr, (int)(szIPAddrEnd - szIPAddr));
                domain[(int)(szIPAddrEnd - szIPAddr)] = '\0';
                snprintf(sPort, sizeof(sPort), "80");
            }
        }
        fprintf(fp, DDNS_CFG_FMT_CUSTOMIZE, ddnscfg->username, ddnscfg->password, domain, ddnscfg->host_name,
                ddnscfg->update_freq);
    } else if (strcmp(ddnscfg->domain, "freedns.afraid.org") == 0) {
        fprintf(fp, DDNS_CFG_FMT_FREEDNS, ddnscfg->username, ddnscfg->password, ddnscfg->domain, ddnscfg->host_name,
                ddnscfg->free_dns_hash, ddnscfg->update_freq);
    } else if (strcmp(ddnscfg->domain, "dyndns.org") == 0) {
        fprintf(fp, DDNS_CFG_FMT_DYNDNS, ddnscfg->username, ddnscfg->password, ddnscfg->domain, ddnscfg->host_name,
                ddnscfg->ddnsurl, ddnscfg->update_freq);
    } else {
        fprintf(fp, DDNS_CFG_FMT, ddnscfg->username, ddnscfg->password, ddnscfg->domain, ddnscfg->host_name,
                ddnscfg->update_freq);
    }
    fflush(fp);
    lockf(fileno(fp), F_ULOCK, 0);

    fclose(fp);

    if (ddnscfg->enable == 0) {
        ms_system("/etc/init.d/ddns disable > /dev/null");
        ms_system("/etc/init.d/ddns stop > /dev/null");
    } else {
        ms_system("/etc/init.d/ddns enable > /dev/null");
        ms_system("/etc/init.d/ddns restart > /dev/null &");
    }

    return 0;
}

/* -------------------------------------------------------------------------------------------------- */
#define BOA_CFG_PATH "/etc/boa/boa.conf"

#define ACCESS_CFG_FMT "\
ACCESS=%d\n\
SSH_PORT=%d\n\
HTTP_PORT=%d\n\
RTSP_PORT=%d\n"

#define BOA_CFG_FMT "\
Port %d\r\n\
SSLPort %d\r\n\
User 0\r\n\
Group 0\r\n\
ErrorLog /var/boa/error.log\r\n\
AccessLog /dev/null\r\n\
CGILog /dev/null\r\n\
DocumentRoot /opt/app/www/webfile/\r\n\
DirectoryIndex /html/home.html\r\n\
DirectoryMaker /usr/lib/boa/boa_indexer\r\n\
KeepAliveMax 128\r\n\
KeepAliveTimeout 256\r\n\
MaxConnections 20\r\n\
MimeTypes /etc/boa/mime.types\r\n\
DefaultType text/plain\r\n\
CGIPath /bin:/usr/bin:/usr/local/bin:/opt/app/bin\r\n\
SinglePostLimit 88080384\r\n\
Alias /doc /usr/share/doc\r\n\
ScriptAlias /cgi/ /opt/app/bin/\r\n\
ServerName %s\n\n"

int ms_set_remote_access(struct network_more *accessDb)
{
    FILE *fp;
    char cmd[256] = {0};

    struct network network;
    ACCESSCFG accfg;
    memset(&accfg, 0, sizeof(accfg));
    accessDbToInternal(accessDb, &accfg);
    read_network(SQLITE_FILE_NAME, &network);

    if ((fp = fopen(BOA_CFG_PATH, "w")) == NULL) {
        printf("open %s failed.\n", BOA_CFG_PATH);
        return -1;
    }
    lockf(fileno(fp), F_LOCK, 0);
    fprintf(fp, BOA_CFG_FMT, accfg.httpport, accfg.httpsport, network.host_name);
    fflush(fp);
    lockf(fileno(fp), F_ULOCK, 0);
    fclose(fp);

    ms_system("killall -9 boa > /dev/null");
    printf("start boa...........................................................\n");
    ms_system("boa -f /etc/boa/boa.conf &");
    ms_system("killall -9 dropbear");
    if (accfg.access) {
        snprintf(cmd, sizeof(cmd), "dropbear -p %d &", accfg.sshport);
        ms_system(cmd);
    }

    return 0;
}


/* -------------------------------------------------------------------------------------------------- */

#define MAIL_CFG_PATH (MS_ETC_DIR "/smtp/ssmtp.conf")
#define MAIL_REVA_PATH (MS_ETC_DIR "/smtp/revaliases")

#define MAIL_CFG_FMT "\
mailhub=%s:%d\n\
FromLineOverride=YES\n\
AuthUser=%s\n\
AuthPass=%s\n\
UseSTARTTLS=%s\n\
UseTLS=%s\n\
root=postmaster\n\
hostname=%s\n\
rewriteDomain=\n\
NeedAuth=yes\n"

#define MAIL_REVA_FMT "root:%s:%s"

int ms_set_maildb(struct email *emailDb)
{
    struct smtpconf conf = {{0}};
    snprintf(conf.emailaddr, sizeof(conf.emailaddr), "%s",  emailDb->sender_addr);
    snprintf(conf.username, sizeof(conf.username), "%s",  emailDb->username);
    snprintf(conf.password, sizeof(conf.password), "%s",  emailDb->password);
    snprintf(conf.smtpserver, sizeof(conf.smtpserver), "%s",  emailDb->smtp_server);
    conf.smtpport = emailDb->port;
    if (strcmp(conf.smtpserver, "smtp.gmail.com") == 0) {
        if (conf.smtpport == 465) {
            conf.smtpport = 587;
        }
    }
    conf.usessl = emailDb->enable_tls;
    conf.auth = 1;
    snprintf(conf.sender, sizeof(conf.sender), "%s", emailDb->sender_name);
    write_smtpconf(EMAIL_DB_PATH, &conf);

    ms_system("killall -9 msmail");
    ms_system("msmail > /dev/null &");

    return 0;
}

int ms_set_mail(struct email *emailDb)//(MAILCFG *mailcfg)
{
    MAILCFG mailcfg;
    memset(&mailcfg, 0, sizeof(mailcfg));
    emailDbToInternal(emailDb, &mailcfg);
    printf("ms_set_mail\n");
    FILE *fp;
    char host[128] = {0};

    if ((fp = fopen(MAIL_CFG_PATH, "w")) == NULL) {
        printf("open %s failed.\n", MAIL_CFG_PATH);
        return -1;
    }

    if (strcmp(mailcfg.smtpserver, "smtp.gmail.com") == 0) {
        if (mailcfg.port == 465) {
            mailcfg.port = 587;
        }
    }

    gethostname(host, sizeof(host) - 1);
    lockf(fileno(fp), F_LOCK, 0);
    fprintf(fp, MAIL_CFG_FMT, mailcfg.smtpserver, mailcfg.port, mailcfg.username[0] ? mailcfg.username : mailcfg.sender,
            mailcfg.passwd,
            mailcfg.enabletls ? "YES" : "NO", mailcfg.enabletls ? "YES" : "NO", host);
    fflush(fp);
    lockf(fileno(fp), F_ULOCK, 0);
    fclose(fp);

    if ((fp = fopen(MAIL_REVA_PATH, "w")) == NULL) {
        printf("open %s failed.\n", MAIL_REVA_PATH);
        return -1;
    }
    lockf(fileno(fp), F_LOCK, 0);
    fprintf(fp, MAIL_REVA_FMT, mailcfg.sender, mailcfg.smtpserver);
    fflush(fp);
    lockf(fileno(fp), F_ULOCK, 0);
    fclose(fp);

    ms_set_maildb(emailDb);

    return 0;
}

int ms_send_test_mail(struct email *emailDb)//, int num, void *data)
{
    int ret, nResult;
//  char host[128] = {0};
    char filename[64] = {0};
    char randstr[32] = {0};
    char cmd[128] = {0};
    struct mailcontent mailcont = {{0}};
    snprintf(mailcont.subject, sizeof(mailcont.subject), "Test mail from %s", emailDb->sender_name);
    snprintf(mailcont.body, sizeof(mailcont.body), "This is a nvr test mail from %s.", emailDb->sender_name);

    generate_rand_str(randstr, sizeof(randstr));
    snprintf(filename, sizeof(filename), "/var/%s.eml", randstr);
    create_ssmtp_conf(filename, emailDb, &mailcont);

    snprintf(cmd, sizeof(cmd), "rm -f %s", SMTP_LOG_PATH);
    ms_system(cmd);// make sure smtp log file remove
    snprintf(cmd, sizeof(cmd), "nice -n 10 smtp -c %s;", filename);
    printf("%s\n", cmd);
    ret = ms_system(cmd) != 0 ? -1 : 0; //0-success ,-1 -fail
    nResult = get_smtp_error_code(SMTP_LOG_PATH);
    if (ret == 0 && nResult == 0) {
        ret = 0;
    } else {
        ret = -1;
    }

    snprintf(cmd, sizeof(cmd), "rm -f %s", SMTP_LOG_PATH);
    ms_system(cmd);
    snprintf(cmd, sizeof(cmd), "rm -f %s", filename);
    ms_system(cmd);

    return ret;
}

/* -------------------------------------------------------------------------------------------------- */

#define SNMP_CFG_PATH (MS_ETC_DIR "/snmpd.conf")

#define SNMP_CFG_FMT "\
port=%d\n\
com2sec rosecname default %s\n\
com2sec rwsecname default %s\n\
group rogroup v1      rosecname\n\
group rogroup v2c     rosecname\n\
group rwgroup v1 rwsecname\n\
group rwgroup v2c rwsecname\n\
view sysview included .1.3.6.1.2.1.1.1\n\
view sysview included .1.3.6.1.2.1.1.2\n\
view sysview included .1.3.6.1.2.1.1.3.0\n\
view sysview included .1.3.6.1.2.1.1.4\n\
view sysview included .1.3.6.1.2.1.1.5\n\
view sysview included .1.3.6.1.2.1.1.6\n\
view sysview included .1.3.6.1.2.1.1.7\n\
view sysview included .1.3.6.1.2.1.1.8\n\
view sysview included .1.3.6.1.2.1.2.1.0\n\
access rogroup "" any noauth exact sysview none none\n\
access rwgroup "" any noauth exact sysview sysview none\n\
rouser qh auth\n\
createUser qh MD5 qh123456 DES\n\
trapcommunity %s\n\
trapsink %s:%d %s\n\
iquerySecName qh\n\
authtrapenable 2\n\
sysDescr Yeastar company hdvr products\n\
sysContact YS\n\
sysLocation Your Location\n\
sysServices 72\n"

#define SNMP_CREATE_PREFIX "/opt/app/snmp/"

int ms_set_SNMP(SNMPCFG *snmpcfg)
{
    FILE *fp;

    if ((fp = fopen(SNMP_CFG_PATH, "w")) == NULL) {
        return -1;
    }

    fprintf(fp, SNMP_CFG_FMT, snmpcfg->port, snmpcfg->rcommunity, snmpcfg->wcommunity,
            snmpcfg->rcommunity, snmpcfg->trapdest, snmpcfg->trapport, snmpcfg->rcommunity);
    fclose(fp);

    if (snmpcfg->enable) {
        ms_system("/etc/init.d/snmp enable > /dev/null");
        ms_system("/etc/init.d/snmp restart > /dev/null &");
    } else {
        ms_system("/etc/init.d/snmp disable > /dev/null");
        ms_system("/etc/init.d/snmp stop > /dev/null");
    }

    return 0;
}

static void snmp_set_conf(struct snmp *snmp, char flag)
{
    if (!snmp) {
        return;
    }

    char cmd[256] = {0};
    if (flag == 0) {
        //if (memcmp(snmp->read_community, snmp->write_community, sizeof(snmp->write_community)) != 0)
        {
            //read_conf_v1/v2
            snprintf(cmd, sizeof(cmd), "sed -i 's/%s/%s/' /tmp/snmpd.conf", "readonly_v12c", snmp->read_community);
            ms_system(cmd);
        }

        //wr_conf_v1/v2
        memset(cmd, 0x0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "sed -i 's/%s/%s/' /tmp/snmpd.conf", "writeread_v12c", snmp->write_community);
        ms_system(cmd);
    } else if (flag == 1) {
        char read_auth_algo[4] = "MD5";
        char read_priv_algo[4] = "DES";
        char write_auth_algo[4] = "MD5";
        char write_priv_algo[4] = "DES";
        char create_read_usr[256] = {0};
        char create_write_usr[256] = {0};

        //ms_system("snmpd -c /tmp/snmpd.conf");
        //ms_system("killall -2 snmpd");
        //ms_system("rm -f /var/net-snmp/snmpd.conf");

        if (snmp->read_auth_algorithm == 1) {
            snprintf(read_auth_algo, sizeof(read_auth_algo), "%s", "SHA");
        }

        if (snmp->read_pri_algorithm == 1) {
            snprintf(read_priv_algo, sizeof(read_auth_algo), "%s", "AES");
        }

        if (snmp->write_auth_algorithm == 1) {
            snprintf(write_auth_algo, sizeof(write_auth_algo), "%s", "SHA");
        }

        if (snmp->write_pri_algorithm == 1) {
            snprintf(write_priv_algo, sizeof(write_priv_algo), "%s", "AES");
        }

        if (snmp->read_level_security == 0) {

            snprintf(create_read_usr, sizeof(create_read_usr), "%snet-snmp-create-v3-user -ro -a %s -A %s -x %s -X %s %s", \
                     SNMP_CREATE_PREFIX, snmp->read_auth_password, read_auth_algo, snmp->read_pri_password, read_priv_algo,
                     snmp->read_security_name);

            snprintf(cmd, sizeof(cmd), "sed -i 's/%s/%s/' /tmp/snmpd.conf", "v3rdprivusername", snmp->read_security_name);
            ms_system(cmd);
        } else if (snmp->read_level_security == 1) {
            snprintf(create_read_usr, sizeof(create_read_usr), "%snet-snmp-create-v3-user -ro -a %s -A %s %s", \
                     SNMP_CREATE_PREFIX, snmp->read_auth_password, read_auth_algo, snmp->read_security_name);
            snprintf(cmd, sizeof(cmd), "sed -i 's/%s/%s/' /tmp/snmpd.conf", "v3rdauthusername", snmp->read_security_name);
            ms_system(cmd);
        } else if (snmp->read_level_security == 2) {
            snprintf(create_read_usr, sizeof(create_read_usr), "%snet-snmp-create-v3-user -ro  %s", SNMP_CREATE_PREFIX,
                     snmp->read_security_name);
            snprintf(cmd, sizeof(cmd), "sed -i 's/%s/%s/' /tmp/snmpd.conf", "v3rdnoauthusername", snmp->read_security_name);
            ms_system(cmd);
        }

        memset(cmd, 0x0, sizeof(cmd));
        if (snmp->write_level_security == 0) {
            snprintf(create_write_usr, sizeof(create_write_usr), "%snet-snmp-create-v3-user -a %s -A %s -x %s -X %s %s", \
                     SNMP_CREATE_PREFIX, snmp->write_auth_password, write_auth_algo, snmp->write_pri_password, write_priv_algo,
                     snmp->write_security_name);
            snprintf(cmd, sizeof(cmd), "sed -i 's/%s/%s/' /tmp/snmpd.conf", "v3wrprivusername", snmp->write_security_name);
            ms_system(cmd);
        } else if (snmp->write_level_security == 1) {
            snprintf(create_write_usr, sizeof(create_write_usr), "%snet-snmp-create-v3-user -a %s -A %s %s", \
                     SNMP_CREATE_PREFIX, snmp->write_auth_password, write_auth_algo, snmp->write_security_name);
            snprintf(cmd, sizeof(cmd), "sed -i 's/%s/%s/' /tmp/snmpd.conf", "v3wrauthusername", snmp->write_security_name);
            ms_system(cmd);

        } else if (snmp->write_level_security == 2) {
            snprintf(create_write_usr, sizeof(create_write_usr), "%snet-snmp-create-v3-user %s", SNMP_CREATE_PREFIX,
                     snmp->write_security_name);
            snprintf(cmd, sizeof(cmd), "sed -i 's/%s/%s/' /tmp/snmpd.conf", "v3wrnoauthusername", snmp->write_security_name);
            ms_system(cmd);

        }

        ms_system(create_write_usr);
        ms_system(create_read_usr);
    }

    return;
}


int ms_set_net_snmp(struct snmp *snmp)
{
    if (!snmp) {
        return -1;
    }

    int flag = 0;
    char cmd[256] = {0};

    ms_system("killall -9 snmpd");
    ms_system("rm -rf /tmp/snmpd.conf");
    ms_system("rm -rf /var/net-snmp/snmpd.conf");

    if (snmp->v3_enable == 1) {
        //v3
        ms_system("cp -f /opt/app/snmp/snmpd.conf /tmp/snmpd.conf");
        flag = 1;
        snmp_set_conf(snmp, 1);
    }

    if (snmp->v1_enable == 1 || snmp->v2c_enable == 1) {
        //v1 v2c
        if (!flag) {
            ms_system("cp -f /opt/app/snmp/snmpd.conf /tmp/snmpd.conf");
        }

        flag = 1;
        snmp_set_conf(snmp, 0);
    }

    if (flag == 1) {
        //snmpd udp:161,udp6:161 -c /tmp/snmpd.conf
        int port = snmp->port;
        if (port == 0) {
            port = 161;
        }

        snprintf(cmd, sizeof(cmd), "snmpd udp:%d,udp6:%d -c /tmp/snmpd.conf", port, port);
        ms_system(cmd);
        //printf("cmd:%s\n", cmd);
    } else {
        //has already stop
    }
    return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////
//system update,reset,import params and so on
static unsigned long get_file_size(const char *path)
{
    unsigned long filesize = -1;
    struct stat statbuff;
    if (stat(path, &statbuff) < 0) {
        return filesize;
    } else {
        filesize = statbuff.st_size;
    }

    return filesize;
}

static int exec_callback(void *arg, int argc, char **argv, char **cols)
{
    int *bad = (int *)arg;
    if (!strcmp(argv[0], "ok")) {
        *bad = 0;
    } else {
        *bad = 1;
    }

    return 0;
}

static int db_check(const char *db_path)
{
    HSQLITE hsql;
    int result = 0, bad = 0;

    if (sqlite_conn(db_path, &hsql)) {
        msdebug(DEBUG_ERR, "connect to %s error.", db_path);
        return -1;
    }
    result = sqlite3_exec(hsql, "PRAGMA integrity_check;", exec_callback, (void *)&bad, NULL);
    if (result != 0 || bad) {
        msdebug(DEBUG_ERR, "bad db.res:%d bad:%d", result, bad);
        sqlite_disconn(hsql);
        return -1;
    }
    sqlite_disconn(hsql);

    return 0;
}

static int db_restore()
{
    return 0;
}

static int db_get_new_ver(int old_ver, int oem, char *db_path)
{
    int i, new_ver = old_ver, nofound = 50;
    char path[256] = {0};
    if (db_path[0] == '\0') {
        return -1;
    }

    for (i = 0; i < 256; i++) {
        if (strcmp(db_path, SQLITE_INIT_MYDB_PATH) == 0) {
            snprintf(path, sizeof(path), "%s/db-%d.txt", db_path, old_ver + i + 1);
            if (access(path, F_OK)) {
                continue;
            } else {
                new_ver = (old_ver + i + 1) > new_ver ? (old_ver + i + 1) : new_ver;
            }
        } else {
            snprintf(path, sizeof(path), "%s/db-%d.txt", db_path, (old_ver + i + 1));
            if (access(path, F_OK)) {
                if (nofound--) {
                    continue;
                }
                break;
            }
            new_ver = old_ver + (i + 1);
        }
    }
    return new_ver;
}

static int file_filter(const struct dirent *pDir)
{
    if (!strncasecmp(pDir->d_name, ".", 1) || !strncasecmp(pDir->d_name, "..", 2)
        || !strncasecmp(pDir->d_name, "db_bak.tar.gz", 13) || !strncasecmp(pDir->d_name, "db.tar.gz", 9)) {
        return 0;
    }

    return 1;
}

static int partition(DB_VER_INFO_S dbVerInfo[], int fileInt[], int low, int high)
{
    int keyInt = fileInt[low];
    char keyStr[32];
    strncpy(keyStr, dbVerInfo[low].file, strlen(dbVerInfo[low].file) + 1);

    while (low < high) {
        while (low < high && fileInt[high] >= keyInt) {
            --high;
        }
        if (low < high) {
            strncpy(dbVerInfo[low].file, dbVerInfo[high].file, strlen(dbVerInfo[high].file) + 1);
            fileInt[low++] = fileInt[high];

        }

        while (low < high && fileInt[low] <= keyInt) {
            ++low;
        }
        if (low < high) {
            strncpy(dbVerInfo[high].file, dbVerInfo[low].file, strlen(dbVerInfo[low].file) + 1);
            fileInt[high--] = fileInt[low];
        }
    }

    strncpy(dbVerInfo[low].file, keyStr, strlen(keyStr) + 1);
    fileInt[low] = keyInt;

    return low;
}

static void db_update_file_sort(DB_VER_INFO_S dbVerInfo[], int fileInt[], int start, int end)
{
    int pos;

    if (start < end) {
        pos = partition(dbVerInfo, fileInt, start, end);

        db_update_file_sort(dbVerInfo, fileInt, start, pos - 1);
        db_update_file_sort(dbVerInfo, fileInt, pos + 1, end);
    }

    return;
}

static int get_int_from_db_update_file(const char *str, size_t strLen)
{
    int value;

    if (strLen == 0) {
        return -1;
    }

    for (value = 0; --strLen; ++str) {
        if (*str < '0' || *str > '9') {
            continue;
        }

        // such as 9.0.12 ==> 9012, 10.0.1 ==> 10001,
        // then 9012 < 10001, make sure 9.0.12 in front 10.0.1
        if (*(str - 1) == '.' && *(str + 1) == '-') {
            value = value * 100 + (*str - '0');
        } else {
            value = value * 10 + (*str - '0');
        }
    }

    if (value < 0) {
        return -1;
    } else {
        return value;
    }
}

static int db_get_new_ver_info(DB_VER_INFO_S dbVerInfo[], const char *dbUpdatePath)
{
    if (!dbVerInfo || dbUpdatePath[0] == '\0') {
        return -1;
    }

    if (strcmp(dbUpdatePath, SQLITE_INIT_SQL_PATH) && strcmp(dbUpdatePath, SQLITE_INIT_MYDB_PATH)) {
        return -1;
    }

    int cnt = 0;
    int sum = 0;
    int fileInt[MAX_DB_UPDATE_FILE] = {0};
    struct dirent **pFileList;
    char *pTrunc;

    sum = scandir(dbUpdatePath, &pFileList, file_filter, alphasort);
    if (sum < 0) {
        return -1;
    }

    while (cnt < sum) {
        // msprintf("[wcm] d_ino：%ld  d_off:%ld d_name: %s\n", pFileList[cnt]->d_ino, pFileList[cnt]->d_off, pFileList[cnt]->d_name);
        strncpy(dbVerInfo[cnt].file, pFileList[cnt]->d_name, sizeof(dbVerInfo[cnt].file) - 1);
        pTrunc = strstr(dbVerInfo[cnt].file, ".txt");
        if (pTrunc) {
            pTrunc[0] = '\0';
            fileInt[cnt] = get_int_from_db_update_file(dbVerInfo[cnt].file, strlen(dbVerInfo[cnt].file) + 1);
            
            dbVerInfo[cnt].execute = 1;
        }

        ms_free(pFileList[cnt]);
        ++cnt;
    }
    ms_free(pFileList);

    db_update_file_sort(dbVerInfo, fileInt, 0, sum - 1);

    return cnt;
}

//static void db_special_version_update(int version)
static void db_special_version_update(DB_VER_INFO_S oldVerInfoArr[], int oldVerCnt,
                                      DB_VER_INFO_S updateArr[], int updateCnt)
{
    int nCnt = 0, i = 0, j = 0;
    int flag = 0;
    struct mosaic mosaic = {0};
    struct db_user *users = NULL;
    int specialExec[2] = {0};

    for (i = 0; i < oldVerCnt; ++i) {
        if (!strncmp(oldVerInfoArr[i].file, "db-1006", sizeof(oldVerInfoArr[i].file)) && oldVerInfoArr[i].execute) {
            specialExec[0] = 1;
        } else if (!strncmp(oldVerInfoArr[i].file, "db-1007", sizeof(oldVerInfoArr[i].file)) && oldVerInfoArr[i].execute) {
            specialExec[1] = 1;
        }
    }
    for (i = 0; i < updateCnt; ++i) {
        if (!strncmp(updateArr[i].file, "db-1006", sizeof(updateArr[i].file)) && updateArr[i].execute) {
            specialExec[0] = 1;
        } else if (!strncmp(updateArr[i].file, "db-1007", sizeof(updateArr[i].file)) && updateArr[i].execute) {
            specialExec[1] = 1;
        }
    }

    //if (version >= 1007) {
    if (specialExec[0] && specialExec[1]) {
        //for update layout mode==8192
        {
            mosaic.layoutmode = 8192;
        }
        for (i = 0; i < MAX_CAMERA; i++) {
            if (mosaic.channels[i] == 0) {
                mosaic.channels[i] = i;
            }
        }
        write_mosaic(SQLITE_FILE_NAME, &mosaic);

        //for update 1006 ex DB date
        users = ms_malloc(sizeof(struct db_user) * MAX_USER);
        if (!users) {
            msprintf("#### user malloc failed! ####");
            return;
        }
        memset(users, 0x0, sizeof(struct db_user) * MAX_USER);
        read_users(SQLITE_FILE_NAME, users, &nCnt);
        for (i = 0; i < nCnt; i++) {
            flag = 0;
            for (j = 0; j < 32; j++) {
                if ((int)((users[i].local_live_view >> j) & 0x01) != (int)(users[i].local_live_view_ex[j] - '0')) {
                    users[i].local_live_view |= 0x01 << j;
                    users[i].local_live_view_ex[j] = '1';
                    flag = 1;
                }

                if ((int)((users[i].local_playback >> j) & 0x01) != (int)(users[i].local_playback_ex[j] - '0')) {
                    users[i].local_playback |= 0x01 << j;
                    users[i].local_playback_ex[j] = '1';
                    flag = 1;
                }

                if ((int)((users[i].remote_live_view >> j) & 0x01) != (int)(users[i].remote_live_view_ex[j] - '0')) {
                    users[i].remote_live_view |= 0x01 << j;
                    users[i].remote_live_view_ex[j] = '1';
                    flag = 1;
                }

                if ((int)((users[i].remote_playback >> j) & 0x01) != (int)(users[i].remote_playback_ex[j] - '0')) {
                    users[i].remote_playback |= 0x01 << j;
                    users[i].remote_playback_ex[j] = '1';
                    flag = 1;
                }
            }

            if (flag) {
                write_user(SQLITE_FILE_NAME, &users[i]);
            }
        }

        ms_free(users);
    }

    return;
}

static int db_update(const char *db)
{
    long sec1 = 0, sec2 = 0;
    char path[128] = {0};
    DB_VER_INFO_S oldVerInfoArr[MAX_DB_UPDATE_FILE];
    DB_VER_INFO_S newVerInfoArr[MAX_DB_UPDATE_FILE];
    DB_VER_INFO_S updateArr[MAX_DB_UPDATE_FILE];
    memset(oldVerInfoArr, 0, sizeof(oldVerInfoArr));
    memset(newVerInfoArr, 0, sizeof(newVerInfoArr));
    memset(updateArr, 0, sizeof(updateArr));
    int oldVerCnt, newVerCnt, updateCnt = 0;
    int i, j;
    int ret = 0;

    time(&sec1);
    if (db_check(db)) {
        msdebug(DEBUG_ERR, "bad db:%s.try to restore.", db);
        db_restore();
    }

    read_db_ver_info(db, oldVerInfoArr, &oldVerCnt);

    ret = access(SQLITE_INIT_UPDATE_PATH, F_OK);
    if (ret) {
        msdebug(DEBUG_WRN, "%s is not exist.", SQLITE_INIT_UPDATE_PATH);
    } else {
        snprintf(path, sizeof(path), "chmod 777 -R %s", SQLITE_INIT_UPDATE_PATH);
        ms_system(path);
        snprintf(path, sizeof(path), "tar -xzf %s -C %s", SQLITE_INIT_UPDATE_PATH, SQLITE_INIT_SQL_PATH);
        ret = ms_system(path);
    }

    if (ret) {
        if (access(SQLITE_INIT_UPDATE_BAK_PATH, F_OK)) {
            msdebug(DEBUG_ERR, "%s is not exist.", SQLITE_INIT_UPDATE_PATH);
            snprintf(path, sizeof(path), "rm %s/db-*.txt", SQLITE_INIT_SQL_PATH);
            ms_system(path);
            return -1;
        } else {
            snprintf(path, sizeof(path), "chmod 777 -R %s", SQLITE_INIT_UPDATE_BAK_PATH);
            ms_system(path);
            snprintf(path, sizeof(path), "cp -f %s %s", SQLITE_INIT_UPDATE_BAK_PATH, SQLITE_INIT_UPDATE_PATH);
            ms_system(path);
            snprintf(path, sizeof(path), "tar -xzf %s -C %s", SQLITE_INIT_UPDATE_BAK_PATH, SQLITE_INIT_SQL_PATH);
            if (ms_system(path)) {
                msdebug(DEBUG_ERR, "msdb update file extract err");
                snprintf(path, sizeof(path), "rm %s/db-*.txt", SQLITE_INIT_SQL_PATH);
                ms_system(path);
                return -1;
            }
        }
    }

    newVerCnt = db_get_new_ver_info(newVerInfoArr, SQLITE_INIT_SQL_PATH);
    for (i = 0; i < newVerCnt; ++i) {
        for (j = 0; j < oldVerCnt; ++j) {
            if (!strncmp(newVerInfoArr[i].file, oldVerInfoArr[j].file, sizeof(newVerInfoArr[i].file))
                && oldVerInfoArr[j].execute) {
                break;
            }
        }

        if (j == oldVerCnt) {
            strncpy(updateArr[updateCnt].file, newVerInfoArr[i].file, sizeof(updateArr[updateCnt].file));
            updateArr[updateCnt].execute = 0;
            ++updateCnt;
        }
    }
    if (!updateCnt) {
        msprintf("no need update db.(oldVerCnt: %d newVerCnt: %d updateCnt: %d)", oldVerCnt, newVerCnt, updateCnt);
        snprintf(path, sizeof(path), "rm %s/db-*.txt", SQLITE_INIT_SQL_PATH);
        ms_system(path);

        return 0;
    }

    write_db_update(db, oldVerInfoArr, oldVerCnt, updateArr, updateCnt, SQLITE_INIT_SQL_PATH);
    if (!strncmp(db, SQLITE_FILE_NAME, sizeof(SQLITE_FILE_NAME))) {
        db_special_version_update(oldVerInfoArr, oldVerCnt, updateArr, updateCnt);
    }
    snprintf(path, sizeof(path), "rm %s/db-*.txt", SQLITE_INIT_SQL_PATH);
    ms_system(path);
    time(&sec2);

    //msprintf("######### db update used sec:%ld update from %d ~ %d oem:%d ######", sec2 - sec1, oldVerCnt, newVerCnt, oem);
    msprintf("######### db: %s update used sec: %ld(s).(oldVerCnt: %d newVerCnt: %d) #########", db, sec2 - sec1, oldVerCnt,
             newVerCnt);

    return 0;
}

static int my_db_update()
{
    int mydb_old_ver = 0, mydb_new_ver = 0;
    long sec1 = 0, sec2 = 0;

    if (db_check(SQLITE_FILE_NAME)) {
        msdebug(DEBUG_ERR, "bad db:%s.try to restore.", SQLITE_FILE_NAME);
        db_restore();
    }


    //get current my db version
    mydb_old_ver = get_param_int(SQLITE_FILE_NAME, PARAM_MY_DB_VERSION, -1);
    if (mydb_old_ver < 0) {
        msdebug(DEBUG_ERR, "get db ver error:%d.", mydb_old_ver);
        mydb_old_ver = 1000;
    }

    // get newest db version, need check db & my db
    mydb_new_ver = db_get_new_ver(mydb_old_ver, 0, SQLITE_INIT_MYDB_PATH);

    if (mydb_old_ver >= mydb_new_ver) {
        msprintf("no need update my db.(old:%d new:%d)", mydb_old_ver, mydb_new_ver);
        return 0;
    }

    sec1 = time(NULL);
    // update my db file txt
    db_version_update(SQLITE_FILE_NAME, mydb_old_ver + 1, mydb_new_ver, SQLITE_INIT_MYDB_PATH);
    set_param_int(SQLITE_FILE_NAME, PARAM_MY_DB_VERSION, mydb_new_ver);
    sec2 = time(NULL);
    msprintf("######### my db update used sec:%ld update from %d ~ %d ######", sec2 - sec1, mydb_old_ver, mydb_new_ver);

    return 0;
}


static int db_init_filesystem()
{
    //char path[256] = {0};
    //snprintf(path, sizeof(path), "echo \"[General]\nlanguage=%d\n\" > %s", device_oem.def_lang, GUI_LANG_INI);
    //ms_system(path);
    return 0;
}

static int db_check_key()
{
    int push = 0, i = 0;
    //parmas push_type
    for (i = 0; i < MAX_PUSH_TYPE; i++) {
        push += (1 << i);
    }

    //add first time
    if (check_param_key(SQLITE_FILE_NAME, PARAM_PUSH_TYPE)) {
        add_param_int(SQLITE_FILE_NAME, PARAM_PUSH_TYPE, push);
    }

    if (check_param_key(SQLITE_FILE_NAME, PARAM_MY_DB_VERSION)) {
        msprintf("add my db version:%d", DB_INIT_VERSION);
        add_param_int(SQLITE_FILE_NAME, PARAM_MY_DB_VERSION, DB_INIT_VERSION);
    }

    //sdkversion
    //set_param_value(SQLITE_FILE_NAME, PARAM_SDK_VERSION, PARAMS_SDK_VERSION);
    return 0;
}

static int oem_sys_db_add_check()
{
    int push = -1;

    //oem parmas push_type
    push = get_param_int(SQLITE_OEM_NAME, PARAM_PUSH_TYPE, 0);

    if (push >= 0) {
        set_param_int(SQLITE_FILE_NAME, PARAM_PUSH_TYPE, push);
    }
    return 0;
}

static int oem_update()
{
    /*
        0.update sql from /opt/app/db/db-xx.txt. all the same as /opt/app/msdb.db.
        1.write SQLITE_FILE_NAME from SQLITE_OEM_NAME(table user,time,network,network_more).
        2.write SQLITE_FILE_NAME(param_oem) from SQLITE_OEM_NAME(param_oem) and update "oem_type".
        3.UI(WEB and QT)
    */
    int oem_index, cur_index;
    int i, j, count = 0, count_oem = 0;
    int fnew_size = 0, fold_size;
    int raidmode = 0;
    int password_clean = 0;
    int transport_protocol = 0;
    char path[2048] = {0}, temp[256] = {0};
    char poe_password[64] = {0};
    reboot_conf conf;
    FILE *fp;
    struct device_info device_oem;
    struct time datetime;
    struct network network;
    struct network_more net_more;
    struct osd osd[MAX_CAMERA];
    struct db_user user[MAX_USER];
    struct db_user_oem user_oem[MAX_USER];
    struct p2p_info p2p;
    struct layout layout;
    struct display display;
    struct ipc_model *model = 0;
    struct ipc_protocol *protocol = 0;
    struct ipc_model ipcmodel = {0};
    struct ipc_protocol ipcpro = {0};
    struct record records[MAX_CAMERA];
    struct record_schedule schedule;
    struct camera cameraArray[MAX_CAMERA];
    struct camera cameraOemArray[MAX_CAMERA];
    Uint64 dbChn = 0;
    int cameraCount = 0;
    int online_update;
    char company[64] = {0};

    if (access(SQLITE_OEM_NAME, F_OK)) { //not exist
        return -1;
    } else {
        int state = -1;
        check_table_exist(SQLITE_OEM_NAME, "db_version", &state);
        if (!state) {
            int oemdbVer = get_param_int(SQLITE_OEM_NAME, PARAM_DB_VERSION, -1);
            create_tbl_db_version(SQLITE_OEM_NAME, oemdbVer);
        }
    }
    oem_index = get_param_int(SQLITE_OEM_NAME, PARAM_OEM_TYPE, OEM_TYPE_STANDARD);
    cur_index = get_param_int(SQLITE_FILE_NAME, PARAM_OEM_TYPE, OEM_TYPE_STANDARD);

    msprintf("OEM oem_type:%d MSDB oem_type:%d", oem_index, cur_index);

    //step 0
    db_update(SQLITE_OEM_NAME);

    //step 1
    //datetime and zone
    if (oem_index != cur_index) {
        msprintf("###Copy oem sqlite###");
        memset(&device_oem, 0, sizeof(device_oem));
        db_get_device(SQLITE_OEM_NAME, &device_oem);

        memset(&datetime, 0, sizeof(datetime));
        read_time(SQLITE_OEM_NAME, &datetime);
        write_time(SQLITE_FILE_NAME, &datetime);

        //network
        memset(&network, 0, sizeof(network));
        read_network(SQLITE_OEM_NAME, &network);
        write_network(SQLITE_FILE_NAME, &network);
        memset(&net_more, 0, sizeof(net_more));
        read_port(SQLITE_OEM_NAME, &net_more);
        write_port(SQLITE_FILE_NAME, &net_more);

        //default osd.CAM2
        count = 0;
        memset(osd, 0, sizeof(osd));
        read_osds(SQLITE_OEM_NAME, osd, &count);
        for (i = 0; i < count; i++) {
            write_osd(SQLITE_FILE_NAME, &osd[i]);
        }

        oem_sys_db_add_check();

        //user
        count = 0;
        memset(user, 0, sizeof(struct db_user)*MAX_USER);
        memset(user_oem, 0, sizeof(struct db_user_oem)*MAX_USER);
        read_users_oem(SQLITE_OEM_NAME, user_oem, &count_oem);
        read_users(SQLITE_FILE_NAME, user, &count);
        for (i = 0 ; i < count_oem; i++) {
            user[i].id = user_oem[i].id;
            user[i].enable = user_oem[i].enable;
            user[i].type = user_oem[i].type;
            user[i].permission = user_oem[i].permission;
            user[i].remote_permission = user_oem[i].remote_permission;
            user[i].local_live_view = user_oem[i].local_live_view;
            user[i].local_playback = user_oem[i].local_playback;
            user[i].remote_live_view = user_oem[i].remote_live_view;
            user[i].remote_playback = user_oem[i].remote_playback;
            snprintf(user[i].username, sizeof(user[i].username), "%s", user_oem[i].username);
            snprintf(user[i].password, sizeof(user[i].password), "%s", user_oem[i].password);
        }
        //modify by david for OEM user password_ex copy
        password_clean = get_param_int(SQLITE_OEM_NAME, PARAM_PSW_CLEAN, 0);
        set_param_int(SQLITE_FILE_NAME, PARAM_PSW_CLEAN, password_clean);

        transport_protocol = get_param_int(SQLITE_OEM_NAME, PARAM_TRANSPORT_PROTOCOL, 0);
        set_param_int(SQLITE_FILE_NAME, PARAM_TRANSPORT_PROTOCOL, transport_protocol);

        online_update = get_param_int(SQLITE_OEM_NAME, PARAM_OEM_UPDATE_ONLINE, 0);
        set_param_int(SQLITE_FILE_NAME, PARAM_OEM_UPDATE_ONLINE, online_update);

        get_param_value(SQLITE_OEM_NAME, PARAM_DEVICE_COMPANY, company, sizeof(company), "Milesight");
        set_param_value(SQLITE_FILE_NAME, PARAM_DEVICE_COMPANY, company);

        set_param_int(SQLITE_FILE_NAME, PARAM_OEM_APP_MSG_PUSH, get_param_int(SQLITE_OEM_NAME, PARAM_OEM_APP_MSG_PUSH, 0));

        set_param_int(SQLITE_FILE_NAME, PARAM_STREAM_PLAY_MODE, get_param_int(SQLITE_OEM_NAME, PARAM_STREAM_PLAY_MODE, 0));

        if (check_table_key(SQLITE_OEM_NAME, "user", "password_ex") == 0) {
            struct db_user o_user[MAX_USER];
            memset(o_user, 0, sizeof(struct db_user_oem)*MAX_USER);
            read_users(SQLITE_OEM_NAME, o_user, &count);
            for (i = 0 ; i < count; i++) {
                if (i == 0 && password_clean == 1) {
                    //memset(user[i].password, 0, sizeof(user[i].password));
                    //memset(user[i].password_ex, 0, sizeof(user[i].password_ex));
                } else {
                    snprintf(user[i].password, sizeof(user[i].password), "%s", o_user[i].password);
                    snprintf(user[i].password_ex, sizeof(user[i].password_ex), "%s", o_user[i].password_ex);
                }
            }
        }
        for (i = 0; i < count; i++) {
            for (j = 0; j < 32; j++) {
                if ((int)((user[i].local_live_view >> j) & 0x01) != (int)(user[i].local_live_view_ex[j] - '0')) {
                    user[i].local_live_view |= 0x01 << j;
                    user[i].local_live_view_ex[j] = '1';
                }
                if ((int)((user[i].local_playback >> j) & 0x01) != (int)(user[i].local_playback_ex[j] - '0')) {
                    user[i].local_playback |= 0x01 << j;
                    user[i].local_playback_ex[j] = '1';
                }
                if ((int)((user[i].remote_live_view >> j) & 0x01) != (int)(user[i].remote_live_view_ex[j] - '0')) {
                    user[i].remote_live_view |= 0x01 << j;
                    user[i].remote_live_view_ex[j] = '1';
                }
                if ((int)((user[i].remote_playback >> j) & 0x01) != (int)(user[i].remote_playback_ex[j] - '0')) {
                    user[i].remote_playback |= 0x01 << j;
                    user[i].remote_playback_ex[j] = '1';
                }
            }
            write_user(SQLITE_FILE_NAME, &user[i]);
        }

        //p2p
        memset(&p2p, 0, sizeof(p2p));
        read_p2p_info(SQLITE_OEM_NAME, &p2p);
        write_p2p_info(SQLITE_FILE_NAME, &p2p);

        //layout
        memset(&layout, 0, sizeof(layout));
        read_layout(SQLITE_OEM_NAME, &layout);
        write_layout(SQLITE_FILE_NAME, &layout);

        //display
        memset(&display, 0, sizeof(display));
        if (read_display(SQLITE_OEM_NAME, &display)) {
            read_display_oem(SQLITE_OEM_NAME, &display);
        }
        write_display(SQLITE_FILE_NAME, &display);

        //ONVIF, RTSP, Milesight MSSP, CANON
        count = 0;
        read_ipc_protocols(SQLITE_OEM_NAME, &protocol, &count);
        delete_ipc_protocols(SQLITE_FILE_NAME);
        for (i = 0; i < count; i++) {
            insert_ipc_protocol(SQLITE_FILE_NAME, &protocol[i]);
        }

        count = 0;
        read_ipc_models(SQLITE_OEM_NAME, &model, &count);
        delete_ipc_models(SQLITE_FILE_NAME);
        for (i = 0; i < count; i++) {
            insert_ipc_model(SQLITE_FILE_NAME, &model[i]);
        }
        //camera record stream
        memset(cameraArray, 0, sizeof(struct camera) * MAX_CAMERA);
        memset(cameraOemArray, 0, sizeof(struct camera) * MAX_CAMERA);
        read_cameras(SQLITE_FILE_NAME, cameraArray, &cameraCount);
        read_cameras(SQLITE_OEM_NAME, cameraOemArray, &cameraCount);
        for (i = 0; i < cameraCount; ++i) {
            cameraArray[i].record_stream = cameraOemArray[i].record_stream;
            dbChn |= (Uint64)1 << i;
        }
        if (dbChn) {
            write_cameras(SQLITE_FILE_NAME, cameraArray, dbChn);
        }

        //NEW SEARCH PROTOCOL
        //ipcpro.pro_id = IPC_PROTOCOL_CANON;
        int no_canon_pro = read_ipc_protocol(SQLITE_FILE_NAME, &ipcpro, IPC_PROTOCOL_CANON);
        int no_canon_mod = read_ipc_model(SQLITE_FILE_NAME, &ipcmodel, IPC_PROTOCOL_CANON);
        if (device_oem.oem_type == OEM_TYPE_CANON && (no_canon_pro || no_canon_mod)) {
            ipcmodel.mod_id = IPC_PROTOCOL_CANON;
            ipcmodel.pro_id = IPC_PROTOCOL_CANON;
            ipcmodel.alarm_type = 0;
            ipcmodel.default_model = 1;
            snprintf(ipcmodel.mod_name, sizeof(ipcmodel.mod_name), "CANON");

            ipcpro.pro_id = IPC_PROTOCOL_CANON;
            ipcpro.function = IPC_FUNC_SEARCHABLE | IPC_FUNC_SETPARAMS | IPC_FUNC_SETIPADDR;
            ipcpro.enable = 1;
            ipcpro.display_model = 0;
            snprintf(ipcpro.pro_name, sizeof(ipcpro.pro_name), "CANON");

            if (no_canon_pro) {
                insert_ipc_model(SQLITE_FILE_NAME, &ipcmodel);
            }
            if (no_canon_mod) {
                insert_ipc_protocol(SQLITE_FILE_NAME, &ipcpro);
            }
        }
        release_ipc_protocol(&protocol);
        if (model) {
            ipc_destroy(model);
        }
        model = 0;

        //record
        memset(&records, 0, sizeof(struct record)*MAX_CAMERA);
        read_records(SQLITE_OEM_NAME, records, &count);
        for (i = 0; i < count; i++) {
            write_record(SQLITE_FILE_NAME, &records[i]);
        }

        //record schdule
        for (i = 0; i < MAX_CAMERA; i++) {
            memset(&schedule, 0, sizeof(struct record_schedule));
            read_record_schedule(SQLITE_OEM_NAME, &schedule, i);
            write_record_schedule(SQLITE_FILE_NAME, &schedule, i);
        }

        //step 2
        db_set_device_oem(SQLITE_FILE_NAME, &device_oem);
        set_param_int(SQLITE_FILE_NAME, PARAM_OEM_TYPE, oem_index);
        set_param_int(SQLITE_FILE_NAME, PARAM_DEVICE_DEF_LANG, device_oem.def_lang);
        set_param_value(SQLITE_FILE_NAME, PARAM_DEVICE_LANG, device_oem.lang);
        set_param_int(SQLITE_FILE_NAME, PARAM_DEVICE_ID, device_oem.devid);
        set_param_int(SQLITE_FILE_NAME, PARAM_MS_DDNS, device_oem.msddns);
        set_param_int(SQLITE_FILE_NAME, PARAM_GUI_CONFIG, get_param_int(SQLITE_OEM_NAME, PARAM_GUI_CONFIG, 1));
        set_param_int(SQLITE_FILE_NAME, PARAM_GUI_CHAN_COLOR, get_param_int(SQLITE_OEM_NAME, PARAM_GUI_CHAN_COLOR, 0));
        set_param_int(SQLITE_FILE_NAME, PARAM_GUI_SKIP_BLANK, get_param_int(SQLITE_OEM_NAME, PARAM_GUI_SKIP_BLANK, 1));
        set_param_int(SQLITE_FILE_NAME, PARAM_GUI_AUTH, get_param_int(SQLITE_OEM_NAME, PARAM_GUI_AUTH, 1));
        set_param_int(SQLITE_FILE_NAME, PARAM_GUI_MENU_AUTH, get_param_int(SQLITE_OEM_NAME, PARAM_GUI_MENU_AUTH, 0));
        set_param_int(SQLITE_FILE_NAME, PARAM_GUI_MENU_TIME_OUT, get_param_int(SQLITE_OEM_NAME, PARAM_GUI_MENU_TIME_OUT, 0));
        set_param_int(SQLITE_FILE_NAME, PARAM_GUI_WIZARD_ENABLE, get_param_int(SQLITE_OEM_NAME, PARAM_GUI_WIZARD_ENABLE, 1));
        set_param_int(SQLITE_FILE_NAME, PARAM_GUI_DISPLAY_MODE, get_param_int(SQLITE_OEM_NAME, PARAM_GUI_DISPLAY_MODE, 1));
        set_param_int(SQLITE_FILE_NAME, PARAM_GUI_AUTO_LOGOUT, get_param_int(SQLITE_OEM_NAME, PARAM_GUI_AUTO_LOGOUT, 0));

        //////poe password
        get_param_value(SQLITE_OEM_NAME, PARAM_POE_PSD, poe_password, sizeof(poe_password), POE_DEFAULT_PASSWORD);
        set_param_value(SQLITE_FILE_NAME, PARAM_POE_PSD, poe_password);

        raidmode = get_param_int(SQLITE_OEM_NAME, PARAM_RAID_MODE, 0);
        set_param_int(SQLITE_FILE_NAME, PARAM_RAID_MODE, raidmode);


        memset(&conf, 0, sizeof(conf));
        if (!read_autoreboot_conf(SQLITE_OEM_NAME, &conf)) {
            write_autoreboot_conf(SQLITE_FILE_NAME, &conf);
        }
    }

    get_param_value(SQLITE_INIT_FILE_PATH, PARAM_DEVICE_SV, path, sizeof(path), "61.7.0.5");
    get_param_value(SQLITE_FILE_NAME, PARAM_DEVICE_SV, temp, sizeof(temp), "61.7.0.5");
    msprintf("optDB software_version:%s MSDB software_version:%s", path, temp);

    if (oem_index == cur_index && !strcmp(path, temp)) {
        return 0;
    } else if (strcmp(path, temp)) {
        set_param_value(SQLITE_INIT_FILE_PATH, PARAM_DEVICE_SV, temp);
    }

    //new update image,must update filesystem
    //step 3
    if (oem_index == cur_index) {
        db_get_device(SQLITE_OEM_NAME, &device_oem);
    }



    //UI.logo.jpg
    snprintf(path, sizeof(path), "%s/%s", OEM_ROOT_DIR, QT_START_LOGO);
    if (!access(path, F_OK)) {
        snprintf(path, sizeof(path), "cp %s/%s %s/", OEM_ROOT_DIR, QT_START_LOGO, UI_PATH);
        ms_system(path);
    }

    //gui.rcc
    snprintf(path, sizeof(path), "%s/%s", OEM_ROOT_DIR, QT_RCC);
    if (!access(path, F_OK)) {
        snprintf(path, sizeof(path), "cp %s/%s %s/", OEM_ROOT_DIR, QT_RCC, UI_PATH);
        ms_system(path);
    }

    //web
    snprintf(path, sizeof(path), "%s/%s", OEM_ROOT_DIR, WEB_RCC);
    if (!access(path, F_OK)) {
        snprintf(path, sizeof(path), "chmod 777 -R %s", MS_WEB_PATH);
        ms_system(path);
        snprintf(path, sizeof(path), "tar -zxvf %s/%s -C %s/", OEM_ROOT_DIR, WEB_RCC, MS_WEB_PATH);
        ms_system(path);
    }

    if (oem_index != cur_index) {
        //Qt language set default lang
        snprintf(path, sizeof(path), "echo \"[General]\nlanguage=%d\n\" > %s", device_oem.def_lang, GUI_LANG_INI);
        ms_system(path);
    }

    //language.json set default lang
    switch (device_oem.def_lang) {
        case LNG_ENGLISH:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "en-US");
            break;
        case LNG_CHINESE:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "zh-CN");
            break;
        case LNG_CHINESE_TW:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "zh-TW");
            break;
        case LNG_MAGYAR:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "hu-HU");
            break;
        case LNG_RUSSIAN:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "ru-RU");
            break;
        case LNG_FRENCH:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "fr-FR");
            break;
        case LNG_POLSKA:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "pl-PL");
            break;
        case LNG_DANISH:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "da-DK");
            break;
        case LNG_ITALIAN:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "it-IT");
            break;
        case LNG_GERMAN:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "de-DE");
            break;
        case LNG_CZECH:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "cz-CZ");
            break;
        case LNG_JAPANESE:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "jp-JP");
            break;
        case LNG_KOREAN:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "ko-KR");
            break;
        case LNG_FINNISH:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "fi-FI");
            break;
        case LNG_TURKEY:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "tr-TR");
            break;
        case LNG_HOLLAND:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "nl-NL");
            break;
        case LNG_ARABIC:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "ar-EG");
            break;
        case LNG_HEBREW:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "hb-HB");
            break;
        case LNG_ISRAEL:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "iw-IL");
            break;
        case LGN_PERSIAN:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "fa-IR");
            break;
        case LGN_NEDERLANDS:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "nl-NL");
            break;
        case LGN_SPAIN:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "es-ES");
            break;
        default:
            snprintf(path, sizeof(path), DEF_LANG_TEXT, "en-US");
            break;
    }
    if (atoi(device_oem.lang) < 0) {
        snprintf(device_oem.lang, sizeof(device_oem.lang), "0");
    }
    if (atoi(device_oem.lang) & (1 << LNG_ENGLISH)) {
        snprintf(path + strlen(path), sizeof(path) - strlen(path),
                 "{\"value\":\"en-US\",\"name\":\"English\",\"isDefault\":true},");
    }
    if (atoi(device_oem.lang) & (1 << LNG_CZECH)) {
        snprintf(path + strlen(path), sizeof(path) - strlen(path),
                 "{\"value\":\"cz-CZ\",\"name\":\"Čeština\",\"isDefault\":false},");
    }
    if (atoi(device_oem.lang) & (1 << LNG_GERMAN)) {
        snprintf(path + strlen(path), sizeof(path) - strlen(path),
                 "{\"value\":\"de-DE\",\"name\":\"Deutsch\",\"isDefault\":false},");
    }
    if (atoi(device_oem.lang) & (1 << LNG_DANISH)) {
        snprintf(path + strlen(path), sizeof(path) - strlen(path),
                 "{\"value\":\"da-DK\",\"name\":\"Danish\",\"isDefault\":false},");
    }
    if (atoi(device_oem.lang) & (1 << LNG_FRENCH)) {
        snprintf(path + strlen(path), sizeof(path) - strlen(path),
                 "{\"value\":\"fr-FR\",\"name\":\"Français\",\"isDefault\":false},");
    }
    if (atoi(device_oem.lang) & (1 << LNG_RUSSIAN)) {
        snprintf(path + strlen(path), sizeof(path) - strlen(path),
                 "{\"value\":\"ru-RU\",\"name\":\"Pусский\",\"isDefault\":false},");
    }
    if (atoi(device_oem.lang) & (1 << LNG_JAPANESE)) {
        snprintf(path + strlen(path), sizeof(path) - strlen(path),
                 "{\"value\":\"jp-JP\",\"name\":\"日本語\",\"isDefault\":false},");
    }
    if (atoi(device_oem.lang) & (1 << LNG_KOREAN)) {
        snprintf(path + strlen(path), sizeof(path) - strlen(path),
                 "{\"value\":\"ko-KR\",\"name\":\"한국어\",\"isDefault\":false},");
    }
    if (atoi(device_oem.lang) & (1 << LNG_FINNISH)) {
        snprintf(path + strlen(path), sizeof(path) - strlen(path),
                 "{\"value\":\"fi-FI\",\"name\":\"Suomi\",\"isDefault\":false},");
    }
    if (atoi(device_oem.lang) & (1 << LNG_TURKEY)) {
        snprintf(path + strlen(path), sizeof(path) - strlen(path),
                 "{\"value\":\"tr-TR\",\"name\":\"Türkçe\",\"isDefault\":false},");
    }
    /*
    if (atoi(device_oem.lang) & (1 << LNG_HOLLAND)) {
        snprintf(path + strlen(path), sizeof(path) - strlen(path),
                 "{\"value\":\"nl-NL\",\"name\":\"Nederlands\",\"isDefault\":false},");
    }
    */
    if (atoi(device_oem.lang) & (1 << LNG_ITALIAN)) {
        snprintf(path + strlen(path), sizeof(path) - strlen(path),
                 "{\"value\":\"it-IT\",\"name\":\"Italiano\",\"isDefault\":false},");
    }
    if (atoi(device_oem.lang) & (1 << LNG_MAGYAR)) {
        snprintf(path + strlen(path), sizeof(path) - strlen(path),
                 "{\"value\":\"hu-HU\",\"name\":\"Magyar\",\"isDefault\":false},");
    }
    if (atoi(device_oem.lang) & (1 << LNG_POLSKA)) {
        snprintf(path + strlen(path), sizeof(path) - strlen(path),
                 "{\"value\":\"pl-PL\",\"name\":\"Polski\",\"isDefault\":false},");
    }
    if (atoi(device_oem.lang) & (1 << LNG_ISRAEL)) {
        snprintf(path + strlen(path), sizeof(path) - strlen(path),
                 "{\"value\":\"iw_IL\",\"name\":\"Hebrew\",\"isDefault\":false},");
    }
    if (atoi(device_oem.lang) & (1 << LGN_PERSIAN)) {
        snprintf(path + strlen(path), sizeof(path) - strlen(path),
                 "{\"value\":\"fa-IR\",\"name\":\"Persian\",\"isDefault\":false},");
    }
    if (atoi(device_oem.lang) & (1 << LGN_NEDERLANDS)) {
        snprintf(path + strlen(path), sizeof(path) - strlen(path),
                 "{\"value\":\"nl-NL\",\"name\":\"Nederlands\",\"isDefault\":false},");
    }
    if (atoi(device_oem.lang) & (1 << LGN_SPAIN)) {
        snprintf(path + strlen(path), sizeof(path) - strlen(path),
                 "{\"value\":\"es-ES\",\"name\":\"Español\",\"isDefault\":false},");
    }
    if (atoi(device_oem.lang) & (1 << LNG_CHINESE)) {
        snprintf(path + strlen(path), sizeof(path) - strlen(path),
                 "{\"value\":\"zh-CN\",\"name\":\"简体中文\",\"isDefault\":false},");
    }
    if (atoi(device_oem.lang) & (1 << LNG_CHINESE_TW)) {
        snprintf(path + strlen(path), sizeof(path) - strlen(path),
                 "{\"value\":\"zh-TW\",\"name\":\"繁體中文\",\"isDefault\":false},");
    }

    if (path[strlen(path) - 1] == ',') {
        snprintf(path + strlen(path) - 1, sizeof(path) - strlen(path) + 1, "]}");
    } else {
        snprintf(path + strlen(path), sizeof(path) - strlen(path), "]}");
    }

    if (device_oem.def_lang || atoi(device_oem.lang) != 65535) {
        snprintf(temp, sizeof(temp), "%s/%s/languages.json", MS_WEB_PATH, WEB_LANG_PATH);
        fp = fopen(temp, "wb");
        if (fp) {
            fwrite(path, strlen(path), 1, fp);
            fflush(fp);
            fclose(fp);
        }
    }

    //change copyright
    snprintf(path, sizeof(path), "%s/%s/en-US.json", MS_WEB_PATH, WEB_LANG_PATH);
    fold_size = get_file_size(path);
    if (!strstr(device_oem.company, "Copyright")) {
        snprintf(path, sizeof(path),
                 "sed -i '/labCopyRight/c \"labCopyRight\":\"Copyright &copy; %s All rights reserved.\",' `find %s/%s -name \"*.json\"`",
                 device_oem.company, MS_WEB_PATH, WEB_LANG_PATH);
    } else {
        snprintf(path, sizeof(path), "sed -i '/labCopyRight/c \"labCopyRight\":\"%s\",' `find %s/%s -name \"*.json\"`",
                 device_oem.company, MS_WEB_PATH, WEB_LANG_PATH);
    }
    ms_system(path);

    //change title
    if (oem_index != OEM_TYPE_STANDARD) {
        /*
        0=标准： Milesight Netwoek Video Recoder
        1=中性： Onvif Netwoek Video Recoder
        其他=oem： 【Company】 Network Video Recoder
        */
        if (oem_index == OEM_TYPE_NORMAL) {
            snprintf(path, sizeof(path),
                 "sed -i '/labDocumentTitle/c \"labDocumentTitle\":\"ONVIF Network Video Recoder\",' `find %s/%s -name \"*.json\"`",
                 MS_WEB_PATH, WEB_LANG_PATH);
        } else {
            snprintf(path, sizeof(path),
                 "sed -i '/labDocumentTitle/c \"labDocumentTitle\":\"%s Network Video Recoder\",' `find %s/%s -name \"*.json\"`",
                 device_oem.company, MS_WEB_PATH, WEB_LANG_PATH);
        }

        ms_system(path);
    }

    count = 10;
    while (count--) {
        ms_system("sync");
        snprintf(path, sizeof(path), "%s/%s/en-US.json", MS_WEB_PATH, WEB_LANG_PATH);
        fnew_size = get_file_size(path);
        if (!fnew_size || (fnew_size < (fold_size - 256))) {
            ms_system("sync");
            continue;
        }
        break;
    }

    return 0;
}
static int is_oem_need_to_update()
{
    char path[256] = {0};
    struct device_info device = {0};
    if (!access(UP_NEW_OEM_CFG, F_OK)) {
        if (!access("/opt/app/bin/oem.tar.gz", F_OK)) {
            snprintf(path, sizeof(path), "chmod 777 -R %s", UI_PATH);
            ms_system(path);
            snprintf(path, sizeof(path), "tar -zxvf %s -C %s/", "/opt/app/bin/oem.tar.gz", UI_PATH);
            ms_system(path);
            unlink("/opt/app/bin/oem.tar.gz");
        }

        //UI.logo.jpg
        snprintf(path, sizeof(path), "%s/%s", OEM_ROOT_DIR, QT_START_LOGO);
        if (!access(path, F_OK)) {
            snprintf(path, sizeof(path), "cp %s/oem/%s %s/", UI_PATH, QT_START_LOGO, OEM_ROOT_DIR);
            ms_system(path);
            snprintf(path, sizeof(path), "cp %s/oem/%s %s/", UI_PATH, QT_START_LOGO, UI_PATH);
            ms_system(path);
        }

        //gui.rcc
        snprintf(path, sizeof(path), "%s/%s", OEM_ROOT_DIR, QT_RCC);
        if (!access(path, F_OK)) {
            snprintf(path, sizeof(path), "cp %s/oem/%s %s/", UI_PATH, QT_RCC, OEM_ROOT_DIR);
            ms_system(path);
            snprintf(path, sizeof(path), "cp %s/oem/%s %s/", UI_PATH, QT_RCC, UI_PATH);
            ms_system(path);
        }

        //web
        snprintf(path, sizeof(path), "%s/%s", OEM_ROOT_DIR, WEB_RCC);
        if (!access(path, F_OK)) {
            snprintf(path, sizeof(path), "cp %s/oem/%s %s/", UI_PATH, WEB_RCC, OEM_ROOT_DIR);
            ms_system(path);
            snprintf(path, sizeof(path), "tar -zxvf %s/oem/%s -C %s/", UI_PATH, WEB_RCC, MS_WEB_PATH);
            ms_system(path);
        }

        //oem.db
        db_get_device(SQLITE_FILE_NAME, &device);
        if (device.oem_type == OEM_TYPE_KAREL || device.oem_type == OEM_TYPE_MARCO) {
            get_param_value(SQLITE_OEM_NAME, PARAM_DEVICE_MODEL, device.model, sizeof(device.model), device.model);
        }
        msdebug(DEBUG_ERR, "[david debug] model:%s oem_type:%d", device.model, device.oem_type);
        snprintf(path, sizeof(path), "%s/%s", OEM_ROOT_DIR, OEM_DB);
        if (!access(path, F_OK)) {
            if (device.model[0] != '\0' && device.oem_type == OEM_TYPE_KAREL) {
                snprintf(path, sizeof(path), "cp %s/oem/%s.db %s/%s", UI_PATH, device.model, OEM_ROOT_DIR, OEM_DB);
            } else {
                snprintf(path, sizeof(path), "cp %s/oem/%s %s/", UI_PATH, OEM_DB, OEM_ROOT_DIR);
            }
            ms_system(path);
        }

        if (device.oem_type == OEM_TYPE_MARCO) {
            if (device.model[0] != '\0') {
                set_param_value(SQLITE_OEM_NAME, PARAM_DEVICE_MODEL, device.model);
            }
        }

        //rm oem dir
        snprintf(path, sizeof(path), "rm -rf %s/oem", UI_PATH);
        ms_system(path);

        unlink(UP_NEW_OEM_CFG);
    }

    return 0;
}


static void is_same_channel_exist()
{
    int i = 0;
    int j = 0;
    int cnt = 0;
    struct camera cameras[MAX_CAMERA];
    struct camera tmp_cam;
    int same_channel = get_param_int(SQLITE_FILE_NAME, PARAM_SAME_CHANNEL, 0);

    if (same_channel != 2) {
        //need calibration
        memset(&tmp_cam, 0x0, sizeof(struct camera));
        memset(cameras, 0x0, sizeof(struct camera) *MAX_CAMERA);
        read_cameras(SQLITE_FILE_NAME, cameras, &cnt);
        for (i = 0; i < MAX_CAMERA; i++) {
            if (cameras[i].enable == 0) {
                continue;
            }
            if (cameras[i].camera_protocol == IPC_PROTOCOL_RTSP) {
                continue;
            }
            for (j = i + 1; j < MAX_CAMERA; j++) {
                if (cameras[j].enable == 0) {
                    continue;
                }
                if (cameras[i].camera_protocol == IPC_PROTOCOL_RTSP) {
                    continue;
                }
                if (cameras[i].minorid != cameras[j].minorid) {
                    continue;
                }

                if (cameras[i].manage_port != cameras[j].manage_port) {
                    continue;
                }

                if (!strcmp(cameras[i].ip_addr, cameras[j].ip_addr)) {
                    cameras[j].enable = 0;
                    tmp_cam.id = cameras[j].id;
                    write_camera(SQLITE_FILE_NAME, &tmp_cam);
                }
            }
        }

        if (same_channel != 1) { //从9.0.10降级删除重复通道
            set_param_int(SQLITE_FILE_NAME, PARAM_SAME_CHANNEL, 1);
        }
    }

    return;
}

static void deal_auto_backup_db()
{
    ESATA_AUTO_BACKUP esataBackupDb;
    int value;

    value = get_param_int(SQLITE_FILE_NAME, PARAM_AUTOBACKUP_DBUP, 0);
    if (!value) {
        return;
    }
    memset(&esataBackupDb, 0, sizeof(ESATA_AUTO_BACKUP));
    read_esata_auto_backup(SQLITE_FILE_NAME, &esataBackupDb);
    if (esataBackupDb.enable == 1 && esataBackupDb.device == AUTO_BACKUP_SELECT) {
        esataBackupDb.device = AUTO_BACKUP_ESATA;
        esataBackupDb.port = 25;
        write_esata_auto_backup(SQLITE_FILE_NAME, &esataBackupDb);
    }
    set_param_int(SQLITE_FILE_NAME, PARAM_AUTOBACKUP_DBUP, 0);
}

static void failover_init()
{
    int i = 0, j = 0, g = 0, cnt = 0;
    char failover_path[128] = {0}, path[256] = {0}, cmd[128] = {0};
    struct record_schedule schedule;
    struct failover_list failover;

    snprintf(failover_path, sizeof(failover_path), "%s/%s", MS_CONF_DIR, FAILOVER_SLAVE_INIT);
    if (!access(failover_path, F_OK)) {
        //1.sechdule
        for (i = 0; i < MAX_CAMERA; i++) {
            reset_record_schedule(SQLITE_FILE_NAME, i);
            memset(&schedule, 0, sizeof(struct record_schedule));
            read_record_schedule(SQLITE_FILE_NAME, &schedule, i);
            for (j = 0; j < MAX_DAY_NUM; j++) {
                g = 0;
                schedule.schedule_day[j].schedule_item[g].action_type = TIMING_RECORD;
                strcpy(schedule.schedule_day[j].schedule_item[g].start_time, "00:00");
                strcpy(schedule.schedule_day[j].schedule_item[g].end_time, "24:00");
                for (g = 1; g < MAX_WND_NUM * MAX_PLAN_NUM_PER_DAY; g++) {
                    schedule.schedule_day[j].schedule_item[g].action_type = NONE;
                    strcpy(schedule.schedule_day[j].schedule_item[g].start_time, "00:00");
                    strcpy(schedule.schedule_day[j].schedule_item[g].end_time, "00:00");
                }
            }

            write_record_schedule(SQLITE_FILE_NAME, &schedule, i);
        }

        reset_schedule_for_failover(SQLITE_FILE_NAME);

        //2.camera
        struct camera cameras[MAX_CAMERA];
        struct camera tmpCam;
        memset(&tmpCam, 0x0, sizeof(struct camera));
        tmpCam.type = 1;
        memset(cameras, 0x0, sizeof(struct camera) *MAX_CAMERA);
        read_cameras(SQLITE_FILE_NAME, cameras, &cnt);
        for (i = 0; i < MAX_CAMERA; i++) {
            if (cameras[i].enable == 1 || cameras[i].record_stream) {
                tmpCam.id = cameras[i].id;
                write_camera(SQLITE_FILE_NAME, &tmpCam);
            }
        }

        //3.time
        struct time times;
        read_time(SQLITE_FILE_NAME, &times);
        times.ntp_enable = 0;
        times.dst_enable = 0;
        write_time(SQLITE_FILE_NAME, &times);

        //4.failover_list
        memset(&failover, 0x0, sizeof(struct failover_list));
        int failover_mode = get_param_int(SQLITE_FILE_NAME, PARAM_FAILOVER_MODE, 0);
        if (failover_mode == FAILOVER_MODE_MASTER) {
            i = 1;
        } else {
            i = 0;
        }
        for (; i < MAX_FAILOVER; i++) {
            failover.id = i;
            write_failover(SQLITE_FILE_NAME, &failover);
        }

        //5.disk recycle
        set_param_int(SQLITE_FILE_NAME, PARAM_RECYCLE_MODE, 1);

        //6.general event popup
        struct display db_display;
        read_display(SQLITE_FILE_NAME, &db_display);
        if (db_display.eventPop_screen != 0) {
            db_display.eventPop_screen = 0;
            write_display(SQLITE_FILE_NAME, &db_display);
        }

        //7.delete cfg file
        snprintf(cmd, sizeof(cmd), "rm -rf %s*.cfg", FAILOVER_CFG_PATH);
        ms_system(cmd);

        //8.auto reboot disable
        reboot_conf conf = {0};
        read_autoreboot_conf(SQLITE_FILE_NAME, &conf);
        conf.enable = 0;
        write_autoreboot_conf(SQLITE_FILE_NAME, &conf);

        //9.p2p
        struct p2p_info p2p;
        memset(&p2p, 0x0, sizeof(struct p2p_info));
        read_p2p_info(SQLITE_FILE_NAME, &p2p);
        if (p2p.enable) {
            p2p.enable = 0;
            write_p2p_info(SQLITE_FILE_NAME, &p2p);
        }

        //end delete
        snprintf(path, sizeof(path), "rm -f %s/%s", MS_CONF_DIR, FAILOVER_ANR_TIME);
        ms_system(path);

        snprintf(path, sizeof(path), "rm -f %s", failover_path);
        ms_system(path);
    }
}

static int update_anpr_db_version()
{
    int cnt = 0;
    int mnt_anpr_ver = -1, opt_anpr_ver = -1;
    char cmd[64] = {0};
    struct anpr_list *anprList = NULL;

    if (ms_check_file_exist(SQLITE_ANPR_NAME)
        || ms_check_file_exist(SQLITE_INIT_ANPR_PATH)) {
        return -1;
    }
    mnt_anpr_ver = read_db_version(SQLITE_ANPR_NAME);
    opt_anpr_ver = read_db_version(SQLITE_INIT_ANPR_PATH);
    if (mnt_anpr_ver < 0 || opt_anpr_ver < 0) {
        return -1;
    }
    if (opt_anpr_ver > mnt_anpr_ver) {
        //msprintf("update anpr.db(ver %d -> %d) now.", mnt_anpr_ver, opt_anpr_ver);
        cnt = read_anpr_list_cnt(SQLITE_ANPR_NAME);
        snprintf(cmd, sizeof(cmd), "cp -f %s %s", SQLITE_INIT_ANPR_PATH, MS_CONF_DIR);
        if (cnt <= 0) {
            ms_system(cmd);
        } else {
            read_anpr_lists(SQLITE_ANPR_NAME, &anprList, &cnt);
            ms_system(cmd);
            write_anpr_lists(SQLITE_ANPR_NAME, anprList, cnt);
            release_anpr_lists(&anprList);
        }
    }

    return 0;
}

//email enable 初始值为-1，判断界面是否有填写内容，有的话enable为1，否则为0
static void update_email_enable()
{
    struct email email;
    int i = 0, receiver_null_cnt = 0;

    memset(&email, 0x0, sizeof(struct email));
    read_email(SQLITE_FILE_NAME, &email);

    if (email.enable != -1) {
        return;
    }

    email.enable = 1;
    if (email.sender_addr[0] == '\0' && email.password[0] == '\0') {
        email.enable = 0;
    } else {
        for (i = 0; i < EMAIL_RECEIVER_NUM; i++) {
            if (email.receiver[i].address[0] == '\0') {
                receiver_null_cnt++;
                continue;
            }
        }
        if (receiver_null_cnt >= EMAIL_RECEIVER_NUM) {
            email.enable = 0;
        }
    }

    write_email(SQLITE_FILE_NAME, &email);
    return;
}

void ms_sys_init()
{
    int i = 0, cnt = 0;
    int x = 0, y = 0;
    int state = -1;
    long ret = -1;
    char path[256] = {0}, ip_path[256] = {0}, user_path[256] = {0};
    char keep_confle[256] = {0}, keep_user_confile[256] = {0}, keep_encrypted_confile[256] = {0};
    char chip_info[64] = {0};
    struct network network;
    struct network net_db;
    struct record records[MAX_CAMERA];
    struct record_schedule schedule;
    struct motion_schedule m_schedule;
    struct video_loss_schedule v_schedule;
    struct alarm_in_schedule a_schedule;
    struct motion move;
    struct video_loss videolost;
    struct alarm_in alarm;

    memset(&schedule, 0, sizeof(struct record_schedule));
    memset(&records, 0, sizeof(struct record)*MAX_CAMERA);
    memset(&network, 0, sizeof(network));
    memset(&net_db, 0, sizeof(net_db));

    snprintf(path, sizeof(path), "%s/%s", MS_CONF_DIR, MS_RESET_FLAG_FILE);
    snprintf(ip_path, sizeof(ip_path), "%s/%s", MS_CONF_DIR, MS_RESET_FLAG_IP_FILE);
    snprintf(user_path, sizeof(user_path), "%s/%s", MS_CONF_DIR, MS_RESET_FLAG_USER_FILE);
    snprintf(keep_confle, sizeof(keep_confle), "%s/%s", OEM_ROOT_DIR, MS_KEEP_CONF_FILE);
    snprintf(keep_user_confile, sizeof(keep_user_confile), "%s/%s", OEM_ROOT_DIR, MS_KEEP_USER_CONF_FILE);
    snprintf(keep_encrypted_confile, sizeof(keep_encrypted_confile), "%s/%s", OEM_ROOT_DIR, MS_KEEP_ENCRYPTED_CONF_FILE);

    if (check_msdb_database(SQLITE_FILE_NAME) > 0
        && check_msdb_database(SQLITE_BAK_NAME) == 0) {
        update_msdb_database();
    }

    if (!access(path, F_OK)) { // reset
        msprintf("eraseall");
        if (!access(SQLITE_FILE_NAME, F_OK) && access(ip_path, F_OK) == -1) { //eraseip no exists, keep ip
            write_keep_config(keep_confle, SQLITE_FILE_NAME);
        }
        if (!access(SQLITE_FILE_NAME, F_OK) && access(user_path, F_OK) == -1) { //eraseuser no exists, keep user
            // keep_user = 1;
            write_keep_user_config(keep_user_confile, SQLITE_FILE_NAME);
            write_keep_encrypted_config(keep_encrypted_confile, SQLITE_FILE_NAME);
        }
        write_keep_params_config(MS_KEEP_POEPWD_CONF_FILE, SQLITE_FILE_NAME);

        snprintf(path, sizeof(path), "rm -f %s/*", MS_CONF_DIR);
        ms_system(path);
        snprintf(path, sizeof(path), "rm -f %s/*", MS_ETC_DIR);
        ms_system(path);
        snprintf(path, sizeof(path), "rm -f %s/*", MS_NAND_DIR);
        ms_system(path);
        snprintf(path, sizeof(path), "rm -f %s/smtp/*", MS_NAND_DIR);
        ms_system(path);

        snprintf(path, sizeof(path), "cp -f %s %s", SQLITE_INIT_FILE_PATH, MS_CONF_DIR);
        ms_system(path);

        snprintf(path, sizeof(path), "cp -f %s %s", SQLITE_INIT_PCNT_PATH, MS_CONF_DIR);
        ms_system(path);

        snprintf(path, sizeof(path), "cp -f %s %s", EMAIL_INIT_FILE_PATH, MS_CONF_DIR);
        ms_system(path);

        snprintf(path, sizeof(path), "cp -f %s %s", SQLITE_INIT_PUSHMSG_PATH, MS_CONF_DIR);
        ms_system(path);

        snprintf(path, sizeof(path), "rm -f %s/%s", MS_CONF_DIR, FAILOVER_ANR_TIME);
        ms_system(path);
    }

    //擦除oem时会重置，这里的logo.jpg和gui.rcc的备份没意义
    snprintf(path, sizeof(path), "%s/%s", OEM_ROOT_DIR, START_LOGO_STANDER);
    //if (access(path, F_OK))
    {
        snprintf(path, sizeof(path), "cp -f %s/logo.jpg %s/%s", MS_EXE_PATH, OEM_ROOT_DIR, START_LOGO_STANDER);
        ms_system(path);
    }
    snprintf(path, sizeof(path), "%s/%s", OEM_ROOT_DIR, QT_RCC_STANDER);
    //if(access(path, F_OK))
    {
        snprintf(path, sizeof(path), "cp -f %s/gui.rcc %s/%s", MS_EXE_PATH, OEM_ROOT_DIR, QT_RCC_STANDER);
        ms_system(path);
    }

    if (access(SQLITE_FILE_NAME, F_OK)) { //not exist
        snprintf(path, sizeof(path), "cp -f %s %s", SQLITE_INIT_FILE_PATH, MS_CONF_DIR);
        ms_system(path);
    }
    
    check_table_exist(SQLITE_FILE_NAME, "db_version", &state);
    if (!state) {
        int msdbVer = get_param_int(SQLITE_FILE_NAME, PARAM_DB_VERSION, -1);
        create_tbl_db_version(SQLITE_FILE_NAME, msdbVer);
    }

    if (access(EMAIL_DB_PATH, F_OK)) { //not exist
        snprintf(path, sizeof(path), "cp -f %s %s", EMAIL_INIT_FILE_PATH, MS_CONF_DIR);
        ms_system(path);
    }
    if (access(SQLITE_PEOPLE_NAME, F_OK)) { //not exist
        snprintf(path, sizeof(path), "cp -f %s %s", SQLITE_INIT_PCNT_PATH, MS_CONF_DIR);
        ms_system(path);
    }
    if (access(SQLITE_ANPR_NAME, F_OK)) { //not exist
        snprintf(path, sizeof(path), "cp -f %s %s", SQLITE_INIT_ANPR_PATH, MS_CONF_DIR);
        ms_system(path);
    } else {
        update_anpr_db_version();
    }
    ret = get_file_size(SQLITE_PUSHMSG_NAME);
    if (ret <= 0) {
        snprintf(path, sizeof(path), "cp -f %s %s", SQLITE_INIT_PUSHMSG_PATH, MS_CONF_DIR);
        ms_system(path);
    }
    ms_system("sync");

    while (get_file_size(SQLITE_FILE_NAME) < get_file_size(SQLITE_INIT_FILE_PATH)) {
        msprintf("ERROR==%lu < %lu", get_file_size(SQLITE_FILE_NAME), get_file_size(SQLITE_INIT_FILE_PATH));
        snprintf(path, sizeof(path), "cp -f %s %s", SQLITE_INIT_FILE_PATH, MS_CONF_DIR);
        ms_system(path);
        ms_system("sync");
        usleep(100 * 1000);
    }
    db_check_key();
    db_update(SQLITE_FILE_NAME);
    my_db_update();

    db_init_filesystem();
    is_oem_need_to_update();
    oem_update();

    //for PoE NVR init Lan2 IPaddr
    ms_sys_get_chipinfo(chip_info, sizeof(chip_info));
    if (!check_param_key(SQLITE_FILE_NAME, PARAM_REPLACE)) {
        if (get_param_int(SQLITE_FILE_NAME, PARAM_REPLACE, -1) == 0) {
            if (chip_info[15] == 'P') {
                read_network(SQLITE_FILE_NAME, &network);
                network.lan2_enable = 1;
                snprintf(network.lan2_ip_address, sizeof(network.lan2_ip_address), POE_DEFAULT_IPADDR);
                write_network(SQLITE_FILE_NAME, &network);
            }
            set_param_int(SQLITE_FILE_NAME, PARAM_REPLACE, 1);
        } else {
            //Compatible with encryption chip bad
            if (chip_info[15] == 'P') {
                read_network(SQLITE_FILE_NAME, &network);
                if (network.lan2_enable != 1) {
                    network.lan2_enable = 1;
                    snprintf(network.lan2_ip_address, sizeof(network.lan2_ip_address), POE_DEFAULT_IPADDR);
                    write_network(SQLITE_FILE_NAME, &network);
                }
            }
        }
    } else {
        add_param_int(SQLITE_FILE_NAME, PARAM_REPLACE, 0);
        if (chip_info[15] == 'P') {
            //for PoE NVR
            read_network(SQLITE_FILE_NAME, &network);
            network.lan2_enable = 1;
            snprintf(network.lan2_ip_address, sizeof(network.lan2_ip_address), POE_DEFAULT_IPADDR);
            write_network(SQLITE_FILE_NAME, &network);
        }
        set_param_int(SQLITE_FILE_NAME, PARAM_REPLACE, 1);
    }

    //for 9.0.1 msfs db update
    if (check_param_key(SQLITE_FILE_NAME, PARAM_MSFS_DBUP)) {
        add_param_int(SQLITE_FILE_NAME, PARAM_MSFS_DBUP, 0);
    }

    read_keep_config(keep_confle, SQLITE_FILE_NAME);
    read_keep_user_config(keep_user_confile, SQLITE_FILE_NAME);
    read_keep_encrypted_config(keep_encrypted_confile, SQLITE_FILE_NAME);
    read_keep_params_config(MS_KEEP_POEPWD_CONF_FILE, SQLITE_FILE_NAME);

    failover_init();

    //is same channel exist
    is_same_channel_exist();
    deal_auto_backup_db();

    snprintf(path, sizeof(path), "rm -f %s/%s", MS_CONF_DIR, MS_RESET_FLAG_FILE);
    ms_system(path);
    snprintf(path, sizeof(path), "rm -f %s/%s", MS_CONF_DIR, MS_RESET_FLAG_IP_FILE);
    ms_system(path);
    snprintf(path, sizeof(path), "rm -f %s/%s", MS_CONF_DIR, MS_RESET_FLAG_USER_FILE);
    ms_system(path);

    ////////////////////7.0.x -> 8.0.x////////////////////
    if (!access(MS_UPDATE_DB_FILE, F_OK)) {
        ////recrod DB
        read_records(SQLITE_FILE_NAME, records, &cnt);
        for (i = 0; i < cnt; i++) {
            if (records[i].mode == MODE_ALWAYS_REC) {
                memset(&schedule, 0, sizeof(schedule));
                read_record_schedule(SQLITE_FILE_NAME, &schedule, i);
                for (x = 0; x < MAX_DAY_NUM; x++) {
                    for (y = 0; y < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; y++) {
                        if (schedule.schedule_day[x].schedule_item[y].action_type == 0) {
                            schedule.schedule_day[x].schedule_item[y].action_type = TIMING_RECORD;
                            snprintf(schedule.schedule_day[x].schedule_item[y].start_time, 32, "%s", "00:00");
                            snprintf(schedule.schedule_day[x].schedule_item[y].end_time, 32, "%s", "24:00");
                            break;
                        }
                    }
                }
                write_record_schedule(SQLITE_FILE_NAME, &schedule, i);
                records[i].mode = MODE_SCHED_REC;
                write_record(SQLITE_FILE_NAME, &records[i]);
            }
        }

        ////motion DB
        for (i = 0; i < MAX_CAMERA; i++) {
            memset(&move, 0, sizeof(struct motion));
            read_motion(SQLITE_FILE_NAME, &move, i);
            if (move.enable == MODE_ALWAYS_REC) {
                memset(&m_schedule, 0, sizeof(struct motion_schedule));
                for (x = 0; x < MAX_DAY_NUM; x++) {
                    for (y = 0; y < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; y++) {
                        if (m_schedule.schedule_day[x].schedule_item[y].action_type == 0) {
                            m_schedule.schedule_day[x].schedule_item[y].action_type = MOTION_ACTION;
                            snprintf(m_schedule.schedule_day[x].schedule_item[y].start_time, 32, "%s", "00:00");
                            snprintf(m_schedule.schedule_day[x].schedule_item[y].end_time, 32, "%s", "24:00");
                            break;
                        }
                    }
                }
                if (move.tri_alarms & (1 << 4)) {
                    write_motion_audible_schedule(SQLITE_FILE_NAME, &m_schedule, i);
                }
                if (move.tri_alarms & (1 << 5)) {
                    write_motion_email_schedule(SQLITE_FILE_NAME, &m_schedule, i);
                }
                move.enable = MODE_SCHED_REC;
            } else if (move.enable == MODE_SCHED_REC) {
                memset(&m_schedule, 0, sizeof(struct motion_schedule));
                read_motion_schedule(SQLITE_FILE_NAME, &m_schedule, i);
                if (move.tri_alarms & (1 << 4)) {
                    write_motion_audible_schedule(SQLITE_FILE_NAME, &m_schedule, i);
                }
                if (move.tri_alarms & (1 << 5)) {
                    write_motion_email_schedule(SQLITE_FILE_NAME, &m_schedule, i);
                }
            }

            move.id = i;
            move.email_buzzer_interval = move.buzzer_interval;
            move.email_enable = move.enable;
            write_motion(SQLITE_FILE_NAME, &move);
        }

        ////Video Loss
        for (i = 0; i < MAX_CAMERA; i++) {
            memset(&videolost, 0, sizeof(videolost));
            read_video_lost(SQLITE_FILE_NAME, &videolost, i);
            if (videolost.enable == MODE_ALWAYS_REC) {
                memset(&v_schedule, 0, sizeof(v_schedule));
                for (x = 0; x < MAX_DAY_NUM; x++) {
                    for (y = 0; y < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; y++) {
                        if (v_schedule.schedule_day[x].schedule_item[y].action_type == 0) {
                            v_schedule.schedule_day[x].schedule_item[y].action_type = VIDEOLOSS_ACTION;
                            snprintf(v_schedule.schedule_day[x].schedule_item[y].start_time, 32, "%s", "00:00");
                            snprintf(v_schedule.schedule_day[x].schedule_item[y].end_time, 32, "%s", "24:00");
                            break;
                        }
                    }
                }
                if (videolost.tri_alarms & (1 << 4)) {
                    write_videoloss_audible_schedule(SQLITE_FILE_NAME, &v_schedule, i);
                }
                if (videolost.tri_alarms & (1 << 5)) {
                    write_videoloss_email_schedule(SQLITE_FILE_NAME, &v_schedule, i);
                }
                videolost.enable = MODE_SCHED_REC;
            } else if (videolost.enable == MODE_SCHED_REC) {
                memset(&v_schedule, 0, sizeof(v_schedule));
                read_video_loss_schedule(SQLITE_FILE_NAME, &v_schedule, i);
                if (videolost.tri_alarms & (1 << 4)) {
                    write_videoloss_audible_schedule(SQLITE_FILE_NAME, &v_schedule, i);
                }
                if (videolost.tri_alarms & (1 << 5)) {
                    write_videoloss_email_schedule(SQLITE_FILE_NAME, &v_schedule, i);
                }
            }

            videolost.id = i;
            videolost.email_enable = videolost.enable;
            videolost.email_buzzer_interval = videolost.buzzer_interval;
            write_video_lost(SQLITE_FILE_NAME, &videolost);
        }

        ////Alarm Input
        for (i = 0; i < MAX_ALARM_IN; i++) {
            memset(&alarm, 0, sizeof(alarm));
            read_alarm_in(SQLITE_FILE_NAME, &alarm, i);
            if (alarm.enable == MODE_ALWAYS_REC) {
                memset(&a_schedule, 0, sizeof(a_schedule));
                for (x = 0; x < MAX_DAY_NUM; x++) {
                    for (y = 0; y < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; y++) {
                        if (a_schedule.schedule_day[x].schedule_item[y].action_type == 0) {
                            a_schedule.schedule_day[x].schedule_item[y].action_type = ALARMIN_ACTION;
                            snprintf(a_schedule.schedule_day[x].schedule_item[y].start_time, 32, "%s", "00:00");
                            snprintf(a_schedule.schedule_day[x].schedule_item[y].end_time, 32, "%s", "24:00");
                            break;
                        }
                    }
                }
                if (alarm.tri_alarms & (1 << 4)) {
                    write_alarmin_audible_schedule(SQLITE_FILE_NAME, &a_schedule, i);
                }
                if (alarm.tri_alarms & (1 << 5)) {
                    write_alarmin_email_schedule(SQLITE_FILE_NAME, &a_schedule, i);
                }
                write_alarmin_ptz_schedule(SQLITE_FILE_NAME, &a_schedule, i);
                alarm.enable = MODE_SCHED_REC;
            } else if (alarm.enable == MODE_SCHED_REC) {
                memset(&a_schedule, 0, sizeof(v_schedule));
                read_alarm_in_schedule(SQLITE_FILE_NAME, &a_schedule, i);
                if (alarm.tri_alarms & (1 << 4)) {
                    write_alarmin_audible_schedule(SQLITE_FILE_NAME, &a_schedule, i);
                }
                if (alarm.tri_alarms & (1 << 5)) {
                    write_alarmin_email_schedule(SQLITE_FILE_NAME, &a_schedule, i);
                }
                write_alarmin_ptz_schedule(SQLITE_FILE_NAME, &a_schedule, i);
            }
            alarm.id = i;
            alarm.email_enable = alarm.enable;
            alarm.email_buzzer_interval = alarm.buzzer_interval;
            write_alarm_in(SQLITE_FILE_NAME, &alarm);
        }

        snprintf(path, sizeof(path), "rm %s", MS_UPDATE_DB_FILE);
        ms_system(path);
    }

    update_email_enable();
}

int ms_start_ssh(int ssh_port, int enable)
{
    char cmd[256] = {0};
    snprintf(cmd, sizeof(cmd), "dropbear -p 22 &");//run default for "support" debug.
    ms_system(cmd);

    return 0;
}

int write_keep_config(const char *localfile, const char *pDbFile)
{
    if (!access(localfile, F_OK)) { // the last reboot didn't delete this file because of the power off
        return 0;
    }

    int ret;
    char buf[2048] = {0};
    struct network dbNet;
    memset(&dbNet, 0, sizeof(struct network));

    ret = read_network(pDbFile, &dbNet);
    if (ret) {
        msprintf("read network error");
        return -1;
    }

    snprintf(buf, sizeof(buf), "mode=%d\n", dbNet.mode);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "miimon=%d\n", dbNet.miimon);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "bond0_primary_net=%d\n", dbNet.bond0_primary_net);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "bond0_enable=%d\n", dbNet.bond0_enable);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "bond0_type=%d\n", dbNet.bond0_type);

    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "bond0_ip_address=%s\n", dbNet.bond0_ip_address);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "bond0_netmask=%s\n", dbNet.bond0_netmask);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "bond0_gateway=%s\n", dbNet.bond0_gateway);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "bond0_primary_dns=%s\n", dbNet.bond0_primary_dns);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "bond0_second_dns=%s\n", dbNet.bond0_second_dns);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "bond0_mtu=%d\n", dbNet.bond0_mtu);

    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan1_enable=%d\n", dbNet.lan1_enable);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan1_type=%d\n", dbNet.lan1_type);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan1_ip_address=%s\n", dbNet.lan1_ip_address);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan1_gateway=%s\n", dbNet.lan1_gateway);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan1_netmask=%s\n", dbNet.lan1_netmask);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan1_primary_dns=%s\n", dbNet.lan1_primary_dns);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan1_second_dns=%s\n", dbNet.lan1_second_dns);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan1_mtu=%d\n", dbNet.lan1_mtu);

    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan2_enable=%d\n", dbNet.lan2_enable);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan2_type=%d\n", dbNet.lan2_type);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan2_ip_address=%s\n", dbNet.lan2_ip_address);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan2_netmask=%s\n", dbNet.lan2_netmask);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan2_gateway=%s\n", dbNet.lan2_gateway);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan2_primary_dns=%s\n", dbNet.lan2_primary_dns);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan2_second_dns=%s\n", dbNet.lan2_second_dns);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan2_mtu=%d\n", dbNet.lan2_mtu);

    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan1_dhcp_gateway=%s\n", dbNet.lan1_dhcp_gateway);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan2_dhcp_gateway=%s\n", dbNet.lan2_dhcp_gateway);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "tri_alarms=%d\n", dbNet.tri_alarms);

    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan1_ip6_address=%s\n",
             dbNet.lan1_ip6_address); //hrz.milesight add for ipv6
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan1_ip6_netmask=%s\n", dbNet.lan1_ip6_netmask);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan1_ip6_gateway=%s\n", dbNet.lan1_ip6_gateway);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan2_ip6_address=%s\n", dbNet.lan2_ip6_address);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan2_ip6_netmask=%s\n", dbNet.lan2_ip6_netmask);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan2_ip6_gateway=%s\n", dbNet.lan2_ip6_gateway);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan1_ip6_dhcp=%d\n", dbNet.lan1_ip6_dhcp);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "lan2_ip6_dhcp=%d\n", dbNet.lan2_ip6_dhcp);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "bond0_ip6_dhcp=%d\n", dbNet.bond0_ip6_dhcp);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "bond0_ip6_address=%s\n", dbNet.bond0_ip6_address);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "bond0_ip6_netmask=%s\n", dbNet.bond0_ip6_netmask);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "bond0_ip6_gateway=%s\n", dbNet.bond0_ip6_gateway);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "defaultRoute=%d\n", dbNet.defaultRoute);

    //printf("##################  write_keep_config [%s %d] addr:%s gw:%s, mask:%s %s %s %s\n", localfile, strlen(buf), dbNet.lan1_ip6_address, dbNet.lan1_ip6_netmask, dbNet.lan1_ip6_gateway,dbNet.lan2_ip6_address, dbNet.lan2_ip6_netmask, dbNet.lan2_ip6_gateway);//hrz.milesight
    //printf("[david debug] write_keep_config addr:%s gw:%s, mask:%s\n", dbNet.lan1_ip_address, dbNet.lan1_gateway, dbNet.lan1_netmask);
    FILE *fp = fopen(localfile, "a+");
    if (!fp) {
        return -1;
    }
    fwrite(buf, 1, strlen(buf), fp);
    fflush(fp);
    fclose(fp);

    ms_system("sync");

    return 0;
}
int write_keep_user_config(const char *localfile, const char *pDbFile)
{
    if (!access(localfile, F_OK)) { // the last reboot didn't delete this file because of the power off
        return 0;
    }

    char buf[2048 * MAX_USER] = {0}, field[25] = {0};
    int nCnt = 0, i = 0, ret = 0;
    struct db_user *users = (struct db_user *)ms_calloc(MAX_USER, sizeof(struct db_user));
    ret = read_users(pDbFile, users, &nCnt);
    if (ret == -1) {
        ms_free(users);
        return -1;
    }

    for (i = 0; i < nCnt; i++) {
        snprintf(field, sizeof(field), "id_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].id);

        snprintf(field, sizeof(field), "enable_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].enable);

        snprintf(field, sizeof(field), "username_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%s\n", field, users[i].username);

        snprintf(field, sizeof(field), "password_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%s\n", field, users[i].password);

        snprintf(field, sizeof(field), "type_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].type);

        snprintf(field, sizeof(field), "permission_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].permission);

        snprintf(field, sizeof(field), "remote_permission_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].remote_permission);

        snprintf(field, sizeof(field), "local_live_view_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%ld\n", field, users[i].local_live_view);

        snprintf(field, sizeof(field), "local_playback_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%ld\n", field, users[i].local_playback);

        snprintf(field, sizeof(field), "remote_live_view_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%ld\n", field, users[i].remote_live_view);

        snprintf(field, sizeof(field), "remote_playback_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%ld\n", field, users[i].remote_playback);

        snprintf(field, sizeof(field), "local_live_view_ex_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%s\n", field, users[i].local_live_view_ex);

        snprintf(field, sizeof(field), "local_playback_ex_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%s\n", field, users[i].local_playback_ex);

        snprintf(field, sizeof(field), "remote_live_view_ex_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%s\n", field, users[i].remote_live_view_ex);

        snprintf(field, sizeof(field), "remote_playback_ex_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%s\n", field, users[i].remote_playback_ex);

        snprintf(field, sizeof(field), "password_ex_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%s\n", field, users[i].password_ex);

        snprintf(field, sizeof(field), "pattern_psw_%d=", i);
        if (strlen(users[i].pattern_psw) < 4) {
            snprintf(users[i].pattern_psw, sizeof(users[i].pattern_psw), "%s", "0");
        }
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%s\n", field, users[i].pattern_psw);

        snprintf(field, sizeof(field), "perm_local_live_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].perm_local_live);
        snprintf(field, sizeof(field), "perm_local_playback_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].perm_local_playback);
        snprintf(field, sizeof(field), "perm_local_retrieve_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].perm_local_retrieve);
        snprintf(field, sizeof(field), "perm_local_smart_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].perm_local_smart);
        snprintf(field, sizeof(field), "perm_local_event_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].perm_local_event);
        snprintf(field, sizeof(field), "perm_local_camera_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].perm_local_camera);
        snprintf(field, sizeof(field), "perm_local_storage_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].perm_local_storage);
        snprintf(field, sizeof(field), "perm_local_settings_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].perm_local_settings);
        snprintf(field, sizeof(field), "perm_local_status_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].perm_local_status);
        snprintf(field, sizeof(field), "perm_local_shutdown_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].perm_local_shutdown);

        snprintf(field, sizeof(field), "perm_remote_live_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].perm_remote_live);
        snprintf(field, sizeof(field), "perm_remote_playback_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].perm_remote_playback);
        snprintf(field, sizeof(field), "perm_remote_retrieve_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].perm_remote_retrieve);
        snprintf(field, sizeof(field), "perm_remote_smart_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].perm_remote_smart);
        snprintf(field, sizeof(field), "perm_remote_event_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].perm_remote_event);
        snprintf(field, sizeof(field), "perm_remote_camera_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].perm_remote_camera);
        snprintf(field, sizeof(field), "perm_remote_storage_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].perm_remote_storage);
        snprintf(field, sizeof(field), "perm_remote_settings_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].perm_remote_settings);
        snprintf(field, sizeof(field), "perm_remote_status_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].perm_remote_status);
        snprintf(field, sizeof(field), "perm_remote_shutdown_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, users[i].perm_remote_shutdown);
    }

    FILE *fp = fopen(localfile, "a+");
    if (!fp) {
        return -1;
    }
    fwrite(buf, 1, strlen(buf), fp);
    fflush(fp);
    fclose(fp);

    ms_free(users);
    ms_system("sync");

    return 0;
}
int read_keep_user_config(const char *localfile, const char *pDbFile)
{
    int i = 0, len = 0;
    char fgetch[40 * MAX_USER][128] = {{0}};
    char cmd[256] = {0}, field[25] = {0};
    char *pchar = NULL;
    int nCnt = 0;
    int fCnt = 37;
    int pLen1 = 0, pLen2 = 0, j = 0;
    struct db_user *users = (struct db_user *)ms_malloc(sizeof(struct db_user) * MAX_USER);

    if (access(localfile, F_OK) == -1) {
        if (users) {
            ms_free(users);
        }
        return 0;
    }
    if (access(OEM_UPLOAD_OPT, F_OK) == 0) {
        snprintf(cmd, sizeof(cmd), "rm -f %s", OEM_UPLOAD_OPT);
        ms_system(cmd);
        if (!access(localfile, F_OK)) {
            snprintf(cmd, sizeof(cmd), "rm -f %s", localfile);
            ms_system(cmd);
        }

        if (users) {
            ms_free(users);
        }
        return 0;
    }
    read_users(pDbFile, users, &nCnt);
    FILE *fp = fopen(localfile, "r");
    if (!fp) {
        if (users) {
            ms_free(users);
        }
        return -1;
    }
    while (!feof(fp)) {
        fgets((char *)&fgetch[i], sizeof(fgetch[i]), fp);
        if ((pchar = strchr((char *)&fgetch[i], '\n')) != NULL) {
            *pchar = '\0';
        }
        //printf("[david debug] i:%d fgetch:%s\n", i, fgetch[i]);
        if ((i++) >= 40 * MAX_USER) {
            break;
        }
    }
    fclose(fp);

    for (i = 0; i < nCnt; i++) {
        snprintf(field, sizeof(field), "id_%d=", i);
        len = strlen(field);
        users[i].id = atoi((char *)&fgetch[0 + i * fCnt][len]);
        if (users[i].id != i) {
            users[i].id = i;
        }

        snprintf(field, sizeof(field), "enable_%d=", i);
        if (!strncmp((char *)&fgetch[1 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].enable = atoi((char *)&fgetch[1 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "username_%d=", i);
        if (!strncmp((char *)&fgetch[2 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            snprintf(users[i].username, sizeof(users[i].username), "%s", (char *)&fgetch[2 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "password_%d=", i);
        if (!strncmp((char *)&fgetch[3 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            snprintf(users[i].password, sizeof(users[i].password), "%s", (char *)&fgetch[3 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "type_%d=", i);
        if (!strncmp((char *)&fgetch[4 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].type = atoi((char *)&fgetch[4 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "permission_%d=", i);
        if (!strncmp((char *)&fgetch[5 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].permission = atoi((char *)&fgetch[5 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "remote_permission_%d=", i);
        if (!strncmp((char *)&fgetch[6 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].remote_permission = atoi((char *)&fgetch[6 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "local_live_view_%d=", i);
        if (!strncmp((char *)&fgetch[7 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].local_live_view = atol((char *)&fgetch[7 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "local_playback_%d=", i);
        if (!strncmp((char *)&fgetch[8 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].local_playback = atol((char *)&fgetch[8 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "remote_live_view_%d=", i);
        if (!strncmp((char *)&fgetch[9 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].remote_live_view = atol((char *)&fgetch[9 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "remote_playback_%d=", i);
        if (!strncmp((char *)&fgetch[10 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].remote_playback = atol((char *)&fgetch[10 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "local_live_view_ex_%d=", i);
        if (!strncmp((char *)&fgetch[11 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            snprintf(users[i].local_live_view_ex, sizeof(users[i].local_live_view_ex), "%s", (char *)&fgetch[11 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "local_playback_ex_%d=", i);
        if (!strncmp((char *)&fgetch[12 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            snprintf(users[i].local_playback_ex, sizeof(users[i].local_playback_ex), "%s", (char *)&fgetch[12 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "remote_live_view_ex_%d=", i);
        if (!strncmp((char *)&fgetch[13 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            snprintf(users[i].remote_live_view_ex, sizeof(users[i].remote_live_view_ex), "%s", (char *)&fgetch[13 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "remote_playback_ex_%d=", i);
        if (!strncmp((char *)&fgetch[14 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            snprintf(users[i].remote_playback_ex, sizeof(users[i].remote_playback_ex), "%s", (char *)&fgetch[14 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "password_ex_%d=", i);
        if (!strncmp((char *)&fgetch[15 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            snprintf(users[i].password_ex, sizeof(users[i].password_ex), "%s", (char *)&fgetch[15 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "pattern_psw_%d=", i);
        if (!strncmp((char *)&fgetch[16 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            snprintf(users[i].pattern_psw, sizeof(users[i].pattern_psw), "%s", (char *)&fgetch[16 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "perm_local_live_%d=", i);
        if (!strncmp((char *)&fgetch[17 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].perm_local_live = atoi((char *)&fgetch[17 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "perm_local_playback_%d=", i);
        if (!strncmp((char *)&fgetch[18 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].perm_local_playback = atoi((char *)&fgetch[18 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "perm_local_retrieve_%d=", i);
        if (!strncmp((char *)&fgetch[19 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].perm_local_retrieve = atoi((char *)&fgetch[19 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "perm_local_smart_%d=", i);
        if (!strncmp((char *)&fgetch[20 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].perm_local_smart = atoi((char *)&fgetch[20 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "perm_local_event_%d=", i);
        if (!strncmp((char *)&fgetch[21 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].perm_local_event = atoi((char *)&fgetch[21 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "perm_local_camera_%d=", i);
        if (!strncmp((char *)&fgetch[22 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].perm_local_camera = atoi((char *)&fgetch[22 + i * fCnt][len]);
        }
        
        snprintf(field, sizeof(field), "perm_local_storage_%d=", i);
        if (!strncmp((char *)&fgetch[23 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].perm_local_storage = atoi((char *)&fgetch[23 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "perm_local_settings_%d=", i);
        if (!strncmp((char *)&fgetch[24 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].perm_local_settings = atoi((char *)&fgetch[24 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "perm_local_status_%d=", i);
        if (!strncmp((char *)&fgetch[25 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].perm_local_status = atoi((char *)&fgetch[25 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "perm_local_shutdown_%d=", i);
        if (!strncmp((char *)&fgetch[26 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].perm_local_shutdown = atoi((char *)&fgetch[26 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "perm_remote_live_%d=", i);
        if (!strncmp((char *)&fgetch[27 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].perm_remote_live = atoi((char *)&fgetch[27 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "perm_remote_playback_%d=", i);
        if (!strncmp((char *)&fgetch[28 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].perm_remote_playback = atoi((char *)&fgetch[28 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "perm_remote_retrieve_%d=", i);
        if (!strncmp((char *)&fgetch[29 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].perm_remote_retrieve = atoi((char *)&fgetch[29 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "perm_remote_smart_%d=", i);
        if (!strncmp((char *)&fgetch[30 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].perm_remote_smart = atoi((char *)&fgetch[30 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "perm_remote_event_%d=", i);
        if (!strncmp((char *)&fgetch[31 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].perm_remote_event = atoi((char *)&fgetch[31 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "perm_remote_camera_%d=", i);
        if (!strncmp((char *)&fgetch[32 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].perm_remote_camera = atoi((char *)&fgetch[32 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "perm_remote_storage_%d=", i);
        if (!strncmp((char *)&fgetch[33 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].perm_remote_storage = atoi((char *)&fgetch[33 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "perm_remote_settings_%d=", i);
        if (!strncmp((char *)&fgetch[34 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].perm_remote_settings = atoi((char *)&fgetch[34 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "perm_remote_status_%d=", i);
        if (!strncmp((char *)&fgetch[35 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].perm_remote_status = atoi((char *)&fgetch[35 + i * fCnt][len]);
        }

        snprintf(field, sizeof(field), "perm_remote_shutdown_%d=", i);
        if (!strncmp((char *)&fgetch[36 + i * fCnt], field, strlen(field))) {
            len = strlen(field);
            users[i].perm_remote_shutdown = atoi((char *)&fgetch[36 + i * fCnt][len]);
        }

        if (i > 0 && users[i].enable) {
            pLen1 = strlen(users[i].local_live_view_ex);
            pLen2 = strlen(users[i].local_playback_ex);
            if (pLen1 < 4 && pLen2 >= 4) {
                for (j = 0; j < pLen2; j++) {
                    users[i].local_live_view_ex[j] = '1';
                }
//              msprintf("len:[%d], users[i].local_live_view_ex:[%s]", strlen(users[i].local_live_view_ex), users[i].local_live_view_ex);
            }
        }

        write_user(SQLITE_FILE_NAME, &users[i]);
    }
    ms_free(users);

    snprintf(cmd, sizeof(cmd), "rm -f %s", localfile);
    ms_system(cmd);

    ms_system("sync");
    return 0;
}

int write_keep_encrypted_config(const char *localfile, const char *pDbFile)
{
    if (!access(localfile, F_OK)) { // the last reboot didn't delete this file because of the power off
        return 0;
    }

    char buf[2048 * MAX_SQA_CNT] = {0}, field[20] = {0};
    int i = 0, ret = 0;
    struct squestion sqas[3];
    memset(sqas, 0, sizeof(struct squestion)*MAX_SQA_CNT);
    ret = read_encrypted_list(pDbFile, sqas);
    if (ret) {
        return -1;
    }

    if (sqas[0].enable != 1) {
        return -1;
    }

    for (i = 0; i < MAX_SQA_CNT; i++) {
        snprintf(field, sizeof(field), "enable_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, sqas[i].enable);

        snprintf(field, sizeof(field), "sqtype_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%d\n", field, sqas[i].sqtype);

        snprintf(field, sizeof(field), "squestion_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%s\n", field, sqas[i].squestion);

        snprintf(field, sizeof(field), "answer_%d=", i);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%s\n", field, sqas[i].answer);
    }

    FILE *fp = fopen(localfile, "a+");
    if (!fp) {
        return -1;
    }
    fwrite(buf, 1, strlen(buf), fp);
    fflush(fp);
    fclose(fp);

    ms_system("sync");

    return 0;
}
int read_keep_encrypted_config(const char *localfile, const char *pDbFile)
{
    int i = 0, len = 0, ret = 0;
    char fgetch[64][128] = {{0}};//hrz.milesight [32][64] --> [64][64]
    char cmd[256] = {0}, field[20] = {0};
    char *pchar = NULL;
    struct squestion sqas[3];
    memset(sqas, 0, sizeof(struct squestion)*MAX_SQA_CNT);

    if (access(localfile, F_OK) == -1) {
        return 0;
    }
    if (access(OEM_UPLOAD_OPT, F_OK) == 0) {
        snprintf(cmd, sizeof(cmd), "rm -f %s", OEM_UPLOAD_OPT);
        ms_system(cmd);
        if (!access(localfile, F_OK)) {
            snprintf(cmd, sizeof(cmd), "rm -f %s", localfile);
            ms_system(cmd);
        }
        return 0;
    }
    ret = read_encrypted_list(pDbFile, sqas);
    if (ret) {
        return -1;
    }
    FILE *fp = fopen(localfile, "r");
    if (!fp) {
        return -1;
    }
    while (!feof(fp)) {
        fgets((char *)&fgetch[i], sizeof(fgetch[i]), fp);
        if ((pchar = strchr((char *)&fgetch[i], '\n')) != NULL) {
            *pchar = '\0';
        }
        //printf("[david debug] i:%d fgetch:%s\n", i, fgetch[i]);
        if ((i++) >= 64) { //32->64
            break;
        }
    }
    fclose(fp);
    for (i = 0; i < MAX_SQA_CNT; i++) {
        snprintf(field, sizeof(field), "enable_%d=", i);
        if (!strncmp(fgetch[0 + i * (MAX_SQA_CNT + 1)], field, strlen(field))) {
            len = strlen(field);
            sqas[i].enable = atoi((char *)&fgetch[0 + i * (MAX_SQA_CNT + 1)][len]);
        }

        snprintf(field, sizeof(field), "sqtype_%d=", i);
        if (!strncmp(fgetch[1 + i * (MAX_SQA_CNT + 1)], field, strlen(field))) {
            len = strlen(field);
            sqas[i].sqtype = atoi((char *)&fgetch[1 + i * (MAX_SQA_CNT + 1)][len]);
        }

        snprintf(field, sizeof(field), "squestion_%d=", i);
        if (!strncmp(fgetch[2 + i * (MAX_SQA_CNT + 1)], field, strlen(field))) {
            len = strlen(field);
            snprintf(sqas[i].squestion, sizeof(sqas[i].squestion), "%s", (char *)&fgetch[2 + i * (MAX_SQA_CNT + 1)][len]);
        }

        snprintf(field, sizeof(field), "answer_%d=", i);
        if (!strncmp(fgetch[3 + i * (MAX_SQA_CNT + 1)], field, strlen(field))) {
            len = strlen(field);
            snprintf(sqas[i].answer, sizeof(sqas[i].answer), "%s", (char *)&fgetch[3 + i * (MAX_SQA_CNT + 1)][len]);
        }
    }

    snprintf(cmd, sizeof(cmd), "rm -f %s", localfile);
    ms_system(cmd);

    for (i = 0; i < MAX_SQA_CNT; i++) {
        //msprintf("[NVR]==sqtype:%d, squestion:%s, answer:%s, enable:%d\n",sqas[i].sqtype, sqas[i].squestion, sqas[i].answer, sqas[i].enable);
        write_encrypted_list(SQLITE_FILE_NAME, &sqas[i], i + 1);
    }

    ms_system("sync");
    return 0;
}

int read_keep_config(const char *localfile, const char *pDbFile)
{
    int i = 0, len = 0;
    char fgetch[64][64] = {{0}};//hrz.milesight [32][64] --> [64][64]
    char cmd[256] = {0};
    char *pchar = NULL;
    struct network net = {0};
    if (access(localfile, F_OK) == -1) {
        return 0;
    }
    if (access(OEM_UPLOAD_OPT, F_OK) == 0) {
        snprintf(cmd, sizeof(cmd), "rm -f %s", OEM_UPLOAD_OPT);
        ms_system(cmd);
        if (!access(localfile, F_OK)) {
            snprintf(cmd, sizeof(cmd), "rm -f %s", localfile);
            ms_system(cmd);
        }
        return 0;
    }
    read_network(pDbFile, &net);
    FILE *fp = fopen(localfile, "r");
    if (!fp) {
        return -1;
    }
    while (!feof(fp)) {
        fgets((char *)&fgetch[i], sizeof(fgetch[i]), fp);
        if ((pchar = strchr((char *)&fgetch[i], '\n')) != NULL) {
            *pchar = '\0';
        }
        //printf("[david debug] i:%d fgetch:%s\n", i, fgetch[i]);
        if ((i++) >= 64) { //32->64
            break;
        }
    }
    fclose(fp);

    if (strncmp(fgetch[0], "mode", strlen("mode")) == 0) {
        len = strlen("mode=");
        net.mode = atoi((char *)&fgetch[0][len]);
    }
    if (strncmp(fgetch[1], "miimon", strlen("miimon")) == 0) {
        len = strlen("miimon=");
        net.miimon = atoi((char *)&fgetch[1][len]);
    }
    if (strncmp(fgetch[2], "bond0_primary_net", strlen("bond0_primary_net")) == 0) {
        len = strlen("bond0_primary_net=");
        net.bond0_primary_net = atoi((char *)&fgetch[2][len]);
    }

    if (strncmp(fgetch[3], "bond0_enable", strlen("bond0_enable")) == 0) {
        len = strlen("bond0_enable=");
        net.bond0_enable = atoi((char *)&fgetch[3][len]);
    }
    if (strncmp(fgetch[4], "bond0_type", strlen("bond0_type")) == 0) {
        len = strlen("bond0_type=");
        net.bond0_type = atoi((char *)&fgetch[4][len]);
    }
    if (strncmp(fgetch[5], "bond0_ip_address", strlen("bond0_ip_address")) == 0) {
        len = strlen("bond0_ip_address=");
        snprintf(net.bond0_ip_address, sizeof(net.bond0_ip_address), "%s", (char *)&fgetch[5][len]);
    }
    if (strncmp(fgetch[6], "bond0_netmask", strlen("bond0_netmask")) == 0) {
        len = strlen("bond0_netmask=");
        snprintf(net.bond0_netmask, sizeof(net.bond0_netmask), "%s", (char *)&fgetch[6][len]);
    }
    if (strncmp(fgetch[7], "bond0_gateway", strlen("bond0_gateway")) == 0) {
        len = strlen("bond0_gateway=");
        snprintf(net.bond0_gateway, sizeof(net.bond0_gateway), "%s", (char *)&fgetch[7][len]);
    }
    if (strncmp(fgetch[8], "bond0_primary_dns", strlen("bond0_primary_dns")) == 0) {
        len = strlen("bond0_primary_dns=");
        snprintf(net.bond0_primary_dns, sizeof(net.bond0_primary_dns), "%s", (char *)&fgetch[8][len]);
    }
    if (strncmp(fgetch[9], "bond0_second_dns", strlen("bond0_second_dns")) == 0) {
        len = strlen("bond0_second_dns=");
        snprintf(net.bond0_second_dns, sizeof(net.bond0_second_dns), "%s", (char *)&fgetch[9][len]);
    }
    if (strncmp(fgetch[10], "bond0_mtu", strlen("bond0_mtu")) == 0) {
        len = strlen("bond0_mtu=");
        net.bond0_mtu = atoi((char *)&fgetch[10][len]);
    }

    if (strncmp(fgetch[11], "lan1_enable", strlen("lan1_enable")) == 0) {
        len = strlen("lan1_enable=");
        net.lan1_enable = atoi((char *)&fgetch[11][len]);
    }
    if (strncmp(fgetch[12], "lan1_type", strlen("lan1_type")) == 0) {
        len = strlen("lan1_type=");
        net.lan1_type = atoi((char *)&fgetch[12][len]);
    }
    if (strncmp(fgetch[13], "lan1_ip_address", strlen("lan1_ip_address")) == 0) {
        len = strlen("lan1_ip_address=");
        snprintf(net.lan1_ip_address, sizeof(net.lan1_ip_address), "%s", (char *)&fgetch[13][len]);
    }
    if (strncmp(fgetch[14], "lan1_gateway", strlen("lan1_gateway")) == 0) {
        len = strlen("lan1_gateway=");
        snprintf(net.lan1_gateway, sizeof(net.lan1_gateway), "%s", (char *)&fgetch[14][len]);
    }
    if (strncmp(fgetch[15], "lan1_netmask", strlen("lan1_netmask")) == 0) {
        len = strlen("lan1_netmask=");
        snprintf(net.lan1_netmask, sizeof(net.lan1_netmask), "%s", (char *)&fgetch[15][len]);
    }
    if (strncmp(fgetch[16], "lan1_primary_dns", strlen("lan1_primary_dns")) == 0) {
        len = strlen("lan1_primary_dns=");
        snprintf(net.lan1_primary_dns, sizeof(net.lan1_primary_dns), "%s", (char *)&fgetch[16][len]);
    }
    if (strncmp(fgetch[17], "lan1_second_dns", strlen("lan1_second_dns")) == 0) {
        len = strlen("lan1_second_dns=");
        snprintf(net.lan1_second_dns, sizeof(net.lan1_second_dns), "%s", (char *)&fgetch[17][len]);
    }
    if (strncmp(fgetch[18], "lan1_mtu", strlen("lan1_mtu")) == 0) {
        len = strlen("lan1_mtu=");
        net.lan1_mtu = atoi((char *)&fgetch[18][len]);
    }

    if (strncmp(fgetch[19], "lan2_enable", strlen("lan2_enable")) == 0) {
        len = strlen("lan2_enable=");
        net.lan2_enable = atoi((char *)&fgetch[19][len]);
    }
    if (strncmp(fgetch[20], "lan2_type", strlen("lan2_type")) == 0) {
        len = strlen("lan2_type=");
        net.lan2_type = atoi((char *)&fgetch[20][len]);
    }
    if (strncmp(fgetch[21], "lan2_ip_address", strlen("lan2_ip_address")) == 0) {
        len = strlen("lan2_ip_address=");
        snprintf(net.lan2_ip_address, sizeof(net.lan2_ip_address), "%s", (char *)&fgetch[21][len]);
    }
    if (strncmp(fgetch[22], "lan2_netmask", strlen("lan2_netmask")) == 0) {
        len = strlen("lan2_netmask=");
        snprintf(net.lan2_netmask, sizeof(net.lan2_netmask), "%s", (char *)&fgetch[22][len]);
    }
    if (strncmp(fgetch[23], "lan2_gateway", strlen("lan2_gateway")) == 0) {
        len = strlen("lan2_gateway=");
        snprintf(net.lan2_gateway, sizeof(net.lan2_gateway), "%s", (char *)&fgetch[23][len]);
    }
    if (strncmp(fgetch[24], "lan2_primary_dns", strlen("lan2_primary_dns")) == 0) {
        len = strlen("lan2_primary_dns=");
        snprintf(net.lan2_primary_dns, sizeof(net.lan2_primary_dns), "%s", (char *)&fgetch[24][len]);
    }
    if (strncmp(fgetch[25], "lan2_second_dns", strlen("lan2_second_dns")) == 0) {
        len = strlen("lan2_second_dns=");
        snprintf(net.lan2_second_dns, sizeof(net.lan2_second_dns), "%s", (char *)&fgetch[25][len]);
    }
    if (strncmp(fgetch[26], "lan2_mtu", strlen("lan2_mtu")) == 0) {
        len = strlen("lan2_mtu=");
        net.lan2_mtu = atoi((char *)&fgetch[26][len]);
    }

    if (strncmp(fgetch[27], "lan1_dhcp_gateway", strlen("lan1_dhcp_gateway")) == 0) {
        len = strlen("lan1_dhcp_gateway=");
        snprintf(net.lan1_dhcp_gateway, sizeof(net.lan1_dhcp_gateway), "%s", (char *)&fgetch[27][len]);
    }
    if (strncmp(fgetch[28], "lan2_dhcp_gateway", strlen("lan2_dhcp_gateway")) == 0) {
        len = strlen("lan2_dhcp_gateway=");
        snprintf(net.lan2_dhcp_gateway, sizeof(net.lan2_dhcp_gateway), "%s", (char *)&fgetch[28][len]);
    }
    if (strncmp(fgetch[29], "tri_alarms", strlen("tri_alarms")) == 0) {
        len = strlen("tri_alarms=");
        net.tri_alarms = atoi((char *)&fgetch[29][len]);
    }

    if (strncmp(fgetch[30], "lan1_ip6_address", strlen("lan1_ip6_address")) == 0) {
        len = strlen("lan1_ip6_address=");
        snprintf(net.lan1_ip6_address, sizeof(net.lan1_ip6_address), "%s", (char *)&fgetch[30][len]);
    }
    if (strncmp(fgetch[31], "lan1_ip6_netmask", strlen("lan1_ip6_netmask")) == 0) {
        len = strlen("lan1_ip6_netmask=");
        snprintf(net.lan1_ip6_netmask, sizeof(net.lan1_ip6_netmask), "%s", (char *)&fgetch[31][len]);
    }
    if (strncmp(fgetch[32], "lan1_ip6_gateway", strlen("lan1_ip6_gateway")) == 0) {
        len = strlen("lan1_ip6_gateway=");
        snprintf(net.lan1_ip6_gateway, sizeof(net.lan1_ip6_gateway), "%s", (char *)&fgetch[32][len]);
    }

    if (strncmp(fgetch[33], "lan2_ip6_address", strlen("lan2_ip6_address")) == 0) {
        len = strlen("lan2_ip6_address=");
        snprintf(net.lan2_ip6_address, sizeof(net.lan2_ip6_address), "%s", (char *)&fgetch[33][len]);
    }
    if (strncmp(fgetch[34], "lan2_ip6_netmask", strlen("lan2_ip6_netmask")) == 0) {
        len = strlen("lan2_ip6_netmask=");
        snprintf(net.lan2_ip6_netmask, sizeof(net.lan2_ip6_netmask), "%s", (char *)&fgetch[34][len]);
    }
    if (strncmp(fgetch[35], "lan2_ip6_gateway", strlen("lan2_ip6_gateway")) == 0) {
        len = strlen("lan2_ip6_gateway=");
        snprintf(net.lan2_ip6_gateway, sizeof(net.lan2_ip6_gateway), "%s", (char *)&fgetch[35][len]);
    }
    if (strncmp(fgetch[36], "lan1_ip6_dhcp", strlen("lan1_ip6_dhcp")) == 0) {
        len = strlen("lan1_ip6_dhcp=");
        net.lan1_ip6_dhcp = atoi((char *)&fgetch[36][len]);
    }
    if (strncmp(fgetch[37], "lan2_ip6_dhcp", strlen("lan2_ip6_dhcp")) == 0) {
        len = strlen("lan2_ip6_dhcp=");
        net.lan2_ip6_dhcp = atoi((char *)&fgetch[37][len]);
    }
    if (strncmp(fgetch[38], "bond0_ip6_dhcp", strlen("bond0_ip6_dhcp")) == 0) {
        len = strlen("bond0_ip6_dhcp=");
        net.bond0_ip6_dhcp = atoi((char *)&fgetch[38][len]);
    }
    if (strncmp(fgetch[39], "bond0_ip6_address", strlen("bond0_ip6_address")) == 0) {
        len = strlen("bond0_ip6_address=");
        snprintf(net.bond0_ip6_address, sizeof(net.bond0_ip6_address), "%s", (char *)&fgetch[39][len]);
    }
    if (strncmp(fgetch[40], "bond0_ip6_netmask", strlen("bond0_ip6_netmask")) == 0) {
        len = strlen("bond0_ip6_netmask=");
        snprintf(net.bond0_ip6_netmask, sizeof(net.bond0_ip6_netmask), "%s", (char *)&fgetch[40][len]);
    }
    if (strncmp(fgetch[41], "bond0_ip6_gateway", strlen("bond0_ip6_gateway")) == 0) {
        len = strlen("bond0_ip6_gateway=");
        snprintf(net.bond0_ip6_gateway, sizeof(net.bond0_ip6_gateway), "%s", (char *)&fgetch[41][len]);
    }

    if (strncmp(fgetch[42], "defaultRoute", strlen("defaultRoute")) == 0) {
        len = strlen("defaultRoute=");
        net.defaultRoute = atoi((char *)&fgetch[42][len]);
    }

    snprintf(cmd, sizeof(cmd), "rm -f %s", localfile);
    ms_system(cmd);

    //printf("##################  read_keep_config addr:%s gw:%s, mask:%s %s %s %s\n", net.lan1_ip6_address, net.lan1_ip6_netmask, net.lan1_ip6_gateway, net.lan2_ip6_address, net.lan2_ip6_netmask, net.lan2_ip6_gateway);//hrz.milesight
    //printf("[david debug] read_keep_config addr:%s gw:%s, mask:%s\n", net.lan1_ip_address, net.lan1_gateway, net.lan1_netmask);
    write_network(pDbFile, &net);

    ms_system("sync");
    return 0;
}

int write_keep_params_config(const char *localfile, const char *pDbFile)
{
    if (!localfile || !pDbFile || access(pDbFile, F_OK)) {
        msprintf("arg err, localfile = %s, pDbFile = %s", localfile, pDbFile);
        return -1;
    }

    if (!access(localfile, F_OK)) { // the last reboot didn't delete this file because of the power off
        return 0;
    }

    char buf[256] = {0};
    char poepwd[MAX_LEN_64] = {0};
    int ret = -1;
    char userPath[256] = {0};

    snprintf(userPath, sizeof(userPath), "%s/%s", MS_CONF_DIR, MS_RESET_FLAG_USER_FILE);
    if (access(userPath, F_OK)) {
        ret = get_param_value(pDbFile, PARAM_POE_PSD, poepwd, sizeof(poepwd), POE_DEFAULT_PASSWORD);
        if (ret == -1) {
            msprintf("get PARAM_POE_PSD failed");
            return -1;
        }
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s=%s\n", PARAM_POE_PSD, poepwd);
    }

    // restore save raidMode
    ret = get_param_int(pDbFile, PARAM_RAID_MODE, 0);
    if (ret == -1) {
        msprintf("get PARAM_RAID_MODE failed");
        return -1;
    }
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s=%d\n", PARAM_RAID_MODE, ret);

    FILE *fp = fopen(localfile, "a+");
    if (!fp) {
        return -1;
    }
    fwrite(buf, 1, strlen(buf), fp);
    fflush(fp);
    fclose(fp);

    ms_system("sync");

    return 0;
}

int read_keep_params_config(const char *localfile, const char *pDbFile)
{
    if (access(localfile, F_OK)) {
        return -1;
    }

    int i = 0, ret = 0;
    char fgetch[64][128] = {{0}};
    char cmd[256] = {0};
    char *pchar = NULL;
    char poepwd[MAX_LEN_64] = {0};

    if (!access(OEM_UPLOAD_OPT, F_OK)) {
        snprintf(cmd, sizeof(cmd), "rm -f %s", OEM_UPLOAD_OPT);
        ms_system(cmd);
        if (!access(localfile, F_OK)) {
            snprintf(cmd, sizeof(cmd), "rm -f %s", localfile);
            ms_system(cmd);
        }
        return 0;
    }

    ret = get_param_value(pDbFile, PARAM_POE_PSD, poepwd, sizeof(poepwd), POE_DEFAULT_PASSWORD);
    if (ret == -1) {
        return -1;
    }

    ret = get_param_int(pDbFile, PARAM_RAID_MODE, 0);
    if (ret == -1) {
        return -1;
    }

    FILE *fp = fopen(localfile, "r");
    if (!fp) {
        return -1;
    }
    while (!feof(fp)) {
        fgets((char *)&fgetch[i], sizeof(fgetch[i]), fp);
        if ((pchar = strchr((char *)&fgetch[i], '\n')) != NULL) {
            *pchar = '\0';
        }
        if ((i++) >= 64) {
            break;
        }
    }
    fclose(fp);

    if (!strncmp(fgetch[0], PARAM_POE_PSD, strlen(PARAM_POE_PSD))) {
        snprintf(poepwd, sizeof(poepwd), "%s", fgetch[0] + strlen(PARAM_POE_PSD) + 1);
    }
    set_param_value(pDbFile, PARAM_POE_PSD, poepwd);

    if (!strncmp(fgetch[1], PARAM_RAID_MODE, strlen(PARAM_RAID_MODE))) {
        ret = atoi(fgetch[1] + strlen(PARAM_RAID_MODE) + 1);
    }
    set_param_int(pDbFile, PARAM_RAID_MODE, ret);

    snprintf(cmd, sizeof(cmd), "rm -f %s", localfile);
    ms_system(cmd);

    return 0;
}

