/**
 *@file front_board.c
 *@date  Created on: 2013-8-6
 *@author: chimmu
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/prctl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "msstd.h"
#include "front.h"
#include "misc.h"
#include "gpio.h"
#include "rs485.h"
#include "dio.h"
#ifdef _HI3798_
    #include "hi_unf_pmoc.h"
#endif
#define UINPUT_DEV  "/dev/uinput"

#if defined(_HI3536A_)
#define RS_DEV  "/dev/ttyAMA2"
#define USART_DEV   "/dev/ttyAMA1"
#elif defined(_HI3536_)
#define RS_DEV  "/dev/ttyAMA3"
#define USART_DEV   "/dev/ttyAMA2"
#endif

#define LIRCD   "/var/run/lirc/lircd"
#define KB_MSG_LEN  6

#define PRESS   1
#define RELEASE 0
#define CMD_LEN 3
#define CMD_NUM 6

static struct timeval   g_tty_select_tmout;
static fd_set           g_fs_read[2];


//Ö¸ÁîA:0xAA+0x01+0x55 :CPUÆô¶¯Íê³ÉºóÍ¨Öªµ¥Æ¬»ú
//Ö¸ÁîB:0xAA+0x03+0x55 :¶Ì°´Ò»´Îºóµ¥Æ¬»úÍ¨ÖªCPUÊÇ·ñÒª¹Ø»ú
//Ö¸ÁîC:0xAA+0x05+0x55 :³¤°´»òÕßÁ¬Ðø¶Ì°´Á½´Îºó£¬µ¥Æ¬»úÍ¨ÖªCPUÇ¿ÖÆ¹Ø»ú
//Ö¸ÁîD:0xAA+0x07+0x55 :CPU×¼±¸ºÃ¹Ø»úºóÍ¨Öªµ¥Æ¬»úÐèÒª¶Ïµç
//Ö¸ÁîE:0xAA+0x09+0x55 :QTÑ¡ÔñÖØÆôºó£¬CPU·¢ËÍÖØÆôÃüÁî
//Ö¸ÁîF:0xAA+0x0B+0x55 :CPUÍ¨Öªµ¥Æ¬»ú×¼±¸¹Ø»ú£¬LEDÉÁË¸µÈ´ý30S
char gCmd[CMD_NUM][CMD_LEN] = {
    {0xAA, 0x01, 0x55},
    {0xAA, 0x03, 0x55},
    {0xAA, 0x05, 0x55},
    {0xAA, 0x07, 0x55},
    {0xAA, 0x09, 0x55},
    {0xAA, 0x0B, 0x55},
};

#define RECORD_PWR_KEY_BUF_LEN      64
typedef struct {
    unsigned char writeIdx;
    unsigned char readIdx;
    unsigned char valArr[RECORD_PWR_KEY_BUF_LEN];
}pwr_key_t;
static pwr_key_t pwrKey = {0};

//#define IR_CTRL
//#define FB_DEBUG

#ifdef FB_DEBUG
    #define TRACE_FB(msg, arg...) msprintf(msg, ##arg)
#else
    #define TRACE_FB(msg, arg...) ((void)0)
#endif

enum key_type {
    KT_IR = 0,
    KT_KB
};

struct key_map {
    char name[16];
    int val;
};

//×¢Òâ°´¼üË³Ðò±ØÐëºÍlircd.confÅäÖÃÒ»ÖÂ
struct key_map ir_key[] = {
    { "KEY_1", KEY_F1 },
    { "KEY_2", KEY_F2 },
    { "KEY_3", KEY_F3 },
    { "KEY_4", KEY_F4 },
    { "KEY_5", KEY_F5 },
    { "KEY_6", KEY_F6 },
    { "KEY_7", KEY_F7 },
    { "KEY_8", KEY_F8 },
    { "KEY_9", KEY_F9 },
    { "KEY_0", KEY_F10 },
    { "KEY_UP", KEY_UP },
    { "KEY_LEFT", KEY_LEFT },
    { "KEY_ENTER", KEY_ENTER },
    { "KEY_RIGHT", KEY_RIGHT },
    { "KEY_DOWN", KEY_DOWN },
    { "KEY_ESC", KEY_ESC },
    { "KEY_L", KEY_L },
    { "KEY_I", KEY_INSERT },
    { "KEY_R", KEY_R },
    { "KEY_Y", KEY_Y },
    { "KEY_P", KEY_P },
    { "KEY_HOME", KEY_HOME },
    { "KEY_O", KEY_O },
    { "KEY_H", KEY_H },
    { "KEY_Q", KEY_Q },
    { "KEY_T", KEY_T },
    { "KEY_U", KEY_U },
    { "KEY_V", KEY_V },
    { "KEY_W", KEY_W },
    { "KEY_K", KEY_K },
    { "KEY_X", KEY_X },
    { "KEY_F1", KEY_F17 },
    { "KEY_F2", KEY_F18 },
    { "KEY_POWER", KEY_POWER },
    { "KEY_F12", KEY_F24 }
};

typedef struct {
    unsigned char inkey;
    unsigned char outkey;
} key_map;

static key_map std_485kb_key[] = { //ys_add //   ÔÚÊý×é³ÉÔ±ºóÃæ¸ú×Å "//"±íÊ¾ÔÚ485¼üÅÌÖÐÊÇ×¨ÓÃµÄ£¬Ç°Ãæ°åÃ»ÓÐ´Ë°´¼ü
    { 0x80, KEY_F1 },   //KB485_¿ªÍ·µÄºê¶¨ÒåÔÚstd_kb.hÖÐ,±ÈÈçKB485_1±íÊ¾485¼üÅÌÉÏµÄ1£¬¶ÔÓ¦µ½ÐéÄâ±ê×¼¼üÅÌÖÐµÄKEY_1
    { 0x40, KEY_F2 },
    { 0xc0, KEY_F3 },
    { 0x20, KEY_F4 },
    { 0xa0, KEY_F5 },
    { 0x60, KEY_F6 },
    { 0xe0, KEY_F7 },
    { 0x10, KEY_F8 },
    { 0x90, KEY_F9 },
    { 0x00, KEY_F10 },
    { 0x24, KEY_F17 },
    { 0x25, KEY_F18},
    { 0xd8, KEY_R },
    { 0x58, KEY_P },
    { 0x24, KEY_F23 },  //ÐèÒªÐÞ¸Ä£¬ÒòÎªKB485_SETUPµÄÖµ¸úKB485_F1Ò»Ñù
    { 0xb0, KEY_UP },
    { 0x70, KEY_DOWN },
    { 0x50, KEY_LEFT },
    { 0xd0, KEY_RIGHT },
    { 0xf0, KEY_ENTER },
    { 0x28, KEY_ESC },
    { 0x44, KEY_POWER },
    { 0x38, KEY_Q },
    { 0x21, KEY_U },
    { 0x82, KEY_HOME},
    { 0xc8, KEY_INSERT },
    { 0xc1, KEY_T },
    { 0xa1, KEY_X },
    { 0x68, KEY_F20 },          //
    { 0x61, KEY_K }, //ZOOM_P±íÊ¾ZOOM£«£¬ZOOM_N±íÊ¾ZOOM£­
    { 0x81, KEY_W },
    { 0x41, KEY_V },
    { 0xb8, KEY_H },
    { 0x78, KEY_F21 },
    { 0xff, 0 }
};

struct fb_ctrl {
    struct input_event event;
    struct snd_stat snd_stat;
    struct callback_prm cb;
    pthread_t tid; ///< main task
    pthread_t ltid; ///< ir long long press thread
    pthread_t ptid; ///< power thread
    pthread_t ktid; ///< keyboard thread
    pthread_t stid; ///< state led thread
    pthread_t ftid; ///< fan ctrl thread
    pthread_t usarttid;
//  int force_stop;
    int kb_fd;
    int input_fd;
    int ir_fd;
    int pwr_fd; ///< /dev/fb_ctrl
    int usart_fd;// /dev/ttyAMA2
    int dev_id;
    int guard_stat;
    enum power_en  pwr;
};

struct fb_ctrl g_ctrl;

struct key {
    int value;
    int status;
    int type; ///< ir, kb,
//  pthread_mutex_t lock;
};

static struct key g_key;

struct snd_ctrl {
    int alarm;
};

struct snd_ctrl g_snd_ctrl;

static int write_pwr_value(pwr_key_t *key, unsigned char keyVal)
{
    if ((key->writeIdx + 1) != key->readIdx) {
        key->valArr[key->writeIdx] = keyVal;
        key->writeIdx = (key->writeIdx + 1) % RECORD_PWR_KEY_BUF_LEN;
        return 0;
    } else {
        TRACE_FB("recode pwr_key buff full!\n");
        return -1;
    }
}

static int read_pwr_value(pwr_key_t *key)
{
    unsigned char temp;
    if (key->readIdx != key->writeIdx) {
        temp = key->valArr[key->readIdx];
        key->readIdx = (key->readIdx + 1) % RECORD_PWR_KEY_BUF_LEN;
        return temp;
    } else {
        return -1;
    }
}

#if defined(_HI3536_) || defined(_HI3536A_)
static int rs485_init(void)
{
    //´®¿Ú³õÊ¼»¯
    int fd;
    fd = rs485OpenDev(RS_DEV);
    if (fd < 0) {
        printf("Can't Open dev_full Serial Port!\n");
        return -1;
    }

    if (rs485SetPortAttr(fd, 9600, 8, 1, 'N') < 0) {
        printf("fd_full Set SetPortAttr Error\n");
        return -1;
    }

    gpio_write(GIO_RS485_CTL_EN, RS485_DIR_RX);

    return fd;
}

static int usart_init(void)
{
    int fd;

    fd = rs485OpenDev(USART_DEV);
    if (fd < 0) {
        printf("Can't Open usart Port!\n");
        return -1;
    }
    if (rs485SetPortAttr(fd, 115200, 8, 1, 'N') < 0) {
        printf("usart Set SetPortAttr Error\n");
        return -1;
    }
    return fd;
}

static int fb_usart_run(int fd)
{
#if defined(_HI3536A_)
    char cmd[] = {0xaf, 0xcc, 0x03, 0xfc, 0x01, 0x01, 0x0};
    int len = sizeof(cmd) / sizeof(cmd[0]);
    for (int i = 2; i < len - 1; i++) {
        cmd[len - 1] += cmd[i];
    }
    write(fd, cmd, len);
#else
    write(fd, gCmd[CMD_A], CMD_LEN);
#endif
    return 0;
}

#endif

int fb_usart_poweroff(int fd)
{
#if defined(_HI3536A_)
    char cmd[] = {0xaf, 0xcc, 0x03, 0xfc, 0x01, 0x0b, 0x0};
    int len = sizeof(cmd) / sizeof(cmd[0]);
    for (int i = 2; i < len - 1; i++) {
        cmd[len - 1] += cmd[i];
    }
    write(fd, cmd, len);
#else
    write(fd, gCmd[CMD_F], CMD_LEN);
#endif
    return 0;
}

int fb_usart_poweroff_end(int fd)
{
#if defined(_HI3536A_)
    char cmd[] = {0xaf, 0xcc, 0x03, 0xfc, 0x01, 0x07, 0x0};
    int len = sizeof(cmd) / sizeof(cmd[0]);
    for (int i = 2; i < len - 1; i++) {
        cmd[len - 1] += cmd[i];
    }
    write(fd, cmd, len);
#else
    write(fd, gCmd[CMD_D], CMD_LEN);
#endif
    return 0;
}

int fb_usart_enter_boot(int fd)
{
#if defined(_HI3536A_)
    char cmd[] = {0xaf, 0xcc, 0x03, 0xfc, 0x01, 0x09, 0x0};
    int len = sizeof(cmd) / sizeof(cmd[0]);
    for (int i = 2; i < len - 1; i++) {
        cmd[len - 1] += cmd[i];
    }
    write(fd, cmd, len);
#else
    write(fd, gCmd[CMD_E], CMD_LEN);
#endif
    return 0;
}

void fb_usart_set_alarm_state(int id, int value)
{
    char cmd[] = {0xaf, 0xcc, 0x05, 0xfa, 0x01, 0x0c, 0x0, 0x0, 0x0};
    int len = sizeof(cmd) / sizeof(cmd[0]);

    cmd[6] = value;
    cmd[7] = id;
    for (int i = 2; i < len - 1; i++) {
        cmd[len - 1] += cmd[i];
    }
    write(g_ctrl.usart_fd, cmd, len);
}
static int get_baud_rate(int baud)
{
    int rate;

    switch (baud) {
        case 2400:
            rate = B2400;
            break;
        case 4800:
            rate = B4800;
            break;
        case 9600:
            rate = B9600;
            break;
        case 19200:
            rate = B19200;
            break;
        case 38400:
            rate = B38400;
            break;
        case 57600:
            rate = B57600;
            break;
        case 115200:
            rate = B115200;
            break;
        default:
            rate = B9600;
            break;
    }
    return rate;
}

static int get_data_bit(int bit)
{
    int data;

    switch (bit) {
        case 5:
            data = CS5;
            break;
        case 6:
            data = CS6;
            break;
        case 7:
            data = CS7;
            break;
        case 8:
            data = CS8;
            break;
        default:
            data = CS8;
            break;
    }
    return data;
}

static int get_power_key_state(struct fb_ctrl *fctrl)
{
    int state = -1;

    if (fctrl->pwr == POWER_IO) {
        if (read(fctrl->pwr_fd, &state, sizeof(state)) <= 0) {
            return -1;
        }
    } else {
#if defined(_HI3536A_)
        state = read_pwr_value(&pwrKey);
#else
        char buff[CMD_LEN];
        if (read(fctrl->usart_fd, buff, sizeof(buff)) <= 0) {
            return -1;
        } else {
            printf("buff[%x][%x][%x]\n", buff[0], buff[1], buff[2]);
            if (!memcmp(buff, gCmd[CMD_B], CMD_LEN)) {
                state = 2;
            } else if (!memcmp(buff, gCmd[CMD_C], CMD_LEN)) {
                state = 1;
            } else {
                state = -1;
            }
        }
#endif
    }

    return state;
}
static int uinput_init(void)
{
    //ÐéÄâ¼üÅÌ³õÊ¼»°
    int fd;
    int ret = 0, i;
    struct uinput_user_dev uinp;

    memset(&uinp, 0, sizeof(uinp));
    fd = open(UINPUT_DEV, O_WRONLY | O_NDELAY);
    if (fd == 0) {
        TRACE_FB("Could not open uinput.\n");
        return -1;
    }

    memset(&uinp, 0, sizeof(struct uinput_user_dev));
    strncpy(uinp.name, "dvr_keypad", 10);
    uinp.id.version = 4;
    uinp.id.bustype = BUS_VIRTUAL;  //BUS_USB//BUS_VIRTUAL

    ioctl(fd, UI_SET_EVBIT, EV_KEY);

    for (i = 0; i < KEY_MAX; i++) {   //KEY_MAXÎª0x2ff  Ã¿Ò»¸öÊý¾Ý¶¼´ú±íÒ»¸ö¾ßÌå±ê×¼¼üÅÌÉÏµÄ¼ü
        ioctl(fd, UI_SET_KEYBIT,
              i); //Ê¹ÄÜÐéÄâ¼üÅÌÖÐµÄ¾ßÌå¼ü(Ã¿Ò»¸ö°´¼ü¶¼ÊÇÓÃºê¶¨Òå£¬±ÈÈç #define KEY_ESC 1)£¬¿É¼û/usr/include/linux/input.h
    }

    //# create input device in input subsystem
    write(fd, &uinp, sizeof(struct uinput_user_dev));

    ret = ioctl(fd, UI_DEV_CREATE);   //´´½¨ÐéÄâ¼üÅÌ
    if (ret) {
        TRACE_FB("Error create uinput device %d.\n", ret);
        close(fd);
        return -1;
    }
    return fd;
}

#ifdef IR_CTRL
static int socket_init(const char *un_path)
{
    int fd = -1;
    struct sockaddr_un sock = {0};

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }
    sock.sun_family = AF_UNIX;
    strncpy(sock.sun_path, un_path, sizeof(sock.sun_path));
    if (connect(fd, (struct sockaddr *)&sock, sizeof(sock)) < 0) {
        perror("connect");
        return -1;
    }
    return fd;
}
#endif

static void send_ev_input(int fd, int keycode, int type)
{
    //ÍùÐéÄâ¼üÅÌ·¢ËÍ¼üÖµºÍ¶¯×÷(ÊÇ°´ÏÂ»¹ÊÇÊÍ·Å)
    struct fb_ctrl *fctrl = &g_ctrl;
    memset(&fctrl->event, 0, sizeof(fctrl->event));
    gettimeofday(&fctrl->event.time, NULL);

    fctrl->event.type = EV_KEY;
    fctrl->event.code = keycode;
    fctrl->event.value = type;          //1 1±íÊ¾°´ÏÂ, 0 ±íÊ¾ÊÍ·Å
    write(fd, &fctrl->event, sizeof(fctrl->event));
    fctrl->event.type = EV_SYN;
    fctrl->event.code = SYN_REPORT;
    fctrl->event.value = 0;
    write(fd, &fctrl->event, sizeof(fctrl->event));
}

#ifdef IR_CTRL
static int ir_handle(int val, int type)
{
    static int devid = -1;
    static int set_dev_flag = 0;
    struct fb_ctrl *fctrl = &g_ctrl;
    int res = -1, tmp = 0;

    if (type == PRESS && ir_key[val - 1].val == KEY_F24) { // set device id
        set_dev_flag = 1;
        devid = -1;
        TRACE_FB("start set devid");
        return -1;
    }
    if (!set_dev_flag) {
//      TRACE_FB("not set devid: %d, fctrl->dev_id: %d", devid, fctrl->dev_id);
        if (fctrl->dev_id == 255 || devid == fctrl->dev_id) {
            res = ir_key[val - 1].val;
        } else {
            res = -1;
        }
    } else {
        if (type == PRESS) {
            if (val >= 1 && val <= 10) {
                tmp = val;
                if (val == 10) {
                    tmp = 0;
                }
                if (devid == -1) {
                    devid = 0;
                }
                devid = devid * 10 + tmp;
                if (devid > 255) {
                    devid = 255;
                }
            }
            if (ir_key[val - 1].val == KEY_ENTER) {
                TRACE_FB("set devid done, device id: %d", devid);
                set_dev_flag = 0;
            }
        }
    }
    return res;
}
#endif

#if defined(_HI3536_) || defined(_HI3536C_)
    #define UPPER_T         (58.0)
    #define FLOOR_T         (52.0)
    #define CPU_T           (105.0)
#else
    #define UPPER_T         (58.0)
    #define FLOOR_T         (52.0)
    #define CPU_T           (105.0)
#endif

static double  fan_get_cpu_temp()
{
    FILE *pfd = NULL;
    char buff[64];
    float t = 0;

    pfd = ms_vpopen("temperature 0 1", "r");
    if (pfd != NULL) {
        if (fgets(buff, sizeof(buff), pfd) != NULL) {
            if (sscanf(buff, "Current T: %f 'C", &t) != 1) {
                printf("get cpu temperature[%f] failed\n", t);
                t = 0.0;
            }
        }
        ms_vpclose(pfd);
    }
    return (double)t;
}

static double fan_get_disk_temp()
{
    FILE *fp = NULL;
    char buf[64];
    int t;

    fp = ms_vpopen("/usr/bin/diskTemp.sh", "r");
    if (!fp) {
        printf("open /usr/bin/diskTemp.sh failed\n");
        return 0.0;
    }
    fgets(buf, sizeof(buf), fp);
    if (sscanf(buf, "The max temperature : %d", &t) != 1) {
        printf("get disk temperature[%d] failed\n", t);
        t = 0;
    }
    ms_vpclose(fp);

    return (double)t * 1.0;
}

static int fan_ctrl_init()
{
    int ret = 0;
    ret = gpio_open();
    if (ret != 0) {
        printf("gpio_open failed with %#x! \n", ret);
        return -1;
    }

    gpio_write(GIO_FAN_EN, 0);//default disable;

    return 0;
}

static int fan_ctrl_uninit()
{
    int ret = 0;

    gpio_write(GIO_FAN_EN, 0);//default disable;

    ret = gpio_close();
    if (ret != 0) {
        printf("gpio_close failed with %#x! \n", ret);
        return -1;
    }

    return 0;
}

static void *task_fan_ctrl(void *prm)
{
    int cnt = 0;
    double Tcpu = 0.0;
    double Tdisk = 0.0;
    ms_task_set_name("fan_ctrl");
    while (1) {
        pthread_testcancel();
        if (cnt % 100 == 0) {
            Tcpu = fan_get_cpu_temp();
            Tdisk = fan_get_disk_temp();
//            printf("Current Tcpu: %.2f 'C\n", Tcpu);
//            printf("Current Tdisk: %.2f 'C\n", Tdisk);
            if (Tdisk > UPPER_T || Tcpu > CPU_T) {
                gpio_write(GIO_FAN_EN, 1);
            } else if (Tdisk < FLOOR_T) {
                gpio_write(GIO_FAN_EN, 0);
            }
        }
        cnt++;
        usleep(100000);
    }
    gpio_write(GIO_FAN_EN, 0);//default disable;
    return NULL;

}

static unsigned char handle_485kb(unsigned char value)   //°Ñ485¼üÅÌ×ª»»³ÉlinuxÐéÄâ¼üÅÌ
{
    key_map *kb485_map = std_485kb_key;

    while (kb485_map->inkey != 0xff) {
        if (kb485_map->inkey == value) {
            return (kb485_map->outkey);
        }
        kb485_map++;
    }
    return 0;
}

static void procotol_handle(unsigned char *buf)
{
    switch (buf[3]) {
        case CMD_B:
            write_pwr_value(&pwrKey, 2);
            break;
        case CMD_C:
            write_pwr_value(&pwrKey, 1);
            break;
#if defined(_HI3536A_)
        case CMD_ALARM_IN:
            set_sensor_state((buf[5] << 8) | buf[4]);
            break;
#endif
        default:
            break;
    }
}

static char mcu_data_handle(unsigned char *buf, int len)
{
    unsigned char checkSum = 0;

    for (int i = 0; i < len - 1; i++) {
        checkSum += buf[i];
    }
    if (checkSum != buf[len -1]) {
        printf("\n mcu data checekSum error \n");
        printf("[recv mcu %d data: ", len);
        for (int i = 0; i < len; i++) {
            printf("%02x ", buf[i]);
        }
        printf("]\n");
        return -1;
    }
    procotol_handle(buf);
    return 0;
}

static unsigned char ms_mcu_data_filter(unsigned char *rawBuf, unsigned int *rawLength,
                                                    unsigned char *outBuf, unsigned char *outLength)
{
    unsigned char rawData = 0;                      /* åŽŸå§‹æ•°æ®å­—èŠ‚è®°å½• */
    unsigned char tmpRawData = 0;
    static unsigned char length = 0;                /* æ•°æ®å¸§é•¿åº¦è®°å½• */
    static unsigned char tmpRawIdx = 0;             /* åŽŸå§‹æ•°æ®åŒ…ä¸‹æ ‡è®°å½• */
    static unsigned char tmpOutIdx = 0;             /* è¾“å‡ºæ•°æ®å¸§buffä¸‹æ ‡ */
    static unsigned int checkSum = 0;               /* æ ¡éªŒå’Œè®°å½• */
    static unsigned int tmpRawLength = 0;           /* åŽŸå§‹æ•°æ®åŒ…æ€»é•¿åº¦è®°å½• */
    static unsigned char tmpOutBuf[32] = {0};       /* è¾“å‡ºæ•°æ®å¸§ */
    static unsigned char tmpRawBuf[255] = {0};      /* åŽŸå§‹æ•°æ®åŒ… */
    static MS_FREAM_FIELD parseSta = MS_FREAM_NONE; /* è§£æžçŠ¶æ€è®°å½• */

    if (tmpRawLength == 0) {
        tmpRawIdx = 0;
        memcpy(tmpRawBuf, rawBuf, *rawLength);
        tmpRawLength = *rawLength;
    }

    do {
        rawData = tmpRawBuf[tmpRawIdx];

        ++tmpRawIdx;
        tmpRawLength--;
        
        switch (parseSta) {
            case MS_FREAM_HEAD: {   /* å¸§é•¿åº¦å­—æ®µ */
                if (length != 0xFF && rawData + length == 0xFF) {
                    tmpOutBuf[0] = length;
                    tmpOutBuf[1] = rawData;
                    checkSum = length + rawData;
                    tmpOutIdx = 0;
                    parseSta = MS_FREAM_VALUE;
                } else {
                    length = rawData;
                }
            }
            break;
            case MS_FREAM_VALUE: { /* æ•°æ®å¸§å†…å®¹(åœ°å€+å‘½ä»¤+æ•°æ®æ®µ) */
                tmpOutBuf[2 + tmpOutIdx] = rawData;
                tmpOutIdx++;
                checkSum += rawData;
                if (tmpOutIdx == length) {
                    tmpOutIdx = 0;
                    parseSta = MS_FREAM_NONE;
                    memcpy(outBuf, tmpOutBuf, length + 2);  /* è¿”å›žä¸€å¸§å®Œæ•´æ•°æ® */
                    *outLength = length + 2;    /* è¿”å›žä¸€å¸§å®Œæ•´æ•°æ®é•¿åº¦ */
                    *rawLength = tmpRawLength;  /* æ›´æ–°åŽŸå§‹æ•°æ®åŒ…é•¿åº¦ */
                    return 1;
                }
            }
            break;
            default: {
                parseSta = MS_FREAM_NONE;
            }
            break;
        }
        
        if(((tmpRawData << 8) | rawData) == 0xAFCC)  { //fix frame header
            parseSta = MS_FREAM_HEAD;
        }
        tmpRawData = rawData;
    } while (tmpRawLength);
    return 0;
}

