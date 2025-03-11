/* 
 * ***************************************************************
 * Filename:      	dio.h
 * Created at:    	2016.01.18
 * Description:   	io ctrl.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
 
#ifndef __DIO_H__
#define __DIO_H__

#ifdef __cplusplus
extern "C" {
#endif


struct fio_info {
	int devid;
	int max_sensor;
	int max_alarm;
	int alarmCtlMode;
};

enum dio_alarm_type {
	ALARM_NO = 0,
	ALARM_NC
};

enum dio_sensor_type {
	SENSOR_NO = 0,
	SENSOR_NC
};

enum dio_notify_type {
	NOTIFY_SENSOR = 0,
	NOTIFY_ALARM
};

enum SENSOR_RESULT_TYPE{
    SENSOR_RESULT_ID_TYPE = 0,
    SENSOR_RESULT_LEVEL_TYPE
};

typedef void (*CALLBACK_NOTIFY)(int type, int id, int evt);

typedef void (* CALLBACK_DIO)(void* arg, int evt);

typedef int (*CALLBACK_WRITE_GUARD2DB)(void *arg, int stat);

struct callback_params {
	CALLBACK_NOTIFY cb_notify;
	CALLBACK_DIO cb_dio;
	CALLBACK_WRITE_GUARD2DB cb_db;
	void *dio_arg;
	void *db_arg;
};

//int dio_init(CALLBACK_DIO diofn, void *dioarg, CALLBACK_WRITE_GUARD2DB dbfn, void *dbarg, struct fio_info *sys);

int dio_init(struct callback_params *cb, struct fio_info *sys);

int dio_deinit(void);

int dio_send_alarm(int id,int enable);

int dio_reverse_guard(int *stat);

void dio_set_sensor(int id, int enable, int type);

void dio_set_alarm(int id, int enable, int type, int delay);

void dio_set_alarm_delay(int id, int delay);

unsigned int dio_get_sensor_status(void);

unsigned int dio_get_alarm_status(void);

void set_sensor_state(unsigned int state);

#ifdef __cplusplus
}
#endif

#endif

