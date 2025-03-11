/*
* Copyright (c) 2012 HiSilicon Technologies Co., Ltd.
*
* This program is free software; you can redistribute  it and/or modify it
* under  the terms of  the GNU General Public License as published by the
* Free Software Foundation;  either version 2 of the  License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/


#ifndef LIBCH
#define LIBCH
/******************************************************************************/

#define NULL ((void *)0)

void *memcpy(void *dst, const void *src, unsigned int len);

void *memset(void *b, int c, unsigned int len);

int memcmp(const void *b1, const void *b2, unsigned int len);

int strcmp(const char *s1, const char *s2);

int strncmp(const char *s1, const char *s2, unsigned int len);

void strcpy(char *dst, const char *src);

void strcat(char *dst, const char *src);

char *strchr(const char *s, char ch);

unsigned int strlen(const char *s);

/******************************************************************************/
#endif /* LIBCH */





