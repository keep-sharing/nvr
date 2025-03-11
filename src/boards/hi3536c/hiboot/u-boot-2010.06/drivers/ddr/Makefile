#/******************************************************************************
# * Copyright (c) 2009-2014 HiSilicon Technologies Co., Ltd.
# *
# * This program is free software; you can redistribute  it and/or modify it
# * under  the terms of  the GNU General Public License as published by the
# * Free Software Foundation;  either version 2 of the  License, or (at your
# * option) any later version.
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this program.  If not, see <http://www.gnu.org/licenses/>.
# *
# *****************************************************************************/
include $(TOPDIR)/config.mk

LIB	:= $(obj)libddr.a

SOBJS   += ddr_cmd_loc.o
COBJS   += ddr_cmd_ctl.o ddr_training_impl.o ddr_training_ctl.o ddr_training_boot.o ddr_training_custom.o

SSRC    := ddr_training_custom.h ddr_training_custom.c

CFLAGS  += -I$(TOPDIR)/arch/$(ARCH)/cpu/$(CPU)/

SRCS	:= $(START:.o=.S) $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))

LINK_FILES = $(SSRC)


all: $(SSRC) $(LIB)
	@rm -f $(LINK_FILES)

ddr_training: $(LIB)

$(LIB):	$(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

$(SSRC):
	ln -sf $(TOPDIR)/arch/$(ARCH)/cpu/$(CPU)/$@ $@
include $(SRCTREE)/rules.mk

#sinclude $(obj).depend

#########################################################################
