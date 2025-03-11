######################################################################
## Filename:      mail.mk
## Author:        bruce.milesight
## Created at:    2015-11-13
##                
## Description:   编译环境配置 mail
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MAIL_SRC_DIR    	:= $(SRC_DIR)/msmail
#输出目录
MAIL_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/msmail
#目标名称
MAIL_TARGET_NAME	:= msmail

MAIL_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(MAIL_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(MAIL_TARGET_DIR) TARGET_NAME=$(MAIL_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

mail:
	$(MS_MAKEFILE_V)$(MAKE) $(MAIL_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(MAIL_TARGET_NAME)
	
mail-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(MAIL_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += mail
CLEANS += mail-clean


