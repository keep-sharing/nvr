######################################################################
## Filename:      nginx.mk
## Author:        david.milesight
## Created at:    2019-02-02
##                
## Description:   编译环境配置 nginx
## Copyright (C)  milesight
##                
######################################################################
#源码目录
SMTP_SRC_DIR    	:= $(TOOLS_DIR)/third-party/nginx/nginx-1.6.2
#输出目录
SMTP_TARGET_DIR 	:= $(SMTP_SRC_DIR)/temp/sbin
#目标名称
SMTP_TARGET_NAME	:= nginx

nginx:
	@echo "====milesight====> 编译: $(SMTP_TARGET_NAME)"
	-mkdir  -p $(SMTP_TARGET_DIR)
	$(MAKE) -C $(SMTP_SRC_DIR) CC=$(TARGET_CC) CXX=$(TARGET_CXX) TARGET_DIR=$(SMTP_TARGET_DIR) TARGET_NAME=$(SMTP_TARGET_NAME)
	$(TARGET_STRIP) $(SMTP_TARGET_DIR)/$(SMTP_TARGET_NAME)
	cp $(SMTP_TARGET_DIR)/$(SMTP_TARGET_NAME) $(PUBLIC_APP_DIR)/$(SMTP_TARGET_NAME)
nginx-clean:
	@echo "====milesight====> 清理: $(SMTP_TARGET_NAME)"
	$(MAKE) -C $(SMTP_SRC_DIR) clean TARGET_DIR=$(SMTP_TARGET_DIR) TARGET_NAME=$(SMTP_TARGET_NAME)
	-rm $(PUBLIC_APP_DIR)/$(SMTP_TARGET_NAME)

#添加至全局LIBS/CLEANS
APPS   += nginx
CLEANS += nginx-clean


