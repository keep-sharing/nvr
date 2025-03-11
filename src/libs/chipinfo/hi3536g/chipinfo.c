#include <stdio.h>
#include "hisi_dm2016_eeprom.h"
#include <stdlib.h>
#include "dlfcn.h"
#include "fw_env.h"
#include "chipinfo.h"

int ms_dm2016_open()
{
    return dm2016_open();
}

void ms_dm2016_close(int fd)
{
    return dm2016_close(fd);
}

int ms_dm2016_eeprom_read(int fd, int offset, int len, unsigned char *data)
{
    return dm2016_eeprom_read(fd, offset, len, data);
}

int ms_env_init(void)
{
    return env_init();
}

int ms_env_deinit(void)
{
    return env_deinit();
}

char *ms_getenv(char *name)
{
    return hs_getenv(name);
}

int ms_setenv(char *name, char *value)
{
    if (hs_setenv(name, value)) {
        return -1;
    }
    return 0;
}

int mschip_write(char *value)
{
    if (value == NULL) {
        return -1;
    }

    if (hs_setenv(CHIPINFO, value)) {
        return -1;
    }

    if (strcmp(hs_getenv(CHIPINFO), value)) {
        return -1;
    }

    return 0;
}

int mschip_read(char *chipinfo)
{
    if (chipinfo == NULL) {
        printf("Chipinfo pointer error!\n");
        return -1;
    }

    char *chipinfo_temp = hs_getenv(CHIPINFO);
    int fd = -1;
    char acBuf[CHIPINFO_LEN + 1] = {0};
    int nResult = 0;
    int i;

    if (chipinfo_temp == NULL) {
        fd = dm2016_open();
        if (fd > 0) {
            nResult = dm2016_eeprom_read(fd, 0, DM2016_VAL_LEN, (unsigned char *)acBuf);
            dm2016_close(fd);
        }

        if (nResult == 0) {
            for (i = DM2016_VAL_LEN; i < CHIPINFO_LEN; i++) {
                acBuf[i] = '0';
            }

            if (mschip_write(acBuf)) {
                printf("Write Chipinfo error.\n");
                nResult = -1;
            } else {
                printf("Recover form dm2016.\n");
                strncpy(chipinfo, acBuf, CHIPINFO_LEN);
                return 0;
            }
        } else if (nResult < 0) {
            strncpy(chipinfo, CHIPINFO_DEFAULT, CHIPINFO_LEN);
            printf("There is no Chipinfo in the env. Use default Chipinfo.\n");
        }
    } else {
        strncpy(chipinfo, chipinfo_temp, CHIPINFO_LEN);
    }
    return 0;
}

