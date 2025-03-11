/*
 * ***************************************************************
 * Filename:        mswh.h
 * Created at:      2021.11.15
 * Description:     ms iniparser api.
 * Author:          zbing&solin
 * Copyright (C)    milesight
 * ***************************************************************
 */

#ifndef __MSHW_H__
#define __MSHW_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_HD_MODULE 16

#define TEST_NAME_ALARMIN   "ALARMIN"
#define TEST_NAME_ALARMOUT  "ALARMOUT"
#define TEST_NAME_LED       "LED"
#define TEST_NAME_FAN       "FAN"
#define TEST_NAME_MCU       "MCU"
#define TEST_NAME_BUZZER    "BUZZER"
#define TEST_NAME_AUDIO     "AUDIO"
#define TEST_NAME_VIDEO     "VIDEO"
#define TEST_NAME_IR        "IR"
#define TEST_NAME_RS485     "RS485"
#define TEST_NAME_ENCRYPT   "ENCRYPT"
#define TEST_NAME_FLASH     "FLASH"
#define TEST_NAME_DISK      "DISK"

typedef enum LogLevel {
    LOG_NONE = 0,
    LOG_INFO,
    LOG_ERR,
    LOG_SUCCESS,
    LOG_NOTE,
}LogLevel;
    
typedef void (* ResultCb)(const char *mod, const char *log, LogLevel flag);

typedef enum GpioModule {
    GPIO_MOD_ALARM_IN = 1,
    GPIO_MOD_ALARM_OUT,
    GPIO_MOD_LED,
    GPIO_MOD_MCU,
    GPIO_MOD_FAN,
} GpioModule;

typedef enum Rs485Module {
    FULL_TX_HALF_RX = 1,
    FULL_RX_HALF_TX,
} Rs485Module;

typedef enum VideoFormat {
    VIDEO_FORMAT_H264 = 1,
    VIDEO_FORMAT_H265,
} VideoFormat;

typedef struct TestGpio {
    GpioModule mod;
} TestGpio;

typedef struct TestRs485 {
    Rs485Module mod;
} TestRs485;

typedef struct TestVideo {
    char ip[32];
    char user[32];
    char password[32];
    VideoFormat format;
} TestVideo;

typedef struct TestList {
    int count;
    char entrys[MAX_HD_MODULE][16];
} TestList;

/**
* \brief ms_hw_test_get_list.
* \param[in] None
* \param[out] pTestList : entry and count;
* \return MS_SUCCESS for count > 0, MS_FAILURE for cont == 0.
*/
MS_S32
ms_hw_test_get_list(TestList *pTestList);

/**
* \brief hardware test start after ms_hw_test_get_list.
* \param[in] name : TEST_NAME_XXX
* \param[in] resultCb : for returning result logs
* \param[in] param : auto-mode:NULL;  user-mode:struct TestXXX;
* \return MS_SUCCESS for success, MS_FAILURE for failure.
*/
MS_S32
ms_hw_test_start(const char *name, ResultCb resultCb, void *param);

/**
* \brief ms_hw_test_stop.
* \param[in] None
* \return MS_SUCCESS for success, MS_FAILURE for failure.
*/
MS_S32
ms_hw_test_stop(void);

#ifdef __cplusplus
}
#endif

#endif

