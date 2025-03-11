#
#
#
#
#
PLATFORM_FLAGS		:= -march=armv7-a -mcpu=cortex-a9 -mfloat-abi=softfp -mfpu=vfpv3-d16 -ffunction-sections -fdata-sections -Wl,--gc-sections -g -rdynamic -funwind-tables
STD_LIB_FLAGS		:= -D_UCLIBC_  -DUSE_AEC -D_GNU_SOURCE -D_XOPEN_SOURCE=600 -D_HI3798_ -g -rdynamic
export PLATFORM_FLAGS
export STD_LIB_FLAGS

QT_PATH 		:= $(TOOLS_DIR)/hi3798/QTLibs/
CHIP_MODEL 	= hi3798
CROSS_TYPE 	= arm-histbv310-linux
export CHIP_MODEL
export CROSS_TYPE

LINUX_TOOLCHAIN_PREFIX		=/opt/hisi-linux/x86-arm

ARM_LINUX_TOOCHAIN_DIR		=$(LINUX_TOOLCHAIN_PREFIX)/$(CROSS_TYPE)
LINUX_TOOLCHAIN_NAME		=$(CROSS_TYPE)
LINUX_TOOLCHAIN_VERSION		=4.9.2

#ELF_TOOLCHAIN_PREFIX		=$(LINUX_TOOLCHAIN_PREFIX)
#ARM_ELF_TOOLCHAIN_DIR		=$(ELF_TOOLCHAIN_PREFIX)/arm-elf
#ARM_ELF_CROSS_COMPILE		=arm-elf-
#ARM_ELF_VERSION		=3.4.4

# define common path of toolchain 
TOOLCHAIN_PATH			=$(ARM_LINUX_TOOCHAIN_DIR)/bin
SYS_LIB_DIR			=$(ARM_LINUX_TOOCHAIN_DIR)/$(LINUX_TOOLCHAIN_NAME)/libc/lib
PREBUILD_DIR			=$(LINUX_TOOLCHAIN_NAME)
CPP_INCLUDE_DIR			=$(ARM_LINUX_TOOCHAIN_DIR)/$(LINUX_TOOLCHAIN_NAME)/include/c++/$(LINUX_TOOLCHAIN_VERSION)
CPP_INCLUDE_DIR_TOOLCHAIN	=$(ARM_LINUX_TOOCHAIN_DIR)/$(LINUX_TOOLCHAIN_NAME)/include/c++/$(LINUX_TOOLCHAIN_VERSION)/$(LINUX_TOOLCHAIN_NAME)

# define cross compile
CROSS_COMPILE		:= $(TOOLCHAIN_PATH)/$(LINUX_TOOLCHAIN_NAME)-
TARGET_HOST   		:= $(TOOLCHAIN_PATH)/$(LINUX_TOOLCHAIN_NAME)
TARGET_CROSS  		:= $(TOOLCHAIN_PATH)/$(LINUX_TOOLCHAIN_NAME)-
TARGET_CC     		:= $(TARGET_CROSS)gcc
TARGET_CXX    		:= $(TARGET_CROSS)g++
TARGET_AR     		:= $(TARGET_CROSS)ar
TARGET_LD     		:= $(TARGET_CROSS)ld
TARGET_STRIP  		:= $(TARGET_CROSS)strip


# define platform name and platform base information
PLATFORM_NAME  		:= hi3798

PLATFORM_DIR   		:= $(TOP_DIR)/platform/$(PLATFORM_NAME)
ROOT_DIR       		:= $(PLATFORM_DIR)/rootfs
INFO_DIR       		:= $(PLATFORM_DIR)/info
PERSISTENT_DIR 		:= $(PLATFORM_DIR)/persistent
CONFIG_DIR		:= $(PLATFORM_DIR)/HisiConfig

# define target directory
TARGET_DIR		:= $(TOP_DIR)/targets/$(PLATFORM_NAME)
TARGET_SYS_DIR		:= $(TARGET_DIR)/sys
FIRMWARE_IMG_DIR	:= $(TARGET_DIR)/firmware
NSFROOT_DIR		:= $(TARGET_DIR)/nfsroot
MODULE_INSTALL_DIR	:= $(TARGET_SYS_DIR)/modules
KERNEL_IMG_DIR 		:= $(FIRMWARE_IMG_DIR)
ROOTFS_IMG_DIR		:= $(FIRMWARE_IMG_DIR)


# define std libs
STD_LIB_DIR    		:= $(ROOT_DIR)/lib
STD_USRLIB_DIR 		:= $(ROOT_DIR)/usr/lib
STD_LOCAL_BIN_DIR	:= $(ROOT_DIR)/usr/local/bin

UPFW_CONFIG_PATH    := 

export FIRMWARE_IMG_DIR
export STD_LIB_DIR
export STD_USRLIB_DIR
export CONFIG_DIR
export PLATFORM_NAME

APP_DIR        		:= $(TARGETS_DIR)/$(PLATFORM_NAME)/app
SHLIB_DIR      		:= $(TARGETS_DIR)/$(PLATFORM_NAME)/shlib
IMG_DIR        		:= $(TARGETS_DIR)/$(PLATFORM_NAME)/image

TFTP_HOME      		:= /tftpboot

image:
	@echo "make image"
	@echo "############################################################"
	@echo "生成 hisi 鏡像成功"
	@echo "############################################################"
	

