#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include "kb_util.h"

U64 get_now_time_ms(void)
{
    struct timeval stTime;

    gettimeofday(&stTime, NULL);

    return (stTime.tv_sec * 1000LLU) + stTime.tv_usec / 1000;
}


