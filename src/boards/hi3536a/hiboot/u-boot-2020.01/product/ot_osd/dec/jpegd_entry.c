// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */
#include "jpegd_entry.h"
#include <common.h>
#include <command.h>
#include <malloc.h>
#include <version.h>
#include <net.h>
#include <asm/io.h>
#include <asm/arch/platform.h>
#include <config.h>
#include <cpu_func.h>
#include "jpegd.h"

unsigned long g_ot_logo = 0;
unsigned long g_jpeg_size = 0;
unsigned long g_video_data_base = 0;
unsigned long g_jpegd_emar_buf = 0;
unsigned int g_output_format = 0;

unsigned long get_ot_logo(void)
{
    return g_ot_logo;
}

unsigned long get_jpeg_size_val(void)
{
    return g_jpeg_size;
}

unsigned long get_video_data_base(void)
{
    return g_video_data_base;
}

unsigned long get_jpegd_emar_buf(void)
{
    return g_jpegd_emar_buf;
}

unsigned int get_output_format(void)
{
    return g_output_format;
}

int jpeg_decode(unsigned int format)
{
    jpegd_handle handle = TD_NULL;
    int ret;

    g_output_format = format;
    if (dcache_status()) {
        flush_dcache_range(g_ot_logo, g_ot_logo + g_jpeg_size);
    }

    handle = jpegd_get_handle();
    if (handle == TD_NULL) {
        printf("handle is invalid!");
        return -1;
    }
    ret = jpegd_start_decoding(handle);
    if (ret != TD_SUCCESS) {
        printf("decoding error!");
        return -1;
    }

    jpegd_finish_decoding(handle);

    return 0;
}

int get_vo_buf_addr(void)
{
    int i = 0;
    char evn_buf[ENV_BUF_LEN] = {0};
    char *str = env_get("vobuf");
    if (str != TD_NULL) {
        while ((*str != '\0') && (i < sizeof(evn_buf) - 1)) {
            evn_buf[i++] = *str++;
        }
        evn_buf[i] = '\0';
        g_video_data_base = simple_strtoul(evn_buf, TD_NULL, 16); /* Base 16 */
    } else {
        printf("Invalid vobuf address!\n");
        return 1;
    }
    if (g_video_data_base == 0) {
        printf("vobuf address is TD_NULL\n");
        return -1;
    }
    return 0;
}

int get_emar_buf_addr(void)
{
    int i = 0;
    char evn_buf[ENV_BUF_LEN] = {0};
    char *str = env_get("jpeg_emar_buf");
    if (str != TD_NULL) {
        while ((*str != '\0') && (i < sizeof(evn_buf) - 1)) {
            evn_buf[i++] = *str++;
        }
        evn_buf[i] = '\0';
        g_jpegd_emar_buf = simple_strtoul(evn_buf, TD_NULL, 16); /* Base 16 */
    } else {
        printf("Invalid jpeg_emar_buf address!\n");
        return 1;
    }
    if (g_jpegd_emar_buf == 0) {
        printf("jpeg_emar_buf address is TD_NULL\n");
        return -1;
    }
    return 0;
}

int need_2_emar_buf(void)
{
    unsigned int len;
    unsigned long interval;
    len = ONE_EMAR_BUF_SIZE * 2; /* 2:need two emar buffer */
    if (g_video_data_base > g_jpegd_emar_buf) {
        interval = g_video_data_base - g_jpegd_emar_buf;
    } else {
        interval = g_jpegd_emar_buf - g_video_data_base;
    }
    if (interval < len) {
        printf("vobuf and jpeg_emar_buf is overlapping! their interval len must larger than %u\n", len);
        return -1;
    }
    return 0;
}

int get_jpeg_size(void)
{
    int i = 0;
    char evn_buf[ENV_BUF_LEN] = {0};
    char *str = env_get("jpeg_size");
    if (str != TD_NULL) {
        while ((*str != '\0') && (i < sizeof(evn_buf) - 1)) {
            evn_buf[i++] = *str++;
        }
        evn_buf[i] = '\0';
        g_jpeg_size = simple_strtoul(evn_buf, TD_NULL, 16); /* Base 16 */
    } else {
        printf("you should set jpeg stream size!\n");
        return -1;
    }
    if (g_jpeg_size == 0) {
        printf("Invalid jpeg_size 0x%08lX\n", g_jpeg_size);
        return -1;
    }
    return 0;
}

int get_jpeg_stream_addr(void)
{
    int i = 0;
    char evn_buf[ENV_BUF_LEN] = {0};
    char *str = env_get("jpeg_addr");
    if (str != TD_NULL) {
        while ((*str != '\0') && (i < sizeof(evn_buf) - 1)) {
            evn_buf[i++] = *str++;
        }
        evn_buf[i] = '\0';
        g_ot_logo = simple_strtoul(evn_buf, TD_NULL, 16);  /* Base 16 */
    } else {
        printf("you should set jpeg picture's address!\n");
        return -1;
    }
    if (g_ot_logo == 0) {
        printf("jpeg_addr is TD_NULL\n");
        return -1;
    }
    return 0;
}

int load_jpeg(void)
{
    int ret;

    printf("jpeg decoding ...\n");

    /* get vo buffer address */
    ret = get_vo_buf_addr();
    if (ret != 0) {
        return ret;
    }

    /* get emar buffer address for jpegd */
    ret = get_emar_buf_addr();
    if (ret != 0) {
        return ret;
    }

    /* jpeg decoder need 2 emar buffer */
    ret = need_2_emar_buf();
    if (ret != 0) {
        return ret;
    }

    /* get jpeg size */
    ret = get_jpeg_size();
    if (ret != 0) {
        return ret;
    }

    /* get jpeg stream address */
    ret = get_jpeg_stream_addr();
    if (ret != 0) {
        return ret;
    }

    printf("<<addr=%#lx, size=%#lx, jpeg_emar_buf=%#lx, vobuf=%#lx>>\n", g_ot_logo, g_jpeg_size, g_jpegd_emar_buf,
           g_video_data_base);
    if ((g_ot_logo & (JPEGD_ADDR_ALIGN - 1)) != 0) {
        printf("jpeg_addr:%#lx should be align to %d bytes!\n", g_ot_logo, JPEGD_ADDR_ALIGN);
        return -1;
    }
    /* 0xFF:jpeg maker 0xD8:SOI maker */
    if ((*(unsigned char *)(uintptr_t)g_ot_logo != 0xFF) || (*(unsigned char *)((uintptr_t)g_ot_logo + 1) != 0xD8)) {
        printf("addr:%#lx,size:%lu,logoaddr:0,:%2x,%2x\n", g_ot_logo, g_jpeg_size,
            *(unsigned char *)(uintptr_t)g_ot_logo, *(unsigned char *)((uintptr_t)g_ot_logo + 1));
        return -1;
    }

    return 0;
}

