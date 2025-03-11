######################################################################
## Filename:      mshc.mk
## Author:        eric@milesight.cn
## Created at:    2014-04-29
##                
## Description:   编译 amballera 的全局配置
##                1.定义源码目录
##                2.定义输出目录
##                3.定义目标名称
## Copyright (C)  milesight
##                
######################################################################

#编译路径
CONFIG_BUILD_DIR		= $(shell pwd)

#编译参数
MS_MAKEFILE_V			= @
MS_CONFIG_CPU_A5S		= $(shell grep 'CONFIG_CPU_A5S' $(CONFIG_BUILD_DIR)/ms_config | sed s/^CONFIG_CPU_A5S=//)
MS_CONFIG_BSP_MEM_128M		= $(shell grep 'CONFIG_BSP_MEM_128M' $(CONFIG_BUILD_DIR)/ms_config | sed s/^CONFIG_BSP_MEM_128M=//)
MS_CONFIG_BSP_MEM_256M		= $(shell grep 'CONFIG_BSP_MEM_256M' $(CONFIG_BUILD_DIR)/ms_config | sed s/^CONFIG_BSP_MEM_256M=//)

MS_CONFIG_BSP_MEM_DDR2		= $(shell grep 'CONFIG_DDR2' $(CONFIG_BUILD_DIR)/ms_config | sed s/^CONFIG_DDR2=//)
MS_CONFIG_BSP_MEM_DDR3		= $(shell grep 'CONFIG_DDR3' $(CONFIG_BUILD_DIR)/ms_config | sed s/^CONFIG_DDR3=//)




BD_CONFIG_CPU_A5S		= $(shell grep 'CONFIG_CPU_A5S' $(CONFIG_BUILD_DIR)/bd_config | sed s/^CONFIG_CPU_A5S=//)
BD_CONFIG_BSP_MEM_128M		= $(shell grep 'CONFIG_BSP_MEM_128M' $(CONFIG_BUILD_DIR)/bd_config | sed s/^CONFIG_BSP_MEM_128M=//)
BD_CONFIG_BSP_MEM_256M		= $(shell grep 'CONFIG_BSP_MEM_256M' $(CONFIG_BUILD_DIR)/bd_config | sed s/^CONFIG_BSP_MEM_256M=//)



.PHONY: mscfg getbdcfg bdcfg cp_bdcfg cp_mscfg

mscfg:
	-$(MS_MAKEFILE_V)rm -rf .config*
	$(MS_MAKEFILE_V)cp -f  $(CONFIG_BUILD_DIR)/ms_config $(CONFIG_BUILD_DIR)/.config 
	$(MS_MAKEFILE_V)if ! $(CONFIG_BUILD_DIR)/mconf $(CONFIG_BUILD_DIR)/msconfig.in; then \
		test -f .config.cmd || rm -f .config; \
	fi
	$(MS_MAKEFILE_V)cp -f $(CONFIG_BUILD_DIR)/.config  $(CONFIG_BUILD_DIR)/ms_config
	-$(MS_MAKEFILE_V)rm -rf .config* config*
	
cp_mscfg:
	#@echo "" #nothing to do for hisi
	
bdcfg:
	@echo "nothing to do for hisi" #nothing to do for hisi
	
	
cp_bdcfg:
	
	
