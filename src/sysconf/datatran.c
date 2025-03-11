#include <stdio.h>
#include "datatran.h"

int netDbToInternal(struct network *dbCfg, NETWORKCFG *interNetCfg)
{
	interNetCfg->mode = dbCfg->mode;
	snprintf(interNetCfg->hostname, sizeof(interNetCfg->hostname), "%s", dbCfg->host_name);
	if(dbCfg->mode != NETMODE_MULTI){
		interNetCfg->bondcfg.primary = dbCfg->bond0_primary_net;
		interNetCfg->bondcfg.miimon = dbCfg->miimon;
		interNetCfg->bondcfg.bondif.enable = dbCfg->bond0_enable;
		interNetCfg->bondcfg.bondif.type = dbCfg->bond0_type;
		snprintf(interNetCfg->bondcfg.bondif.ipaddr, sizeof(interNetCfg->bondcfg.bondif.ipaddr), "%s", dbCfg->bond0_ip_address);
		snprintf(interNetCfg->bondcfg.bondif.netmask, sizeof(interNetCfg->bondcfg.bondif.netmask), "%s", dbCfg->bond0_netmask);
		snprintf(interNetCfg->bondcfg.bondif.gateway, sizeof(interNetCfg->bondcfg.bondif.gateway), "%s", dbCfg->bond0_gateway);
		snprintf(interNetCfg->bondcfg.bondif.dns1, sizeof(interNetCfg->bondcfg.bondif.dns1), "%s", dbCfg->bond0_primary_dns);
		snprintf(interNetCfg->bondcfg.bondif.dns2, sizeof(interNetCfg->bondcfg.bondif.dns2), "%s", dbCfg->bond0_second_dns);
		interNetCfg->bondcfg.bondif.mtu = dbCfg->bond0_mtu;

        snprintf(interNetCfg->bondcfg.bondif.ip6addr, sizeof(interNetCfg->bondcfg.bondif.ip6addr), "%s", dbCfg->bond0_ip6_address);//hrz.milesight
        snprintf(interNetCfg->bondcfg.bondif.ip6netmask, sizeof(interNetCfg->bondcfg.bondif.ip6netmask), "%s", dbCfg->bond0_ip6_netmask);
        snprintf(interNetCfg->bondcfg.bondif.ip6gateway, sizeof(interNetCfg->bondcfg.bondif.ip6gateway), "%s", dbCfg->bond0_ip6_gateway);
        interNetCfg->bondcfg.bondif.ip6type = dbCfg->bond0_ip6_dhcp;//0=static 1=Router Advertisement, 2=dhcp
        
        
	}else{
		interNetCfg->netif1.enable = dbCfg->lan1_enable;
		interNetCfg->netif1.type = dbCfg->lan1_type;
		interNetCfg->netif1.mtu = dbCfg->lan1_mtu;
		snprintf(interNetCfg->netif1.ipaddr, sizeof(interNetCfg->netif1.ipaddr), "%s", dbCfg->lan1_ip_address);
		snprintf(interNetCfg->netif1.netmask, sizeof(interNetCfg->netif1.netmask), "%s", dbCfg->lan1_netmask);
		snprintf(interNetCfg->netif1.gateway, sizeof(interNetCfg->netif1.gateway), "%s", dbCfg->lan1_gateway);
		snprintf(interNetCfg->netif1.dns1, sizeof(interNetCfg->netif1.dns1), "%s", dbCfg->lan1_primary_dns);
		snprintf(interNetCfg->netif1.dns2, sizeof(interNetCfg->netif1.dns2), "%s", dbCfg->lan1_second_dns);

		snprintf(interNetCfg->netif1.ip6addr, sizeof(interNetCfg->netif1.ip6addr), "%s", dbCfg->lan1_ip6_address);//hrz.milesight
		snprintf(interNetCfg->netif1.ip6netmask, sizeof(interNetCfg->netif1.ip6netmask), "%s", dbCfg->lan1_ip6_netmask);
		snprintf(interNetCfg->netif1.ip6gateway, sizeof(interNetCfg->netif1.ip6gateway), "%s", dbCfg->lan1_ip6_gateway);
         interNetCfg->netif1.ip6type = dbCfg->lan1_ip6_dhcp;//end

		interNetCfg->netif2.enable = dbCfg->lan2_enable;
		interNetCfg->netif2.type = dbCfg->lan2_type;
		interNetCfg->netif2.mtu = dbCfg->lan2_mtu;
		snprintf(interNetCfg->netif2.ipaddr, sizeof(interNetCfg->netif2.ipaddr), "%s", dbCfg->lan2_ip_address);
		snprintf(interNetCfg->netif2.netmask, sizeof(interNetCfg->netif2.netmask), "%s", dbCfg->lan2_netmask);
		snprintf(interNetCfg->netif2.gateway, sizeof(interNetCfg->netif2.gateway), "%s", dbCfg->lan2_gateway);
		snprintf(interNetCfg->netif2.dns1, sizeof(interNetCfg->netif2.dns1), "%s", dbCfg->lan2_primary_dns);
		snprintf(interNetCfg->netif2.dns2, sizeof(interNetCfg->netif2.dns2), "%s", dbCfg->lan2_second_dns);
         snprintf(interNetCfg->netif2.ip6addr, sizeof(interNetCfg->netif2.ip6addr), "%s", dbCfg->lan2_ip6_address);//hrz.milesight
		snprintf(interNetCfg->netif2.ip6netmask, sizeof(interNetCfg->netif2.ip6netmask), "%s", dbCfg->lan2_ip6_netmask);
		snprintf(interNetCfg->netif2.ip6gateway, sizeof(interNetCfg->netif2.ip6gateway), "%s", dbCfg->lan2_ip6_gateway);
         interNetCfg->netif2.ip6type = dbCfg->lan2_ip6_dhcp;//end
	}
	return 0;
}

int accessDbToInternal(struct network_more *accessDb, ACCESSCFG *accessCfg)
{
	accessCfg->access = accessDb->enable_ssh;
	accessCfg->sshport= accessDb->ssh_port;
	accessCfg->httpport= accessDb->http_port;
	accessCfg->rtspport = accessDb->rtsp_port;
	accessCfg->httpsport = accessDb->https_port;
	return 0;
}

int emailDbToInternal(struct email *emailDb, MAILCFG *mailcfg)
{
	snprintf(mailcfg->smtpserver, sizeof(mailcfg->smtpserver), "%s", emailDb->smtp_server);
	snprintf(mailcfg->username, sizeof(mailcfg->username), "%s", emailDb->username);
	snprintf(mailcfg->passwd, sizeof(mailcfg->passwd), "%s", emailDb->password);
	snprintf(mailcfg->sendername, sizeof(mailcfg->sendername), "%s", emailDb->sender_name);
	snprintf(mailcfg->sender, sizeof(mailcfg->sender), "%s", emailDb->sender_addr);
	mailcfg->port = emailDb->port;
	mailcfg->enabletls = emailDb->enable_tls;
	return 0;
}

