CIPHER_BASE_DIR := $(srctree)/product/security_subsys/cipher/v2
CIPHER_PREFIX := v2/

cflags-y     += -I$(CIPHER_BASE_DIR)/../../ext_inc

ccflags-y  += $(cflags-y)
HOSTCFLAGS += $(cflags-y)
CPPFLAGS   += $(cflags-y)

obj-y   += $(CIPHER_PREFIX)/api/ree_mpi_cipher.o
