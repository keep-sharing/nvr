######################################################################
## Filename:      kconf.mk
## Author:        eric@milesight.cn
## Created at:    2014-04-29
##                
## Description:   编译 amboot 的全局配置
##                1.定义源码目录
##                2.定义输出目录
##                3.定义目标名称
## Copyright (C)  milesight
##                
######################################################################

#源码目录
KCONF_SRC_DIR    		:= $(TOOLS_DIR)/kconfig

#目标名称
KCONF_TARGET_NAME		:= kconf

.PHONY: kconf kconf-clean ms_config bd_config

kconf:
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(KCONF_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V)$(MAKE) $(MS_MAKE_PARA) -C $(KCONF_SRC_DIR)/ clean
	$(MS_MAKEFILE_V)$(MAKE) $(MS_MAKE_PARA) -C $(KCONF_SRC_DIR)/ all
	$(MS_MAKEFILE_V)cp $(KCONF_SRC_DIR)/mconf $(CONFIG_DIR)/
	$(MS_MAKEFILE_V)$(MAKE) $(MS_MAKE_PARA) -C $(KCONF_SRC_DIR)/ clean
	$(MS_MAKEFILE_V)printf  "$(COMPILE_SUCCESS_P) $(KCONF_TARGET_NAME) success $(NOCOLOR)\n"

kconf-clean:
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(KCONF_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V)$(MAKE) $(MS_MAKE_PARA) -C $(KCONF_SRC_DIR)/ clean
	-$(MS_MAKEFILE_V)rm -f $(CONFIG_DIR)/mconf
	$(MS_MAKEFILE_V)printf  "$(CLEAN_SUCCESS_P) $(KCONF_TARGET_NAME) success $(NOCOLOR)\n"
	
ms_config: 
	$(MS_MAKEFILE_V)[ -x $(CONFIG_DIR)/mconf ] || $(MAKE) kconf
	$(MS_MAKEFILE_V)printf "$(CONFIG_P) $@ $(NOCOLOR)\n"
	$(MS_MAKEFILE_V)$(MAKE) $(MS_MAKE_PARA) -C $(CONFIG_DIR)/ mscfg
	$(MS_MAKEFILE_V)$(MAKE) $(MS_MAKE_PARA) -C $(CONFIG_DIR)/ cp_mscfg
	$(MS_MAKEFILE_V)printf "$(CONFIG_SUCCESS_P) $@ success $(NOCOLOR)\n"

	
bd_config:
	$(MS_MAKEFILE_V)[ -x $(CONFIG_DIR)/mconf ] || $(MAKE) kconf
	$(MS_MAKEFILE_V)printf "$(CONFIG_P) $@ $(NOCOLOR)\n"
	$(MS_MAKEFILE_V)$(MAKE) $(MS_MAKE_PARA) -C $(CONFIG_DIR)/ bdcfg
	$(MS_MAKEFILE_V)$(MAKE) $(MS_MAKE_PARA) -C $(CONFIG_DIR)/ cp_bdcfg
	$(MS_MAKEFILE_V)printf "$(CONFIG_SUCCESS_P) $@ success $(NOCOLOR)\n"

		
#添加至全局LIBS/CLEANS
SYS   		+= 
SYSCLEANS	+= 
DISTCLEANS	+= kconf-clean


