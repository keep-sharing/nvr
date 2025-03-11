// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "ddrtrn_training.h"
#include "ddrtrn_interface.h"

void *ddrtr_copy_data(void *dst, const void *src, unsigned int len)
{
	const char *s = src;
	char *d = dst;

	while (len--)
		*d++ = *s++;

	return dst;
}

void *ddrtr_set_data(void *b, int c, unsigned int len)
{
	char *bp = b;

	while (len--)
		*bp++ = (unsigned char)c;

	return b;
}