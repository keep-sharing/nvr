######################################################################
## Filename:      smtp.mk
## Author:        bruce.milesight
## Created at:    2015-11-13
##                
## Description:   编译环境配置 smtp
## Copyright (C)  milesight
##                
######################################################################
#源码目录
SMTP_SRC_DIR    	:= $(SRC_DIR)/smtp
#输出目录
SMTP_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/smtp
#目标名称
SMTP_TARGET_NAME	:= smtp

smtp:
	-mkdir  -p $(SMTP_TARGET_DIR)
	$(MAKE) -C $(SMTP_SRC_DIR) CC=$(TARGET_CC) CXX=$(TARGET_CXX) TARGET_DIR=$(SMTP_TARGET_DIR) TARGET_NAME=$(SMTP_TARGET_NAME)
	cp $(SMTP_TARGET_DIR)/$(SMTP_TARGET_NAME) $(PUBLIC_APP_DIR)/$(SMTP_TARGET_NAME)
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(SMTP_TARGET_NAME)

smtp-clean:
	$(MAKE) -C $(SMTP_SRC_DIR) clean TARGET_DIR=$(SMTP_TARGET_DIR) TARGET_NAME=$(SMTP_TARGET_NAME)
	-rm $(PUBLIC_APP_DIR)/$(SMTP_TARGET_NAME)

#添加至全局LIBS/CLEANS
APPS   += smtp
CLEANS += smtp-clean


