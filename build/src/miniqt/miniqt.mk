######################################################################
## Filename:      gui.mk
## Author:        bruce
## Created at:    2015-12-11
##                
## Description:   编译 gui
## Copyright (C)  milesight
##                
######################################################################

#源码目录
MINIQT_SRC_DIR    	:= $(SRC_DIR)/miniqt
#输出目录
MINIQT_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/miniqt
#目标名称
MINIQT_NAME	:= miniqt

miniqt:
	-mkdir  -p $(MINIQT_TARGET_DIR)
	$(MAKE) -fMakefile.MK -C $(MINIQT_SRC_DIR) MINIQT_TARGET_DIR=$(MINIQT_TARGET_DIR) QT_PATH=$(QT_PATH) MINIQT_NAME=$(MINIQT_NAME)
	cp $(MINIQT_TARGET_DIR)/$(MINIQT_NAME) $(PUBLIC_APP_DIR)/
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(MINIQT_NAME)

miniqt-clean:
	$(MAKE) -fMakefile.MK -C $(MINIQT_SRC_DIR) clean MINIQT_TARGET_DIR=$(MINIQT_TARGET_DIR)
	rm -rf $(MINIQT_TARGET_DIR)
	rm -rf $(PUBLIC_APP_DIR)/$(MINIQT_NAME)

