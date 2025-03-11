#
# Copyright (c) 2015 HiSilicon Technologies Co., Ltd.
#
# This program is free software; you can redistribute  it and/or modify it
# under  the terms of  the GNU General Public License as published by the
# Free Software Foundation;  either version 2 of the  License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

SRC := hi3516a

all: $(addsuffix .build,$(SRC))

clean: $(addsuffix .clean,$(SRC))

%.build: %
	make -C $(@:.build=) TOPDIR=$(TOPDIR) CROSS_COMPILE=$(CROSS_COMPILE)

%.clean: %
	make -C $(@:.clean=) TOPDIR=$(TOPDIR) CROSS_COMPILE=$(CROSS_COMPILE) \
		clean

.PHONY: all clean