//è¿™ä¸ªçº¿ç¨‹æŽ¥æ”¶å•ç‰‡æœºå‘æ¥çš„åè®®ç›¸å…³æ•°æ®å¤„ç†é€»è¾‘
static void *task_protocol_recv(void *prm)
{
    struct fb_ctrl *fctrl = (struct fb_ctrl *)prm;
    int retval = 0;
    int totalLen = 0;
    int dataLen = 0;
    unsigned char buf[255] = {0}, cnt, *ptmp, buf_once[32];

    ms_task_set_name("mcu_uart");
    while(1) {
        FD_ZERO(&g_fs_read[0]);
        FD_SET(fctrl->usart_fd, &g_fs_read[0]);
        g_tty_select_tmout.tv_sec = 0;
        g_tty_select_tmout.tv_usec = 200000; //200ms
        retval = select(fctrl->usart_fd + 1, &g_fs_read[0], NULL, NULL, &g_tty_select_tmout);

        if (retval <= 0) {
            continue;
        }

        /* èŽ·å–æ•°æ® */
        memset(buf, 0, sizeof(buf));
        totalLen = read(fctrl->usart_fd, buf, 255);
        /* è§£æžæ•°æ® */
        while(totalLen > 0 && ms_mcu_data_filter(buf, &totalLen, buf_once, &dataLen)) {
            mcu_data_handle(buf_once, dataLen);
        }
    }
    return NULL;
}

