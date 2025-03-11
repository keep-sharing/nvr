#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chipinfo.h"

int ms_dm2016_open()
{
    return -1;
}
void ms_dm2016_close(int fd)
{
}

int ms_dm2016_eeprom_read(int fd, int offset, int len, unsigned char *data)
{
    return -1;
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

    if (chipinfo_temp == NULL) {
        strncpy(chipinfo, CHIPINFO_DEFAULT, CHIPINFO_LEN);
        printf("There is no Chipinfo in the env. Use default Chipinfo.\n");
    } else {
        strncpy(chipinfo, chipinfo_temp, CHIPINFO_LEN);
    }
    return 0;
}

