/* 
 * Filename:      	hd_module
 * Created at:    	2015.03.17
 * Description:   	module controller.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ****************************************************************/
#ifndef _HD_MODULE_H_
#define _HD_MODULE_H_

#define MAX_HD_MODULE 16

typedef struct hd_module_info{
	const char* name;
	int (*load)(void);
	int (*unload)(void);
}hd_module_info;

#define TEST_NAME_GPIO	    "GPIO"
#define TEST_NAME_AUDIO	    "AUDIO"
#define TEST_NAME_VIDEO	    "VIDEO"
#define TEST_NAME_IR	    "IR"
#define TEST_NAME_RS485	    "RS485"
#define TEST_NAME_ENCRYPT	"ENCRYPT"
#define TEST_NAME_FLASH	    "FLASH"

void hd_module_register(const struct hd_module_info *info);
void hd_module_unregister(const struct hd_module_info *info);

#define HD_MODULE_INFO(name, load_func, unload_func)	\
	static struct hd_module_info __mod_info = {	\
		name,					\
		load_func,				\
		unload_func,			\
	};							\
	static void  __attribute__((constructor)) __reg_module(void) \
	{ \
		hd_module_register(&__mod_info); \
	} \
	static void  __attribute__((destructor)) __unreg_module(void) \
	{ \
		hd_module_unregister(&__mod_info); \
	}

int hd_get_modules(struct hd_module_info** modules, int* capability);
int hd_get_module_by_name(struct hd_module_info** module, const char* name);
void hd_module_mutex_init(void);
void hd_module_mutex_deinit(void);
void hd_module_mutex_lock(void);
void hd_module_mutex_unlock(void);
int hd_load_modules(const char **names, int cnt);
int hd_unload_modules(const char **names, int cnt);

#endif