void *task_keyboard(void *prm)
{
    struct fb_ctrl *fctrl = (struct fb_ctrl *)prm;
    unsigned char msg[KB_MSG_LEN];
    int len, val;
//  int lighton = 0, on_flag = 0;

    ms_task_set_name("keyboard");

    while (1) {
        pthread_testcancel();
        if ((len = rs485HalfRead(fctrl->kb_fd, msg, sizeof(msg))) != KB_MSG_LEN) {
//          TRACE_FB("read error");
            continue;
        }
        if (fctrl->dev_id != 255 && msg[0] != (unsigned char)((fctrl->dev_id + 0xdf) % 256)) {
//          lighton = 0;
//          on_flag = 0;
            continue;
        }
//      lighton = 1;
//      if (lighton && !on_flag) { //µÆ²»ÁÁ
//          fb_send2front(SEND_LED_STATUS_RED);
//          on_flag = 1;
//      }
        if ((g_key.value = val = handle_485kb(msg[3])) == 0) {
            continue;
        }
        if (msg[4]) {
            g_key.status = PRESS;
        } else {
            g_key.status = RELEASE;
        }
        g_key.type = KT_KB;
        if (g_key.value >= KEY_F13 && g_key.value <= KEY_F24) {
            send_ev_input(fctrl->input_fd, KEY_LEFTSHIFT, msg[4]);
            send_ev_input(fctrl->input_fd, val - (KEY_F13 - KEY_F1), msg[4]);
        } else {
            send_ev_input(fctrl->input_fd, val, msg[4]);
        }
//      for (val = 0; val < len; val++)
//          printf("%x ", msg[val]);
        printf("\n");

    }
    return NULL;
}

