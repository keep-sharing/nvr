#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>

#include "hardware.h"
#include "gpio.h"

extern char *ms_getenv(char *name);
static int g_exit = 0;
static int max_alarm_in = 0;
static int max_alarm_out = 0;
#define USART_DEV   "/dev/ttyAMA1"
static int g_fd = 0;

static int test_alarmin_start(void *param)
{
    int i;
    static int key[16];
    static int pass[16];
    int flag = 0;
    int tmp[16];
    char log[64];
    int reset = 0;
    unsigned char ver[64] = {0};
    int dataFromMcu = -1;
    int len;
    char buff[9];
    int temp = 0;

    hdinfo(">>>>test_alarm_in\n");
    if (get_hardware_version(ver)) {
        hdinfo(">>>>get_hardware_version fail\n");
        return NULL;
    }
    memset(pass, 0, sizeof(pass));
    reset = gpio_read(GIO_RESET_FACTORY);
#if defined(_HI3536A_)
    if (ver[3] == '7' || ver[3] == '8') {
        dataFromMcu = 0;
        ms_system("killall -9 fio");
        g_fd = open(USART_DEV, O_RDWR | O_NOCTTY | O_NONBLOCK );
        if (g_fd < 0) {
            printf("Can't Open usart Port!\n");
            return -1;
        }
    }
#endif
    if (!dataFromMcu) {
        char cmd[] = {0xaf, 0xcc, 0x03, 0xfc, 0x01, 0x0e, 0x0};
        len = sizeof(cmd) / sizeof(cmd[0]);
        for (i = 2; i < len - 1; i++) {
            cmd[len - 1] += cmd[i];
        }
        write(g_fd, cmd, len);
        g_exit = 0;
        while(!g_exit) {
            if (read(g_fd, buff, sizeof(buff)) == 9) {
                printf("hardware buff:%02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    buff[0], buff[1], buff[2], buff[3], buff[4], buff[5],
                    buff[6], buff[7], buff[8]);
                if (buff[0] == 0xaf && buff[1] == 0xcc) {
                    for (i = 2; i < sizeof(buff) - 1; i++) {
                        temp += buff[i];
                    }
                    printf("%x %x\n", temp & 0xff, buff[sizeof(buff) - 1]);
                    if ((temp & 0xff) == buff[sizeof(buff) - 1]) {
                        temp = (buff[6] | (buff[7] << 8));
                        printf("\n");
                        for (i = 0; i < max_alarm_in; i++) {
                            tmp[i] = key[i] = ((temp>> i) & 0x1);
                            printf("key[%d]: %d  ", i, key[i]);
                        }
                        printf("\n");
                        break;
                    }
                }
            }
        }
    } else {
        key[0] = GIO_ALARM_IN_0 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_0) : 0;
        key[1] = GIO_ALARM_IN_1 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_1) : 0;
        key[2] = GIO_ALARM_IN_2 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_2) : 0;
        key[3] = GIO_ALARM_IN_3 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_3) : 0;
        key[4] = GIO_ALARM_IN_4 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_4) : 0;
        key[5] = GIO_ALARM_IN_5 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_5) : 0;
        key[6] = GIO_ALARM_IN_6 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_6) : 0;
        key[7] = GIO_ALARM_IN_7 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_7) : 0;
        key[8] = GIO_ALARM_IN_8 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_8) : 0;
        key[9] = GIO_ALARM_IN_9 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_9) : 0;
        key[10] = GIO_ALARM_IN_10 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_10) : 0;
        key[11] = GIO_ALARM_IN_11 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_11) : 0;
        key[12] = GIO_ALARM_IN_12 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_12) : 0;
        key[13] = GIO_ALARM_IN_13 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_13) : 0;
        key[14] = GIO_ALARM_IN_14 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_14) : 0;
        key[15] = GIO_ALARM_IN_15 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_15) : 0;
    }

    printf("\nplease input a key !\n");
    printf("expection : show the value that you input\n");
    g_exit = 0;
    while(!g_exit) {
        if (!dataFromMcu) {
            temp = 0;
            if (read(g_fd, buff, sizeof(buff)) == 9) {
                printf("hardware buff:%02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    buff[0], buff[1], buff[2], buff[3], buff[4], buff[5],
                    buff[6], buff[7], buff[8]);
                if (buff[0] == 0xaf && buff[1] == 0xcc) {
                    for (i = 2; i < sizeof(buff) - 1; i++) {
                        temp += buff[i];
                    }
                    if ((temp & 0xff) == buff[sizeof(buff) - 1]) {
                        temp = (buff[6] | (buff[7] << 8));
                        printf("\n");
                        for (i = 0; i < max_alarm_in; i++) {
                            tmp[i] = ((temp >> i) & 0x1);
                            printf("tmp[%d]: %d  ", i, tmp[i]);
                        }
                        printf("\n");
                    }
                }
            }
        } else {
            tmp[0] = GIO_ALARM_IN_0 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_0) : 0;
            tmp[1] = GIO_ALARM_IN_1 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_1) : 0;
            tmp[2] = GIO_ALARM_IN_2 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_2) : 0;
            tmp[3] = GIO_ALARM_IN_3 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_3) : 0;
            tmp[4] = GIO_ALARM_IN_4 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_4) : 0;
            tmp[5] = GIO_ALARM_IN_5 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_5) : 0;
            tmp[6] = GIO_ALARM_IN_6 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_6) : 0;
            tmp[7] = GIO_ALARM_IN_7 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_7) : 0;
            tmp[8] = GIO_ALARM_IN_8 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_8) : 0;
            tmp[9] = GIO_ALARM_IN_9 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_9) : 0;
            tmp[10] = GIO_ALARM_IN_10 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_10) : 0;
            tmp[11] = GIO_ALARM_IN_11 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_11) : 0;
            tmp[12] = GIO_ALARM_IN_12 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_12) : 0;
            tmp[13] = GIO_ALARM_IN_13 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_13) : 0;
            tmp[14] = GIO_ALARM_IN_14 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_14) : 0;
            tmp[15] = GIO_ALARM_IN_15 != GPIO_INVALID ? gpio_read(GIO_ALARM_IN_15) : 0;
        }

        for (i = 0; i < max_alarm_in; i++) {
            if (key[i] != tmp[i]) {
                snprintf(log, sizeof(log), "alarm in[%d] value :%d\n", i + 1, tmp[i]);
                hdinfo(log);
                pass[i] = 1;
                key[i] = tmp[i];
            }
        }
        if (reset != gpio_read(GIO_RESET_FACTORY)) {
            reset = !reset;
            g_exit = 1;
        }
        usleep(100000);
    }

    hdinfo("\n");
    for (i = 0; i < max_alarm_in; i++) {
        if (pass[i] == 0) {
            snprintf(log, sizeof(log), "alarm in[%d] failed\n", i + 1);
            hdno(log);
            flag = 1;
        }
    }

    if (flag) {
        SHOW_NO;
    } else {
        SHOW_YES;
    }

    return 0;
}


