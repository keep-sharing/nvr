######################################################################
## Filename:      profile.mk
## Author:        bruce.milesight
## Created at:    2015-11-17
##                
## Description:   编译中心程序 profile
## Copyright (C)  milesight
##                
######################################################################
#源码目录
PROFILE_SRC_DIR    	:= $(SRC_DIR)/profile
#输出目录
PROFILE_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/profile
#目标名称
PROFILE_TARGET_NAME	:= msprofile

PROFILE_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(PROFILE_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(PROFILE_TARGET_DIR) TARGET_NAME=$(PROFILE_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

profile:
	$(MS_MAKEFILE_V)$(MAKE) $(PROFILE_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(PROFILE_TARGET_NAME)
	
profile-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(PROFILE_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += profile
CLEANS += profile-clean


