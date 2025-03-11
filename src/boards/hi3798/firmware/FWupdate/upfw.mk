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
UBOOT_DIR 			= $(FW_TOPDIR)/../hiboot/fastboot
UPFW_INPUT_FILE		= $(UPFW_SRC_DIR)/baseinfo.h
UPFW_VERSION_FILE	= $(FW_TOPDIR)/../include/hiboot_version.h
UPFW_UPFW_IMG		= $(FW_TOPDIR)/update_IMG
UPFW_BUILD_DIR		= $(FW_TOPDIR)/build

TARGET_CC 	  	= $(CROSS)gcc
TARGET_STRIP  	= $(CROSS)strip
HOSTCC 			= gcc
CFLGS_VER		= -DVER_CPU=$(VERSION_CPU) -DVER_FILESYS=$(VERSION_FS) \
				-DVER_FLASH=$(VERSION_FLASH) -DVER_DDR3_2G=$(DDR3_VER_2G) \
				-DVER_DDR3_1G=$(DDR3_VER_1G) -DVER_LINUX=$(VERSION_KER) \
				-DVER_FS_MAIN=$(IPC_VER0) -DVER_FS_SUB=$(IPC_VER1) \
				-DVER_FS_THIRD=$(IPC_VER2)

CFLGS_FILE		= -DOUTPUT_UPFW_IMG=$(UPFW_UPFW_IMG) \
				-DINPUT_BLD_IMG=$(UPFW_BLD_IMG) \
				-DINPUT_BLD2_IMG=$(UPFW_BLD2_IMG) \
				-DINPUT_PARAM_IMG=$(UPFW_PARAM_IMG) \
				-DINPUT_JPG_IMG=$(UPFW_JPG_IMG) \
				-DINPUT_KER_IMG=$(UPFW_KER_IMG) \
				-DINPUT_FS_IMG=$(UPFW_FS_IMG) \
				-DINPUT_CMD_IMG=$(UPFW_CMD_IMG) \
				-DINPUT_TOOL_IMG=$(UPFW_TOOL_IMG)
				
HOST_CFLAGS   	= -Wall -DUSE_HOSTCC \
				-I$(UPFW_SRC_DIR) -I$(FW_TOPDIR)/../include \
				-I. -I$(PUBLIC_INC_DIR) \
				-I$(ENV_SRC_DIR) \
				-I$(UBOOT_DIR)/include \
				$(CFLGS_VER) $(CFLGS_FILE)
				
UPFW_CFLAGS		= -Wall $(PLATFORM_FLAGS) -D_XOPEN_SOURCE=500 -D__YEAR__=$(YEAR) \
				-D__MONTH__=$(MONTH) -D__DAY__=$(DAY) $(CFLGS_VER) $(CFLGS_FILE) \
				-Wall -DBUILD_NANDWRITE -L$(PUBLIC_LIB_DIR) -ldl -lpthread -lm -lrt -L$(STD_USRLIB_DIR) -lhi_msp -lhi_common -lhi_jpeg \
				-I$(UPFW_SRC_DIR) -I$(FW_TOPDIR)/../include \
				-I$(PUBLIC_INC_DIR)/vapi/$(PLATFORM_TYPE) \
				-I. -I$(PUBLIC_INC_DIR) \
				-I$(ENV_SRC_DIR)

__MAKE_UPFW_OBJ = \
		$(TARGET_CC) $(UPFW_CFLAGS)  -c $< -o $@
		
__MAKE_UPFW_EXE = \
		$(TARGET_CC) $(UPFW_CFLAGS) -o $@

__MAKE_HOST_OBJ = \
		$(HOSTCC) $(HOST_CFLAGS)  -c $< -o $@
		
__MAKE_HOST_EXE = \
		$(HOSTCC) $(HOST_CFLAGS) -o $@

UPFW_CMD_EXE  := $(FW_TOPDIR)/update
		
ifeq ($(MAKE_VERBOSE),no)
	MAKE_UPFW_OBJ =	$(__MAKE_UPFW_OBJ)
	MAKE_UPFW_EXE =	$(__MAKE_UPFW_EXE)
	MAKE_HOST_OBJ =	$(__MAKE_HOST_OBJ)
	MAKE_HOST_EXE =	$(__MAKE_HOST_EXE)
else
	MAKE_UPFW_OBJ =	@echo "  UPFWCC    $@" ; $(__MAKE_UPFW_OBJ)
	MAKE_UPFW_EXE =	@echo "  UPFWLD    $@" ; $(__MAKE_UPFW_EXE)
	MAKE_HOST_OBJ =	@echo "  HOSTCC    $@" ; $(__MAKE_HOST_OBJ)
	MAKE_HOST_EXE =	@echo "  HOSTLD    $@" ; $(__MAKE_HOST_EXE)
endif

ifeq ($(CONFIG_UPFW_HIBOOT),y)
UPFW_BLD_IMG	:= $(FIRMWARE_IMG_DIR)/uboot_$(DDR_SIZE_1G)
UPFW_BLD2_IMG	:= $(FIRMWARE_IMG_DIR)/uboot_$(DDR_SIZE_2G)
else
UPFW_BLD_IMG	:= 
endif

