#!/bin/bash
sed -i 's/#-g/-g/'  platform/hi3536g/Makefile
sed -i 's/#-g/-g/'  platform/hi3536c/Makefile
sed -i 's/#-g/-g/'  platform/hi3798/Makefile
sed -i 's/:= y/:= n/'  build/src/common.mk
sed -i 's/$(TARGET_STRIP)/#$(TARGET_STRIP)/'  build/src/gui/gui.mk
sed -i -e '/QMAKE_CFLAGS/{s/#-g/-g/}'  src/msgui/msgui.pro

if [ "$1" = "-ovf" ];then
	cp src/libs/ovfcli/ovflib-g/* src/libs/ovfcli
fi