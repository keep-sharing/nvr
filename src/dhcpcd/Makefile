#CC = $(CC)
#CFLAGS += -Wall -O3

#EXEC = $(TARGET_NAME)
#OBJS = arp.o buildmsg.o client.o dhcpcd.o peekfd.o signals.o udpipgen.o dhcpconfig.o cache.o
#INSTALLDIR = $(TARGET_DIR)

#all: $(EXEC)

#$(EXEC): $(OBJS)
#	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)
#	install -d $(INSTALLDIR)
#	install $(EXEC) $(INSTALLDIR)

#clean:
#	-rm -f *.elf *.gdb *.o
#	-rm -f $(INSTALLDIR)/$(EXEC)


#需要包含的目录
#-DEVM_DEBUG 
INCLUDE			:= -I$(PUBLIC_INC_DIR) -I$(PUBLIC_INC_DIR)/msdb -I$(SRC_DIR)/msgui/mscore #-I$(PUBLIC_INC_DIR)/$(PLATFORM_TYPE)

ifeq ($(PLATFORM_TYPE), nt98323)
INCLUDE			+= -D_NT98323_
endif

CFLAGS 			:= -O3

#需要的lib库
LDFLAGS         := -lpthread -L$(PUBLIC_LIB_DIR) -lmsstd -lmsdb -L$(STD_USRLIB_DIR) -lsqlite3

#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE		:="exe"

#导入通用mk文件
include $(BUILD_COMMON_MK)
