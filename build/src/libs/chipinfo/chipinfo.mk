######################################################################
## Filename:      chipinfo.mk
## Author:        young
## Created at:    2017-11-15
##                
## Description:   编译 chipinfo 的全局配置
## Copyright (C)  milesight
##                
######################################################################
#源码目录
CHIPINFO_SRC_DIR    	:= $(SRC_DIR)/libs/chipinfo/$(PLATFORM_TYPE)

#输出目录
CHIPINFO_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/libs/chipinfo

#目标名称
CHIPINFO_TARGET_NAME	:= libchipinfo.so

CHIPINFO_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(CHIPINFO_SRC_DIR) CC=$(TARGET_CC) TARGET_DIR=$(CHIPINFO_TARGET_DIR) TARGET_NAME=$(CHIPINFO_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

libchipinfo:
	$(MS_MAKEFILE_V)$(MAKE) $(CHIPINFO_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(CHIPINFO_TARGET_NAME)
	
libchipinfo-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(CHIPINFO_COMPILE_OPT) clean
	
#添加至全局LIBS/CLEANS
LIBS   += libchipinfo
CLEANS += libchipinfo-clean


