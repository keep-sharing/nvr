/**
 *@file dio_alarm.c
 *@date  2013-07-26: created by chimmu
 *@section
 *-  2013-12-21: modified by lzm
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/prctl.h>
#include "msdb.h"
#include "dio.h"
#include "msdefs.h"
#include "gpio.h"
#include "front.h"

/*----------------------------------------------------------------------------
 Definitions and macro
-----------------------------------------------------------------------------*/
#define GPIO_DEV            "/dev/dvr_io"
#define DIO_CFG_FILE        "/opt/app/dio.cfg"

#define SENSOR_DURATION     1       //60 Sec

#define MAX_IO_NUM          16
typedef struct dio_prm {
    unsigned int    gpio;       ///< alarm id
    unsigned int    dir;        ///< 0:input, 1:output
    unsigned int    trigger;    ///< 1:falling, 2:rising
    unsigned int    val;        ///<
} dvrio_t;

//# ioctl command
#define DVRIO_CMD_INIT      _IOW('g', 1, dvrio_t)
#define DVRIO_CMD_CHG_IRQ   _IOW('g', 2, dvrio_t)
#define DVRIO_CMD_RD        _IOR('g', 3, dvrio_t)

#define GPIO_INPUT          0
#define GPIO_OUTPUT         1
#define RISING              0x00000001
#define FALLING             0x00000002

#define RECORD_SENSOR_BUF_LEN      256
typedef struct {
    unsigned char writeIdx[MAX_IO_NUM];
    unsigned char readIdx[MAX_IO_NUM];
    unsigned char valArr[MAX_IO_NUM][RECORD_SENSOR_BUF_LEN];
}sensor_info_t;
static sensor_info_t sensorInfo = {0};

/*----------------------------------------------------------------------------
 Declares variables
-----------------------------------------------------------------------------*/

unsigned char *dvr_sensor_gpio = NULL;
unsigned char *dvr_alarm_gpio = NULL;
int alarm_flag = 0;

typedef struct sensor_prm {
    int id;
    int enable;
    int type;
    int status;
    int updatetime;
} SENSOR;

typedef struct alarm_prm {
    int id;
    int enable;
    int type;
    int delay;
    int status;
    int updatetime;
} ALARM;

struct dio_ctrl_prm {
//  SENSOR sensor[g_dio_ctrl.sensor_max];
//  ALARM alarm[g_dio_ctrl.alarm_max];
    SENSOR *sensor;
    ALARM *alarm;
    int sensor_max;
    int alarm_max;
    ALARM_CTL_TYPE alarmCtlMode;
    int dio_fd;
    int guard_stat;
    CALLBACK_DIO cb_dio;
    void *dio_arg;
    CALLBACK_WRITE_GUARD2DB cb_db;
    void *db_arg;
    CALLBACK_NOTIFY cb_notify;
    pthread_t task_alarm;
    pthread_t task_sensor;
};

struct dio_ctrl_prm g_dio_ctrl;

pthread_mutex_t g_lock;

time_t cur_time;
/*----------------------------------------------------------------------------
 Declares a function prototype
-----------------------------------------------------------------------------*/
#define FMT_PRODUCT_OFFSET  3
#define FMT_PRODUCT_LENGTH  2
#define FMT_HWVER_OFFSET    6
#define FMT_HWVER_LENGTH    4

