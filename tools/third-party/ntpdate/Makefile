# Makefile for ntpclient/adjtimex
#
# Makefile by Uwe Beutin, modified by John Williams
# 
# ntpclient by Larry Dolittle

#CC = $(CC)
	
#EXEC = $(TARGET_NAME)

#OBJS = ntpclient.o phaselock.o
#INSTALLDIR = $(TARGET_DIR)

#CFLAGS += -Wall -O3 -D__USE_BSD $(PLATFORM_FLAGS) $(STD_LIB_FLAGS)		# 2015.3.13
#all: $(EXEC)
#	cp $(EXEC) $(TARGET_DIR)
#$(EXEC): $(OBJS)
#	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS$(LDLIBS_$@))

#install: $(EXEC)
#	install -d $(INSTALLDIR)
#	install $(EXEC) $(INSTALLDIR)

#clean:
#	rm -f $(EXEC) *.gdb *.elf *.o
#	-rm -f $(INSTALLDIR)/$(EXEC)

#需要包含的目录
INCLUDE			:= -I$(PUBLIC_INC_DIR) -I.


#不需要编译的文件
DELETE_FILE		= adjtimex.c

#需要的cflag选项
CFLAGS			:= -O3 -D__USE_BSD

#需要的lib库
LDFLAGS			:= -L$(PUBLIC_LIB_DIR) -lpthread

#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE:="exe"

#导入通用mk文件
include $(BUILD_COMMON_MK)
