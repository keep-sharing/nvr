######################################################################
## Filename:      common.mk
## Author:        bruce
## Created at:    2014-04-29
## Modify at:     2015-06-25
##                
## Description:   通用mk文件.
##                
## Copyright (C)  milesight
##                
######################################################################

include $(BUILD_RULES_MK)

SRC_DIRS = $(shell find ./ -maxdepth 3 -type d)
INCLUDE += $(addprefix -I,$(SRC_DIRS))

#所有的源文件
ALL_SRC_FILES   := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
ALL_CPP_FILES   := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.cpp))

#删除不需要编译的文件
ifneq ($(DELETE_FILE),)
ALL_SRC_FILES	:= $(filter-out $(DELETE_FILE),$(ALL_SRC_FILES))
ALL_CPP_FILES	:= $(filter-out $(DELETE_FILE),$(ALL_CPP_FILES))
endif

#所有的目标文件
ALL_TARGET_OBJS	:= $(patsubst %.c,$(TARGET_DIR)/%.o,$(ALL_SRC_FILES))
ALL_TARGET_OBJS	+= $(patsubst %.cpp,$(TARGET_DIR)/%.o,$(ALL_CPP_FILES))

#依赖文件
DEPEND_FILE		:= $(TARGET_DIR)/MAKEFILE.DEPEND

#避免进入无效的编译目录
TARGET_DIR		?= xxxooo

#避免TARGET_NAME为空时,导致误删除
TARGET_NAME		?= $(shell basename $(TARGET_DIR))

#避免没有定义编译类型
TARGET_TYPE		?= exe

#确认是否需要安装拷贝到其他目录
INSTALL_DIR     ?= 
MK_INSTALL_DIR	:= $(INSTALL_DIR)

# Add debug switch
STRIP_FLAG		:= n
CCACHE			:=ccache 
# Add common build cflags
ifeq ($(STRIP_FLAG),y)
CFLAGS			+= -Wall $(PLATFORM_FLAGS) $(STD_LIB_FLAGS)
else
CFLAGS			+= -g -Wall $(PLATFORM_FLAGS) $(STD_LIB_FLAGS)
endif


#确定编译目标类型
ifeq ($(TARGET_TYPE),"so")
ifeq ($(STRIP_FLAG),y)
CFLAGS			+= -s
endif
CFLAGS			+= -fPIC #64位库编译
MK_DYNAMIC		:= $(TARGET_DIR)/$(TARGET_NAME)
MK_TARGET  		:= $(MK_DYNAMIC)
endif

ifeq ($(TARGET_TYPE),"exe")
ifeq ($(STRIP_FLAG),y)
CFLAGS			+= -s 
endif
MK_EXE			:= $(TARGET_DIR)/$(TARGET_NAME)
MK_TARGET  		:= $(MK_EXE)
endif

ifeq ($(TARGET_TYPE),"a")
MK_STATIC		:= $(TARGET_DIR)/$(TARGET_NAME)
MK_TARGET		:= $(MK_STATIC)
endif


#定义默认编译的目标
all: 
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(TARGET_NAME) $(NOCOLOR)\n"
	-$(MS_MAKEFILE_V)mkdir -p $(TARGET_DIR)
	$(MS_MAKEFILE_V)$(MAKE) $(MK_TARGET)
	$(MS_MAKEFILE_V)$(MAKE) install
	$(MS_MAKEFILE_V)printf  "$(COMPILE_SUCCESS_P) $(TARGET_NAME) success $(NOCOLOR)\n"

#通用install规则
install:
ifneq ("$(MK_INSTALL_DIR)","")
	cp -dpRf $(MK_TARGET) $(MK_INSTALL_DIR)/$(TARGET_NAME)
endif	
	
#生成依赖文件
depend:
	@set -e; \
#	@echo "Making $@ ..."; \
	$(CCACHE)$(CC) $(INCLUDE) -E -MM $(ALL_SRC_FILES) $(ALL_CPP_FILES) | sed 's,\(.*\)\.o[ :]*,$(TARGET_DIR)/\1.o $@:,g'>$(DEPEND_FILE); 

#执行clean操作
clean:
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(TARGET_NAME) $(NOCOLOR)\n"
	-rm -rf $(TARGET_DIR)/*
ifneq ("$(MK_INSTALL_DIR)","")
	-rm -rf $(MK_INSTALL_DIR)/$(TARGET_NAME)
endif
#	-rm -rf $(DEPEND_FILE)
	$(MS_MAKEFILE_V)printf  "$(CLEAN_SUCCESS_P) $(TARGET_NAME) success $(NOCOLOR)\n"
	
#包含依赖文件
ifneq ($(MAKECMDGOALS), clean)
#include $(DEPEND_FILE)#bruce.milesight delete for make warning.
endif

#动态库生成规则
$(MK_DYNAMIC): $(ALL_TARGET_OBJS)
	$(CCACHE)$(CC) $(CFLAGS) -shared -o $@ $^ $(LDFLAGS)

#静态库生成规则
$(MK_STATIC): $(ALL_TARGET_OBJS)
	$(CCACHE)$(AR) rcs -o $@ $^

#exe生成规则
$(MK_EXE): $(ALL_TARGET_OBJS)
ifeq ($(strip $(ALL_CPP_FILES)), )
	$(CCACHE)$(CC) $(INCLUDE) $(CFLAGS) -o $@ $^ $(LDFLAGS)
else
	$(CCACHE)$(CXX) $(INCLUDE) $(CFLAGS) -o $@ $^ $(LDFLAGS)
endif
	
#.o文件生成规则
$(TARGET_DIR)/%.o:%.c
	$(CCACHE)$(CC) $(INCLUDE) $(CFLAGS) -o $@ -c $< 

$(TARGET_DIR)/%.o:%.cpp
	$(CCACHE)$(CXX) $(INCLUDE) $(CFLAGS) -o $@ -c $< 


