#ifndef __HDMITX_MAIN_H__
#define __HDMITX_MAIN_H__
#include <linux/cdev.h>
#include <linux/types.h>
#include "hdmitx_drv.h"


#define MODULE_MINOR_ID      0
#define MODULE_MINOR_COUNT   1
#define MODULE_NAME          "nvt_hdmitx"

typedef struct hdmitx_drv_info {
	HDMITX_MODULE_INFO module_info;

	struct class *pmodule_class;
	struct device *pdevice[MODULE_MINOR_COUNT];
	struct resource *presource[MODULE_REG_NUM];
	struct cdev cdev;
	dev_t dev_id;

	// proc entries
	struct proc_dir_entry *pproc_module_root;
	struct proc_dir_entry *pproc_help_entry;
	struct proc_dir_entry *pproc_cmd_entry;
} HDMITX_DRV_INFO, *PHDMITX_DRV_INFO;


#endif
