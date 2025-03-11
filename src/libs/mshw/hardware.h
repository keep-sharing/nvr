/*
 * ***************************************************************
 * Filename:        hardware.h
 * Created at:      2016.01.05
 * Description:     hardware test api.
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "msstd.h"
#include "hd_module.h"
#include "mshw.h"

#define RED             "\033[0;31m"
#define GREEN           "\033[0;32m"
#define CLRCOLOR        "\033[0;39m "

extern ResultCb g_log;
extern struct hd_module_info *g_test_info;

#define hdprintf(log, level) \
    do {\
        if (g_log) {\
            g_log(g_test_info->name, log, level);\
        }\
    }while(0)


#define hdinfo(log) hdprintf(log, LOG_INFO)
#define hdno(log)    hdprintf(log, LOG_ERR)
#define hdyes(log) hdprintf(log, LOG_SUCCESS)
#define hdprompt(log) hdprintf(log, LOG_NOTE)

#define hderr(format, ...) \
    do{\
        printf(RED format CLRCOLOR, ##__VA_ARGS__);\
    }while(0)

#define SHOW_YES     \
    hdinfo("\n");\
    hdinfo("---------------------\n");\
    hdinfo("|       YES         |\n");\
    hdinfo("---------------------\n");\
    hdyes("\n");

#define SHOW_NO     \
    hdinfo("\n");\
    hdinfo("---------------------\n");\
    hdinfo("|        NO          |\n");\
    hdinfo("---------------------\n");\
    hdno("\n");

int get_hardware_version(char *device_info);

#ifdef __cplusplus
}
#endif

#endif

