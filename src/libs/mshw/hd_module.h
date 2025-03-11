/* 
 * Filename:      	hd_module
 * Created at:    	2015.03.17
 * Description:   	module controller.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ****************************************************************/
#ifndef _HD_MODULE_H_
#define _HD_MODULE_H_

typedef struct hd_module_info{
	const char* name;
    int (*find)(void);
	int (*load)(void *);
	int (*unload)(void);
}hd_module_info;


void hd_module_register(const struct hd_module_info *info);
void hd_module_unregister(const struct hd_module_info *info);

#define HD_MODULE_INFO(name, find_func, load_func, unload_func)	\
	static struct hd_module_info __mod_info_##name = {	\
		name,					\
		find_func,				\
        load_func,              \
		unload_func,			\
	};							\
	static void  __attribute__((constructor)) __reg_module_##name(void) \
	{ \
		hd_module_register(&__mod_info_##name); \
	} \
	static void  __attribute__((destructor)) __unreg_module_##name(void) \
	{ \
		hd_module_unregister(&__mod_info_##name); \
	}

int hd_get_modules(struct hd_module_info** modules, int* capability);
int hd_get_module_by_name(struct hd_module_info** module, const char* name);
int hd_dump_module(char entrys[][16]);

void hd_module_mutex_init(void);
void hd_module_mutex_deinit(void);
void hd_module_mutex_lock(void);
void hd_module_mutex_unlock(void);
int hd_load_modules(const char **names, int cnt);
int hd_unload_modules(const char **names, int cnt);

#endif


