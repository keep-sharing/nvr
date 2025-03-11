/**
 *@file fio.c
 *@date Created on: 2016-01-18
 *@author zbing
 *@todo
 */
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "msstd.h"
#include "msdb.h"
#include "msg.h"
#include "msdefs.h"
#include "mssocket.h"
#include "front.h"
#include "dio.h"
#include "gpio.h"

// unsigned int global_debug_mask = DEBUG_LEVEL_DEFAULT;
// hook_print global_debug_hook = 0;

//#define FIO_DEBUG 1
#define PRINT

static int g_stop = 0;

static struct fio_info g_info = {0};
static int g_power_key = 1;
static enum power_en g_power_type = POWER_IO;
/**
 * @todo deal with received message
 */
static int getmsg = 0;

static int trans_ring_type(int ring)
{
    int result = A_ALARM;
    switch (ring) {
        case FAT_ALARMOUT:
            result = A_ALARM;
            break;
        case FAT_DISK_FULL:
            result = A_DISK_FULL;
            break;
        case FAT_RECORD_FAIL:
            result = A_RECORD_FAIL;
            break;
        case FAT_VIDEO_LOSS:
            result = A_VLOSS;
            break;
        case FAT_NETWORK_DISCONN:
            result = A_NETWORK_DISCONN;
            break;
        case FAT_DISK_NULL:
            result = A_NETWORK_DISCONN;
            break;
        case FAT_DISK_UNINIT:
            result = A_NETWORK_DISCONN;
            break;
        case FAT_MOTION_DETECT:
            result = A_MOTION_DETECT;
            break;
        case FAT_DISK_FAIL:
            result = A_DISK_FAIL;
            break;
        case FAT_CAMERAIO:
            result = A_CAMERAIO;
            break;
        case FAT_VOLUMEDETECT:
            result = A_VOLUMEDETECT;
            break;
        default:
            break;
    }
    return result;
}

static int check_is_8000(void)
{
    FILE *pfd = NULL;
    char buff[64];
    int result = 0;

    pfd = popen("mschip_update", "r");
    if (pfd != NULL) {
        if (fgets(buff, sizeof(buff), pfd) != NULL) {
            if (strstr(buff, "MSN8")) {
                result = 1;
            } else {
                result = 0;
            }
        }
        pclose(pfd);
    }

    return result;

}

#if defined(_HI3536A_)
static int check_is_7000(void)
{
    FILE *pfd = NULL;
    char buff[64];
    int result = 0;

    pfd = popen("mschip_update", "r");
    if (pfd != NULL) {
        if (fgets(buff, sizeof(buff), pfd) != NULL) {
            if (strstr(buff, "MSN7")) {
                result = 1;
            } else {
                result = 0;
            }
        }
        pclose(pfd);
    }

    return result;

}
#endif