/*static int parse_cfg(const char *file)
{
    FILE *fp = NULL;
    char buf[256] = {0}, *p = NULL;
    int i = 0, j = 0;
    char model[10];
    float hw_version,hwmin,hwmax;
    unsigned int match_flag = 0;
    struct device_info info = {0};

    if(db_get_device(SQLITE_FILE_NAME, &info))
    {
        msprintf("get_hardware_info error!\n");
        return -1;
    }

    strcpy(model,info.model);
    hw_version = atof(info.hardver);
    msprintf("Productno : %s ,Hardware version : %.1f\n",model,hw_version);

    if(!strcmp(model,"02"))
        alarm_flag = 1;
    if(alarm_flag)
        return 0;
    if (!(fp = fopen(file, "r"))) {
        msprintf("open %s error", file);
        return -1;
    }
    while (fgets(buf, sizeof(buf), fp)) {
        if (buf[0] == '#')
            continue;
        if (strstr(buf, "PRODUCT")) {
            if (strstr(buf,model))
                match_flag = 1;
            else
                match_flag = 0;
            continue;
        }

        if(match_flag) {
            if (strstr(buf, "hwmin")) {
                if ((p = strrchr(buf, ' '))) {
                    p++;
                    hwmin = atof(p);
                }
                continue;
            }
            if (strstr(buf, "hwmax")) {
                if ((p = strrchr(buf, ' '))) {
                    p++;
                    hwmax = atof(p);
                }
                continue;
            }
            if((hw_version >= hwmin) && (hw_version <= hwmax))
            {
                if (strstr(buf, "sid")) {
                    if ((p = strrchr(buf, ' '))) {
                        p++;
                        dvr_sensor_gpio[i] = (unsigned char)atoi(p);
                        msprintf("dvr_sensor_gpio[%d]: %d, p: %s", i, (int)dvr_sensor_gpio[i],p);
                        i++;
                        continue;
                    }
                }
                if (strstr(buf, "aid")) {
                    if ((p = strrchr(buf, ' '))) {
                        p++;
                        dvr_alarm_gpio[j] = (unsigned char)atoi(p);
                        msprintf("dvr_alarm_gpio[%d]: %d, p: %s", j, (int)dvr_alarm_gpio[j], p);
                        j++;
                    }
                }
            }
        }
    }

    fclose(fp);
    return 0;
}*/

void set_sensor_state(unsigned int state)
{
    int i;
    unsigned char temp;
    static  unsigned char preSensorState[MAX_IO_NUM] = {
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1
    };

    for (i = 0; i < g_dio_ctrl.sensor_max; i++) {
        temp = (state >> i) & 0x01;
        if ((sensorInfo.writeIdx[i] + 1) != sensorInfo.readIdx[i]) {
            if (preSensorState[i] != temp) {
                msprintf("11111 preSensorState[%d]:%d curState:%d", i, preSensorState[i], temp);
                preSensorState[i] = temp;
                sensorInfo.valArr[i][sensorInfo.writeIdx[i]] = temp;
                sensorInfo.writeIdx[i] = (sensorInfo.writeIdx[i] + 1) % RECORD_SENSOR_BUF_LEN;
            }
        } else {
            msprintf("write sensor_state buff full!\n");
            continue;
        }
    }
}

