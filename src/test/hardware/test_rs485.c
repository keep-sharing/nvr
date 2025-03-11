#include    <stdio.h>      /*标准输入输出定义*/
#include    <stdlib.h>     /*标准函数库定义*/
#include    <unistd.h>     /*Unix标准函数定义*/
#include    <sys/types.h>  /**/
#include    <sys/stat.h>   /**/
#include    <fcntl.h>      /*文件控制定义*/
#include    <termios.h>    /*PPSIX终端控制定义*/
#include    <errno.h>      /*错误号定义*/
#include    <string.h>

#include    "hardware.h"
#include    "gpio.h"
#include    "rs485.h"

typedef void(*TEST_FUNC)(void);

static int g_exit = 0;
static int fd_full = -1;
static int fd_half = -1;

static void test_rs485_init()
{
#if defined(_HI3536A_)
	char *dev_full ="/dev/ttyAMA3";
	char *dev_half ="/dev/ttyAMA2";
#else
	char *dev_full ="/dev/ttyAMA1";
	char *dev_half ="/dev/ttyAMA3";
#endif
    
	fd_full = rs485OpenDev(dev_full);
	if (fd_full<0)
	{
    	printf("Can't Open dev_full Serial Port!\n");
    	exit(0);
	}

    if (rs485SetPortAttr(fd_full, 9600, 8, 1, 'N') < 0)
    {
        printf("fd_full Set SetPortAttr Error\n");
        exit(1);
    }

	fd_half = rs485OpenDev(dev_half);
	if (fd_half<0)
	{
    	printf("Can't Open dev_half Serial Port!\n");
    	exit(0);
	}
	
    if (rs485SetPortAttr(fd_half, 9600, 8, 1, 'N') < 0)
    {
        printf("fd_full Set SetPortAttr Error\n");
        exit(1);
    }

}

static void test_rs485_deinit()
{
    if (fd_full != -1)
    {
        rs485CloseDev(fd_full);
    }
    
    if (fd_half != -1)
    {
        rs485CloseDev(fd_half);
    }

}

static void test_full_tx_half_rx()
{
    char txbuff[64] = "test_full_tx_half_rx";
    char rxbuff[64];
    int len = 0;
//    int count = 0;

    hdinfo("\n>>>>>>full_tx->half_rx\n");
    g_exit = 0;
    while(!g_exit)
    {
        memset(rxbuff, 0, sizeof(rxbuff));
        
        rs485FullWrite(fd_full, txbuff, sizeof(txbuff));
        
        len = rs485HalfRead(fd_half, rxbuff, 64);
        
        if (len)
        {
//            printf("Half recevie [%d] data : %s\n", len, rxbuff);
            if (strcmp(txbuff, rxbuff) == 0)
            {
                SHOW_YES;
                g_exit = 1;            
            }
            len = 0;
        } 

//        if (count > 2)
//        {
//            SHOW_NO;
//            g_exit = 1;
//        }
//        count++;
        
        usleep(500000);
    }
}

static void test_full_rx_half_tx()
{
    char txbuff[64] = "test_full_rx_half_tx";
    char rxbuff[64];
    int len = 0;
//    int count = 0;
    
    hdinfo("\n>>>>>>full_rx->half_tx\n");
    g_exit = 0;
    while(!g_exit)
    {
        memset(rxbuff, 0, sizeof(rxbuff));
        
        len = rs485HalfWrite(fd_half, txbuff, sizeof(txbuff));
        len = rs485FullRead(fd_full, rxbuff, 64);
        
        if (len)
        {
//            printf("Full recevie [%d] data : %s\n", len, rxbuff);
            if (strcmp(txbuff, rxbuff) == 0)
            {
                SHOW_YES;
                g_exit = 1;            
            }  
            len = 0;
        }      

//        if (count > 2)
//        {
//            SHOW_NO;
//            g_exit = 1;
//        }
//        count++;
        usleep(500000);
    }

}

static void test_half_tx()
{
    char txbuff[] = "test_half_tx";
    int len = 0;
    
    hdinfo("\n>>>>>half_tx\n");
    g_exit = 0;
    while(!g_exit)
    {        
        len = rs485HalfWrite(fd_half, txbuff, sizeof(txbuff));        
        if (len)
        {
//            printf("rs485Half send [%d] data : %s\n", len, txbuff);
        }
        usleep(500000);
    }
}

static void test_half_rx()
{
    char rxbuff[64];
    int len = 0;
//    int count = 0;

    hdinfo("\n>>>>>half_rx\n");
    g_exit = 0;
    while(!g_exit)
    {
        memset(rxbuff, 0, sizeof(rxbuff));
        
        len = rs485HalfRead(fd_half, rxbuff, 64);        
        if (len)
        {
//            printf("rs485Half receive [%d] data : %s\n", len, rxbuff);
            if (strcmp(rxbuff, "test_half_tx") == 0)
            {
                SHOW_YES;
                g_exit = 1;            
            }
            len = 0;
        }
//        if (count > 2)
//        {
//            SHOW_NO;
//            g_exit = 1;
//        }
//        count++;
        usleep(500000);
    }
}

static int test_start()
{
    char ch;
    int res = -1;
    unsigned char ver[64] = {0};
    TEST_FUNC cb_test_1, cb_test_2;
    
//    system("clear");
//    hdinfo("\n\n\n**********rs485_test_start**********\n\n");
    res = get_hardware_version(ver);
    if (res != -1)
    {
        switch(ver[3])
        {
            case '8': 
                printf("N04  1:full_tx->half_rx  2: full_rx->half_tx q:quit\n");
                test_rs485_init();
                cb_test_1 = test_full_tx_half_rx;
                cb_test_2 = test_full_rx_half_tx;
                break;
            case '5':
#if defined (_HI3536_)
                test_rs485_init();
                printf("N02  1:half_rx 2:half_tx q:quit\n");  
                cb_test_1 = test_half_rx;
                cb_test_2 = test_half_tx;
#else
                cb_test_1 = NULL;
                cb_test_2 = NULL;
#endif
                break;
            case '1':
                cb_test_1 = NULL;
                cb_test_2 = NULL;
                break;
            default :
                test_rs485_init();
                printf("N03  1:full_tx->half_rx  2: full_rx->half_tx q:quit\n");
                cb_test_1 = test_full_tx_half_rx;
                cb_test_2 = test_full_rx_half_tx;
                break;
            }
    }
    if (g_isAuto)
    {
        if (cb_test_1)
            cb_test_1();
        if (cb_test_2)
            cb_test_2();
    }
    else
    {
        printf("choose : ");
        ch = getchar();        
        switch(ch)
        {
            case '1':
            {
                if (cb_test_1)
                    cb_test_1();
                break;
            }
            case '2':
            {
                if (cb_test_2)
                    cb_test_2();
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

    test_rs485_deinit();
//    hdinfo("**********rs485_test_stop**********\n");
    return 0;
}

static int test_stop()
{
    g_exit = 1;
    return 0;
}

HD_MODULE_INFO(TEST_NAME_RS485, test_start, test_stop);

