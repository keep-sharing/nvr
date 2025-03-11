#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include "ccsqlite.h"
#include "msstd.h"
#include "msdb.h"
#include "msdefs.h"
#include "emaildb.h"

#define TAR_CONFIG_PASSWORD "20160127"
#define KEY_FILE_PATH		"/tmp/.key_file"

struct file_key
{
	char model[32];
	char soft_ver[32];
	char hard_ver[32];
};

// unsigned int global_debug_mask = DEBUG_LEVEL_DEFAULT;
// hook_print global_debug_hook = 0;
int global_exit = 0;

static void signal_handler(int signal)
{
	msprintf("profile recv signal %d", signal);
	global_exit = 1;
}


static void set_profile_signal()
{
	signal(SIGINT,  signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGKILL, signal_handler);
	//signal(SIGPIPE, signal_handler);
}

//导入规则
//1.型号相同
//2.版本支持升级，禁止降级 72.9.0.1->72.8.0.6
static int import_config(const char *path)
{
	char cmd[512], *old_ver, *new_ver;
	int result;
	FILE *fp;
	struct file_key file_key = {{0}};
	struct device_info device = {0}, device_oem = {0};
    if(!path || !strstr(path, ".cfg"))
		return -1;
 	
    snprintf(cmd, sizeof(cmd), "cd %s; dd if=\"%s\" |openssl des3 -d -k %s -md md5|tar zxf -", "/tmp/", path, TAR_CONFIG_PASSWORD);
	result = ms_system(cmd);
	if(result < 0)
	{
		msprintf("tar config file to /tmp/ error.");
		return result;
	}
	snprintf(cmd, sizeof(cmd), "/tmp/%s", KEY_FILE_PATH);
	fp = fopen(cmd, "rb");
	if(!fp)
	{
		msprintf("get key file failed.");
		return -1;
	}
	fread(&file_key, sizeof(file_key), 1, fp);
	fclose(fp);

	db_get_device(SQLITE_FILE_NAME, &device);
	
	//david.milesight read network info 
	struct network net = {0};
	struct network net_conf = {0};
	read_network(SQLITE_FILE_NAME, &net);

	if(device.oem_type){
		db_get_device_oem(SQLITE_FILE_NAME, &device_oem);
		snprintf(device.model, sizeof(device.model), "%s", device_oem.model);
	}
	
	if(strcmp(device.model, file_key.model))
	{
		msprintf("model not match.%s==%s", file_key.model, device.model);
		return -1;
	}

	if((old_ver = strchr(file_key.soft_ver, '-')) != NULL)
		*old_ver = '\0';
	if((new_ver = strchr(device.softver, '-')) != NULL)
		*new_ver = '\0';
	if(strlen(file_key.soft_ver) == strlen(device.softver)){
		if(strcmp(file_key.soft_ver, device.softver) > 0){
			msprintf("software version not match.%s len:%d==%s len:%d===", file_key.soft_ver, strlen(file_key.soft_ver), device.softver, strlen(device.softver));	
			return -1;
		}
	}else{
		//msdebug(DEBUG_ERR, "software version not match.%s len:%d==%s len:%d=bool:%d=%c %c=", file_key.soft_ver, strlen(file_key.soft_ver), device.softver, strlen(device.softver), ((file_key.soft_ver[0] > device.softver[0])?1:0), file_key.soft_ver[0], device.softver[0]);
		if((file_key.soft_ver[3] > device.softver[3]) && (file_key.soft_ver[4] == device.softver[4]) && (file_key.soft_ver[4] == '.'))
			return -1;
		if((file_key.soft_ver[3] == device.softver[3]) && (strlen(file_key.soft_ver) > strlen(device.softver)))
			return -1;
	}
	snprintf(cmd, sizeof(cmd), "mv /tmp/%s %s", SQLITE_FILE_NAME, SQLITE_FILE_NAME);
	result = ms_system(cmd);
	if(result < 0){
		msprintf("mv database failed.");
		return -1;
	}
	
	//david.milesight save network info 
	read_network(SQLITE_FILE_NAME, &net_conf);
	snprintf(net.host_name, sizeof(net.host_name), "%s", net_conf.host_name);
	write_network(SQLITE_FILE_NAME, &net);
	
	snprintf(cmd, sizeof(cmd), "sysconf -s time > /dev/null &");
	ms_system(cmd);

    return 0;
}