static void fio_recv_msg(void *arg, struct packet_header *hdr, void *data)
{
    int devid, key1, key2;
    struct snd_stat *snd;
    struct rs485_info *rs;
    struct front_alarm *fa;
    struct dio_alarm *da;
    struct dio_alarm_attr *daa;
    struct dio_alarm_delay *dio_delay;
    struct dio_sensor_attr *sa;
    struct device_info *sys;
    unsigned int status;

    if (!hdr || !data /*|| hdr->from != SOCKET_TYPE_CORE*/) {
        return;
    }
    //msdebug(DEBUG_INF, "FIO:Receive New MSG: type:%d, count:%d, from:%d, size:%d\n", hdr->type, hdr->count, hdr->from, hdr->size);
    switch (hdr->type) {

        case FIOREQ_FRONT_SET_DEVID:
            devid = *((int *)data);
            fb_set_dev_id(devid);
            break;

        case FIOREQ_FRONT_POWEROFF:
            fb_send2front(SYSTEM_POWEROFF);
            //g_stop = 1;
            break;

        case FIOREQ_FRONT_REBOOT:
            fb_send2front(SYSTEM_REBOOT);
            //g_stop = 1;
            break;

        case FIOREQ_FRONT_SET_LED_READY:
            fb_send2front(SEND_LED_READY_GREEN);
            break;

        case FIOREQ_FRONT_SET_RECORD:
            fb_send2front(RECORD_START);
            break;

        case FIOREQ_FRONT_STOP_RECORD:
            fb_send2front(RECORD_STOP);
            break;

        case FIOREQ_FRONT_SET_SOUND:
            snd = (struct snd_stat *)data;
            fb_set_buzz_mode(snd);
            break;

        case FIOREQ_FRONT_SET_RS485:
            rs = (struct rs485_info *)data;
            fb_set_rs485(rs);
            break;

        case FIOREQ_FRONT_SET_ALARM: {
            fa = (struct front_alarm *)data;
            int res = trans_ring_type(fa->type);
            fb_set_alarm(res, fa->enable, fa->delay);
        }
        break;

        case FIOREQ_DIO_GET_ALARM_STATUS:
            status = dio_get_alarm_status();
            ms_sock_send_msg(SOCKET_TYPE_CORE, FIORESP_DIO_ALARM_STATUS, &status, sizeof(status), 0);
            break;

        case FIOREQ_DIO_GET_SENSOR_STATUS:
            status = dio_get_sensor_status();
            ms_sock_send_msg(SOCKET_TYPE_CORE, FIORESP_DIO_SENSOR_STATUS, &status, sizeof(status), 0);
            break;

        case FIOREQ_DIO_SEND_ALARM:
            da = (struct dio_alarm *)data;
            dio_send_alarm(da->id, da->enable);
            break;

        case FIOREQ_DIO_SET_ALARM:
            daa = (struct dio_alarm_attr *)data;
            dio_set_alarm(daa->id, daa->enable, daa->type, daa->delay);
            break;

        case FIOREQ_DIO_SET_ALARM_DELAY:
            dio_delay = (struct dio_alarm_delay *)data;
            dio_set_alarm_delay(dio_delay->id, dio_delay->delay);
            break;

        case FIOREQ_DIO_SET_SENSOR:
            sa = (struct dio_sensor_attr *)data;
            dio_set_sensor(sa->id, sa->enable, sa->type);
            break;

        case FIOREQ_QUIT:
            g_stop = 1;
            break;

        case RESPONSE_FLAG_GET_SYSINFO:
            sys = (struct device_info *)data;
            g_info.devid = sys->devid;
            g_info.max_alarm = sys->max_alarm_out;
            g_info.max_sensor = sys->max_alarm_in;
            g_info.alarmCtlMode = sys->alarm_ctl_mode;
            g_power_key = get_param_int(SQLITE_FILE_NAME, PARAM_POWER_KEY, 0);
            if (get_param_int(SQLITE_FILE_NAME, PARAM_POE_NUM, 0) == POE_24_CHAN) {
                g_power_type = POWER_USART;
            } else {
                if (getmsg) {
                    break;
                }
#if !defined(_HI3536A_)
                key1 = gpio_read(GIO_MCU_P1_1);
                gpio_write(GIO_MCU_P1_0, key1);
                usleep(100 * 1000);
                key2 = gpio_read(GIO_MCU_P1_1);
                gpio_read(GIO_MCU_P1_0);//set input mode
                //printf("key1:[%d] key2:[%d]\n", key1, key2);
                if ((key1 != key2) && check_is_8000()) {
#else
                 if (check_is_8000() || check_is_7000()) {
#endif
                    g_power_type = POWER_USART;
                } else {
                    g_power_type = POWER_IO;
                }
            }
            getmsg = 1;
            break;

        default:
            break;
    }
}

static int fio_reverse_guard(void *arg, int *stat)
{
    int guard = dio_reverse_guard(stat);
    if (!guard) {
        fb_send2front(SEND_LED_ALARM_OFF);
        fb_set_alarm(0, 0, 0);
    }
    return guard;
}

static int fio_write_guard2db(void *arg, int guard_stat)
{
    return 0;
}

static int fio_send_power2gui(void *arg, int type)
{
    int resp, data = 0;

    if (type == POWER_LONG) {
        resp = FIORESP_FRONT_POWER_LONG;
    } else {
        resp = FIORESP_FRONT_POWER_SHORT;
    }
#ifndef FIO_DEBUG
    ms_sock_send_msg(SOCKET_TYPE_CORE, resp, &data, sizeof(data), 0);
#endif
    return 0;
}

static void fio_reserved(void *arg, int evt)
{
    if (evt) {
        //@todo turn on the alarm sound of front board
        msdebug(DEBUG_INF, "turn alarm light on");
        fb_send2front(SEND_LED_ALARM_ON);
//      fb_set_alarm(A_ALARM, 1, 5);
    } else {
        //@todo turn off the alarm sound of front board
        msdebug(DEBUG_INF, "turn alarm light off");
        fb_send2front(SEND_LED_ALARM_OFF);
//      fb_set_alarm(0, 0, 0);
    }
}


static int fio_write_snd2db(void *arg, struct snd_stat *stat)
{
    return 0;
}

static void fio_read_db(void)
{
    struct alarm_in alarms[MAX_ALARM_IN];
    struct alarm_out outs[MAX_ALARM_OUT];
    int cnt, i;

    if (g_info.max_alarm == 0 && g_info.max_sensor == 0) {
        return;
    }
    read_alarm_ins(SQLITE_FILE_NAME, alarms, &cnt);
    for (i = 0; i < cnt; i++) {
        dio_set_sensor(alarms[i].id, 1, alarms[i].type);
    }
    read_alarm_outs(SQLITE_FILE_NAME, outs, &cnt);
    for (i = 0; i < cnt; i++) {
        msdebug(DEBUG_INF, "id: %d, enable: %d, type: %d, delay: %d\n", outs[i].id, outs[i].enable, outs[i].type,
                outs[i].duration_time);
        dio_set_alarm(outs[i].id, 1, outs[i].type, outs[i].duration_time);
    }
}

static void fio_notify(int type, int id, int evt)
{
    struct dio_alarm_notify notify = {0};
    int msg = 0, change = 0;
    static int sensor_evt[MAX_ALARM_IN] = {0}, alarm_evt[MAX_ALARM_OUT] = {0};

    notify.evt = evt;
    notify.id = id;
    if (type == NOTIFY_SENSOR) {
        if (sensor_evt[id] != evt) {
            sensor_evt[id] = evt;
            change = 1;
        }
        msg = FIORESP_DIO_SENSOR_NOTIFY;
    } else {
        if (alarm_evt[id] != evt) {
            alarm_evt[id] = evt;
            change = 1;
        }
        msg = FIORESP_DIO_ALARM_NOTIFY;
    }
#ifndef FIO_DEBUG
    if (change) {
        ms_sock_send_msg(SOCKET_TYPE_CORE, msg, &notify, sizeof(notify), 0);
    }
#endif
}

int fio_init(void)
{
    struct snd_stat snd_stat = {0};
    struct callback_prm cb = {0};
    struct callback_params dio_cb = {0};
    int devid = 255;

    msprintf("fio init\n");
#ifndef FIO_DEBUG
    ms_sock_init(fio_recv_msg, NULL, SOCKET_TYPE_FIO, SOCK_MSSERVER_PATH);

    for (;;) {
        ms_sock_send_msg(SOCKET_TYPE_CORE, REQUEST_FLAG_GET_SYSINFO, NULL, 0, 0);
        usleep(500 * 1000);
        if (getmsg) {
            break;
        }
    }
#else
    g_info.devid = 255;
    g_info.max_alarm = 4;
    g_info.max_sensor = 16;
#endif

    devid = g_info.devid;
    cb.cb_guard = fio_reverse_guard;
    cb.guard_arg = NULL;
    cb.cb_snd = fio_write_snd2db;
    cb.snd_arg = NULL;
    cb.cb_pwr = fio_send_power2gui;
    cb.pwr_arg = NULL;
    if (fb_init(&cb, &snd_stat, devid, g_power_key, g_power_type)) {
        msprintf("fb_init error\n");
    }

    dio_cb.cb_db = fio_write_guard2db;
    dio_cb.cb_dio = fio_reserved;
    dio_cb.cb_notify = fio_notify;
    dio_cb.db_arg = NULL;
    dio_cb.dio_arg = NULL;

    if (dio_init(&dio_cb, &g_info)) {
        msprintf("dio_init error\n");
    }
    fio_read_db();

    sleep(3);
    fb_send2front(SEND_LED_READY_GREEN);

    msprintf("fio init done\n");
    return 0;
}

int fio_deinit(void)
{
    fb_deinit();
    dio_deinit();
    ms_sock_uninit();

    return 0;
}

static void signal_handler(int signal)
{
    switch (signal) {
        case SIGINT:
            fb_send2front(SYSTEM_POWEROFF_END);//bruce.milesight add for shutdown
            break;
        default:
            break;
    }
    g_stop = 1;
    getmsg = 1;
}

static void set_test_signal()
{
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGPIPE, SIG_IGN);//notyet
}

int main(int argc, char **argv)
{
#ifdef FIO_DEBUG
    int i;
#endif
    int timer_cnt = 0;
    nice(15);
    set_test_signal();
    fio_init();

    while (!g_stop) {
#ifdef FIO_DEBUG
        if (timer_cnt % 10 == 0) {
            if (getchar() == 'a') {
                for (i = 0; i < 4; i++) {
                    dio_send_alarm(i, 1);
                }
            } else if (getchar() == 'r') {
                msprintf("set alarm");
                fb_set_alarm(0, 1, 10);
            } else if (getchar() == 's') {
                fb_send2front(SYSTEM_POWEROFF);
                system("reboot");
            } else if (getchar() == 'q') {
                g_stop = 1;
            }
        }
#endif
        if (timer_cnt % 300 == 0) {
            global_debug_mask = (unsigned int)get_param_int(SQLITE_FILE_NAME, PARAM_DEBUG_LEVEL, DEBUG_LEVEL_DEFAULT);
        }
        timer_cnt++;
        usleep(100 * 1000);
    }

    fio_deinit();
    sleep(1);
    return 0;
}