static void *task_key_power(void *prm)
{
    struct fb_ctrl *fctrl = (struct fb_ctrl *)prm;
    int status;
    ms_task_set_name("key_power");

    while (1) {
        pthread_testcancel();
        if (!fctrl->guard_stat) {
            //printf("==========no power key ===========\n\n");
            usleep(100000);
            continue;
        }
        status = get_power_key_state(fctrl);
        if (status < 0) {
            usleep(50000);
            continue;
        }
        if (status == 1) {
            TRACE_FB("board power long press");
            if (fctrl->pwr == POWER_USART) {
                fb_usart_poweroff(fctrl->usart_fd);
            } else {
                fb_ctrl_poweroff(fctrl->pwr_fd);
            }
//          ms_vsystem("reboot");
            if (fctrl->cb.cb_pwr) {
                fctrl->cb.cb_pwr(fctrl->cb.pwr_arg, POWER_LONG);
            }
            sleep(30);
            ms_vsystem("reboot");
        } else if (status == 2) {
            TRACE_FB("board power short press");
            if (fctrl->cb.cb_pwr) {
                fctrl->cb.cb_pwr(fctrl->cb.pwr_arg, POWER_SHORT);
            }
        }
    }
    return NULL;
}

static void *task_long_press(void *prm)
{
    struct fb_ctrl *fctrl = (struct fb_ctrl *)prm;
    struct key *fkey = &g_key;
    int keyval = -1;
    int guard;

    ms_task_set_name("task_long");
    while (1) {
        pthread_testcancel();
        if (keyval == -1) {
            if (fkey->type == KT_KB) {
                usleep(1000);
                continue;
            }
            if (fkey->value != -1 && fkey->status != RELEASE) {
                keyval = fkey->value;
                sleep(3);
                if (keyval == fkey->value && fkey->status != RELEASE) { // long pressed
                    if (keyval == KEY_HOME) {
                        TRACE_FB("menu long pressed");
                        fctrl->snd_stat.ir = !fctrl->snd_stat.ir; // turn off the sound
                    } else if (keyval == KEY_ESC) {
                        TRACE_FB("esc long pressed");
                        if (fctrl->cb.cb_guard) {
                            fctrl->cb.cb_guard(fctrl->cb.guard_arg, &guard);
                        }
                    } else if (keyval == KEY_POWER) {
                        TRACE_FB("power long pressed");
                        fb_ctrl_poweroff(fctrl->pwr_fd);
//                      ms_vsystem("reboot");
                        if (fctrl->cb.cb_pwr) {
                            fctrl->cb.cb_pwr(fctrl->cb.pwr_arg, POWER_LONG);
                        }
                        sleep(30);
                        ms_vsystem("reboot");
                    }
                }
                fkey->value = -1;
                keyval = -1;
            }
        }
        usleep(5000);
    }
    return NULL;
}