static int get_sensor_state(char idx, enum SENSOR_RESULT_TYPE retType)
{
    unsigned char temp;
    static  unsigned char preSensorState[MAX_IO_NUM] = {
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1
    };
    int level = -1, id = -1;

    if (sensorInfo.readIdx[idx] != sensorInfo.writeIdx[idx]) {
        temp = sensorInfo.valArr[idx][sensorInfo.readIdx[idx]];

        sensorInfo.readIdx[idx] = (sensorInfo.readIdx[idx] + 1) % RECORD_SENSOR_BUF_LEN;
        if (temp != preSensorState[idx]) {
            if (g_dio_ctrl.sensor[idx].type == SENSOR_NO) {//FALLING
                if (preSensorState[idx] == 1 && temp == 0) {
                    id = idx;
                    msprintf("preSensorState[%d]:%d  temp[%d]:%d  type:SENSOR_NO-FALLING", idx, preSensorState[idx],
                                idx, temp);
                }
            } else if (g_dio_ctrl.sensor[idx].type == SENSOR_NC) {//RISING
                if (preSensorState[idx] == 0 && temp == 1) {
                    id = idx;
                    msprintf("preSensorState[%d]:%d  temp[%d]:%d  type:SENSOR_NC-RISING", idx, preSensorState[idx],
                                idx, temp);
                }
            }
        }
        preSensorState[idx] = temp;
        level = temp;
        return (retType == SENSOR_RESULT_ID_TYPE) ? id : level;
    } else {
        return (retType == SENSOR_RESULT_ID_TYPE) ? -1 : preSensorState[idx];
    }
}
static int dio_get_resource(struct fio_info *sys)
{
    struct dio_ctrl_prm *g_ctrl = &g_dio_ctrl;
    g_ctrl->sensor_max = sys->max_sensor;
    g_ctrl->alarm_max = sys->max_alarm;
    g_ctrl->alarmCtlMode = sys->alarmCtlMode;
    if (!(g_ctrl->sensor = calloc(g_ctrl->sensor_max, sizeof(SENSOR)))) {
        msprintf("calloc sensor error");
        goto EXIT;
    }
    if (!(g_ctrl->alarm = calloc(g_ctrl->alarm_max, sizeof(ALARM)))) {
        msprintf("calloc alarm error");
        goto EXIT;
    }
    if (g_ctrl->alarmCtlMode == CPU_CTL) {
        if (!(dvr_sensor_gpio = calloc(MAX_IO_NUM, sizeof(unsigned char)))) {
            msprintf("calloc dvr_sensor_gpio error");
            goto EXIT;
        }
        if (!(dvr_alarm_gpio = calloc(MAX_IO_NUM, sizeof(unsigned char)))) {
            msprintf("calloc dvr_alarm_gpio error");
            goto EXIT;
        }
    }
    /*if (!parse_cfg(DIO_CFG_FILE)) {
        return 0;
    }*/
    return 0;
EXIT:
    if (g_ctrl->sensor) {
        free(g_ctrl->sensor);
    }
    if (g_ctrl->alarm) {
        free(g_ctrl->alarm);
    }
    if (g_ctrl->alarmCtlMode == CPU_CTL) {
        if (dvr_sensor_gpio) {
            free(dvr_sensor_gpio);
        }
        if (dvr_alarm_gpio) {
            free(dvr_alarm_gpio);
        }
        dvr_sensor_gpio = NULL;
        dvr_alarm_gpio = NULL;
    }
    memset(g_ctrl, 0, sizeof(struct dio_ctrl_prm));
    return -1;
}

static int dio_get_sensor_id(char recv)
{
    int i;

    for (i = 0; i < g_dio_ctrl.sensor_max; i++) {
        if (recv == dvr_sensor_gpio[i]) {
            return i;
        }
    }
    return -1;
}

static unsigned int dio_read_sensor(int id)
{
    dvrio_t io;
    int dio_fd = g_dio_ctrl.dio_fd;

    io.gpio = dvr_sensor_gpio[id];
    ioctl(dio_fd, DVRIO_CMD_RD, &io);

    return io.val;
}

static void *dio_task_sensor(void *prm)
{
    char recv;
    int sensoridx;
    int i;

    int dio_fd = g_dio_ctrl.dio_fd;

    if (dio_fd < 0) {
        msprintf("\nerr dvr_io_init\n");
        return NULL;
    }
    ms_task_set_name("task_sensor");
    msprintf("data read start\n");
    while (1) {
        pthread_testcancel();
        if (g_dio_ctrl.sensor_max == 0) {
            usleep(100000);
            continue;
        }
        if (g_dio_ctrl.alarmCtlMode == CPU_CTL) {
            read(dio_fd, &recv, 1);
            sensoridx = dio_get_sensor_id(recv);
        } else if (g_dio_ctrl.alarmCtlMode == MCU_CTL) {
            sensoridx = get_sensor_state(i, SENSOR_RESULT_ID_TYPE);
            i = (i + 1) % g_dio_ctrl.sensor_max;
            if (i != 0 && sensoridx < 0) {
                continue;
            }
        }
        if (sensoridx < 0) {
            usleep(100000);
        } else {
            pthread_mutex_lock(&g_lock);
            if (g_dio_ctrl.sensor[sensoridx].enable) {
                g_dio_ctrl.sensor[sensoridx].status = 0x01;
                if (g_dio_ctrl.cb_notify) {
                    g_dio_ctrl.cb_notify(NOTIFY_SENSOR, sensoridx, 1);
                }
                g_dio_ctrl.sensor[sensoridx].updatetime = cur_time;
            }
            pthread_mutex_unlock(&g_lock);
        }
    }

    return NULL;
}


