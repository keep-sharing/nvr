CIPHER_CFLAGS += -I$(CIPHER_BASE_DIR)/drv/compat/

DRV_OBJS += $(CIPHER_PREFIX)/drv/compat/drv_ks.o
DRV_OBJS += $(CIPHER_PREFIX)/drv/compat/rsa_padding.o
