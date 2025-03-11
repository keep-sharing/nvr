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

static int g_exit = 0;
static int max_alarm_in = 0;
static int max_alarm_out = 0;
#define USART_DEV   "/dev/ttyAMA1"
static int g_fd = 0;

static void test_alarm_in()
{   
    int i;
    static int key[16];
    static int pass[16];
    int flag = 0;
    int tmp[16];
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
                hdinfo("hardware buff:%02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    buff[0], buff[1], buff[2], buff[3], buff[4], buff[5],
                    buff[6], buff[7], buff[8]);
                if (buff[0] == 0xaf && buff[1] == 0xcc) {
                    for (i = 2; i < sizeof(buff) - 1; i++) {
                        temp += buff[i];
                    }
                    if ((temp & 0xff) == buff[sizeof(buff) - 1]) {
                        temp = (buff[6] | (buff[7] << 8));
                        hdinfo("\n");
                        for (i = 0; i < max_alarm_in; i++) {
                            tmp[i] = key[i] = ((temp>> i) & 0x1);
                            hdinfo("key[%d]: %d  ", i, key[i]);
                        }
                        hdinfo("\n");
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
    while(!g_exit)
    {
        if (!dataFromMcu) {
            temp = 0;
            if (read(g_fd, buff, sizeof(buff)) == 9) {
                hdinfo("hardware buff:%02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    buff[0], buff[1], buff[2], buff[3], buff[4], buff[5],
                    buff[6], buff[7], buff[8]);
                if (buff[0] == 0xaf && buff[1] == 0xcc) {
                    for (i = 2; i < sizeof(buff) - 1; i++) {
                        temp += buff[i];
                    }
                    if ((temp & 0xff) == buff[sizeof(buff) - 1]) {
                        temp = (buff[6] | (buff[7] << 8));
                        hdinfo("\n");
                        for (i = 0; i < max_alarm_in; i++) {
                            tmp[i] = ((temp >> i) & 0x1);
                            hdinfo("tmp[%d]: %d  ", i, tmp[i]);
                        }
                        hdinfo("\n");
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

        for (i=0; i<max_alarm_in; i++)
        {
            if(key[i] != tmp[i])
            {
//                printf("alarm in[%d] value :%d\n", i+1, tmp[i]);
                pass[i] = 1;
                key[i] = tmp[i];
            }
        }
        if (reset != gpio_read(GIO_RESET_FACTORY))
        {
//            printf("reset factory value : %d\n", !reset);
            reset = !reset;
            g_exit = 1;
        }
        usleep(100000);
    }

    hdinfo("\n");
    for (i=0; i<max_alarm_in; i++)
    {
        if (pass[i] == 0)
        {
            hderr("alarm in[%d] failed\n", i+1);
            flag = 1;
        }
    }

    if (flag)
    {
        SHOW_NO;
    }
    else
    {
        SHOW_YES;
    }
}

static void test_alarm_out()
{
    int count = 2;
    int dataToMcu = -1;
    unsigned char ver[64] = {0};
    int len, i, j;

    hdinfo(">>>>test_alarm_out\n");
    printf("expection : loop alarm_out 0->alrm out 3->buzz\n");

    if (get_hardware_version(ver)) {
        hdinfo(">>>>get_hardware_version fail\n");
        return NULL;
    }
#if defined(_HI3536A_)
    if (ver[3] == '7' || ver[3] == '8') {
        dataToMcu = 0;
    }
#endif
    g_exit = 0;
    while(!g_exit && count) {
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

static void test_state_led()
{
    int count = 2;
    
    hdinfo(">>>>test_state_led\n");
    printf("expection : loop led greed->red->HD\n");
    g_exit = 0;
    while(!g_exit && count)
    {
        gpio_write(STATE_LED_GREEN, 0);
        usleep(500000);
        gpio_write(STATE_LED_GREEN, 1);
        usleep(500000);        
//        gpio_write(STATE_LED_RED, 0);
//        usleep(500000);
//        gpio_write(STATE_LED_RED, 1);
//        usleep(500000);        
        gpio_write(STATE_LED_HDD, 0);
        usleep(500000);
        gpio_write(STATE_LED_HDD, 1);
        usleep(500000);
        count--;
    }
}

static void test_mcu_pin()
{
    int key = 0;
    int count = 0;
    
    hdinfo(">>>>test_mcu_pin\n");
    printf("expection : short press SW will print 'YES'\n");

    gpio_write(GIO_MCU_P1_1, 0);
    usleep(10000);
    gpio_write(GIO_MCU_P1_1, 1);
    usleep(1000000);
//    gpio_write(GIO_MCU_P1_1, 0);
    
    g_exit = 0;
    while(!g_exit)
    {
        key = gpio_read(GIO_MCU_P1_0);
        if (key == 1)
        {
            count++;
            key = 0;
        }
        if (count >= 2)
        {
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
    printf("expection : short press SW will print 'YES'\n");
    ms_system("killall -9 fio");
    g_exit = 0;
    while(!g_exit)
    {
        if (read(fd, buff, sizeof(buff)) == 3)
        {
            printf("hardware buff[%x][%x][%x]\n", buff[0], buff[1], buff[2]);
            if (!memcmp(buff, cmd, sizeof(buff))) 
            {
                SHOW_YES;
                break;
            }
        }
        usleep(500000);
    }
    close(fd);
}

static void test_fan_pin()
{
    hdinfo(">>>>test_fan_pin\n");
    printf("expection : fan run 1s -> stop 0.5s -> run 1s ->stop\n");

    gpio_write(GIO_FAN_EN, 0);  
    g_exit = 0;
    while(!g_exit)
    {
        gpio_write(GIO_FAN_EN, 1);
        usleep(1000000);
        gpio_write(GIO_FAN_EN, 0);
        usleep(500000);
        gpio_write(GIO_FAN_EN, 1);
        usleep(1000000);
        gpio_write(GIO_FAN_EN, 0);  
    }
    gpio_write(GIO_FAN_EN, 0);  
}

static int test_start()
{
    int res = -1;
    char ch;
	char *hard_ver = NULL;
    unsigned char ver[64] = {0};

    gpio_open();

//    system("clear");
    res = get_hardware_version(ver);
    if (res != 1)
    {
        switch(ver[3])
        {
            case '8': 
            #if defined(_HI3536A_)
                ms_system("killall -9 fio");
                g_fd = open(USART_DEV, O_RDWR | O_NOCTTY | O_NONBLOCK );
                if (g_fd < 0) {
                    printf("Can't Open usart Port!\n");
                    return -1;
                }
            #endif
                printf("N04 1: alarm in 2: alarm out 3: state led 4: MCU 5: fan q:quit\n");
                max_alarm_in = 16;
                max_alarm_out = 4;
                break;
            case '5':            
                printf("N02 1: alarm in 2: alarm out 3: state led 5: fan  q:quit\n");
                max_alarm_in = 4;
                max_alarm_out = 1;
                break;
            case '1':            
                printf("N02 1: alarm in 2: alarm out 3: state led 5: fan q:quit\n");
                max_alarm_in = 0;
                max_alarm_out = 0;
                break;
            default:
                printf("N03 1: alarm in 2: alarm out 3: state led 4: MCU 5: fan q:quit\n");
                max_alarm_in = 16;
                max_alarm_out = 4;
                break;
        }
    }
    else
    {
        hdinfo("**********encrypt chip is failed !!**********\n");
        return -1;
    }
//    hdinfo("\n\n\n**********gpio_test_start**********\n\n\n");
    if (g_isAuto)
    {
        test_alarm_in();
        test_alarm_out();
        if (ver[3] != '1')
        {
            test_state_led();
        }
        if (ver[3] == '5' || (ver[3] == '1' && ver[15] == 'P'))
        {
//            test_mcu_pin();
        }
        else if (ver[3] == '7' && ver[15] == 'P' && ver[16] == '6') //7032(24POE)
        {
            test_mcu_pin_usart();
        }
        else
        {
			hard_ver = ms_getenv("hard_version");
			if (hard_ver != NULL  && hard_ver[8] >= '2')
				test_mcu_pin_usart();
            else
				test_mcu_pin();
        }
        test_fan_pin();
        system("df");
		system("ls -al /dev/MF*");
    }
    else
    {
        printf("choose : ");
        ch = getchar();
        switch(ch)
        {
            case '1':
            {
                test_alarm_in();
                break;
            }
            case '2':
            {
                test_alarm_out();
                break;
            }
            case '3':
            {
                test_state_led();
                break;
            }
            case '4':
            {
                test_mcu_pin();
                break;
            }
            case '5':
            {
                test_fan_pin();
                break;
            }
            case 'q':
            default :
            {
                g_exit = 1;
                break;
            }
        }
    }

//    hdinfo("**********gpio_test_stop**********\n");
    return 0;
}

static int test_stop()
{
    g_exit = 1;
    if (g_fd) {
        close(g_fd);
    }
//    gpio_close();
    return 0;
}

HD_MODULE_INFO(TEST_NAME_GPIO, test_start, test_stop);

