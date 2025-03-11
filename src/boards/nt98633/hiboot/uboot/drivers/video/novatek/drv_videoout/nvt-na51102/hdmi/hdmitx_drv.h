#ifndef __HDMITX_DRV_H__
#define __HDMITX_DRV_H__
#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/clk.h>

#define MODULE_IRQ_NUM          1
#define MODULE_REG_NUM          1
#define MODULE_CLK_NUM          1

#define WRITE_REG(value, addr)  iowrite32(value, addr)
#define READ_REG(addr)          ioread32(addr)

typedef struct hdmitx_module_info {
	struct clk *pclk[MODULE_CLK_NUM];
	void __iomem *io_addr[MODULE_REG_NUM];
	int iinterrupt_id[MODULE_IRQ_NUM];
} HDMITX_MODULE_INFO, *PHDMITX_MODULE_INFO;

int nvt_hdmitx_drv_open(PHDMITX_MODULE_INFO pmodule_info, unsigned char uc_if);
int nvt_hdmitx_drv_release(PHDMITX_MODULE_INFO pmodule_info, unsigned char uc_if);
int nvt_hdmitx_drv_init(PHDMITX_MODULE_INFO pmodule_info);
int nvt_hdmitx_drv_remove(PHDMITX_MODULE_INFO pmodule_info);
int nvt_hdmitx_drv_suspend(HDMITX_MODULE_INFO *pmodule_info);
int nvt_hdmitx_drv_resume(HDMITX_MODULE_INFO *pmodule_info);
int nvt_hdmitx_drv_ioctl(unsigned char uc_if, HDMITX_MODULE_INFO *pmodule_info, unsigned int ui_cmd, unsigned long ul_arg);
int nvt_hdmitx_drv_write_reg(PHDMITX_MODULE_INFO pmodule_info, unsigned long addr, unsigned long value);
int nvt_hdmitx_drv_read_reg(PHDMITX_MODULE_INFO pmodule_info, unsigned long addr);
#endif