static void *dio_task_alarm(void *prm)
{
    int i;
    int dio_fd = g_dio_ctrl.dio_fd;
    ALARM *tmp_alarm = g_dio_ctrl.alarm;

    if (g_dio_ctrl.alarmCtlMode == CPU_CTL) {
        if (dio_fd < 0) {
            msprintf("\nerr dvr_io_init\n");
            return NULL;
        }
    }
    ms_task_set_name("task_alarm");
    msprintf("alarm_handler start\n");
    while (1) {
        pthread_testcancel();
        time(&cur_time);
        for (i = 0; i < g_dio_ctrl.alarm_max; i++) {
            if (tmp_alarm[i].enable && (tmp_alarm[i].status & 0x01)) {
                if (cur_time - tmp_alarm[i].updatetime >= (tmp_alarm[i].delay + 1)) {
//                  msprintf("current: %ld, update: %ld, delay: %d", cur_time, tmp_alarm[i].updatetime, tmp_alarm[i].delay);
                    dvrio_t io;
                    memset(&io, 0, sizeof(io));
                    if (g_dio_ctrl.alarmCtlMode == CPU_CTL) {
                        io.gpio = dvr_alarm_gpio[i];
                    }
                    tmp_alarm[i].updatetime = cur_time;
                    if (tmp_alarm[i].type == ALARM_NO) { //NO
                        io.val = 0;
                    } else if (tmp_alarm[i].type == ALARM_NC) { //NC
                        io.val = 1;
                    }
                    pthread_mutex_lock(&g_lock);
                    printf("guard[%d] write dio, val: %d: diofd: %d\n", g_dio_ctrl.guard_stat, io.val, dio_fd);
                    if (g_dio_ctrl.guard_stat) {
                        if (g_dio_ctrl.alarmCtlMode == CPU_CTL) {
                            write(dio_fd, &io, sizeof(io));
                        } else if (g_dio_ctrl.alarmCtlMode == MCU_CTL) {
                            fb_usart_set_alarm_state(i, io.val);
                        }
                        if (g_dio_ctrl.cb_dio) {
                            g_dio_ctrl.cb_dio(g_dio_ctrl.dio_arg, io.val);
                        }
                    } else {
                        io.val = !io.val;
                        if (g_dio_ctrl.alarmCtlMode == CPU_CTL) {
                            write(dio_fd, &io, sizeof(io));
                        }  else if (g_dio_ctrl.alarmCtlMode == MCU_CTL) {
                            fb_usart_set_alarm_state(i, io.val);
                        }
                        if (g_dio_ctrl.cb_dio) {
                            g_dio_ctrl.cb_dio(g_dio_ctrl.dio_arg, io.val);
                        }
                    }
                    pthread_mutex_unlock(&g_lock);
                    tmp_alarm[i].status &= 0x10;
                    if (g_dio_ctrl.cb_notify) {
                        g_dio_ctrl.cb_notify(NOTIFY_ALARM, i, 0);
                    }
                }
            }
        }
        for (i = 0; i < g_dio_ctrl.sensor_max; i++) {
            if ((g_dio_ctrl.sensor[i].status) && (cur_time - g_dio_ctrl.sensor[i].updatetime >= SENSOR_DURATION) &&
                g_dio_ctrl.cb_dio) {

                int sensorval = -1;
                if (g_dio_ctrl.alarmCtlMode == CPU_CTL) {
                    sensorval = dio_read_sensor(i);
                } else if (g_dio_ctrl.alarmCtlMode == MCU_CTL) {
                    sensorval = get_sensor_state(i, SENSOR_RESULT_LEVEL_TYPE);
                    if (sensorval < 0) {
                        continue;
                    }
                }
                if (g_dio_ctrl.sensor[i].type == SENSOR_NO) { //NO
                    if (sensorval == 1) {
                        pthread_mutex_lock(&g_lock);
                        g_dio_ctrl.sensor[i].status &= 0xF0;
                        if (g_dio_ctrl.cb_notify) {
                            g_dio_ctrl.cb_notify(NOTIFY_SENSOR, i, 0);
                        }
                        pthread_mutex_unlock(&g_lock);
                    } else {
                        g_dio_ctrl.sensor[i].updatetime = cur_time;
                        if (g_dio_ctrl.cb_notify) {
                            g_dio_ctrl.cb_notify(NOTIFY_SENSOR, i, 1);
                        }
                    }
                } else if (g_dio_ctrl.sensor[i].type == SENSOR_NC) { //NC
                    if (sensorval == 0) {
                        pthread_mutex_lock(&g_lock);
                        g_dio_ctrl.sensor[i].status &= 0xF0;
                        if (g_dio_ctrl.cb_notify) {
                            g_dio_ctrl.cb_notify(NOTIFY_SENSOR, i, 0);
                        }
                        pthread_mutex_unlock(&g_lock);
                    } else {
                        g_dio_ctrl.sensor[i].updatetime = cur_time;
                        if (g_dio_ctrl.cb_notify) {
                            g_dio_ctrl.cb_notify(NOTIFY_SENSOR, i, 1);
                        }
                    }
                }
            }
        }

        usleep(200000);
    }
    return NULL;
}