static int test_alarmin_stop()
{
    g_exit = 1;
    if (g_fd) {
        close(g_fd);
    }
    return 0;
}

static int test_alarmin_find()
{
    char ver[64] = {0};
    max_alarm_in = 0;
    if (get_hardware_version(ver) != -1) {
        switch (ver[3]) {
            case '8':
            case '7':
                max_alarm_in = 16;
                break;
            case '5':
                max_alarm_in = 4;
                break;
            case '1':
                max_alarm_in = 0;
                break;
            default:
                max_alarm_in = 0;
                break;
        }
    }

    return !!max_alarm_in;
}

HD_MODULE_INFO(TEST_NAME_ALARMIN, test_alarmin_find, test_alarmin_start, test_alarmin_stop);


static int test_alarmout_find()
{
    char ver[64] = {0};
    max_alarm_out = 0;
    if (get_hardware_version(ver) != -1) {
        switch (ver[3]) {
            case '8':
            case '7':
                max_alarm_out = 4;
                break;
            case '5':
                max_alarm_out = 1;
                break;
            case '1':
                max_alarm_out = 0;
                break;
            default:
                max_alarm_out = 0;
                break;
        }
    }

    return !!max_alarm_out;
}

static int test_alarmout_stop()
{
#if defined(_HI3536A_)
    int i = 0, j = 0;
    int len = 0;
    char cmd[] = {0xaf, 0xcc, 0x05, 0xfa, 0x01, 0x0c, 0x0, 0x0, 0x0};
    g_exit = 1;
    usleep(500);
    len = sizeof(cmd) / sizeof(cmd[0]);
    for (j = 0; j < max_alarm_out; j++) {
        cmd[6] = 0;
        cmd[7] = j;
        cmd[len - 1] = 0;
        for (i = 2; i < len - 1; i++) {
            cmd[len - 1] += cmd[i];
        }
        write(g_fd, cmd, len);
		usleep(500000);
    }
    if (g_fd) {
        close(g_fd);
    }
#else
    g_exit = 1;
#endif
    return 0;
}

