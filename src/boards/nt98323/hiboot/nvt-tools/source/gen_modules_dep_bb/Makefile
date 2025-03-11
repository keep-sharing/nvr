# output prefix
# e.g. $(MAIN_NAME).exe, lib$(MAIN_NAME).a  $(MAIN_NAME)
MAIN_NAME = gen_modules_dep_bb

#source files
SRC = \
	./gen_modules_dep_bb.c

#don't edit following for every platform
ifeq "$(shell uname)" "Linux"
ifeq "$(NVT_CROSS)" ""
# ubuntu
include _ubuntu.mk
else
# NVT-LINUX
include _linux.mk
endif
endif
