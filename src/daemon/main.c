#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "msstd.h"
#include "watchdog.h"
#include "msdefs.h"
#include "msdb.h"

static int g_server_dead = 0;
static int g_server_pause = 0;
// unsigned int global_debug_mask = DEBUG_LEVEL_DEFAULT;
// hook_print global_debug_hook = 0;
static int watchdog_close = 0;
static int g_watchdog_conf = 0;
static int g_watchdog_feed = 0;

#define MSCORE_DEBUG
#ifdef MSCORE_DEBUG
static int mscore_debug = 0;
static int mscore_cnt = 0;
#endif 

#define MS_HI_NVR_VERSION  "9.0.19-opensource"

int mu_is_dir(const char *sDir)
{
	struct stat filestat;
	if(stat(sDir, &filestat) != 0)
	{
		perror(sDir);
		/*char cmd[32] = {0};
		sprintf(cmd, "cd %s", sDir);
		system(cmd);
		sprintf(cmd, "ls %s", sDir);
		system(cmd);
		system("ps");*/
		return 0;
	}
	
	if(S_ISDIR(filestat.st_mode))
	{
		return 1;
	}

	return 0;
}

int mu_proc_exist(char *proc_path)
{
	pid_t pId;
	char sBuf[32] = {0};
	FILE *fp = fopen(proc_path, "rb");
	if(!fp)
		return 0;
	fgets(sBuf, sizeof(sBuf), fp);
	fclose(fp);
	if(sBuf[0] == '\0')
		return 0;
	pId = atoi(sBuf);
	snprintf(sBuf, sizeof(sBuf), "/proc/%d", pId);

	return mu_is_dir(sBuf);
}

static void system_reboot()
{
	ms_system("killall -2 mscore");
	ms_system("killall -2 mssp");	
	ms_system("killall -2 msmail");
	ms_system("killall -2 backup");
	ms_system("killall -2 picexport");
	ms_system("killall -2 boa");
	ms_system("killall -2 dropbear");
}

static void signal_handler(int signal)
{
	msprintf("daemon recv signal %d", signal);
	switch(signal)
	{
		case SIGKILL:
		case SIGTERM:
			{
			    g_server_dead = 1;
                g_watchdog_feed = 0;
			}
			break;			
		case SIGHUP:// 1
			{
			msprintf("demo recv:SIGINT==reboot");
			system_reboot();
			g_server_pause = 300;
			while(mu_proc_exist(MS_CORE_PID))
			{
				if(g_server_pause)
					g_server_pause--;
				if(g_server_pause <= 0)
					break;			
				usleep(100*1000);			
			}
			g_server_dead = 1;
            g_watchdog_feed = 0;
			ms_system("/bin/umount -a -r");//version:7.0.5-beta2 add
			msdebug(DEBUG_ERR, "[daivd debug] daemon recv signal %d reboot now...\n", signal);
			ms_system("reboot");
			}
			break;
		case SIGINT:// 2
			{
				msprintf("demo recv:SIGINT==shutdown");
				system_reboot();
				g_server_pause = 300;
				while(mu_proc_exist(MS_CORE_PID))
				{
					if(g_server_pause)
						g_server_pause--;
					if(g_server_pause <= 0)
						break;		
					if(!watchdog_close && (g_server_pause % 50 == 0))
						watchdog_keepalive();				
					usleep(100*1000);			
				}
				ms_system("killall -2 fio");//shutdown at once
				g_server_dead = 1;
                g_watchdog_feed = 0;
				ms_system("/bin/umount -a -r");//version:7.0.5-beta2 add
				msdebug(DEBUG_ERR, "[daivd debug] daemon recv signal %d reboot now...\n", signal);
				ms_system("reboot");			
			}
			break;
		case SIGUSR1:
			{
				msprintf("daemon user1 pause(%d) signal.", g_server_pause);
				while(g_server_pause)
				{
					usleep(10*1000);
				}
				g_server_pause = 1800;//wait for 3+1 minutes for restart_app.sh
				ms_system("killall -2 mscore");
				while(mu_proc_exist(MS_CORE_PID))
				{
					if(g_server_pause)
						g_server_pause--;
					if(g_server_pause <= 0)
						break;
					if(!watchdog_close && (g_server_pause % 50 == 0))
						watchdog_keepalive();
					usleep(100*1000);			
				}
				if(g_server_pause <= 0)
				{
					msprintf("restart mscore failed.");
					ms_system("/bin/umount -a -r");//version:7.0.5-beta2 add
					msdebug(DEBUG_ERR, "[daivd debug] daemon recv signal %d reboot now...\n", signal);
					g_watchdog_feed = 0;
					ms_system("reboot");
					break;
				}
				msprintf("g_server_pause:%d", g_server_pause);
				ms_system("mscore -qws &");
			}
			break;
		case SIGUSR2: 
			{
			msprintf("daemon user2 signal.");
			g_server_pause = 0;
#ifdef MSCORE_DEBUG			
			mscore_cnt = 1;
			mscore_debug = 0;			
				msprintf("daemon mscore_debug=0");
#endif		
			}
			break;
		case SIGURG:
			{
				if(!g_watchdog_conf)
				{
					ms_system("killall -2 mscore");
				}
				else
				{
					ms_system("mscore -qws &");
				}
				g_watchdog_conf = !g_watchdog_conf;
			}
			break;
	}
}