int dio_send_alarm(int id, int enable)
{
    if (alarm_flag) {
        return 0;
    }

    //# send command to front
    dvrio_t io;
    int dio_fd;
    msprintf("dio_send_alarm() id=%d enable=%d\n", id, enable);
    int guard_stat = g_dio_ctrl.guard_stat;
    if (g_dio_ctrl.alarmCtlMode == CPU_CTL) {
        dio_fd = g_dio_ctrl.dio_fd;

        if (dio_fd < 0) {
            msprintf("dio_send_alarm\n");
            return -1;
        }
        if (!dvr_alarm_gpio) {
            msprintf("fio not init yet");
            return -1;
        }
        io.gpio = dvr_alarm_gpio[id];
    }

    if (enable && g_dio_ctrl.alarm[id].enable) {
//      if(guard_stat)
//      {
        pthread_mutex_lock(&g_lock);
        if (g_dio_ctrl.alarm[id].type == ALARM_NO) { //NO
            io.val = 1;
            g_dio_ctrl.alarm[id].status |= 0x10;
            g_dio_ctrl.cb_notify(NOTIFY_ALARM, id, 1);
        } else if (g_dio_ctrl.alarm[id].type == ALARM_NC) { //NC
            io.val = 0;
            g_dio_ctrl.alarm[id].status &= 0x01;
            g_dio_ctrl.cb_notify(NOTIFY_ALARM, id, 1);
        }
        if (io.val) {
            if (guard_stat) {
                io.val = 1;
            } else {
                io.val = 0;
            }
        }
        if (g_dio_ctrl.alarmCtlMode == CPU_CTL) {
            write(dio_fd, &io, sizeof(io));
        } else if (g_dio_ctrl.alarmCtlMode == MCU_CTL) {
            fb_usart_set_alarm_state(id, io.val);
        }
        g_dio_ctrl.alarm[id].updatetime = cur_time;
        if (g_dio_ctrl.cb_dio) {
            //if (guard_stat && io.val)
            //  g_dio_ctrl.cb_dio(g_dio_ctrl.dio_arg, io.val);
            //else
            g_dio_ctrl.cb_dio(g_dio_ctrl.dio_arg, io.val);
        }
        pthread_mutex_unlock(&g_lock);
//      }
        g_dio_ctrl.alarm[id].status |= 0x01;
    } else {
        if (g_dio_ctrl.alarm[id].status & 0x01) {
            pthread_mutex_lock(&g_lock);
            if (g_dio_ctrl.alarm[id].type == ALARM_NO) { //NO
                io.val = 0;
                g_dio_ctrl.alarm[id].status &= 0x01;
                g_dio_ctrl.cb_notify(NOTIFY_ALARM, id, 0);
            } else if (g_dio_ctrl.alarm[id].type == ALARM_NC) { //NC
                io.val = 1;
                g_dio_ctrl.alarm[id].status |= 0x10;
                g_dio_ctrl.cb_notify(NOTIFY_ALARM, id, 0);
            }

            if (io.val) {
                if (guard_stat) {
                    io.val = 1;
                } else {
                    io.val = 0;
                }
            }
            if (g_dio_ctrl.alarmCtlMode == CPU_CTL) {
                write(dio_fd, &io, sizeof(io));
            } else if (g_dio_ctrl.alarmCtlMode == MCU_CTL) {
                fb_usart_set_alarm_state(id, io.val);
            }
            if (g_dio_ctrl.cb_dio) {
                //if (guard_stat && io.val)
                //  g_dio_ctrl.cb_dio(g_dio_ctrl.dio_arg, io.val);
                //else
                g_dio_ctrl.cb_dio(g_dio_ctrl.dio_arg, io.val);
            }
            pthread_mutex_unlock(&g_lock);
        }
        g_dio_ctrl.alarm[id].status &= 0x10;
    }
    return 0;
}

