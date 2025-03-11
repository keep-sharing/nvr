######################################################################
## Filename:      gpio.mk
## Author:        zbing
## Created at:    2015-12-24
##                
## Description:   编译gpio lib
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
GPIO_SRC_DIR		:= $(SRC_DIR)/libs/gpio/$(PLATFORM_TYPE)
#输出目录
GPIO_TARGET_DIR		:= $(TEMP_TARGET_DIR)/libs/gpio
#目标名称
GPIO_TARGET_NAME	:= libgpio.so

GPIO_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(GPIO_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(GPIO_TARGET_DIR) TARGET_NAME=$(GPIO_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

gpio:
	$(MS_MAKEFILE_V)$(MAKE) $(GPIO_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(GPIO_TARGET_NAME)
	
gpio-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(GPIO_COMPILE_OPT) clean
	
#添加至全局LIBS/CLEANS
LIBS   += gpio
CLEANS += gpio-clean


