# INTER_DRV defined before include arch/build.mak
ifeq ($(CONFIG_PRODUCTNAME), $(filter $(CONFIG_PRODUCTNAME), "ss524v100" "ss522v100"))
INTER_DRV := ss524v100
else ifeq ($(CONFIG_PRODUCTNAME), $(filter $(CONFIG_PRODUCTNAME), "ss528v100" "ss625v100"))
INTER_DRV := ss528v100
else ifeq ($(CONFIG_PRODUCTNAME), $(filter $(CONFIG_PRODUCTNAME), "ss928v100" "ss000v100"))
INTER_DRV := ss928v100
else ifeq ($(CONFIG_PRODUCTNAME), $(filter $(CONFIG_PRODUCTNAME), "ss626v100"))
INTER_DRV := ss626v100
endif

OTP_BASE_DIR := $(srctree)/product/security_subsys/otp/src

# Add objs
include $(OTP_BASE_DIR)/mpi/build.mak
include $(OTP_BASE_DIR)/mkp/build.mak
include $(OTP_BASE_DIR)/arch/build.mak
include $(OTP_BASE_DIR)/osal/build.mak
OTP_CFLAGS += -I$(OTP_BASE_DIR)/include
OTP_CFLAGS += -I$(OTP_BASE_DIR)/../include
OTP_CFLAGS += -I$(OTP_BASE_DIR)/../../ext_inc

cflags-y += $(OTP_CFLAGS)
cflags-y += -DOT_OTP_DEBUG=0

ccflags-y += $(cflags-y)

obj-y += $(MPI_OBJS) $(DRV_OBJS)
