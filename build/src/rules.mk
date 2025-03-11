######################################################################
## Filename:      rules.mk
## Author:        eric@milesight.cn
## Created at:    2014-05-13
##                
## Description:   编译打印的全局配置
## Copyright (C)  milesight
##                
######################################################################

#编译参数
MS_MAKEFILE_V		:= @
MS_MAKE_PARA		:= -s
MS_MAKE_JOBS		:= -j$(shell cat /proc/cpuinfo | grep processor | wc -l)

export MS_MAKEFILE_V
export MS_MAKE_PARA
export MS_MAKE_JOBS

#定义颜色
GREEN			:= \\033[2;32m
LIGHTGREEN		:= \\033[1;32m
GREENU			:= \\033[4;32m
NOCOLOR			:= \\033[0;39m
BLUE			:= \\033[2;34m
LIGHTBLUE		:= \\033[1;34m
RED				:= \\033[0;31m
WARNING			:= \\033[0;33m

SUCCESS				:= $(LIGHTGREEN)
MS_COMPILE_PRX		:= ====milesight====> 编译:
MS_CLEAN_PRX		:= ====milesight====> 清理:
MS_CONFIG_PRX		:= ====milesight====> 配置:

COMPILE_P			:= $(NOCOLOR)$(MS_COMPILE_PRX)
COMPILE_SUCCESS_P	:= $(SUCCESS)$(MS_COMPILE_PRX)
CLEAN_P				:= $(NOCOLOR)$(MS_CLEAN_PRX)
CLEAN_SUCCESS_P		:= $(SUCCESS)$(MS_CLEAN_PRX)
CONFIG_P			:= $(LIGHTBLUE)$(MS_CONFIG_PRX)
CONFIG_SUCCESS_P	:= $(SUCCESS)$(MS_CONFIG_PRX)

