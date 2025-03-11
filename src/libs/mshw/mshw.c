#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/stat.h>

#include "hardware.h"
#include "hisi_dm2016_eeprom.h"
#include "msoem.h"
#include "chipinfo.h"


int g_isAuto = 1;
ResultCb g_log = NULL;
struct hd_module_info *g_test_info = NULL;
char g_devName[16];

int get_hardware_version(char *device_info)
{
    char *chipinfo = NULL;
    ms_env_init();

    chipinfo = ms_getenv("chipinfo");
    if (chipinfo) {
        strncpy((char *)device_info, chipinfo, DM2016_VAL_LEN);
        ms_env_deinit();
        return 0;
    }
    ms_env_deinit();

    return -1;
}

MS_S32
ms_hw_test_get_list(TestList *pTestList)
{
    pTestList->count = hd_dump_module(pTestList->entrys);

    return pTestList->count > 0 ? MS_SUCCESS : MS_FAILURE;
}

MS_S32
ms_hw_test_start(const char *name, ResultCb resultCb, void *param)
{
    if (hd_get_module_by_name(&g_test_info, name)) {
        printf("[%s] do not support!\n", name);
        return MS_FAILURE;
    }

    g_log = resultCb;
    if (g_test_info->load) {
        return g_test_info->load(param);
    } else {
        return MS_FAILURE;
    }
}

MS_S32
ms_hw_test_stop(void)
{
    if (g_test_info->unload) {
        return g_test_info->unload();
    } else {
        return MS_FAILURE;
    }
}


