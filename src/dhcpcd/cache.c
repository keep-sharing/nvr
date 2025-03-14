/*
 * dhcpcd - DHCP client daemon -
 * Copyright (C) 1996 - 1997 Yoichi Hariguchi <yoichi@fore.com>
 * Copyright (C) January, 1998 Sergei Viznyuk <sv@phystech.com>
 * 
 * dhcpcd is an RFC2131 and RFC1541 compliant DHCP client daemon.
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "pathnames.h"
#include "kversion.h"
#include "client.h"
#include "dhcpcd.h"

//extern	char		*ConfigDir;
extern	char		*IfNameExt;
extern	int		prev_ip_addr;
extern	dhcpInterface	DhcpIface;

char cache_file[128];
/*****************************************************************************/
int readDhcpCache()
{
  int i,o;
  snprintf(cache_file,sizeof(cache_file),DHCP_CACHE_FILE,IfNameExt);
  i=open(cache_file,O_RDONLY);
  if ( i == -1 ) return -1;
  o=read(i,(char *)&DhcpIface,sizeof(dhcpInterface));
  close(i);
  if ( o != sizeof(dhcpInterface) ) return -1;
  if ( strncmp((char*)DhcpIface.version,VERSION,sizeof(DhcpIface.version)) ) return -1;
  prev_ip_addr = DhcpIface.ciaddr;
  return 0;
}
/*****************************************************************************/
void deleteDhcpCache()
{
  snprintf(cache_file,sizeof(cache_file),DHCP_CACHE_FILE,IfNameExt);
  unlink(cache_file);
}
