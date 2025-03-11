#!/bin/bash

case "$1" in
    "on")
        echo "asan on begin"
        # mscore
        sed -i '/.*# asan*/{n; s/.*/QMAKE_LFLAGS   += -fsanitize=address -fno-omit-frame-pointer -g/}' src/msgui/msgui.pro
        # mscore other denpendent libs and apps
        sed -i '/.*PLATFORM_FLAGS		:= -mcpu=cortex-a55 -ffunction-sections -fno-aggressive-loop-optimizations -rdynamic -funwind-tables -Wl,-gc-sections -g*/c\PLATFORM_FLAGS		:= -mcpu=cortex-a55 -ffunction-sections -fno-aggressive-loop-optimizations -rdynamic -funwind-tables -Wl,-gc-sections -g -fsanitize=address -fno-omit-frame-pointer' platform/hi3536a/Makefile
        sed -i '/.*TARGET_STRIP  		:= $(TARGET_CROSS)strip*/c\TARGET_STRIP  		:= : $(TARGET_CROSS)strip' platform/hi3536a/Makefile
        sed -i '/.*LDFLAGS += -lsecurec */c\LDFLAGS += -lsecurec -fsanitize=address -fno-omit-frame-pointer -g' platform/hi3536a/Makefile
        sed -i '/.*LDFLAGS += -lsecurec */c\LDFLAGS += -lsecurec -fsanitize=address -fno-omit-frame-pointer -g' src/test/hardware/Makefile
        # rtsp
        sed -i '/.*HI_PLATFORM_FLAGS 		:= "-march=armv8-a -ffunction-sections -fdata-sections -O2*"/c\HI_PLATFORM_FLAGS 		:= "-march=armv8-a -ffunction-sections -fdata-sections -fsanitize=address -fno-omit-frame-pointer -g"' build/src/libs/msrtsp/msrtsp.mk
        sed -i '/.*MS_RTSP_LIB_COMPILE_OPT := $(MS_MAKE_JOBS) -C $(MS_RTSP_LIB_SRC_DIR) HISI_CROSS_TYPE=$(CROSS_PREFIX) INSTALL_DIR=$(MS_RTSP_LIB_TARGET_DIR) PLATFORM_FLAGS=$(HI_PLATFORM_FLAGS) HEADFILE_INSTALL_DIR=$(MS_RTSP_INC_SRC_DIR) RELEASE=1 MSPLATFORM=$(MS_PLATFORM_FLAG)*/c\MS_RTSP_LIB_COMPILE_OPT := $(MS_MAKE_JOBS) -C $(MS_RTSP_LIB_SRC_DIR) HISI_CROSS_TYPE=$(CROSS_PREFIX) INSTALL_DIR=$(MS_RTSP_LIB_TARGET_DIR) PLATFORM_FLAGS=$(HI_PLATFORM_FLAGS) HEADFILE_INSTALL_DIR=$(MS_RTSP_INC_SRC_DIR) RELEASE=0 MSPLATFORM=$(MS_PLATFORM_FLAG)' build/src/libs/msrtsp/msrtsp.mk
        sed -i '/.*#	CFLAGS += -fsanitize=address*/c\	CFLAGS += -fsanitize=address -fno-omit-frame-pointer' src/libs/msrtsp/media-server/gcc.mk
        sed -i '/.*	# for assert*/{n; s/.*/	DEFINES += NDEBUG/}' src/libs/msrtsp/media-server/gcc.mk
        sed -i '/.*#	CFLAGS += -fsanitize=address*/c\	CFLAGS += -fsanitize=address -fno-omit-frame-pointer' src/libs/msrtsp/sdk/gcc.mk
        sed -i '/.*	# for assert*/{n; s/.*/	DEFINES += NDEBUG/}' src/libs/msrtsp/sdk/gcc.mk
        sed -i '/.*LDFLAGS         	:=  -L$(STD_USRLIB_DIR) -lssl -lcrypto -lnghttp2*/c\LDFLAGS         	:=  -L$(STD_USRLIB_DIR) -lssl -lcrypto -lnghttp2 -fsanitize=address -fno-omit-frame-pointer -g' src/libs/msrtsp/media-server/test/Makefile
        # firmware size
        sed -i '/.*	$(DEFAULTMKEXT4FS) -l 256M -s $(BUILD_ROOTFS_DIR)\/ext4fs $(FAKEROOT_DIR)*/c\	$(DEFAULTMKEXT4FS) -l 512M -s $(BUILD_ROOTFS_DIR)\/ext4fs $(FAKEROOT_DIR)' tools/hi3536a/makfs/Makefile

        echo "asan on end"
        ;;
    "off")
        echo "asan off begin"
        # mscore
        sed -i '/.*# asan*/{n; s/.*/# QMAKE_LFLAGS   += -fsanitize=address -fno-omit-frame-pointer -g/}' src/msgui/msgui.pro
        # mscore other denpendent libs and apps
        sed -i '/.*PLATFORM_FLAGS		:= -mcpu=cortex-a55 -ffunction-sections -fno-aggressive-loop-optimizations -rdynamic -funwind-tables -Wl,-gc-sections -g -fsanitize=address -fno-omit-frame-pointer*/c\PLATFORM_FLAGS		:= -mcpu=cortex-a55 -ffunction-sections -fno-aggressive-loop-optimizations -rdynamic -funwind-tables -Wl,-gc-sections -g' platform/hi3536a/Makefile
        sed -i '/.*TARGET_STRIP  		:= : $(TARGET_CROSS)strip*/c\TARGET_STRIP  		:= $(TARGET_CROSS)strip' platform/hi3536a/Makefile
        sed -i '/.*LDFLAGS += -lsecurec -fsanitize=address -fno-omit-frame-pointer -g*/c\LDFLAGS += -lsecurec' platform/hi3536a/Makefile
        sed -i '/.*LDFLAGS += -lsecurec -fsanitize=address -fno-omit-frame-pointer -g*/c\LDFLAGS += -lsecurec' src/test/hardware/Makefile
        # rtsp
        sed -i '/.*HI_PLATFORM_FLAGS 		:= "-march=armv8-a -ffunction-sections -fdata-sections -fsanitize=address -fno-omit-frame-pointer -g*"/c\HI_PLATFORM_FLAGS 		:= "-march=armv8-a -ffunction-sections -fdata-sections -O2"' build/src/libs/msrtsp/msrtsp.mk
        sed -i '/.*MS_RTSP_LIB_COMPILE_OPT := $(MS_MAKE_JOBS) -C $(MS_RTSP_LIB_SRC_DIR) HISI_CROSS_TYPE=$(CROSS_PREFIX) INSTALL_DIR=$(MS_RTSP_LIB_TARGET_DIR) PLATFORM_FLAGS=$(HI_PLATFORM_FLAGS) HEADFILE_INSTALL_DIR=$(MS_RTSP_INC_SRC_DIR) RELEASE=0 MSPLATFORM=$(MS_PLATFORM_FLAG)*/c\MS_RTSP_LIB_COMPILE_OPT := $(MS_MAKE_JOBS) -C $(MS_RTSP_LIB_SRC_DIR) HISI_CROSS_TYPE=$(CROSS_PREFIX) INSTALL_DIR=$(MS_RTSP_LIB_TARGET_DIR) PLATFORM_FLAGS=$(HI_PLATFORM_FLAGS) HEADFILE_INSTALL_DIR=$(MS_RTSP_INC_SRC_DIR) RELEASE=1 MSPLATFORM=$(MS_PLATFORM_FLAG)' build/src/libs/msrtsp/msrtsp.mk
        sed -i '/.*	CFLAGS += -fsanitize=address -fno-omit-frame-pointer*/c\#	CFLAGS += -fsanitize=address' src/libs/msrtsp/media-server/gcc.mk
        sed -i '/.*	# for assert*/{n; s/.*/	DEFINES += DEBUG _DEBUG/}' src/libs/msrtsp/media-server/gcc.mk
        sed -i '/.*	CFLAGS += -fsanitize=address -fno-omit-frame-pointer*/c\#	CFLAGS += -fsanitize=address' src/libs/msrtsp/sdk/gcc.mk
        sed -i '/.*	# for assert*/{n; s/.*/	DEFINES += DEBUG _DEBUG/}' src/libs/msrtsp/sdk/gcc.mk
        sed -i '/.*LDFLAGS         	:=  -L$(STD_USRLIB_DIR) -lssl -lcrypto -lnghttp2 -fsanitize=address -fno-omit-frame-pointer -g*/c\LDFLAGS         	:=  -L$(STD_USRLIB_DIR) -lssl -lcrypto -lnghttp2' src/libs/msrtsp/media-server/test/Makefile
        # firmware size
        sed -i '/.*	$(DEFAULTMKEXT4FS) -l 512M -s $(BUILD_ROOTFS_DIR)\/ext4fs $(FAKEROOT_DIR)*/c\	$(DEFAULTMKEXT4FS) -l 256M -s $(BUILD_ROOTFS_DIR)\/ext4fs $(FAKEROOT_DIR)' tools/hi3536a/makfs/Makefile
        echo "asan off end"
        ;;
    *)
        echo "Usage: $0 {on|off}"

    exit 1
esac