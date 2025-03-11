######################################################################
## Filename:      iperf.mk
## Author:        zbing
## Created at:    2016-01-06
##                
## Description:   编译 iperf 的全局配置
## Copyright (C)  milesight
##                
######################################################################
#源码目录
IPERF_SRC_DIR    	:= $(SRC_DIR)/test/iperf

#输出目录
IPERF_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/test/iperf

#目标名称
IPERF_TARGET_NAME	:= iperf

iperf-config:
	$(MS_MAKEFILE_V)if test ! -f $(IPERF_SRC_DIR)/Makefile; then \
		cd $(IPERF_SRC_DIR); chmod +x configure;  CC=$(TARGET_CC) ./configure --prefix=$(IPERF_TARGET_DIR) --host=$(CROSS_TYPE) --target=$(CROSS_COMPILE); fi

iperf: iperf-config
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(IPERF_TARGET_NAME) $(NOCOLOR)\n"
	-$(MS_MAKEFILE_V)mkdir  -p $(IPERF_TARGET_DIR)
	#$(MS_MAKEFILE_V)$(MAKE) -C $(IPERF_SRC_DIR) distclean
	$(MS_MAKEFILE_V)$(MAKE) $(MS_MAKE_PARA) iperf-config
	$(MS_MAKEFILE_V)$(MAKE) $(MS_MAKE_PARA) $(MS_MAKE_JOBS) -C $(IPERF_SRC_DIR) #CC=$(TARGET_CC) CXX=$(TARGET_CXX) TARGET_DIR=$(IPERF_TARGET_DIR) TARGET_NAME=$(IPERF_TARGET_NAME)
	$(MS_MAKEFILE_V)$(TARGET_STRIP) $(IPERF_SRC_DIR)/src/$(IPERF_TARGET_NAME)
	$(MS_MAKEFILE_V)cp $(IPERF_SRC_DIR)/src/$(IPERF_TARGET_NAME) $(PUBLIC_APP_DIR)/$(IPERF_TARGET_NAME)
	$(MS_MAKEFILE_V)printf  "$(COMPILE_SUCCESS_P) $(IPERF_TARGET_NAME) success $(NOCOLOR)\n"

iperf-clean:
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(IPERF_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V)$(MAKE) -C $(IPERF_SRC_DIR) distclean TARGET_DIR=$(IPERF_TARGET_DIR) TARGET_NAME=$(IPERF_TARGET_NAME)
	-$(MS_MAKEFILE_V)rm -rf $(PUBLIC_APP_DIR)/$(IPERF_TARGET_NAME)
	$(MS_MAKEFILE_V)printf  "$(CLEAN_SUCCESS_P) $(IPERF_TARGET_NAME) success $(NOCOLOR)\n"

#添加至全局LIBS/CLEANS
APPS   += #iperf
CLEANS += #iperf-clean


