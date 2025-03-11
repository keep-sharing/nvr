export PLATFORM := $(HISI_CROSS_TYPE)
export CROSS := $(PLATFORM)-
export RELEASE := $(RELEASE)
export INSTALL_DIR :=$(INSTALL_DIR)
export PFLAGS := $(PLATFORM_FLAGS) $(STD_LIB_FLAGS)

export EXTRA_INC := $(PUBLIC_INC_DIR)
export EXTRA_LIB := $(PUBLIC_LIB_DIR)\
		    $(STD_USRLIB_DIR)



all: 
	$(MAKE) -C sdk
	$(MAKE) -C media-server
clean:
	$(MAKE) -C media-server clean
	$(MAKE) -C sdk clean
