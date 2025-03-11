#ifndef __HISI_DM2016_EEPROM_H__
#define __HISI_DM2016_EEPROM_H__

//#define DM2016_ADDR 	0xA0	//N04-00 V1.0
#define DM2016_ADDR 	0xA2 	//N04-00 V1.1

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
#include <linux/i2c.h>
#include <linux/i2c-dev.h> 
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#define I2C_16BIT_REG  0x0709
#define I2C_16BIT_DATA 0x070A


int dm2016_open()
{
	int fd;
	fd = open("/dev/i2c-0", O_RDWR);
	if(fd < 0){
		printf("Unable to open i2c bus 2!!!\n");
		return fd;
	}
//	ioctl(fd, I2C_TIMEOUT, 4);
//	ioctl(fd, I2C_RETRIES, 1);
//	ioctl(fd, I2C_TENBIT, 0);
	ioctl(fd, I2C_SLAVE, DM2016_ADDR);
//	usleep(12000);
	
	return fd;
}

void dm2016_close(int fd)
{
	close(fd);
}

int dm2016_eeprom_read(int fd, unsigned int offset, unsigned int len, unsigned char * data)
{
	int res;
	unsigned int cur_addr;
	unsigned char *buf = data;
	//struct i2c_rdwr_ioctl_data work_queue;
	//struct i2c_msg msg[1];

	if(data == NULL){
		return -1;
	}

	ioctl(fd, I2C_16BIT_REG, 0);
	ioctl(fd, I2C_16BIT_DATA, 0);

	for(cur_addr = offset; cur_addr < offset + len; cur_addr ++){
		buf[cur_addr-offset] = cur_addr & 0xff;
		res = read(fd, buf+cur_addr-offset, 1);
		if(res < 0) {
			printf("dm2016 read addr 0x%x error. \n", cur_addr);
			return res;
		}
	}

	return 0;
}

int dm2016_eeprom_write(int fd, unsigned int offset, unsigned int len, char *data)
{
	int crypt = 0;
	if(data == NULL){
		return -1;
	}
	if(offset == 0x90 && len == 8){
		crypt = 1;
	}

	unsigned int i;
	int res;
	unsigned char buf[32] = {0};
	
	ioctl(fd, I2C_16BIT_REG, 0);
	ioctl(fd, I2C_16BIT_DATA, 0);

	if(crypt > 0){
		for(i = 0; i < len; i ++){
			buf[0] = offset + i;
			buf[1] = data[i];
			res = write(fd, buf, 2);

			if(res < 0){
				perror("write_dm2016_eeprom");
				return res;
			}
		}
		usleep(12000);
		return res;
	}

	for(i = offset; i < offset + len; i ++){
		buf[0] = i;
		buf[1] = data[i - offset];
		//res = ioctl(fd, I2C_RDWR, (unsigned long) &work_queue);
		res = write(fd, buf, 2);
		if(res < 0){
			perror("write_dm2016_eeprom");
			return res;
		}
		usleep(12000);
	}
	return 0;
}

#endif	/* __HISI_DM2016_EEPROM_H__ */
