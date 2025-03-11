dnl fedora.m4 - autoconf functions specific to Fedora-derived distributions
dnl
dnl Copyright (C) 2009  Red Hat, Inc.
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU Lesser General Public License as published
dnl by the Free Software Foundation; either version 2.1 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU Lesser General Public License for more details.
dnl
dnl You should have received a copy of the GNU Lesser General Public License
dnl along with this program.  If not, see <http://www.gnu.org/licenses/>.
dnl
dnl Author(s): David Cantrell <dcantrell@redhat.com>

AC_DEFUN([AM_FEDORA_NETWORK_FUNCTIONS],[
    fedora_network_functions=/etc/sysconfig/network-scripts/network-functions
    if test -f $fedora_network_functions ; then
        AC_DEFINE_UNQUOTED([NETWORK_FUNCTIONS_SCRIPT],
                           ["$fedora_network_functions"],
                           [network-functions script on Fedora-based systems])
    fi
])