static void ms_init_signal()
{
	signal(SIGHUP, 	signal_handler);
	signal(SIGINT,  signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGKILL, signal_handler);
	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);
	signal(SIGURG, signal_handler);
}

int printf_sys_up_time()
{
	long nSeconds;
	char buffer[128] = {0};
	FILE *fp = fopen("/proc/uptime", "rb");
	if(fp)
	{
		fgets(buffer, sizeof(buffer), fp);
		sscanf(buffer, "%127s ", buffer);
		fclose(fp);
	}
	sscanf(buffer,"%ld",&nSeconds);
	msdebug(DEBUG_ERR, 
		"#########System up time :%ldday %ldhour %ld minute %ld second##########",
		nSeconds/86400, (nSeconds%86400)/3600,
		((nSeconds%86400)%3600)/60, ((nSeconds%86400)%3600)%60);
	return 0;	
}

static int get_software_version(char *version, int size)
{
	FILE *fp = NULL;
	char value[256] = {0};
	ms_system("cat /proc/cpuinfo | grep Hardware > /tmp/cpu.txt");
	fp = fopen("/tmp/cpu.txt", "rb");
	if(fp)
	{
		if(fgets(value, sizeof(value), fp))
		{
			if(strstr(value, "hi3536"))
			{
				snprintf(version, size, "71.%s", MS_HI_NVR_VERSION);
			}
			else if(strstr(value, "bigfish"))  //hi3798's alias
			{
				snprintf(version, size, "72.%s", MS_HI_NVR_VERSION);
			}
			else if(strstr(value, "Hi3536C"))
			{
				snprintf(version, size, "73.%s", MS_HI_NVR_VERSION);
			}
			else if(strstr(value, "xxxx"))
			{
				get_param_value(SQLITE_FILE_NAME, PARAM_DEVICE_SV, version, size, "7x.9.0.9");
			}
		}
		fclose(fp);
	}
	else
	{
		get_param_value(SQLITE_FILE_NAME, PARAM_DEVICE_SV, version, size, "7x.9.0.9");
	}
	
	return 0;
}

static void sig_handler_event(int sig)
{
    system("touch /mnt/nand3/out_of_memory.txt");
    msprintf("received interested_event signal.\n");
}

static int out_of_memory_signal_init()
{
    struct sigaction usr_action;
    sigset_t block_mask;

    sigfillset(&block_mask);
    usr_action.sa_handler = sig_handler_event;
    usr_action.sa_mask = block_mask;//block all signal inside signal handler.
    usr_action.sa_flags = SA_NODEFER;//do not block SIGUSR1 within sig_handler_int.
    msprintf("handle signal %d\n", SIGRTMIN + 1);
    sigaction(SIGRTMIN + 1, &usr_action, NULL);
	
    return 0;
}


int main(int argc, char *argv[])
{
	int cnt = 0;
	char version[256] = {0};
	
#ifdef MSCORE_DEBUG
	int restart_cnt = 0;	
	int mscore_test = !access(MS_CONF_DIR "/test_mscore", F_OK);
#endif
	
	get_software_version(version, sizeof(version));

	ms_init_signal();

	out_of_memory_signal_init(); //goli add 2022.12.15
	
	if(watchdog_start())
	{
		watchdog_close = 1;
	}
	else
	{
		g_server_pause = 120;
	}
	while(!g_server_dead)
	{
		cnt++;
		if(!watchdog_close && cnt % 50 == 0)// 5seconds
		{
		    if (g_watchdog_feed)
    			watchdog_keepalive();
		}
		else if(g_server_pause)
		{
			g_server_pause--;
		}
		
		if(g_watchdog_conf)
		{
			usleep(100*1000);
			continue;
		}
#ifdef MSCORE_DEBUG//debug		
		if(mscore_test)
		{
			mscore_cnt++;
			if(mscore_cnt % 300 == 0 && !mscore_debug)
			{
				ms_system("killall -2 mscore");
				msprintf("====daemon killall -2 mscore====");
				mscore_debug = 1;
				mscore_cnt = 1;
			}
			if(mscore_cnt % 300 == 0 && mscore_debug == 1)
			{
				mscore_debug = -1;
				mscore_cnt = 0;
				if(!mu_proc_exist(MS_CORE_PID))
				{
					ms_system("mscore -qws &");
					restart_cnt++;
					msprintf("====daemon mscore &= nrestart times:%d========", restart_cnt);
				}
			}
		}	
#endif		
		if(!watchdog_close && cnt % 600 == 0 && !g_server_pause)//60 seconds
		{
			if(!mu_proc_exist(MS_CORE_PID))
			{
                g_watchdog_feed = 0; 
				msdebug(DEBUG_ERR, "mscore die.");
				mslogerr(DEBUG_ERR, "mscore die! reboot now. version:%s", version);
				printf_sys_up_time();
				ms_system("/bin/umount -a -r");
				ms_system("reboot &");
			}
			else
			{
                g_watchdog_feed = 1; 
    			watchdog_keepalive();
			}
			
			if(!mu_proc_exist(MS_BOA_PID))
			{
				msdebug(DEBUG_ERR, "boa die,reboot boa now. version:%s", version);
				ms_system("killall -9 boa");	
				ms_system("boa -f /etc/boa/boa.conf &");				
			}
		}	
		usleep(100*1000);
	}
	if(!watchdog_close)
		watchdog_stop();

	return 0;
}
