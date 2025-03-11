######################################################################
## Filename:      arch.mk
## Author:        eric
## Created at:    2015.6.18
##                
## Description:   切换平台mk文件.
##                
## Copyright (C)  milesight
##                
######################################################################

PLATFORM_CONFIG	:= $(BUILD_DIR)/platform/platform.mk

#修改编译平台
$(PLATFORM_ALL):
	@echo 'Change to platform $@'
	@sed -i -e "s,PLATFORM_TYPE 	=.*,PLATFORM_TYPE 	= $@," $(PLATFORM_CONFIG); 

	
