######################################################################
## Filename:      firmware.mk
## Author:        zbing@milesight.cn
## Created at:    2015-05-20
##                
## Description:   编译 firmware 的全局配置
##                1.定义源码目录
##                2.定义输出目录
##                3.定义目标名称
## Copyright (C)  milesight
######################################################################

#源码目录
FW_SRC_DIR    		:= $(SRC_DIR)/boards/$(PLATFORM_TYPE)/firmware

#目标名称
FW_TARGET_NAME		:= firmware
FWTOOLS_TARGET_NAME		:= fmtools

FW_FLAGS 			:=  $(MS_MAKE_OBJS) -C $(FW_SRC_DIR) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE)

.PHONY: firmware firmware-clean fmtools fmtools-clean

firmware:
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(FW_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V)$(MAKE) -s $(FW_FLAGS) install 
	$(MS_MAKEFILE_V)printf "$(COMPILE_SUCCESS_P) $(FW_TARGET_NAME) success $(NOCOLOR)\n"

firmware-clean:
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(FW_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V)$(MAKE) -s $(FW_FLAGS) uninstall
	$(MS_MAKEFILE_V)printf  "$(CLEAN_SUCCESS_P) $(FW_TARGET_NAME) success $(NOCOLOR)\n"

fmtools:
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(FWTOOLS_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V)$(MAKE) $(FW_FLAGS) tools 
	$(MS_MAKEFILE_V)printf "$(COMPILE_SUCCESS_P) $(FWTOOLS_TARGET_NAME) success $(NOCOLOR)\n"

fmtools-clean:
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(FWTOOLS_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V)$(MAKE) $(FW_FLAGS) tools-clean
	$(MS_MAKEFILE_V)printf  "$(CLEAN_SUCCESS_P) $(FWTOOLS_TARGET_NAME) success $(NOCOLOR)\n"

hiboot-config: 
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(HIBOOT_TARGET_NAME) $(NOCOLOR)\n"
	$(MAKE) $(FW_FLAGS) hiboot_config
	$(MS_MAKEFILE_V)printf "$(COMPILE_SUCCESS_P) $(HIBOOT_TARGET_NAME) success $(NOCOLOR)\n"	
hiboot:
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(HIBOOT_TARGET_NAME) $(NOCOLOR)\n"
	$(MAKE) $(FW_FLAGS) hiboot
	$(MS_MAKEFILE_V)printf "$(COMPILE_SUCCESS_P) $(HIBOOT_TARGET_NAME) success $(NOCOLOR)\n"

hiboot-clean:
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(HIBOOT_TARGET_NAME) $(NOCOLOR)\n"
	$(MAKE)  $(FW_FLAGS) hiboot-clean
	$(MS_MAKEFILE_V)printf "$(CLEAN_SUCCESS_P) $(HIBOOT_TARGET_NAME) success $(NOCOLOR)\n"
		
#添加至全局LIBS/CLEANS
SYS		+= fmtools firmware 
SYSCLEANS	+= fmtools-clean firmware-clean
DISTCLEANS	+= fmtools-clean firmware-clean hiboot-clean