static int test_alarmout_start(void *param)
{
    int count = 2;
    int dataToMcu = -1;
    unsigned char ver[64] = {0};
    int len, i, j;

    hdinfo(">>>>test_alarm_out\n");

    if (get_hardware_version(ver)) {
        hdinfo(">>>>get_hardware_version fail\n");
        return NULL;
    }
#if defined(_HI3536A_)
    if (ver[3] == '7' || ver[3] == '8') {
        dataToMcu = 0;
        ms_system("killall -9 fio");
        g_fd = open(USART_DEV, O_RDWR | O_NOCTTY | O_NONBLOCK );
        if (g_fd < 0) {
            printf("Can't Open usart Port!\n");
            return -1;
        }
    }
#endif
    g_exit = 0;
    while (!g_exit) {
        if (!dataToMcu) {
            char cmd[] = {0xaf, 0xcc, 0x05, 0xfa, 0x01, 0x0c, 0x0, 0x0, 0x0};
            len = sizeof(cmd) / sizeof(cmd[0]);
            for (j = 0; j < max_alarm_out; j++) {
                cmd[6] = 0;
                cmd[7] = j;
                cmd[len - 1] = 0;
                for (i = 2; i < len - 1; i++) {
                    cmd[len - 1] += cmd[i];
                }
                write(g_fd, cmd, len);
                usleep(500000);
                cmd[6] = 1;
                cmd[len - 1] +=cmd[6];
                write(g_fd, cmd, len);
            }
            //buzz test
            gpio_write(GIO_BUZZ_EN, 1);
            usleep(500000);
            gpio_write(GIO_BUZZ_EN, 0);
        } else {
            if (GIO_ALARM_OUT_0 != GPIO_INVALID)
            {
                gpio_write(GIO_ALARM_OUT_0, 1);
                usleep(500000);
                gpio_write(GIO_ALARM_OUT_0, 0);
            }
            if (GIO_ALARM_OUT_1 != GPIO_INVALID)
            {
                gpio_write(GIO_ALARM_OUT_1, 1);
                usleep(500000);
                gpio_write(GIO_ALARM_OUT_1, 0);
            }
            if (GIO_ALARM_OUT_2 != GPIO_INVALID)
            {
                gpio_write(GIO_ALARM_OUT_2, 1);
                usleep(500000);
                gpio_write(GIO_ALARM_OUT_2, 0);
            }
            if (GIO_ALARM_OUT_3 != GPIO_INVALID)
            {
                gpio_write(GIO_ALARM_OUT_3, 1);
                usleep(500000);
                gpio_write(GIO_ALARM_OUT_3, 0);
            }
            //buzz test
            gpio_write(GIO_BUZZ_EN, 0);
            usleep(500000);
            gpio_write(GIO_BUZZ_EN, 1);
        }
        count--;
    }
}


HD_MODULE_INFO(TEST_NAME_ALARMOUT, test_alarmout_find, test_alarmout_start, test_alarmout_stop);

static int test_led_start(void *param)
{
    hdinfo(">>>>test_state_led\n");
    g_exit = 0;
    while (!g_exit) {
#if !defined(_HI3536A_)
        gpio_write(STATE_LED_GREEN, 0);
        usleep(500000);
        gpio_write(STATE_LED_GREEN, 1);
        usleep(500000);
        gpio_write(STATE_LED_HDD, 0);
        usleep(500000);
        gpio_write(STATE_LED_HDD, 1);
        usleep(500000);
#else 
        gpio_write(STATE_LED_GREEN, 1);
        usleep(500000);
        gpio_write(STATE_LED_GREEN, 0);
        usleep(500000);
        gpio_write(STATE_LED_HDD, 1);
        usleep(500000);
        gpio_write(STATE_LED_HDD, 0);
        usleep(500000);
#endif
    }
    return 0;
}

