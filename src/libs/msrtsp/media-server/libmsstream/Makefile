ROOT:=../../sdk

#--------------------------------Output------------------------------
# OUTTYPE: 0-exe, 1-dll, 2-static
#--------------------------------------------------------------------
OUTTYPE = 2
OUTFILE = libmsstream.a
OUTTYPE_INCLUDE = 1
INNER_INCLUDE_PATH = ./include/*
OUTFILE_INCLUDE_PATH = ./

#-------------------------------Include------------------------------
#
# INCLUDES = $(addprefix -I,$(INCLUDES)) # add -I prefix
#--------------------------------------------------------------------
INCLUDES = . \
					./include \
					$(ROOT)/libaio/include \
					$(ROOT)/include \
					../librtp/include \
					../librtsp/include \
					$(EXTRA_INC)

#-------------------------------Source-------------------------------
#
#--------------------------------------------------------------------
SOURCE_PATHS = src 
SOURCE_FILES = $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.cpp))
SOURCE_FILES += $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.c))

SOURCE_FILES += $(ROOT)/source/port/ip-route.c
#-----------------------------Library--------------------------------
#
# LIBPATHS = $(addprefix -L,$(LIBPATHS)) # add -L prefix
#--------------------------------------------------------------------
LIBPATHS =
ifdef RELEASE
# relase library path
LIBPATHS +=
else
LIBPATHS +=
endif

LIBPATHS += $(EXTRA_LIB)

#LIBS = rt ssl crypto
#LIBS += osa
STATIC_LIBS = $(ROOT)/libaio/$(BUILD).$(PLATFORM)/libaio.a

#-----------------------------DEFINES--------------------------------
#
# DEFINES := $(addprefix -D,$(DEFINES)) # add -L prefix
#--------------------------------------------------------------------
DEFINES =

include ../gcc.mk
