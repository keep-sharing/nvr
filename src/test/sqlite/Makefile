
LINUX_TOOLCHAIN_PREFIX		=/usr/local
ARM_LINUX_TOOCHAIN_DIR		=$(LINUX_TOOLCHAIN_PREFIX)/arm-2011.09
TOOLCHAIN_PATH			=$(ARM_LINUX_TOOCHAIN_DIR)/bin

LINUX_TOOLCHAIN_NAME		=arm-none-linux-gnueabi
TARGET_CROSS  	:= $(TOOLCHAIN_PATH)/$(LINUX_TOOLCHAIN_NAME)-

CC := $(TARGET_CROSS)gcc

#所有的源文件
ALL_SRC_FILES   := $(wildcard *.c)

#所有的目标文件
ALL_TARGET_OBJS := $(patsubst %.c,%.o,$(ALL_SRC_FILES))

MK_EXE = sql_test
LDFLAGS = -lsqlite3

.PHONY: $(MK_EXE) clean

#exe生成规则
$(MK_EXE):$(ALL_TARGET_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o:%.c
	$(CC) -o $@ -c $<

clean:
	-rm -rf $(MK_EXE) $(ALL_TARGET_OBJS)
	