#ifdef IR_CTRL
static void *task_main(void *prm)
{
    struct fb_ctrl *fctrl = (struct fb_ctrl *)prm;
    struct snd_ctrl *sctrl = &g_snd_ctrl;
    char buf[128];
    int val, type, res;
    time_t press, release;

    ms_task_set_name("frontboard");
    TRACE_FB("11111111 ir: %d", fctrl->snd_stat.ir);
    while (1) {
        pthread_testcancel();
        if (read(fctrl->ir_fd, buf, sizeof(buf)) <= 0) {
            perror("read");
            continue;
        }

//      TRACE_FB("%s\n", buf);
        if (sscanf(buf, "%x 00", &val) != 1 && val >= sizeof(ir_key) / sizeof(struct key_map)) {
            perror("sscanf");
            continue;
        }
        if (strstr(buf, "KEY_UP")) {
            if (strstr(buf, "_UP_UP")) {
                type = RELEASE;
//              TRACE_FB("release");
            } else {
                type = PRESS;
//              TRACE_FB("press");
            }
        } else {
            if (strstr(buf, "_UP")) {
                type = RELEASE;
//              TRACE_FB("release");
            } else {
                type = PRESS;
//              TRACE_FB("press");
            }
        }
        TRACE_FB("ir on: %d, type: %d, alarm: %d", fctrl->snd_stat.ir, type, sctrl->alarm);
        if (fctrl->snd_stat.ir && type == PRESS && !sctrl->alarm) {
            TRACE_FB("RING KEY");
            misc_ctrl(RING_KEY, 1, 0);
        }
        if ((res = ir_handle(val, type)) != -1) {
            TRACE_FB("res: %d\n", res);
            if (res == KEY_POWER) {
                if (type == PRESS) {
//                  pwr_flag = 1;
                    time(&press);
                } else {
                    time(&release);
                    if (press && release - press <= 1) {
                        TRACE_FB("key power short pressed\n");
                        if (fctrl->cb.cb_pwr) {
                            fctrl->cb.cb_pwr(fctrl->cb.pwr_arg, POWER_SHORT);
                        }
                    }
                    press = 0;
                    release = 0;
                }
            }
            g_key.value = res;
            g_key.status = type;
            g_key.type = KT_IR;
            if (res >= KEY_F13 && res <= KEY_F24) {
//              TRACE_FB("key f13 to f24");
                send_ev_input(fctrl->input_fd, KEY_LEFTSHIFT, type);
                send_ev_input(fctrl->input_fd, res - (KEY_F13 - KEY_F1), type);
            } else {
                send_ev_input(fctrl->input_fd, res, type);
            }
//          TRACE_FB("send res: %d, type: %d", res, type);
        }
    }
    return NULL;
}
#endif

