// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DDRTRN_CONSOLE_H
#define DDRTRN_CONSOLE_H

#define ALIGN_COUNT 4

#define isprint(c) ((c) >= ' ' && (c) <= '~')
#define isspace(c) ((c) == ' ' || ((c) >= '\t' && (c) <= '\r'))
#define isdigit(c) ((c) >= '0' && (c) <= '9')
#define isxdigit(c) (isdigit(c) || ((c) >= 'A' && (c) <= 'F') || ((c) >= 'a' && (c) <= 'f'))
#endif /* DDRTRN_CONSOLE_H */