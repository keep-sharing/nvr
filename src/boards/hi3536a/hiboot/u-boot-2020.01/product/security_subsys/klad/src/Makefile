# INTER_DRV defined before include arch/build.mak
ifeq ($(CONFIG_PRODUCTNAME), $(filter $(CONFIG_PRODUCTNAME), "ss928v100" "ss000v100"))
INTER_DRV := ss928v100
else ifeq ($(CONFIG_PRODUCTNAME), $(filter $(CONFIG_PRODUCTNAME), "ss626v100"))
INTER_DRV := ss626v100
endif

KLAD_BASE_DIR := $(srctree)/product/security_subsys/klad/src

# Add objs
include $(KLAD_BASE_DIR)/mpi/build.mak
include $(KLAD_BASE_DIR)/mkp/build.mak
include $(KLAD_BASE_DIR)/arch/build.mak
include $(KLAD_BASE_DIR)/osal/build.mak
KLAD_CFLAGS += -I$(KLAD_BASE_DIR)/include
KLAD_CFLAGS += -I$(KLAD_BASE_DIR)/../include
KLAD_CFLAGS += -I$(KLAD_BASE_DIR)/../../ext_inc

cflags-y += $(KLAD_CFLAGS)
cflags-y += -DOT_KLAD_DEBUG=0

ccflags-y   += $(cflags-y)

obj-y   += $(KLAD_API_OBJS) $(KLAD_DRV_OBJS)