static void *task_state_led(void *prm)
{
//  struct fb_ctrl *fctrl = (struct fb_ctrl *)prm;
    int status = LED_OFF;
    int cnt = 0;

    ms_task_set_name("state_led");
    while (1) {
        pthread_testcancel();
        if (cnt % 10 == 0) {
            gpio_write(STATE_LED_GREEN, status);
            status = !status;
        }
        cnt++;
        usleep(100000);
    }
    gpio_write(STATE_LED_GREEN, LED_OFF);
    return NULL;
}

int fb_init(struct callback_prm *cb, struct snd_stat *stat, int dev_id, int guard_stat, enum power_en  pwr)
{
    struct fb_ctrl *fctrl = &g_ctrl;
    int ret = 0;

    memset(fctrl, 0, sizeof(struct fb_ctrl));
    fctrl->dev_id = dev_id;
    fctrl->guard_stat = guard_stat;
    fctrl->pwr = pwr;
    printf("====fctrl->pwr[%d]========\n", fctrl->pwr);
    fb_set_buzz_mode(stat);
    TRACE_FB("ir stat: %d", fctrl->snd_stat.ir);
    if (cb) {
        fctrl->cb.cb_guard = cb->cb_guard;
        fctrl->cb.cb_pwr = cb->cb_pwr;
        fctrl->cb.cb_snd = cb->cb_snd;
        fctrl->cb.guard_arg = cb->guard_arg;
        fctrl->cb.pwr_arg = cb->pwr_arg;
        fctrl->cb.snd_arg = cb->snd_arg;
    }
    if (misc_init(&fctrl->pwr_fd) < 0) {
        TRACE_FB("init misc failed");
        return -1;
    }
#if defined(_HI3536_) || defined(_HI3536A_)
    if ((fctrl->kb_fd = rs485_init()) < 0) {
        TRACE_FB("init rs485 failed");
        return -1;
    }
    if (pwr == POWER_USART) {
        if ((fctrl->usart_fd = usart_init()) < 0) {
            TRACE_FB("init usart failed");
            return -1;
        }
        fb_usart_run(fctrl->usart_fd);
#if defined(_HI3536A_)
        if ((ret = pthread_create(&fctrl->usarttid, NULL, task_protocol_recv, fctrl)) < 0) {
            perror("pthread create");
            close(fctrl->kb_fd);
            close(fctrl->usart_fd);
            return -1;
        }
#endif /* _HI3536A_ */
    }
#endif
    if ((fctrl->input_fd = uinput_init()) < 0) {
        TRACE_FB("init uinput failed");
#if defined(_HI3536_) || defined(_HI3536A_)
        close(fctrl->kb_fd);
#endif
        return -1;
    }

    if (fan_ctrl_init() < 0) {
        return -1;
    }

#ifdef IR_CTRL
    if ((fctrl->ir_fd = socket_init(LIRCD)) < 0) {
#if defined(_HI3536_) || defined(_HI3536A_)
        close(fctrl->kb_fd);
#endif
        close(fctrl->input_fd);
        TRACE_FB("init socket failed");
        return -1;
    }
    if ((ret = pthread_create(&fctrl->tid, NULL, task_main, fctrl)) < 0) {
        perror("pthread create");
#if defined(_HI3536_) || defined(_HI3536A_)
        close(fctrl->kb_fd);
#endif
        close(fctrl->input_fd);
        close(fctrl->ir_fd);
        return -1;
    }
#endif
    if ((ret = pthread_create(&fctrl->ltid, NULL, task_long_press, fctrl)) < 0) {
        perror("pthread create");
#if defined(_HI3536_) || defined(_HI3536A_)
        close(fctrl->kb_fd);
#endif
        close(fctrl->input_fd);
#ifdef IR_CTRL
        close(fctrl->ir_fd);
        pthread_cancel(fctrl->tid);
#endif
        return -1;
    }
    if ((ret = pthread_create(&fctrl->ptid, NULL, task_key_power, fctrl)) < 0) {
        perror("pthread create");
#if defined(_HI3536_) || defined(_HI3536A_)
        close(fctrl->kb_fd);
#endif
        close(fctrl->input_fd);
#ifdef IR_CTRL
        close(fctrl->ir_fd);
        pthread_cancel(fctrl->tid);
#endif
        pthread_cancel(fctrl->ltid);
        return -1;
    }
#if defined(_HI3536_) || defined(_HI3536A_)
    if ((ret = pthread_create(&fctrl->ktid, NULL, task_keyboard, fctrl)) < 0) {
        perror("pthread create");
#if defined(_HI3536_) || defined(_HI3536A_)
        close(fctrl->kb_fd);
#endif
        close(fctrl->input_fd);
#ifdef IR_CTRL
        close(fctrl->ir_fd);
        pthread_cancel(fctrl->tid);
#endif
        pthread_cancel(fctrl->ltid);
        pthread_cancel(fctrl->ptid);
    }
#endif
    if ((ret = pthread_create(&fctrl->stid, NULL, task_state_led, fctrl)) < 0) {
        perror("pthread create");
#if defined(_HI3536_) || defined(_HI3536A_)
        close(fctrl->kb_fd);
#endif
        close(fctrl->input_fd);
#ifdef IR_CTRL
        close(fctrl->ir_fd);
        pthread_cancel(fctrl->tid);
#endif
        pthread_cancel(fctrl->ltid);
        pthread_cancel(fctrl->ptid);
#if defined(_HI3536_) || defined(_HI3536A_)
        pthread_cancel(fctrl->ktid);
#endif
        return -1;
    }

    if ((ret = pthread_create(&fctrl->ftid, NULL, task_fan_ctrl, fctrl)) < 0) {
        perror("pthread create");
#if defined(_HI3536_) || defined(_HI3536A_)
        close(fctrl->kb_fd);
#endif
        close(fctrl->input_fd);
#ifdef IR_CTRL
        close(fctrl->ir_fd);
        pthread_cancel(fctrl->tid);
#endif
        pthread_cancel(fctrl->ltid);
        pthread_cancel(fctrl->ptid);
#if defined(_HI3536_) || defined(_HI3536A_)
        pthread_cancel(fctrl->ktid);
#endif
        pthread_cancel(fctrl->stid);
        return -1;
    }
    return 0;
}