static void dio_enable_guard(int flag)
{
#if 0
    int i ;
    if (flag) {
        if (!g_dio_ctrl.guard_stat) {
            g_dio_ctrl.guard_stat = 1;
            for (i = 0; i < g_dio_ctrl.alarm_max; i++) {
                dio_send_alarm(i, 1);
            }
        }
    } else {
        g_dio_ctrl.guard_stat = 0;
        for (i = 0; i < g_dio_ctrl.alarm_max; i++) {
            dio_send_alarm(i, 0);
        }
    }
#endif
    g_dio_ctrl.guard_stat = flag;
}

/**
 * @todo write to db
 */
int dio_reverse_guard(int *stat)
{
    if (alarm_flag) {
        return 0;
    }

    int guard = g_dio_ctrl.guard_stat;
    dio_enable_guard(!guard);
    *stat = g_dio_ctrl.guard_stat;
    if (g_dio_ctrl.cb_db) {
        g_dio_ctrl.cb_db(g_dio_ctrl.db_arg, guard);
    }
    return 0;
}

void dio_set_sensor(int id, int enable, int type)
{
    if (alarm_flag) {
        return ;
    }

    if (id >= g_dio_ctrl.sensor_max) {
        return;
    }
    int dio_fd = g_dio_ctrl.dio_fd;

    g_dio_ctrl.sensor[id].enable = enable;
    if (g_dio_ctrl.sensor[id].type != type) {
        //Reset Sensor Type
        dvrio_t io;

        g_dio_ctrl.sensor[id].type = type;
        io.dir = GPIO_INPUT;
        if (g_dio_ctrl.sensor[id].type == SENSOR_NO) { //NO
            io.trigger = FALLING;
        } else if (g_dio_ctrl.sensor[id].type == SENSOR_NC) { //NC
            io.trigger = RISING;
        }
        if (g_dio_ctrl.alarmCtlMode == CPU_CTL) {
            io.gpio = dvr_sensor_gpio[id];
            ioctl(dio_fd, DVRIO_CMD_CHG_IRQ, &io);
        }
    }
}

