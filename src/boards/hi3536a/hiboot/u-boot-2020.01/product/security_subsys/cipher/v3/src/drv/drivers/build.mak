
CIPHER_CFLAGS += -I$(CIPHER_BASE_DIR)/drv/drivers/core/include
CIPHER_CFLAGS += -I$(CIPHER_BASE_DIR)/drv/drivers/crypto/include
CIPHER_CFLAGS += -I$(CIPHER_BASE_DIR)/drv/drivers/extend
CIPHER_CFLAGS += -I$(CIPHER_BASE_DIR)/drv/drivers/extend/include

DRV_OBJS += $(CIPHER_PREFIX)/drv/drivers/core/drv_symc_v300.o          \
            $(CIPHER_PREFIX)/drv/drivers/core/drv_hash_v300.o          \
            $(CIPHER_PREFIX)/drv/drivers/core/drv_pke_v200.o           \
            $(CIPHER_PREFIX)/drv/drivers/core/drv_trng_v200.o          \
            $(CIPHER_PREFIX)/drv/drivers/core/drv_lib.o

DRV_OBJS += $(CIPHER_PREFIX)/drv/drivers/crypto/cryp_symc.o             \
            $(CIPHER_PREFIX)/drv/drivers/crypto/cryp_hash.o             \
            $(CIPHER_PREFIX)/drv/drivers/crypto/cryp_trng.o             \
            $(CIPHER_PREFIX)/drv/drivers/crypto/cryp_rsa.o              \
            $(CIPHER_PREFIX)/drv/drivers/crypto/cryp_sm2.o

DRV_OBJS += $(CIPHER_PREFIX)/drv/drivers/kapi_symc.o                    \
            $(CIPHER_PREFIX)/drv/drivers/kapi_hash.o                    \
            $(CIPHER_PREFIX)/drv/drivers/kapi_rsa.o                     \
            $(CIPHER_PREFIX)/drv/drivers/kapi_trng.o                    \
            $(CIPHER_PREFIX)/drv/drivers/kapi_sm2.o                     \
            $(CIPHER_PREFIX)/drv/drivers/kapi_dispatch.o

