/*
 * misc.h
 *
 *  Created on: 2013-7-30
 *      Author: root
 */

#ifndef MISC_H_
#define MISC_H_

#define ALARM_PER_TIME 2.26 //报警一次的时间

enum ring_type {
	RING_KEY = 0,
	RING_ALARM,
	RING_RECORD_FAIL,
	RING_DISK_FULL,
	RING_NETWORK_DISCONN,
	RING_VLOSS,
	RING_MOTION_DETECT,
	RING_DISK_FAIL,
    RING_CAMERAIO,
    RING_VOLUMEDETECT,
};

int fb_ctrl_poweroff(int fd);
int fb_ctrl_poweroff_end(int fd);

int misc_ctrl(int type, int on, int delay);

int misc_init(int *dupfd);

int misc_deinit(void);

#endif /* MISC_H_ */

