#
# 顶层 makefile
#

#基本目录定义
TOP_DIR			:= ${shell pwd}
SRC_DIR			:= $(TOP_DIR)/src
BUILD_DIR		:= $(TOP_DIR)/build
#OEM_DIR		:= $(TOP_DIR)/oem
TOOLS_DIR		:= $(TOP_DIR)/tools
TARGETS_DIR		:= $(TOP_DIR)/targets

#导出给所有人使用
export TOP_DIR
export SRC_DIR
export BUILD_DIR
export TOOLS_DIR
export TARGETS_DIR

#定义公文件目录(导出为全局使用),具体在build.mk中定义
TEMP_TARGET_DIR			:=
PUBLIC_INC_DIR			:=
PUBLIC_LIB_DIR  		:=
PUBLIC_STATIC_LIB_DIR 	:=
PUBLIC_APP_DIR  		:=
PUBLIC_CGI_DIR			:=
PUBLIC_SYS_DIR  		:=
PUBLIC_DB_DIR			:=

#导出给所有人使用
export TEMP_TARGET_DIR
export PUBLIC_INC_DIR
export PUBLIC_STATIC_LIB_DIR
export PUBLIC_LIB_DIR
export PUBLIC_APP_DIR
export PUBLIC_SYS_DIR
export PUBLIC_CGI_DIR
export PUBLIC_DB_DIR


#定义标准头文件目录/标准库(导出为全局使用),在具体的平台 Makefile 中定义
STD_INC_DIR     	:= 
STD_LIB_DIR     	:=
STD_USRLIB_DIR  	:=

export STD_INC_DIR
export STD_LIB_DIR
export STD_USRLIB_DIR


nothing: 
	@echo "PLATFORM:  $(PLATFORM_TYPE)"

#导入bulid下具体mk文件
include build/build.mk







