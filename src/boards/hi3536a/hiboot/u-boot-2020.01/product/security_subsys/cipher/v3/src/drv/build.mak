CIPHER_CFLAGS += -I$(CIPHER_BASE_DIR)/drv/include
CIPHER_CFLAGS += -I$(CIPHER_BASE_DIR)/drv/arch/$(INTER_DRV)

include $(CIPHER_BASE_DIR)/drv/compat/build.mak
include $(CIPHER_BASE_DIR)/drv/drivers/build.mak
include $(CIPHER_BASE_DIR)/drv/osal/build.mak
