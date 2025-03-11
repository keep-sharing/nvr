#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include "msstd.h"
#include "syscfg.h"

// unsigned int global_debug_mask = DEBUG_LEVEL_DEFAULT;
// hook_print global_debug_hook = 0;

static int sysconf_set_hostname()
{
	struct network networkDb;
	memset(&networkDb, 0, sizeof(networkDb));
	read_network(SQLITE_FILE_NAME, &networkDb);
	return ms_set_hostname(networkDb.host_name);
}

static int sysconf_set_network()
{
	struct network networkDb;
	memset(&networkDb, 0, sizeof(networkDb));
	read_network(SQLITE_FILE_NAME, &networkDb);
	
	return ms_set_network(&networkDb);
}

static int sysconf_set_ethname()
{
    return ms_set_ethname();    
}

static int sysconf_set_network_applynow()
{
	struct network networkDb;
	memset(&networkDb, 0, sizeof(networkDb));
	read_network(SQLITE_FILE_NAME, &networkDb);
	return ms_set_network_apply(&networkDb);
}

static int sysconf_set_pppoe()
{
	struct pppoe pppoeDb;
	memset(&pppoeDb, 0, sizeof(pppoeDb));
	read_pppoe(SQLITE_FILE_NAME, &pppoeDb);

	int networkMode = 0;
	read_network_mode(SQLITE_FILE_NAME, &networkMode);
	return ms_set_PPPOE(&pppoeDb, networkMode);
}

static int sysconf_set_ddns()
{
	struct ddns ddnsDb;
	memset(&ddnsDb, 0, sizeof(ddnsDb));
	read_ddns(SQLITE_FILE_NAME, &ddnsDb);
	return ms_set_DDNS(&ddnsDb);
}

static int sysconf_set_mail()
{
	struct email emailDb;
	memset(&emailDb, 0, sizeof(emailDb));
	read_email(SQLITE_FILE_NAME, &emailDb);
	return ms_set_mail(&emailDb);
}

static int sysconf_test_mail()
{
	struct email emailDb;
	memset(&emailDb, 0, sizeof(emailDb));
	read_email(SQLITE_FILE_NAME, &emailDb);
	return ms_send_test_mail(&emailDb);
}

static int sysconf_set_more()
{
	struct network_more netMoreDb;
	memset(&netMoreDb, 0, sizeof(netMoreDb));
	read_port(SQLITE_FILE_NAME, &netMoreDb);
	return ms_set_remote_access(&netMoreDb);
}

static int sysconf_set_snmp()
{
	struct snmp snmp = {0};
	memset(&snmp, 0, sizeof(snmp));
	if (read_snmp(SQLITE_FILE_NAME, &snmp) == 0)
	{
	  ms_set_net_snmp(&snmp);
	}

	return 0;
}

static int set_nginx_conf()
{
	char conf[1024*10] = {0};
	char host[10] = {0};
	int i = 0, cameracnt = 0, j = 0, num = 0, b_exist = 0;
	int port[MAX_CAMERA] = {-1};
	struct camera cameras[MAX_CAMERA];	
	snprintf(conf,sizeof(conf),"%s",MS_NGINX_CONF_HEAD);
		
	FILE* fp = fopen(MS_NGINX_CONF, "w");	
	if (fp)
		fwrite(conf, strlen(conf), 1, fp);
	else
		return -1;	
	
	memset(cameras, 0, sizeof(cameras));
	read_cameras(SQLITE_FILE_NAME, cameras, &cameracnt);
	for (i = 0; i < cameracnt; i++){
		snprintf(host,sizeof(host),"%s%d","ch_",i);
		if(!cameras[i].enable) continue;
		if(cameras[i].manage_port <= 0) cameras[i].manage_port = 80;
		if(cameras[i].physical_port)
		{
			for(j = 0; j < num; j++)
			{
				if(cameras[i].physical_port == port[j])
				{
					b_exist = 1;
					continue;
				}
			}
			if(b_exist)
			{
				b_exist = 0;
				continue;
			}
			port[num] = cameras[i].physical_port;
			num++;
			snprintf(conf,sizeof(conf),"%s%s%s%s%s%s%d%s%s%s",
				"	upstream ",host," {\r\n",
				"		server	",cameras[i].ip_addr,":",cameras[i].manage_port," max_fails=10 fail_timeout=180s;\r\n",
				"		keepalive	4;\r\n"
				"	}\r\n",
				"	server {\r\n"
			);
			fwrite(conf, strlen(conf), 1, fp);
			if(cameras[i].physical_port > 9)
			{
				snprintf(conf,sizeof(conf),"%s%d%s%s%d%s%s%s%s%s%s",
					"		listen	230",cameras[i].physical_port,";\r\n",
					"		listen	[::]:230",cameras[i].physical_port,"	ipv6only=on default_server;\r\n",
					MS_NGINX_CONF_BADY,
		            "			proxy_pass http://",host,";\r\n",
		            MS_NGINX_CONF_LOCAL
				);			
			}
			else
			{
				snprintf(conf,sizeof(conf),"%s%d%s%s%d%s%s%s%s%s%s",
					"		listen	2300",cameras[i].physical_port,";\r\n",
					"		listen	[::]:2300",cameras[i].physical_port,"	ipv6only=on default_server;\r\n",
					MS_NGINX_CONF_BADY,
		            "			proxy_pass http://",host,";\r\n",
		            MS_NGINX_CONF_LOCAL
				);				
			}
			fwrite(conf, strlen(conf), 1, fp);
		}
	}
	snprintf(conf,sizeof(conf),"%s",
		"}\r\n"
	);
	fwrite(conf, strlen(conf), 1, fp);
	fclose(fp);
	fp = NULL;
	return 0;
}

