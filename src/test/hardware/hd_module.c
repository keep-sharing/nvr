#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "msstd.h"
#include "hd_module.h"

static int global_md_cnt = 0;
static struct hd_module_info global_modules[MAX_HD_MODULE];
static MUTEX_OBJECT g_mutex;

static struct hd_module_info* find_module(const char* name)
{
	int i = 0;
	for ( ; i < MAX_HD_MODULE; i++)
	{
		if (global_modules[i].name && strcasecmp(global_modules[i].name, name)==0) 
			return &global_modules[i];
	}
	return NULL;
}

static void insert_module_info(struct hd_module_info modules[MAX_HD_MODULE], struct hd_module_info* info)
{
	int idx = 0;
	for ( ; idx < MAX_HD_MODULE; idx++)
	{
		if (modules[idx].name == NULL)
		{
            memcpy(modules+idx, info, sizeof(struct hd_module_info));
            global_md_cnt++;
			break;
		}
	}
}

void hd_module_register(const struct hd_module_info* info)
{
	if (!info || !info->name) 
		return;
	if (find_module(info->name)) 
		return;
	if (global_md_cnt >= MAX_HD_MODULE)
		return;
	int i = 0;
	for ( ; i < MAX_HD_MODULE; i++)
	{
		if (global_modules[i].name)
			continue;
		insert_module_info(global_modules, (struct hd_module_info*)(info));
		break;
	}
}

void hd_module_unregister(const struct hd_module_info* info)
{
	if (!info || !info->name) 
		return;
	struct hd_module_info* mod = find_module(info->name);
	if (!mod) 
		return;
	memset(mod, 0, sizeof(struct hd_module_info));
	global_md_cnt--;
}

int hd_get_modules(struct hd_module_info** modules, int* capability)
{
	if (!modules || !capability) 
		return -1;
	*modules	= global_modules;
	*capability	= global_md_cnt;
	return 0;
}

int hd_get_module_by_name(struct hd_module_info** module, const char* name)
{
    *module = find_module(name);
    if (*module != NULL)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

void hd_module_mutex_init(void)
{
    ms_mutex_init(&g_mutex);
}

void hd_module_mutex_deinit(void)
{
    ms_mutex_uninit(&g_mutex);
}

void hd_module_mutex_lock(void)
{
    ms_mutex_lock(&g_mutex);
}

void hd_module_mutex_unlock(void)
{
    ms_mutex_unlock(&g_mutex);
}

