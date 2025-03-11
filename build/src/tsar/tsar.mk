######################################################################
## Filename:      tsar.mk
## Author:        hrz.milesight
## Created at:    2022-04-16
##                
## Description:   编译环境配置 tsar
## Copyright (C)  milesight
##                
######################################################################
#源码目录
TSAR_SRC_DIR    	:= $(SRC_DIR)/tsar
#输出目录
TSAR_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/tsar
#目标名称
TSAR_TARGET_NAME	:= tsar

TSAR_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(TSAR_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(TSAR_TARGET_DIR) TARGET_NAME=$(TSAR_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

tsar: tsar-clean
	$(MS_MAKEFILE_V)$(MAKE) $(TSAR_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(TSAR_TARGET_NAME)
	
tsar-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(TSAR_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += tsar
CLEANS += tsar-clean