void fb_deinit(void)
{
    struct fb_ctrl *fctrl = &g_ctrl;

#ifdef IR_CTRL
    pthread_cancel(fctrl->tid);
#endif
    pthread_cancel(fctrl->ltid);
    pthread_cancel(fctrl->ptid);
#if defined(_HI3536_) || defined(_HI3536A_)
    pthread_cancel(fctrl->ktid);
#if defined(_HI3536A_)
    pthread_cancel(fctrl->usarttid);
#endif /* _HI3536A_ */
#endif
    pthread_cancel(fctrl->stid);
    ioctl(fctrl->input_fd, UI_DEV_DESTROY);
#if defined(_HI3536_) || defined(_HI3536A_)
    if (fctrl->kb_fd) {
        close(fctrl->kb_fd);
    }
    if (fctrl->usart_fd) {
        close(fctrl->usart_fd);
    }
#endif
    if (fctrl->input_fd) {
        close(fctrl->input_fd);
    }
#ifdef IR_CTRL
    if (fctrl->ir_fd) {
        close(fctrl->ir_fd);
    }
#endif
    misc_deinit();
#ifdef _HI3798_
    pthread_cancel(fctrl->ftid);
#endif
    fan_ctrl_uninit();
}

void fb_set_buzz_mode(struct snd_stat *mode)
{
    if (!mode) {
        return;
    }
    struct fb_ctrl *fctrl = &g_ctrl;
    memcpy(&fctrl->snd_stat, mode, sizeof(struct snd_stat));
}

