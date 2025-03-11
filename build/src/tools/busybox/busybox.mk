######################################################################
## Filename:      busybox.mk
## Author:        eric@milesight.cn
## Created at:    2014-04-29
##                
## Description:   编译 busybox 的全局配置
##                1.定义源码目录
##                2.定义输出目录
##                3.定义目标名称
## Copyright (C)  milesight
##                
######################################################################

#源码目录
BUSYBOX_SRC_DIR    		:= $(TOOLS_DIR)/$(PLATFORM_NAME)/makfs

#目标名称
BUSYBOX_TARGET_NAME		:= busybox


.PHONY: busybox busybox-clean busybox-distclean

busybox:
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(BUSYBOX_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V)$(MAKE) $(MS_MAKE_PARA) -C $(BUSYBOX_SRC_DIR) busybox
	$(MS_MAKEFILE_V)printf  "$(COMPILE_SUCCESS_P) $(BUSYBOX_TARGET_NAME) success $(NOCOLOR)\n"

busybox-clean:
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(BUSYBOX_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V)$(MAKE) $(MS_MAKE_PARA) -C $(BUSYBOX_SRC_DIR) busybox-clean
	$(MS_MAKEFILE_V)printf  "$(CLEAN_SUCCESS_P) $(BUSYBOX_TARGET_NAME) success $(NOCOLOR)\n"
		

busybox-menuconfig:
	$(MS_MAKEFILE_V)printf "$(CONFIG_P) $(BUSYBOX_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V)$(MAKE) $(MS_MAKE_PARA) -C $(BUSYBOX_SRC_DIR) busybox_menuconfig
	$(MS_MAKEFILE_V)printf "$(CONFIG_SUCCESS_P) $(BUSYBOX_TARGET_NAME) success $(NOCOLOR)\n"
	
busybox-distclean:
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(BUSYBOX_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V)$(MAKE) $(MS_MAKE_PARA) -C $(BUSYBOX_SRC_DIR) busybox-distclean
	$(MS_MAKEFILE_V)printf  "$(CLEAN_SUCCESS_P) $(BUSYBOX_TARGET_NAME) success $(NOCOLOR)\n"
	
#添加至全局LIBS/CLEANS
SYS   		+= 
SYSCLEANS	+= 
DISTCLEANS	+= 
#DISTCLEANS	+= busybox-distclean


