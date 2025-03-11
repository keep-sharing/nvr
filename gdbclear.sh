#!/bin/bash
git checkout -- platform/hi3536g/Makefile
git checkout -- platform/hi3536c/Makefile
git checkout -- platform/hi3798/Makefile
git checkout -- build/src/common.mk
git checkout -- build/src/gui/gui.mk
git checkout -- src/msgui/msgui.pro

git checkout -- src/libs/ovfcli/libonvifcli_hi.a
git checkout -- src/libs/ovfcli/libonvifcli_hi3798.a