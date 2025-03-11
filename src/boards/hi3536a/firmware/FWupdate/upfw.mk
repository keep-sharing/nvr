##
## amboot/config/Makefile.upfw
## Author:        zbing@milesight.cn
## Created at:    2015-05-20
##                
## Description:   ±‡“Î firmware Makefileµƒ»´æ÷≈‰÷√
##                1.image package
##                2.image update
##                3.update tools
## Copyright (C)  milesight
##

ifndef DATE
DATE 			:= $(shell date +%Y%m%d)
endif

YEAR 			:= $(shell date +%Y | sed s/^0//)
MONTH 			:= $(shell date +%m | sed s/^0//)
DAY 			:= $(shell date +%d | sed s/^0//)

## Define 
UPFW_SRC_DIR 		:= $(FW_TOPDIR)/FWupdate
UBOOT_DIR 		= $(FW_TOPDIR)/../hiboot/u-boot-2020.01
UPFW_INPUT_FILE		= $(UPFW_SRC_DIR)/baseinfo.h
UPFW_VERSION_FILE	= $(FW_TOPDIR)/../include/hiboot_version.h
UPFW_UPFW_IMG		= $(FW_TOPDIR)/update_IMG
UPFW_UPFW_ONLINE_IMG	= $(FW_TOPDIR)/update_ONLINE_IMG
UPFW_BUILD_DIR		= $(FW_TOPDIR)/build

TARGET_CC 	  	= $(CROSS)gcc
TARGET_STRIP  		= $(CROSS)strip

ifeq ($(CONFIG_UPFW_HIBOOT),y)
	UPFW_BLD_IMG    := $(FIRMWARE_IMG_DIR)/uboot_$(DDR_SIZE_2G)
	UPFW_BLD2_IMG 	:= $(FIRMWARE_IMG_DIR)/uboot_$(DDR_SIZE_4G)
else
	UPFW_BLD_IMG    :=
endif

ifeq ($(CONFIG_UPFW_KERNEL),y)
	UPFW_KER_IMG	:= $(FIRMWARE_IMG_DIR)/uImage_$(DDR_SIZE_2G)
	UPFW_KER2_IMG	:= $(FIRMWARE_IMG_DIR)/uImage_$(DDR_SIZE_4G)
else
	UPFW_KER_IMG    :=
endif

ifeq ($(CONFIG_UPFW_FS),y)
	UPFW_FS_IMG     := $(FIRMWARE_IMG_DIR)/ext4fs
else
	UPFW_FS_IMG     :=
endif

ifeq ($(CONFIG_UPFW_CMD),y)
	UPFW_CMD_IMG    := $(FW_TOPDIR)/update
else
	UPFW_CMD_IMG    :=
endif

ifeq ($(CONFIG_UPFW_TOOL),y)
	UPFW_TOOL_IMG   := $(FIRMWARE_IMG_DIR)/tool
else
	UPFW_TOOL_IMG   :=
endif

UPFW_JPG_IMG    := $(FIRMWARE_IMG_DIR)/boot.jpg

FORCE:

#---------------------packer:host tool--------------------------------------------------------------
HOSTCC 			= gcc
CFLGS_VER		= -DVER_CPU=$(VERSION_CPU) -DVER_FILESYS=$(VERSION_FS) \
				-DVER_FLASH=$(VERSION_FLASH) -DVER_DDR3_2G=$(DDR3_VER_2G) \
				-DVER_DDR3_4G=$(DDR3_VER_4G) -DVER_LINUX=$(VERSION_KER) \
				-DVER_FS_MAIN=$(IPC_VER0) -DVER_FS_SUB=$(IPC_VER1) \
				-DVER_FS_THIRD=$(IPC_VER2)

CFLGS_FILE		= -DOUTPUT_UPFW_IMG=$(UPFW_UPFW_IMG) \
			  	-DOUTPUT_UPFW_ONLINE_IMG=$(UPFW_UPFW_ONLINE_IMG) \
				-DINPUT_BLD_IMG=$(UPFW_BLD_IMG) \
				-DINPUT_BLD2_IMG=$(UPFW_BLD2_IMG)\
				-DINPUT_JPG_IMG=$(UPFW_JPG_IMG) \
				-DINPUT_KER_IMG=$(UPFW_KER_IMG) \
				-DINPUT_KER2_IMG=$(UPFW_KER2_IMG) \
				-DINPUT_FS_IMG=$(UPFW_FS_IMG) \
				-DINPUT_CMD_IMG=$(UPFW_CMD_IMG) \
				-DINPUT_TOOL_IMG=$(UPFW_TOOL_IMG)
				
HOST_CFLAGS   	= -Wall -DUSE_HOSTCC \
				-I$(UPFW_SRC_DIR) -I$(FW_TOPDIR)/../include \
				-I. -I$(PUBLIC_INC_DIR) \
				-I$(ENV_SRC_DIR) \
				$(CFLGS_VER) $(CFLGS_FILE) \
				-D__YEAR__=$(YEAR) \
				-D__MONTH__=$(MONTH) \
				-D__DAY__=$(DAY)

HOST_LAFLAGS = 
HOST_TARGET = $(UPFW_SRC_DIR)/packer

HOST_SRCS = $(UPFW_SRC_DIR)/md5.c \
	    $(UPFW_SRC_DIR)/crc32.c \
	    $(UPFW_SRC_DIR)/host_upfw.c \
	    $(UPFW_SRC_DIR)/updateFW.c


HOST_OBJ = $(patsubst %.c,%_host.o,$(HOST_SRCS))

%_host.o: FORCE
	$(HOSTCC) $(HOST_CFLAGS) -c $(patsubst %_host.o,%.c,$@) -o $@ $(HOST_LAFLAGS)
	

packer: $(HOST_OBJ)
	$(HOSTCC) $(HOST_CFLAGS) $^ -o $(HOST_TARGET) $(HOST_LAFLAGS)

packer-clean:
	$(MS_MAKEFILE_V)rm -rf $(HOST_OBJ)
	$(MS_MAKEFILE_V)rm -rf $(HOST_TARGET)

#---------------------update:arm tool--------------------------------------------------------------
ARM_CFLAGS		= -Wall $(PLATFORM_FLAGS) -D_XOPEN_SOURCE=500 -D__YEAR__=$(YEAR) \
				-D__MONTH__=$(MONTH) -D__DAY__=$(DAY) $(CFLGS_VER) $(CFLGS_FILE) \
				-Wall -DBUILD_NANDWRITE \
				-I$(UPFW_SRC_DIR) -I$(FW_TOPDIR)/../include \
				-I$(PUBLIC_INC_DIR)/vapi/$(PLATFORM_TYPE) \
				-I. -I$(PUBLIC_INC_DIR) \
				-I$(ENV_SRC_DIR)/

ARM_LDFLAGS = -L$(PUBLIC_STATIC_LIB_DIR) -larm-env
ARM_TARGET  = $(FW_TOPDIR)/update

ARM_SRCS = $(UPFW_SRC_DIR)/md5.c \
	   $(UPFW_SRC_DIR)/crc32.c \
	   $(UPFW_SRC_DIR)/updateFW.c \
	   $(UPFW_SRC_DIR)/nandwrite.c \
	   $(UPFW_SRC_DIR)/update.c

ARM_OBJ = $(patsubst %.c,%_arm.o,$(ARM_SRCS))

%_arm.o: FORCE
	$(TARGET_CC) $(ARM_CFLAGS) -c $(patsubst %_arm.o,%.c,$@) -o $@ $(ARM_LDFLAGS)

update: $(ARM_OBJ)
	$(TARGET_CC) $(ARM_CFLAGS) $^ -o $(ARM_TARGET) $(ARM_LDFLAGS)
	$(TARGET_STRIP) $(ARM_TARGET)
	$(MS_MAKEFILE_V)cp -dpRf $(ARM_TARGET) $(PUBLIC_APP_DIR)

update-clean:
	-$(MS_MAKEFILE_V)rm -rf $(ARM_OBJ)
	-$(MS_MAKEFILE_V)rm -rf $(ARM_TARGET)
	-$(MS_MAKEFILE_V)rm -rf $(PUBLIC_APP_DIR)/update

#---------------------pack image--------------------------------------------------------------	
mkimage:
	$(HOST_TARGET) -c
	$(HOST_TARGET) -v

mkimage-clean:
	-$(MS_MAKEFILE_V)rm -rf $(UPFW_UPFW_IMG) $(UPFW_UPFW_ONLINE_IMG)
