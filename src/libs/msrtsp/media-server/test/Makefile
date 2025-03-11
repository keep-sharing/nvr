ROOT:=../../sdk
NOVERSION = 1

#--------------------------------Output------------------------------
# OUTTYPE: 0-exe, 1-dll, 2-static
#--------------------------------------------------------------------
OUTTYPE = 0
OUTFILE = media-server-test

#-------------------------------Include------------------------------
#
# INCLUDES = $(addprefix -I,$(INCLUDES)) # add -I prefix
#--------------------------------------------------------------------
INCLUDES = . $(ROOT)/include $(ROOT)/libhttp/include \
			../librtp/include \
			../librtsp/include \
			../libmsstream/include \
			$(EXTRA_INC)

#-------------------------------Source-------------------------------
#
#--------------------------------------------------------------------
SOURCE_PATHS = . 
SOURCE_FILES = $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.cpp))
SOURCE_FILES += $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.c))
#SOURCE_FILES += $(ROOT)/source/uri-parse.c
#SOURCE_FILES += $(ROOT)/source/urlcodec.c
#SOURCE_FILES += $(ROOT)/source/ntp-time.c
#SOURCE_FILES += $(ROOT)/source/time64.c
#SOURCE_FILES += $(ROOT)/source/base64.c
#SOURCE_FILES += $(ROOT)/source/unicode.c
#SOURCE_FILES += $(ROOT)/source/thread-pool.c
#SOURCE_FILES += $(ROOT)/source/app-log.c
#SOURCE_FILES += $(ROOT)/source/port/ip-route.c
#SOURCE_FILES += $(ROOT)/source/sockpair.c
#SOURCE_FILES += $(ROOT)/source/darray.c
#SOURCE_FILES += $(ROOT)/source/channel.c
#SOURCE_FILES += $(ROOT)/source/uuid.c
#SOURCE_FILES += $(ROOT)/source/random.c
#SOURCE_FILES += $(ROOT)/source/port/sysnetconfig.c
#SOURCE_FILES += $(ROOT)/libice/test/ice-transport.c
#SOURCE_FILES += $(ROOT)/deprecated/tools.c

#_SOURCE_FILES = $(ROOT)/libhttp/test/main.c
#SOURCE_FILES := $(filter-out $(_SOURCE_FILES),$(SOURCE_FILES))

#-----------------------------Library--------------------------------
#
# LIBPATHS = $(addprefix -L,$(LIBPATHS)) # add -L prefix
#--------------------------------------------------------------------
#LIBPATHS = $(ROOT)/libaio/$(BUILD).$(PLATFORM)
LIBPATHS += $(EXTRA_LIB)

LIBS = rt pthread dl
ifeq ($(PLATFORM_TYPE), hi3798)
LIBPATHS += $(STD_USRLIB_DIR)
LIBPATHS += $(PUBLIC_LIB_DIR)
LIBS += hi_msp hi_common hi_jpeg gpio msstd
endif
STATIC_LIBS = ../libmsstream/$(BUILD).$(PLATFORM)/libmsstream.a \
				../librtsp/$(BUILD).$(PLATFORM)/librtsp.a \
				../librtp/$(BUILD).$(PLATFORM)/librtp.a \
				$(ROOT)/libhttp/$(BUILD).$(PLATFORM)/libhttp.a \
				$(ROOT)/libmediasdk/$(BUILD).$(PLATFORM)/libmediasdk.a
#-----------------------------DEFINES--------------------------------
#
# DEFINES := $(addprefix -D,$(DEFINES)) # add -L prefix
#--------------------------------------------------------------------
DEFINES = OS_INT64_TYPE

include ../gcc.mk
LDFLAGS         	:=  -L$(STD_USRLIB_DIR) -lssl -lcrypto -lnghttp2
CXXFLAGS += -std=c++0x 