static int sysconf_set_nginx()
{
	int enable_access = get_param_int(SQLITE_FILE_NAME,PARAM_ACCESS_ENABLE,0);
	if(enable_access){
		set_nginx_conf();
		ms_system("killall -9 nginx");
		ms_system("nginx &");
	}
	return 0;
}

static int sysconf_set_time(char *timeStr)
{
	struct time timeDb;
	memset(&timeDb, 0, sizeof(timeDb));
	read_time(SQLITE_FILE_NAME, &timeDb);
	msprintf("ntp_enable=%d\n", timeDb.ntp_enable);
	return ms_set_time(&timeDb, timeStr);
}

static int sysconf_start_ssh()
{
	struct network_more netMoreDb;
	memset(&netMoreDb, 0, sizeof(netMoreDb));
	read_port(SQLITE_FILE_NAME, &netMoreDb);

	return ms_start_ssh(netMoreDb.ssh_port, 1);
}

/*
	set time:
1. 	sysconf.sh 
	./bin/sysconf $1 $2 "$3"

2.
	./bin/sysconf -s time "2012-12-12 11:11:11" 
*/
int main(int argc, char **argv)
{
	int j = 0;
	char cmd[1024] = {0};
	for (j = 0; j < argc; j++) {
		snprintf(cmd + strlen(cmd), sizeof(cmd)-strlen(cmd), "%s ", argv[j]);
	}
	msprintf("sysconf argc=%d argv=%s", argc, cmd);
	//msprintf("sysconf argc=%d", argc);
	
	if(argc < 3 || strcmp(argv[1],"-s") || (strcmp(argv[2], "sysinit") && strcmp(argv[2], "time") && strcmp(argv[2], "network")))
	{
			fprintf(stderr,"Usage1:%s -[s] [sysinit|network|time] [network|netapply|pppoe|ddns|mail|more|hostname|all|\"YYYY-MM-DD hh:mm:ss\"]\n\n", argv[0]);
			return 0; 
	}
	///////////////////////////////////////////////////////
	//pppoe use tcmalloc has problem.
	//msprintf("sysconf LD_PRELOAD= %s", getenv("LD_PRELOAD"));
	unsetenv("sysconf LD_PRELOAD");
	////////////////////////////////////////////////////////
	
	if(strcmp(argv[1], "-s") == 0)
	{
		if(strcmp(argv[2], "sysinit") == 0)
		{
			ms_sys_init();				
			sysconf_start_ssh();
		}
		else if(strcmp(argv[2], "network") == 0)
		{
			if(argv[3] == NULL)
			{
				fprintf(stderr,"Usage2:%s -[s] [network] [rename|network|netapply|pppoe|ddns|mail|more|hostname|snmp|all]\n\n", argv[0]);
				return 0;
			}
			if(strcmp(argv[3], "rename") == 0)
			{
				msprintf("rename eth...");
				sysconf_set_ethname();	
			}
			else if(strcmp(argv[3], "network") == 0)
			{
				msprintf("setting network...");
				sysconf_set_network();	
			}
			else if(strcmp(argv[3], "netapply") == 0){
				msprintf("setting network apply...");
				sysconf_set_network_applynow();			
			}			
			else if(strcmp(argv[3], "hostname") == 0)
			{
				msprintf("setting hostname...");
				sysconf_set_hostname();
			}
			else if(strcmp(argv[3], "pppoe") == 0)
			{
				msprintf("setting pppoe...");
				sysconf_set_pppoe();
			}
			else if(strcmp(argv[3], "ddns") == 0)
			{
				msprintf("setting ddns...");
				sysconf_set_ddns();
			}
			else if(strcmp(argv[3], "mail") == 0)
			{
				msprintf("setting mail...");
				sysconf_set_mail();
			}
			else if(strcmp(argv[3], "testmail") == 0)
			{
				msprintf("test mail...");
				return sysconf_test_mail();//0 success, -1 fail;
			}
			else if(strcmp(argv[3], "more") == 0){
				msprintf("settting more...");
				sysconf_set_more();
			}
			else if (strcmp(argv[3], "snmp") == 0)
			{
				msprintf("setting snmp...");
				sysconf_set_snmp();
			}
			else if (strcmp(argv[3], "nginx") == 0)
			{
				msprintf("setting nginx...");
				sysconf_set_nginx();
			}
			else if(strcmp(argv[3], "all") == 0)
			{
				msprintf("settting network all...");
				sysconf_set_network();
				sysconf_set_pppoe();
				sysconf_set_ddns();	
				sysconf_set_mail();
				sysconf_set_more();
				sysconf_set_snmp();
				sysconf_set_nginx();
			}
			else
			{
				fprintf(stderr,"Usage3:%s -[s] [network] [rename|network|netapply|pppoe|ddns|mail|more|hostname|all|snmp]\n\n", argv[0]);
				return 0;
			}
		}
		else if(strcmp(argv[2], "time") == 0)
		{
			if(argv[3] == NULL || strcmp(argv[3],"")==0)
			{
				msprintf("settting time NULL...");
				sysconf_set_time(NULL);
			}
			else
			{
				msprintf("argv[3]=%s", argv[3]);
				msprintf("settting time %s...",argv[3]);
				sysconf_set_time(argv[3]);
			}
		}
		else
		{
			fprintf(stderr,"Usage4:%s -[s] [network|time] [pppoe|ddns|mail|more|hostname|all|\"YYYY-MM-DD hh:mm:ss\"]\n\n", argv[0]);
			return 0;
		}
			
	}	
	else
	{
		fprintf(stderr,"Usage5:%s -[s] [network|time] [pppoe|ddns|mail|more|hostname|all|\"YYYY-MM-DD hh:mm:ss\"]\n\n", argv[0]);
		return 0;
	}
	
	return 0;
}
