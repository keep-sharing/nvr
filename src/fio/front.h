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


#define RECORD_START	0x1    ///< 开始录像命令，对应前面板REC灯
#define RECORD_STOP		0x2   ///< 停止录像

#define SYSTEM_REBOOT	0x3   ///< 重启，在PC机上测试此命令，将会重启系统，因为内部调用了system("reboot")
#define SYSTEM_POWEROFF	0x4    ///< 关闭电源，调用此命令前必须自己执行关闭电源程序，跟SEND_SYSTEM_REBOOT不一样，发送SEND_SYSTEM_REBOOT自动执行了reboot
#define SYSTEM_POWEROFF_END 0x5

#define SEND_LED_ALARM_OFF	0x0b   ///< 报警命令
#define SEND_LED_ALARM_ON	0x0c

#define SEND_LED_READY_GREEN	0x0d    ///< 就绪灯命令
#define SEND_LED_READY_RED	0x0e

#define SEND_GUARD_ON	0x0f		///< 布防灯命令
#define SEND_GUARD_OFF	0x10

#define BUZZ_ON		1 ///< 前面板按键声音
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

/**!改变布防状态回调函数*/
typedef int (*CALLBACK_REVERSE_GUARD_STAT)(void *arg, int *guard_stat);

/**!保存前面板、遥控器、键盘声音状态到数据库*/
typedef int (*CALLBACK_SAVE_SOUND_STAT)(void *arg, struct snd_stat *stat);

/**!电源键长短按处理*/
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
 * @brief 初始化
 */
int fb_init(struct callback_prm *callback, struct snd_stat *stat, int dev_id, int guard_stat, enum power_en  pwr);

/**
 * @brief 退出
 */
void fb_deinit(void);

/**
 * @brief 设置报警声音
 */
void fb_set_alarm(int type, int enable, int delay);

/**
 * @brief 设置前面板声音启停,目前只实现前面板及遥控器
 */
void fb_set_buzz_mode(struct snd_stat *stat);

/**
 * @brief 设置设备id号
 */
void fb_set_dev_id(int id_value);

/**
 * @brief 向前面板发送状态灯改变等命令
 */
int fb_send2front(int cmd);

/**
 * @brief 设置串口信息,对于DVR,只实现设置波特率
 */
int fb_set_rs485(struct rs485_info *rs);

/**
 * @brief 通过串口向前面板发送命令控制单片机开始关机
 */
int fb_usart_poweroff(int fd);

/**
 * @brief 通过串口向前面板发送命令控制单片机立即关机
 */
int fb_usart_poweroff_end(int fd);

void fb_usart_set_alarm_state(int id, int value);

#ifdef __cplusplus
}
#endif

#endif /* FRONT_PUBLIC_H_ */

