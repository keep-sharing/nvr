# INTER_DRV defined before include arch/build.mak
ifeq ($(CONFIG_PRODUCTNAME), $(filter $(CONFIG_PRODUCTNAME), "ss928v100"))
INTER_DRV := ss928v100
else ifeq ($(CONFIG_PRODUCTNAME), $(filter $(CONFIG_PRODUCTNAME), "ss626v100"))
INTER_DRV := ss626v100
endif

CIPHER_BASE_DIR := $(srctree)/product/security_subsys/cipher/v3/src/
CIPHER_PREFIX := v3/src/

CIPHER_CFLAGS += -I$(CIPHER_BASE_DIR)/../include/
CIPHER_CFLAGS += -I$(CIPHER_BASE_DIR)/../../../ext_inc/

# Add objs
include $(CIPHER_BASE_DIR)/api/build.mak
include $(CIPHER_BASE_DIR)/drv/build.mak

cflags-y += $(CIPHER_CFLAGS)

ccflags-y += $(cflags-y)

obj-y += $(DRV_OBJS) $(API_OBJS)