static void get_file_name_filter(char *src, int src_len, char *des, int des_len, char mask)
{
	int i = 0;
	char temp = 0;

	if (!src || !src_len || !des || !des_len || des_len < src_len)
		return;

	for (i = 0; i < src_len; i++)
	{
		temp = src[i];
		if (temp == '\0')
		{
			des[i] = '\0';
			break;
		}
		
		if (temp == '/' 
			|| temp == '\\' 
			|| temp == ':'
			|| temp == '*'
			|| temp == '?'
			|| temp == '"'
			|| temp == '>'
			|| temp == '<'
			|| temp == '|')
		{
			temp = mask;
		}
		des[i] = temp;
	}

	return;
}

static int export_config(const char *dir)
{
	struct device_info device = {0};
	struct network network = {0};
	char date_time[32] = {0}, file_path[512] = {0}, cmd[1024];
	char pModel[MAX_LEN_64] = {0};
	time_t now;
	struct tm tm_now;
	int result;
	FILE *fp; 
	struct file_key file_key;
	if(!dir)
		return -1;
	
	db_get_device(SQLITE_FILE_NAME, &device);
    read_network(SQLITE_FILE_NAME, &network); 
	snprintf(file_key.soft_ver, sizeof(file_key.soft_ver), "%s", device.softver);
	snprintf(file_key.hard_ver, sizeof(file_key.hard_ver), "%s", device.hardver);
	if(device.oem_type){
		db_get_device_oem(SQLITE_FILE_NAME, &device);
		snprintf(file_key.model, sizeof(file_key.model), "%s", device.model);
	}else{
		snprintf(file_key.model, sizeof(file_key.model), "%s", device.model);
	}
	
    fp = fopen(KEY_FILE_PATH, "wb");
    if(!fp)
		return -1;
    fwrite(&file_key, sizeof(struct file_key), 1, fp);
	fflush(fp);
    fclose(fp);	

	time(&now);
    localtime_r(&now, &tm_now);
    strftime(date_time, sizeof(date_time), "%Y%m%d%H%M%S", &tm_now);

	get_file_name_filter(device.model, sizeof(device.model), pModel, sizeof(pModel), '-');
	char host_name[256] = {0};
	get_file_name_filter(network.host_name, sizeof(network.host_name), host_name, sizeof(host_name), '-');
	snprintf(file_path, sizeof(file_path), "%s/%s-%s-%s.cfg", dir, host_name, pModel, date_time);
	//msprintf("file_path:%s", file_path);
    snprintf(cmd, sizeof(cmd), "tar -zcvf - \"%s\" \"%s\"|openssl des3 -salt -k %s -md md5| dd of=\"%s\"", SQLITE_FILE_NAME, KEY_FILE_PATH, TAR_CONFIG_PASSWORD, file_path);
	//msprintf("cmd:%s", cmd);
	result = ms_system(cmd);
	if(result < 0)
	{
		msprintf("tar failed.");
		return -1;
	}
    ms_system("sync");

    fp = fopen(WEB_DOWNLOAD_NAME_FILE, "wb");
    if(!fp) 
		return -1;
    fprintf(fp, "%s", file_path);
	fflush(fp);
    fclose(fp);

    return result;
}

int main(int argc, char **argv)
{	
	if(argc < 3 || (strcmp(argv[1], "-i") && strcmp(argv[1], "-e")))
	{
		fprintf(stderr, "profile -[i|e] <file_path>\n");
		return -1;
	}
	set_profile_signal();

	if(strcmp(argv[1], "-i") == 0)
		return import_config(argv[2]);
	else if(strcmp(argv[1], "-e") == 0)
		return export_config(argv[2]);

	return -1;
}