ifeq ($(CONFIG_UPFW_KERNEL),y)
UPFW_KER_IMG	:= $(FIRMWARE_IMG_DIR)/uImage
else
UPFW_KER_IMG	:= 
endif

ifeq ($(CONFIG_UPFW_FS),y)
UPFW_FS_IMG		:= $(FIRMWARE_IMG_DIR)/ubifs
else
UPFW_FS_IMG		:= 
endif

ifeq ($(CONFIG_UPFW_CMD),y)
UPFW_CMD_IMG	:= $(UPFW_CMD_EXE)
else
UPFW_CMD_IMG	:= 
endif

ifeq ($(CONFIG_UPFW_TOOL),y)
UPFW_TOOL_IMG	:= $(FIRMWARE_IMG_DIR)/tool
else
UPFW_TOOL_IMG	:= 
endif

UPFW_PARAM_IMG := $(FIRMWARE_IMG_DIR)/baseparam.img
UPFW_JPG_IMG	:= $(FIRMWARE_IMG_DIR)/logo.img 


$(FW_TOPDIR)/host_upfw.o: $(UPFW_SRC_DIR)/host_upfw.c $(UPFW_SRC_DIR)/updateFW.h	
	$(MAKE_HOST_OBJ)
	
$(FW_TOPDIR)/host_md5.o: $(UPFW_SRC_DIR)/md5.c $(UPFW_SRC_DIR)/md5.h
	$(MAKE_HOST_OBJ)
	
$(FW_TOPDIR)/host_crc32.o: $(ENV_SRC_DIR)/crc32.c
	$(MAKE_HOST_OBJ)

$(FW_TOPDIR)/host_updateFW.o: $(UPFW_SRC_DIR)/updateFW.c \
		$(PUBLIC_INC_DIR)/update_cmd.h \
		$(UBT_VER_FILE) \
		$(UPFW_SRC_DIR)/updateFW.h \
		$(UPFW_SRC_DIR)/baseinfo.h \
		$(UPFW_SRC_DIR)/md5.h \
		$(UPFW_SRC_DIR)/nandwrite.h
	$(MAKE_HOST_OBJ) \
		-D__YEAR__=$(YEAR) \
		-D__MONTH__=$(MONTH) \
		-D__DAY__=$(DAY)

host_prepared: $(ENV_SRC_DIR)/crc32.c
	
host_upfw: $(FW_TOPDIR)/host_crc32.o \
		$(FW_TOPDIR)/host_md5.o \
		$(FW_TOPDIR)/host_updateFW.o \
		$(FW_TOPDIR)/host_upfw.o
	$(MAKE_HOST_EXE) $^

$(FW_TOPDIR)/nandwrite.o: $(UPFW_SRC_DIR)/nandwrite.c \
	$(UPFW_SRC_DIR)/nandwrite.h
	$(MAKE_UPFW_OBJ)
	
$(FW_TOPDIR)/md5.o: $(UPFW_SRC_DIR)/md5.c \
		$(UPFW_SRC_DIR)/md5.h	
	$(MAKE_UPFW_OBJ)
	
$(FW_TOPDIR)/updateFW.o: $(UPFW_SRC_DIR)/updateFW.c \
		$(PUBLIC_INC_DIR)/update_cmd.h \
		$(UBT_VER_FILE) \
		$(UPFW_SRC_DIR)/updateFW.h \
		$(UPFW_SRC_DIR)/baseinfo.h \
		$(UPFW_SRC_DIR)/md5.h \
		$(UPFW_SRC_DIR)/nandwrite.h
	$(MAKE_UPFW_OBJ)
	
$(FW_TOPDIR)/update.o: $(UPFW_SRC_DIR)/update.c \
		$(UPFW_SRC_DIR)/updateFW.h
	$(MAKE_UPFW_OBJ)

$(FW_TOPDIR)/chipinfo.o: $(SRC_DIR)/libs/chipinfo/$(PLATFORM_TYPE)/chipinfo.c 
	$(MAKE_UPFW_OBJ)
	
update: $(FW_TOPDIR)/md5.o \
		$(FW_TOPDIR)/updateFW.o \
		$(FW_TOPDIR)/nandwrite.o \
		$(FW_TOPDIR)/chipinfo.o \
		$(FW_TOPDIR)/update.o $(ENV_OBJS)
	$(MAKE_UPFW_EXE) $^ -o $(UPFW_CMD_EXE)
	$(TARGET_STRIP) $(UPFW_CMD_EXE)
	$(MS_MAKEFILE_V)cp -dpRf $(UPFW_CMD_EXE) $(PUBLIC_APP_DIR)/
#	-$(MS_MAKEFILE_V)cp  update /tftpboot/

updatefw: host_upfw fw_env update
	./host_upfw -c
	./host_upfw -v
#	-$(MS_MAKEFILE_V)cp  updatefw/* /tftpboot/

upfwclean:
	-$(MS_MAKEFILE_V)rm -rf $(FW_TOPDIR)/*.o $(FW_TOPDIR)/.update $(FW_TOPDIR)/host_upfw  
	-$(MS_MAKEFILE_V)rm -rf $(PUBLIC_APP_DIR)/update $(UPFW_UPFW_IMG) $(UPFW_CMD_EXE)

