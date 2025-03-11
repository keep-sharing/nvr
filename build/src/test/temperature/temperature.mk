######################################################################
## Filename:      temperature.mk
## Author:        huangwx
## Created at:    2016-03-18
##                
## Description:   编译 temperature 的全局配置
## Copyright (C)  milesight
##                
######################################################################
#源码目录
GB_SRC_DIR      := $(SRC_DIR)/test/temperature
#输出目录
GB_TARGET_DIR   := $(TEMP_TARGET_DIR)/test/temperature
#目标名称
GB_TARGET_NAME  := temperature

GB_COMPILE_OPT  := $(MS_MAKE_JOBS) -C $(GB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(GB_TARGET_DIR) TARGET_NAME=$(GB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

temperature:
	$(MS_MAKEFILE_V)$(MAKE) $(GB_COMPILE_OPT) all   
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(GB_TARGET_NAME)
     
temperature-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(GB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += temperature
CLEANS += temperature-clean

