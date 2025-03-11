#
#
#
#
#
PLATFORM_FLAGS		:= -march=armv8-a -mtune=cortex-a53 -ftree-vectorize -fno-builtin -fno-common -Wformat=1 -ffunction-sections -fdata-sections -Wl,--gc-sections -fgnu89-inline -g -fstack-protector -fstack-protector-all -rdynamic -funwind-tables
STD_LIB_FLAGS		:= -D_NT98633_ -g
export PLATFORM_FLAGS
export STD_LIB_FLAGS

QT_PATH 		:= $(TOOLS_DIR)/nt98633/QTLibs/
CHIP_MODEL 	= nt98633
CROSS_TYPE 	= aarch64-ca53-linux-gnu
export CHIP_MODEL
export CROSS_TYPE

# define common path of toolchain 
ARM_LINUX_TOOCHAIN_DIR	= /opt/ivot/aarch64-ca53-linux-gnueabihf-8.4.01/
LINUX_TOOLCHAIN_NAME		=$(CROSS_TYPE)
TOOLCHAIN_PATH		=$(ARM_LINUX_TOOCHAIN_DIR)/usr/bin

# define cross compile
CROSS_COMPILE		:= $(TOOLCHAIN_PATH)/$(CROSS_TYPE)-
TARGET_HOST   		:= $(TOOLCHAIN_PATH)/$(CROSS_TYPE)
TARGET_CROSS  		:= $(TOOLCHAIN_PATH)/$(CROSS_TYPE)-
TARGET_CC     		:= $(TARGET_CROSS)gcc
TARGET_CXX    		:= $(TARGET_CROSS)g++
TARGET_AR     		:= $(TARGET_CROSS)ar
TARGET_LD     		:= $(TARGET_CROSS)ld
TARGET_STRIP  		:= $(TARGET_CROSS)strip


# define platform name and platform base information
PLATFORM_NAME  		:= nt98633

PLATFORM_DIR   		:= $(TOP_DIR)/platform/$(PLATFORM_NAME)
ROOT_DIR       		:= $(PLATFORM_DIR)/rootfs
INFO_DIR       		:= $(PLATFORM_DIR)/info
PERSISTENT_DIR 		:= $(PLATFORM_DIR)/persistent
CONFIG_DIR		:= $(PLATFORM_DIR)/Config

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


APP_DIR        		:= $(TARGETS_DIR)/$(PLATFORM_NAME)/app
SHLIB_DIR      		:= $(TARGETS_DIR)/$(PLATFORM_NAME)/shlib
IMG_DIR        		:= $(TARGETS_DIR)/$(PLATFORM_NAME)/image

TFTP_HOME      		:= /tftpboot

image:
	@echo "make image"
	@echo "############################################################"
	@echo "生成 hisi 鏡像成功"
	@echo "############################################################"
	