static int test_led_find()
{
    char ver[64] = {0};

    if (get_hardware_version(ver) != -1) {
        if (ver[3] == '1') {
            return 0;
        } else {
            return 1;
        }
    }

    return 0;
}

static int test_led_stop()
{
    g_exit = 1;
    return 0;
}


HD_MODULE_INFO(TEST_NAME_LED, test_led_find, test_led_start, test_led_stop);


static int test_fan_start(void *param)
{
    hdinfo(">>>>test_fan_pin\n");

    gpio_write(GIO_FAN_EN, 0);
    g_exit = 0;
    while (!g_exit) {
        gpio_write(GIO_FAN_EN, 1);
        usleep(1000000);
        gpio_write(GIO_FAN_EN, 0);
        usleep(1500000);
        gpio_write(GIO_FAN_EN, 1);
        usleep(1000000);
        gpio_write(GIO_FAN_EN, 0);
    }
    gpio_write(GIO_FAN_EN, 0);
    return 0;

}

static int test_fan_find()
{
    return 1;
}

static int test_fan_stop()
{
    g_exit = 1;
    return 0;
}

HD_MODULE_INFO(TEST_NAME_FAN, test_fan_find, test_fan_start, test_fan_stop);

static int test_buzzer_start(void *param)
{
    hdinfo(">>>>test_buzzer_pin\n");

    g_exit = 0;
    while (!g_exit) {
#if !defined(_HI3536A_)
        gpio_write(GIO_BUZZ_EN, 0);
        usleep(500000);
        gpio_write(GIO_BUZZ_EN, 1);
#else
        gpio_write(GIO_BUZZ_EN, 1);
        usleep(500000);
        gpio_write(GIO_BUZZ_EN, 0);
#endif
    }
    return 0;

}

static int test_buzzer_find()
{
    return 1;
}

static int test_buzzer_stop()
{
    g_exit = 1;
    return 0;
}


HD_MODULE_INFO(TEST_NAME_BUZZER, test_buzzer_find, test_buzzer_start, test_buzzer_stop);


#if 0
static void test_mcu_pin()
{
    int key = 0;
    int count = 0;

    hdinfo(">>>>test_mcu_pin\n");
    hdinfo("expection : short press SW will print 'YES'\n");

    gpio_write(GIO_MCU_P1_1, 0);
    usleep(10000);
    gpio_write(GIO_MCU_P1_1, 1);
    usleep(1000000);

    g_exit = 0;
    while (!g_exit) {
        key = gpio_read(GIO_MCU_P1_0);
        if (key == 1) {
            count++;
            key = 0;
        }
        if (count >= 2) {
            SHOW_YES;
            g_exit = 1;
        }
        usleep(50000);
    }
}

static void test_mcu_pin_usart()
{
    int fd = open("/dev/ttyAMA2", O_RDWR | O_NOCTTY | O_NONBLOCK);
    char cmd[3] = {0xAA, 0x03, 0x55};
    char buff[3];

    hdinfo(">>>>test_mcu_pin\n");
    hdinfo("expection : short press SW will print 'YES'\n");
//    ms_system("killall -9 fio");
    g_exit = 0;
    while (!g_exit) {
        if (read(fd, buff, sizeof(buff)) == 3) {
            printf("hardware buff[%x][%x][%x]\n", buff[0], buff[1], buff[2]);
            if (!memcmp(buff, cmd, sizeof(buff))) {
                SHOW_YES;
                break;
            }
        }
        usleep(500000);
    }
    close(fd);
}
#endif

static int test_mcu_start(void *param)
{
    hdinfo(">>>>test_mcu\n");
    hdprompt("please short press SW");
#if 0
    test_mcu_pin_usart();
    test_mcu_pin();
#endif
    return 0;
}

static int test_mcu_find()
{
    char ver[64] = {0};
    if (get_hardware_version(ver) != -1) {
        switch (ver[3]) {
            case '8':
            case '7':
                return 1;
            case '1':
                if (ver[19] == '2') {
                    return 1;
                } else {
                    return 0;
                }
            default:
                return 0;
        }
    }

    return 0;
}

static int test_mcu_stop()
{
    g_exit = 1;
    return 0;
}


HD_MODULE_INFO(TEST_NAME_MCU, test_mcu_find, test_mcu_start, test_mcu_stop);

