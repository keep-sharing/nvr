/* 
 * ***************************************************************
 * Filename:      	front.h
 * Created at:    	2016.01.18
 * Description:   	front board ctrl.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
 
#ifndef __FRONT_H__
#define __FRONT_H__

#ifdef __cplusplus
extern "C" {
#endif


#define RECORD_START	0x1    ///< ��ʼ¼�������Ӧǰ���REC��
#define RECORD_STOP		0x2   ///< ֹͣ¼��

#define SYSTEM_REBOOT	0x3   ///< ��������PC���ϲ��Դ������������ϵͳ����Ϊ�ڲ�������system("reboot")
#define SYSTEM_POWEROFF	0x4    ///< �رյ�Դ�����ô�����ǰ�����Լ�ִ�йرյ�Դ���򣬸�SEND_SYSTEM_REBOOT��һ��������SEND_SYSTEM_REBOOT�Զ�ִ����reboot
#define SYSTEM_POWEROFF_END 0x5

#define SEND_LED_ALARM_OFF	0x0b   ///< ��������
#define SEND_LED_ALARM_ON	0x0c

#define SEND_LED_READY_GREEN	0x0d    ///< ����������
#define SEND_LED_READY_RED	0x0e

#define SEND_GUARD_ON	0x0f		///< ����������
#define SEND_GUARD_OFF	0x10

#define BUZZ_ON		1 ///< ǰ��尴������
#define BUZZ_OFF	0

struct rs485_info {
	int baudrate;
	int databit;
	int paritybit;
	int stopbit;
};

struct snd_stat {
	int front;
	int ir;
	int keyboard;
};

enum power_state {
	POWER_LONG = 0,
	POWER_SHORT
};

enum power_en{
    POWER_IO = 0,
    POWER_USART,
};

enum alarm_type {
	A_ALARM = 0,
	A_VLOSS,
	A_NETWORK_DISCONN,
	A_DISK_FULL,
	A_RECORD_FAIL,
	A_MOTION_DETECT,
    A_CAMERAIO,
    A_VOLUMEDETECT,
	A_DISK_FAIL,
	A_MAX
};

#if defined(_HI3536A_)
typedef enum{
    CMD_NONE = -1,
    CMD_A = 0x01,
    CMD_B = 0x03,
    CMD_C = 0x05,
    CMD_D = 0x07,
    CMD_E = 0x09,
    CMD_F = 0x0B,
    CMD_ALARM_OUT = 0x0C,
    CMD_ALARM_IN = 0x0D
}CMD_EN;
#else
typedef enum{
    CMD_NONE = -1,
    CMD_A = 0,
    CMD_B,
    CMD_C,
    CMD_D,
    CMD_E,
    CMD_F,
    CMD_NUM,
}CMD_EN;
#endif

typedef enum {
    MS_FREAM_NONE = -1,
    MS_FREAM_HEAD = 0,
    MS_FREAM_VALUE,
} MS_FREAM_FIELD;

/**!�ı䲼��״̬�ص�����*/
typedef int (*CALLBACK_REVERSE_GUARD_STAT)(void *arg, int *guard_stat);

/**!����ǰ��塢ң��������������״̬�����ݿ�*/
typedef int (*CALLBACK_SAVE_SOUND_STAT)(void *arg, struct snd_stat *stat);

/**!��Դ�����̰�����*/
typedef int (*CALLBACK_DEAL_WITH_POWER_STAT)(void *arg, int stat);

typedef struct callback_prm {
	CALLBACK_REVERSE_GUARD_STAT cb_guard;
	void *guard_arg;
	CALLBACK_SAVE_SOUND_STAT cb_snd;
	void *snd_arg;
	CALLBACK_DEAL_WITH_POWER_STAT cb_pwr;
	void *pwr_arg;
}CALLBACK_PRM;

/**
 * @brief ��ʼ��
 */
int fb_init(struct callback_prm *callback, struct snd_stat *stat, int dev_id, int guard_stat, enum power_en  pwr);

/**
 * @brief �˳�
 */
void fb_deinit(void);

/**
 * @brief ���ñ�������
 */
void fb_set_alarm(int type, int enable, int delay);

/**
 * @brief ����ǰ���������ͣ,Ŀǰֻʵ��ǰ��弰ң����
 */
void fb_set_buzz_mode(struct snd_stat *stat);

/**
 * @brief �����豸id��
 */
void fb_set_dev_id(int id_value);

/**
 * @brief ��ǰ��巢��״̬�Ƹı������
 */
int fb_send2front(int cmd);

/**
 * @brief ���ô�����Ϣ,����DVR,ֻʵ�����ò�����
 */
int fb_set_rs485(struct rs485_info *rs);

/**
 * @brief ͨ��������ǰ��巢��������Ƶ�Ƭ����ʼ�ػ�
 */
int fb_usart_poweroff(int fd);

/**
 * @brief ͨ��������ǰ��巢��������Ƶ�Ƭ�������ػ�
 */
int fb_usart_poweroff_end(int fd);

void fb_usart_set_alarm_state(int id, int value);

#ifdef __cplusplus
}
#endif

#endif /* FRONT_PUBLIC_H_ */