void dio_set_alarm(int id, int enable, int type, int delay)
{
    if (alarm_flag) {
        return ;
    }

    if (id >= g_dio_ctrl.alarm_max) {
        return;
    }
    int dio_fd = g_dio_ctrl.dio_fd;
    dvrio_t io;
    ALARM *tmp_alarm = g_dio_ctrl.alarm;
    unsigned char *alarm_gpio;
    if (g_dio_ctrl.alarmCtlMode == CPU_CTL) {
        alarm_gpio = dvr_alarm_gpio;
    }

    tmp_alarm[id].enable = enable;
    if (tmp_alarm[id].enable && tmp_alarm[id].type != type) {
        //Reset Alarm Type
        if (g_dio_ctrl.alarmCtlMode == CPU_CTL) {
            io.gpio = alarm_gpio[id];
        }
        io.dir = GPIO_OUTPUT;
        tmp_alarm[id].type = type;
        if (tmp_alarm[id].type == ALARM_NO) { //NO
            io.val = 0;
        } else if (tmp_alarm[id].type == ALARM_NC) { //NC
            io.val = 1;
        }

        if (io.val) {
            if (!g_dio_ctrl.guard_stat) {
                io.val = 0;
            }
        }
        if (g_dio_ctrl.alarmCtlMode == CPU_CTL) {
            write(dio_fd, &io, sizeof(io));
        } else if (g_dio_ctrl.alarmCtlMode == MCU_CTL) {
            fb_usart_set_alarm_state(id, io.val);
        }
    } else if (!tmp_alarm[id].enable) {
        io.val = 0;
        if (g_dio_ctrl.alarmCtlMode == CPU_CTL) {
            io.gpio = alarm_gpio[id];
            write(dio_fd, &io, sizeof(io));
        } else if (g_dio_ctrl.alarmCtlMode == MCU_CTL) {
            fb_usart_set_alarm_state(id, io.val);
        }
    }
    tmp_alarm[id].delay = delay;
    if (tmp_alarm[id].delay == -1) { //zzf ??????
        tmp_alarm[id].delay = 1999999999;
    }
}

void dio_set_alarm_delay(int id, int delay)
{
    if (alarm_flag) {
        return ;
    }

    g_dio_ctrl.alarm[id].delay = delay;
}

unsigned int dio_get_sensor_status(void)
{
    if (alarm_flag) {
        return 0;
    }

    unsigned int ret = 0;
    int i;

    for (i = 0; i < g_dio_ctrl.sensor_max; i++) {
        if (g_dio_ctrl.sensor[i].status & 0x01) {
            ret |= (0x1 << i);
        }
    }

    return ret;
}

unsigned int dio_get_alarm_status(void)
{
    if (alarm_flag) {
        return 0;
    }

    unsigned int ret = 0;
    int i;
    ALARM *tmp_alarm = g_dio_ctrl.alarm;
    for (i = 0; i < g_dio_ctrl.alarm_max; i++) {
        if (tmp_alarm[i].status & 0x01) {
            ret |= (0x1 << i);
        }
    }

    return ret;
}

