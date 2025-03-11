#ifndef __HISI_DM2016_EEPROM_H__
#define __HISI_DM2016_EEPROM_H__

//#define DM2016_ADDR 	0xA0	//N04-00 V1.0
#define DM2016_ADDR 	0xA2 	//N04-00 V1.1
#define I2C_DEV_DM2016   1

#define EEPROM_LEN 	128

int dm2016_open();
void dm2016_close(int fd);
int dm2016_eeprom_read(int fd, unsigned int offset, unsigned int len, unsigned char * data);
int dm2016_eeprom_write(int fd, unsigned int offset, unsigned int len, char *data);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "hi_unf_i2c.h"

int dm2016_open()
{
	int ret = 0;
	
    ret = HI_UNF_I2C_Init();
	if(ret != 0){
		printf("HI_UNF_I2C_Init failed with %#x!!!\n", ret);
		return -1;
	}

	return 1;
}

void dm2016_close(int fd)
{
    HI_UNF_I2C_DeInit();
}

int dm2016_eeprom_read(int fd, unsigned int offset, unsigned int len, unsigned char * data)
{
	int ret = 0;
	
    ret = HI_UNF_I2C_Read(I2C_DEV_DM2016, DM2016_ADDR, offset, 1, data, len);
	if(ret != 0){
		printf("HI_UNF_I2C_Read failed with %#x!!!\n", ret);
		return -1;
	}

	return 0;
}

int dm2016_eeprom_write(int fd, unsigned int offset, unsigned int len, char *data)
{   
    unsigned int i;
    int ret = 0;
	for(i = offset; i < offset + len; i ++){
        ret = HI_UNF_I2C_Write(I2C_DEV_DM2016, DM2016_ADDR, i, 1, (unsigned char *)data, 1);
        if (ret != 0)
        {
            printf("HI_UNF_I2C_Write failed with %#x!!!\n", ret);
            return -1;
        }
        data++;
		usleep(12000);
	}
	return 0;
}

#endif	/* __HISI_DM2016_EEPROM_H__ */
