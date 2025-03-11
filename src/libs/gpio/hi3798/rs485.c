/* 
 * ***************************************************************
 * Filename:      	rs485.c
 * Created at:    	2016.04.30
 * Description:   	RS485 API
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
#include     <stdio.h>      /*标准输入输出定义*/
#include     <stdlib.h>     /*标准函数库定义*/
#include     <unistd.h>     /*Unix标准函数定义*/
#include     <sys/types.h>  /**/
#include     <sys/stat.h>   /**/
#include     <fcntl.h>      /*文件控制定义*/
#include     <termios.h>    /*PPSIX终端控制定义*/
#include     <errno.h>      /*错误号定义*/
#include     <string.h>
#include     "gpio.h"

static struct termios termios_new;

static int BAUDRATE (int baudrate)
{
    switch (baudrate) {
    case 0:
        return (B0);
    case 50:
        return (B50);
    case 75:
        return (B75);
    case 110:
        return (B110);
    case 134:
        return (B134);
    case 150:
        return (B150);
    case 200:
        return (B200);
    case 300:
        return (B300);
    case 600:
        return (B600);
    case 1200:
        return (B1200);
    case 2400:
        return (B2400);
    case 9600:
        return (B9600);
    case 19200:
        return (B19200);
    case 38400:
        return (B38400);
    case 57600:
        return (B57600);
    case 115200:
        return (B115200);
    default:
        return (B9600);
    }
}
static void SetBaudrate (int baudrate)
{
    termios_new.c_cflag = BAUDRATE (baudrate);  /* set baudrate */
}

static void SetDataBit (int databit)
{
    termios_new.c_cflag &= ~CSIZE;
    switch (databit) 
    {
        case 8:
            termios_new.c_cflag |= CS8;
            break;
        case 7:
            termios_new.c_cflag |= CS7;
            break;
        case 6:
            termios_new.c_cflag |= CS6;
            break;
        case 5:
            termios_new.c_cflag |= CS5;
            break;
        default:
            termios_new.c_cflag |= CS8;
            break;
    }
}

static void SetStopBit (int stopbits)
{
    switch (stopbits)
      {
          case 1:
              termios_new.c_cflag &= ~CSTOPB;
              break;
          case 2:
              termios_new.c_cflag |= CSTOPB;
              break;
          default:
              fprintf(stderr,"Unsupported stop bits\n");
              break;
      }

}

static void SetParityCheck (char parity)
{
    switch (parity)
    {
        case 'N':                  /* no parity check */
            termios_new.c_cflag &= ~PARENB;
            break;
        case 'E':                  /* even */
            termios_new.c_cflag |= PARENB;
            termios_new.c_cflag &= ~PARODD;
            break;
        case 'O':                  /* odd */
            termios_new.c_cflag |= PARENB;
            termios_new.c_cflag |= ~PARODD;
            break;
        default:                   /* no parity check */
            termios_new.c_cflag &= ~PARENB;
            break;
    }
}

int rs485SetPortAttr (int fd,int baudrate, int databit, int stopbit, char parity)
{
    bzero (&termios_new, sizeof (termios_new));
    cfmakeraw (&termios_new);
    SetBaudrate (baudrate);
    termios_new.c_cflag |= CLOCAL | CREAD;      /* | CRTSCTS */
   // termios_new.c_cflag |=CRTSCTS;
   // termios_new.c_cflag &= ~CRTSCTS;      /*  | CRTSCTS */

    SetDataBit (databit);
    SetParityCheck (parity);
    SetStopBit (stopbit);
    termios_new.c_oflag = 0;
    termios_new.c_lflag |= 0;
    termios_new.c_oflag &= ~OPOST;
    termios_new.c_cc[VTIME] = 1;        /* unit: 1/10 second. */
    termios_new.c_cc[VMIN] = 1; /* minimal characters for reading */
    tcflush (fd, TCIFLUSH);
    return (tcsetattr (fd, TCSANOW, &termios_new));
}

int rs485OpenDev(char *Dev)
{
    int	fd = open( Dev, O_RDWR | O_NOCTTY | O_NONBLOCK );
    if (-1 == fd)
    { /*设置数据位数*/
        perror("Can't Open Serial Port");
        return -1;
    }
    
    return fd;
}

void rs485CloseDev(int fd)
{
    close(fd);
}

int rs485FullWrite(int fd, void *data, int len)
{
    int res = 0; 
    int nlen = 0;
    struct timeval TimeoutVal;    
    fd_set fs_write;
    
    FD_ZERO(&fs_write);
    FD_SET(fd, &fs_write);
    TimeoutVal.tv_sec  = 1;
    TimeoutVal.tv_usec = 0;
    
    res = select(fd + 1, NULL, &fs_write, NULL, &TimeoutVal);    
    if (res > 0)
    {
        if (FD_ISSET(fd, &fs_write))
        {
            nlen = write(fd, data, len);
        }
    }
    
    return nlen;
}

int rs485FullRead(int fd, void *data, int len)
{
    int res = 0; 
    int nlen = 0;
    struct timeval TimeoutVal;    
    fd_set fs_read;
    
    FD_ZERO(&fs_read);
    FD_SET(fd, &fs_read);
    TimeoutVal.tv_sec  = 1;
    TimeoutVal.tv_usec = 0;
    
    res = select(fd + 1, &fs_read, NULL, NULL, &TimeoutVal);    
    if (res > 0)
    {
        if (FD_ISSET(fd, &fs_read))
        {
            nlen = read(fd, data, len);
        }
    }
    
    return nlen;
}

int rs485HalfWrite(int fd, void *data, int len)
{
    int res = 0; 
    int nlen = 0;
    struct timeval TimeoutVal;    
    fd_set fs_write;
    
    FD_ZERO(&fs_write);
    FD_SET(fd, &fs_write);
    TimeoutVal.tv_sec  = 1;
    TimeoutVal.tv_usec = 0;
    
    gpio_write(GIO_RS485_CTL_EN, RS485_DIR_TX);
    res = select(fd + 1, NULL, &fs_write, NULL, &TimeoutVal);    
    if (res > 0)
    {
        if (FD_ISSET(fd, &fs_write))
        {
            nlen = write(fd, data, len);
        }
    }
    
    return nlen;

}

int rs485HalfRead(int fd, void *data, int len)
{
    int res = 0; 
    int nlen = 0;
    struct timeval TimeoutVal;    
    fd_set fs_read;
    
    FD_ZERO(&fs_read);
    FD_SET(fd, &fs_read);
    TimeoutVal.tv_sec  = 1;
    TimeoutVal.tv_usec = 0;
    
    gpio_write(GIO_RS485_CTL_EN, RS485_DIR_RX);
    res = select(fd + 1, &fs_read, NULL, NULL, &TimeoutVal);    
    if (res > 0)
    {
        if (FD_ISSET(fd, &fs_read))
        {
            nlen = read(fd, data, len);
        }
    }
    
    return nlen;
}