//int dio_init(CALLBACK_DIO diofn, void *dioarg, CALLBACK_WRITE_GUARD2DB dbfn, void *dbarg, struct fio_info *sys)
int dio_init(struct callback_params *cb, struct fio_info *sys)
{
//  int fd;
    dvrio_t io;
    int i;

    memset(&g_dio_ctrl, 0, sizeof(g_dio_ctrl));
    if (dio_get_resource(sys)) {
        msprintf("get resource error");
        return -1;
    }
    if (alarm_flag) {
        return 0;
    }

    sleep(2);
    if (g_dio_ctrl.alarmCtlMode == CPU_CTL) {
        if ((g_dio_ctrl.dio_fd = open(GPIO_DEV, O_RDWR)) < 0) {
            perror("open");
            return -1;
        }
        //zbing debug
        dvr_alarm_gpio[0] = GIO_ALARM_OUT_0;
        dvr_alarm_gpio[1] = GIO_ALARM_OUT_1;
        dvr_alarm_gpio[2] = GIO_ALARM_OUT_2;
        dvr_alarm_gpio[3] = GIO_ALARM_OUT_3;

        dvr_sensor_gpio[0] = GIO_ALARM_IN_0;
        dvr_sensor_gpio[1] = GIO_ALARM_IN_1;
        dvr_sensor_gpio[2] = GIO_ALARM_IN_2;
        dvr_sensor_gpio[3] = GIO_ALARM_IN_3;
        dvr_sensor_gpio[4] = GIO_ALARM_IN_4;
        dvr_sensor_gpio[5] = GIO_ALARM_IN_5;
        dvr_sensor_gpio[6] = GIO_ALARM_IN_6;
        dvr_sensor_gpio[7] = GIO_ALARM_IN_7;
        dvr_sensor_gpio[8] = GIO_ALARM_IN_8;
        dvr_sensor_gpio[9] = GIO_ALARM_IN_9;
        dvr_sensor_gpio[10] = GIO_ALARM_IN_10;
        dvr_sensor_gpio[11] = GIO_ALARM_IN_11;
        dvr_sensor_gpio[12] = GIO_ALARM_IN_12;
        dvr_sensor_gpio[13] = GIO_ALARM_IN_13;
        dvr_sensor_gpio[14] = GIO_ALARM_IN_14;
        dvr_sensor_gpio[15] = GIO_ALARM_IN_15;
        //end debug

        for (i = 0; i < g_dio_ctrl.alarm_max; i++) {
            io.gpio = dvr_alarm_gpio[i];
            io.dir = GPIO_OUTPUT;
            io.val = 0;
            ioctl(g_dio_ctrl.dio_fd, DVRIO_CMD_INIT, &io);
            g_dio_ctrl.alarm[i].id = i;
        }
        for (i = 0; i < g_dio_ctrl.sensor_max; i++) {
            io.gpio = dvr_sensor_gpio[i];
            io.dir = GPIO_INPUT;
            io.trigger = FALLING;
            ioctl(g_dio_ctrl.dio_fd, DVRIO_CMD_INIT, &io);
            g_dio_ctrl.sensor[i].id = i;
        }
    } else if (g_dio_ctrl.alarmCtlMode == MCU_CTL) {
        for (i = 0; i < g_dio_ctrl.alarm_max; i++) {
            g_dio_ctrl.alarm[i].id = i;
        }
        for (i = 0; i < g_dio_ctrl.sensor_max; i++) {
            g_dio_ctrl.sensor[i].id = i;
        }
    }
    g_dio_ctrl.cb_dio = cb->cb_dio;
    g_dio_ctrl.dio_arg = cb->dio_arg;
    g_dio_ctrl.cb_db = cb->cb_db;
    g_dio_ctrl.db_arg = cb->db_arg;
    g_dio_ctrl.cb_notify = cb->cb_notify;
    g_dio_ctrl.guard_stat = 1;
    pthread_mutex_init(&g_lock, NULL);
    if (pthread_create(&g_dio_ctrl.task_alarm, NULL, dio_task_alarm, NULL) < 0) {
        perror("pthread_create");
        return -1;
    }
    if (pthread_create(&g_dio_ctrl.task_sensor, NULL, dio_task_sensor, NULL) < 0) {
        perror("pthread_create");
        return -1;
    }
    return 0;

}

int dio_deinit(void)
{
    if (alarm_flag) {
        return 0;
    }

    struct dio_ctrl_prm *g_ctrl = &g_dio_ctrl;
    pthread_cancel(g_ctrl->task_alarm);
    pthread_cancel(g_ctrl->task_sensor);
    pthread_mutex_destroy(&g_lock);
    close(g_ctrl->dio_fd);
    if (g_ctrl->sensor) {
        free(g_ctrl->sensor);
    }
    if (g_ctrl->alarm) {
        free(g_ctrl->alarm);
    }
    if (g_dio_ctrl.alarmCtlMode == CPU_CTL) {
        if (dvr_sensor_gpio) {
            free(dvr_sensor_gpio);
        }
        if (dvr_alarm_gpio) {
            free(dvr_alarm_gpio);
        }
        dvr_sensor_gpio = NULL;
        dvr_alarm_gpio = NULL;
    }
    memset(g_ctrl, 0, sizeof(struct dio_ctrl_prm));
    return 0;
}

#if 0
int main(void)
{
    struct sysinfo sys = {0};
    sys.resource.max_sensor = 16;
    sys.resource.max_alarm = 4;
    dio_init(NULL, NULL, NULL, NULL, &sys);
    while (1) {
        if (getchar() == 'q') {
            break;
        }
        usleep(6000);
    }
    dio_deinit();
    return 0;
}
#endif

