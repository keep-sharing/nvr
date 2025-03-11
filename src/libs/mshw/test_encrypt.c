#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "hardware.h"

static int test_start(void *param)
{
    char ver[64] = {0};
    if (!get_hardware_version(ver)) {
        hdinfo(ver);
    }

    return 0;
}

static int test_stop()
{
    return 0;
}

static int test_find()
{
    return 1;
}

HD_MODULE_INFO(TEST_NAME_ENCRYPT, test_find, test_start, test_stop);