int fb_set_rs485(struct rs485_info *rs)
{
    struct fb_ctrl *fctrl = &g_ctrl;
    struct termios oldrs;

    if (!rs || !fctrl->kb_fd) {
        return -1;
    }
    memset(&oldrs, 0, sizeof(oldrs));
    tcgetattr(fctrl->kb_fd, &oldrs);
    oldrs.c_cflag |= get_baud_rate(rs->baudrate);
    oldrs.c_cflag |= get_data_bit(rs->databit);
    if (rs->stopbit == 2) {
        oldrs.c_cflag |= CSTOPB;
    } else {
        oldrs.c_cflag &= ~CSTOPB;
    }
    if (rs->paritybit == 0) { // closed
        oldrs.c_cflag &= ~PARENB;
    } else if (rs->paritybit == 1) { // odd
        oldrs.c_cflag |= PARENB;
    } else { // even
        oldrs.c_cflag |= PARENB;
        oldrs.c_cflag |= PARODD;
    }
    tcsetattr(fctrl->kb_fd, TCSANOW, &oldrs);
    return 0;
}

void fb_set_dev_id(int id_value)
{
    if (id_value <= 0 || id_value > 255) {
        return;
    }
    struct fb_ctrl *fctrl = &g_ctrl;

    fctrl->dev_id = id_value;
}

/**
 * @todo
 */
void fb_set_alarm(int type, int enable, int delay)
{
    struct snd_ctrl *sctrl = &g_snd_ctrl;
    int ring_type = -1;

    sctrl->alarm = enable;
    switch (type) {
        case A_ALARM:
            ring_type = RING_ALARM;
            break;
        case A_VLOSS: {
            ring_type = RING_VLOSS;
        }
        break;
        case A_NETWORK_DISCONN:
            ring_type = RING_NETWORK_DISCONN;
            break;
        case A_DISK_FULL:
            ring_type = RING_DISK_FULL;
            break;
        case A_RECORD_FAIL:
            ring_type = RING_RECORD_FAIL;
            break;
        case A_MOTION_DETECT:
            ring_type = RING_MOTION_DETECT;
            break;
        case A_DISK_FAIL:
            ring_type = RING_DISK_FAIL;
            break;
        case A_CAMERAIO:
            ring_type = RING_CAMERAIO;
            break;
        case A_VOLUMEDETECT:
            ring_type = RING_VOLUMEDETECT;
            break;
        default:
            TRACE_FB("unknown type");
            break;
    }
    if (ring_type != -1) {
        misc_ctrl(ring_type, enable, delay);
    }
}

/**
 * @brief
 */
int fb_send2front(int cmd)
{
    switch (cmd) {
        case SYSTEM_POWEROFF:
            if (g_ctrl.pwr == POWER_USART) {
                fb_usart_poweroff(g_ctrl.usart_fd);
            } else {
                fb_ctrl_poweroff(g_ctrl.pwr_fd);
            }
            msprintf("front board power off.");
            //ms_vsystem("reboot");//bruce.milesight delete
            break;
        case SYSTEM_POWEROFF_END:
            msprintf("front board power off end.shutdown at once.");
            if (g_ctrl.pwr == POWER_USART) {
                fb_usart_poweroff_end(g_ctrl.usart_fd);
            } else {
                fb_ctrl_poweroff_end(g_ctrl.pwr_fd);
            }
            break;
        case SEND_LED_READY_GREEN:
            msprintf("front board SEND_LED_READY_GREEN.");
//      gpio_write(STATE_LED_RED, LED_OFF);
            gpio_write(STATE_LED_GREEN, LED_ON);
            break;
        case RECORD_START:
            gpio_write(STATE_LED_HDD, LED_ON);
            break;
        case RECORD_STOP:
            gpio_write(STATE_LED_HDD, LED_OFF);
            break;
        case SYSTEM_REBOOT:
            if (g_ctrl.pwr == POWER_USART) {
                fb_usart_enter_boot(g_ctrl.usart_fd);
            }
            break;
        default:
            //TRACE_FB("invalid command");
            break;
    }
    return 0;
}

#if 0
static int guard_test(void *arg, int *stat)
{
    static int flag = 0;

    flag = !flag;
    *stat = flag;
    return 0;
}

int main(int argc, char **argv)
{
    struct callback_prm cb;
    struct snd_stat snd = {0};
    struct fb_ctrl *fctrl = &g_ctrl;

    memset(&cb, 0, sizeof(cb));
    snd.ir = 1;
    cb.cb_guard = guard_test;
    fb_init(&cb, &snd, 2, 0);
//  misc_init(&g_ctrl);
    int quit;

    while (1) {
        sleep(1);
        if ((quit = getchar()) == 'q') {
            break;
        } else if (getchar() == 'a') {
            fb_set_alarm(A_ALARM, 1);
            TRACE_FB("set alarm done");
        } else if (getchar() == 'o') {
            fb_set_alarm(A_ALARM, 0);
        }
    }
    fb_deinit();
    return 0;
}
#endif

