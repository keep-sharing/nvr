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
GUI_SRC_DIR    	:= $(SRC_DIR)/msgui
#输出目录
GUI_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/gui
#目标名称
GUI_NAME	:= mscore
GUI_RCC_NAME    := gui.rcc

mscore:
	@find $(GUI_SRC_DIR)/mscore/*.c | xargs touch

gui:
	-mkdir  -p $(GUI_TARGET_DIR)
	$(MAKE) -fMakefile.MK -C $(GUI_SRC_DIR) GUI_TARGET_DIR=$(GUI_TARGET_DIR) QT_PATH=$(QT_PATH) GUI_NAME=$(GUI_NAME)
	cp $(GUI_TARGET_DIR)/$(GUI_NAME) $(PUBLIC_APP_DIR)/
	cp $(GUI_SRC_DIR)/$(GUI_RCC_NAME) $(PUBLIC_APP_DIR)/
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(GUI_NAME)
	#cp -rf $(GUI_SRC_DIR)/Lang $(PUBLIC_APP_DIR)/
gui-clean:
	$(MAKE) -fMakefile.MK -C $(GUI_SRC_DIR) clean GUI_TARGET_DIR=$(GUI_TARGET_DIR)
	rm -rf $(GUI_TARGET_DIR)
	rm -rf $(PUBLIC_APP_DIR)/$(GUI_NAME)
	rm -rf $(GUI_SRC_DIR)/$(GUI_RCC_NAME) $(PUBLIC_APP_DIR)/$(GUI_RCC_NAME)
	#rm -rf $(PUBLIC_APP_DIR)/Lang
#把gui加到apps
APPS   += gui
#添加本地clean操作到全局clean中
CLEANS += gui-clean
