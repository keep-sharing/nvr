/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : himd.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2005/7/1
  Last Modified :
  Description   : HI Memory Modify
  Function List :
  History       :
  1.Date        : 2005/7/27
    Author      : T41030
    Modification: Created file

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "memmap.h"
//#include "hi.h"
//#include "strfunc.h"
#define T_MAX(a,b)                    (((a) > (b)) ? (a) : (b))
#define T_MIN(a,b)                    (((a) < (b)) ? (a) : (b))

#define DEFAULT_MD_LEN 128
#define PMC_BASE_ADDR 0x120E0000
#define PMC_OFFSET_PMC68 0x0110 
#define PMC_OFFSET_PMC70 0x0118
#define PMC_OFFSET_PMC71 0x011C
#define PMC_OFFSET_PMC72 0x0120
#define PMC_OFFSET_PMC73 0x0124
#define OPEN_PMC68_BIT 0x60010000
#define CLOSE_PMC68_BIT 0x00000000
#define PMC_OFFSET_LOW 0x3ff
#define PMC_OFFSET_HIGH 0x03ff0000
#define FALSE  0
#define TRUE   1

// for hi3536a
#define TSENSOR_BASE_ADDR 0x1102A000
#define TSENSOR_OFFSET_CTL0 0x0
#define TSENSOR_OFFSET_CTL2 0x8
#define TSENSOR_OFFSET_CTL3 0xC
#define TSENSOR_OFFSET_CTL4 0x10
#define TSENSOR_OFFSET_CTL5 0x14
#define OPEN_CTL0_BIT 0x401FFC00
#define CLOSE_CTL0_BIT 0x00000000

 
static int QUIT_SIGNAL = FALSE;
static void signal_handler(int signal)
{	
	printf("\nquiting...\n");
	QUIT_SIGNAL = TRUE;
}

#if defined(_HI3536_) || defined(_HI3536C_)
void enable_tempture(void)
{
	unsigned int* pSet = NULL;
	pSet = (unsigned int *)memmap(PMC_BASE_ADDR+PMC_OFFSET_PMC68, DEFAULT_MD_LEN);
	*pSet = OPEN_PMC68_BIT;
	memunmap(pSet);
}
void close_tempture(void)
{
	unsigned int* pSet = NULL;
	pSet = (unsigned int *)memmap(PMC_BASE_ADDR+PMC_OFFSET_PMC68, DEFAULT_MD_LEN);
	*pSet = CLOSE_PMC68_BIT;
	memunmap(pSet);
}
double calculate_tempture(int * pRead)
{
	int ul_low , ul_high ;
	double t_low , t_high;
	
	ul_low = *pRead & PMC_OFFSET_LOW;
	ul_high= *pRead & PMC_OFFSET_HIGH;
	ul_high >>= 16;
	t_low = ((double)(ul_low-125)/806)*165-40;
	t_high = ((double)(ul_high-125)/806)*165-40;
	
	return (t_low+t_high);
}
double Read_Tempture()
{
	int i ;
	double ulRead[4] , sum = 0; 
	int* pRead = NULL;
	
	pRead = (int *)memmap(PMC_BASE_ADDR+PMC_OFFSET_PMC70, DEFAULT_MD_LEN);
	ulRead[0] = calculate_tempture(pRead);
	memunmap(pRead);

	pRead = (int *)memmap(PMC_BASE_ADDR+PMC_OFFSET_PMC71, DEFAULT_MD_LEN);
	ulRead[1] = calculate_tempture(pRead);
	memunmap(pRead);

	pRead = (int *)memmap(PMC_BASE_ADDR+PMC_OFFSET_PMC72, DEFAULT_MD_LEN);	
	ulRead[2] = calculate_tempture(pRead);
	memunmap(pRead);

	pRead = (int *)memmap(PMC_BASE_ADDR+PMC_OFFSET_PMC73, DEFAULT_MD_LEN);	
	ulRead[3] = calculate_tempture(pRead);
	memunmap(pRead);

	for(i=0 ; i <= 3 ; i++)
	{
		sum += ulRead[i];
	}

	return (sum /= 8);	
}


#elif defined(_HI3798_)
void enable_tempture(void)
{
    ;
}
void close_tempture(void)
{
    ;
}

double Read_Tempture()
{
    FILE *pfd = NULL;
    char buff[64];
    int tempture = 0;
    
    pfd = popen("cat /proc/msp/pm_cpu | grep temperature", "r");
    if (pfd != NULL)
    if (fgets(buff, sizeof(buff), pfd) != NULL)
    {
        if (sscanf(buff, "%*[^0-9]%d", &tempture) != 1)
        {
            printf("sscanf failed \n");
        }
    }
    pclose(pfd);

    return tempture;
}
#elif defined(_NT98323_) || defined(_NT98633_)
void enable_tempture(void)
{
    ;
}
void close_tempture(void)
{
    ;
}

double Read_Tempture()
{
    FILE *pfd = NULL;
    char buff[64];
    int tempture = 0;
    
    pfd = popen("cat /sys/class/thermal/thermal_zone0/temp", "r");
    if (pfd != NULL)
    if (fgets(buff, sizeof(buff), pfd) != NULL)
    {
        if (sscanf(buff, "%d", &tempture) != 1)
        {
            printf("sscanf failed \n");
        }
    }
    pclose(pfd);

    return tempture;
}

#elif defined(_HI3536A_)
void enable_tempture(void)
{
    unsigned int *pSet = NULL;
    pSet = (unsigned int *)memmap(TSENSOR_BASE_ADDR + TSENSOR_OFFSET_CTL0, DEFAULT_MD_LEN);
    *pSet = OPEN_CTL0_BIT;
    memunmap(pSet);
}

void close_tempture(void)
{
    unsigned int *pSet = NULL;
    pSet = (unsigned int *)memmap(TSENSOR_BASE_ADDR + TSENSOR_OFFSET_CTL0, DEFAULT_MD_LEN);
    *pSet = CLOSE_CTL0_BIT;
    memunmap(pSet);
}

double calculate_tempture(int *pRead)
{
	int ul_low, ul_high;
	double t_low, t_high;

	ul_low = *pRead & PMC_OFFSET_LOW;
	ul_high = *pRead & PMC_OFFSET_HIGH;
	ul_high >>= 16;
	t_low = ((double)(ul_low - 127) / 784) * 165 - 40;
	t_high = ((double)(ul_high - 127) / 784) * 165 - 40;

	return (t_low + t_high);
}

double Read_Tempture()
{
    int i;
    double ulRead[4] , sum = 0;
    int *pRead = NULL;

    pRead = (int *)memmap(TSENSOR_BASE_ADDR + TSENSOR_OFFSET_CTL2, DEFAULT_MD_LEN);
    ulRead[0] = calculate_tempture(pRead);
    memunmap(pRead);

    pRead = (int *)memmap(TSENSOR_BASE_ADDR + TSENSOR_OFFSET_CTL3, DEFAULT_MD_LEN);
    ulRead[1] = calculate_tempture(pRead);
    memunmap(pRead);

    pRead = (int *)memmap(TSENSOR_BASE_ADDR + TSENSOR_OFFSET_CTL4, DEFAULT_MD_LEN);
    ulRead[2] = calculate_tempture(pRead);
    memunmap(pRead);

    pRead = (int *)memmap(TSENSOR_BASE_ADDR + TSENSOR_OFFSET_CTL5, DEFAULT_MD_LEN);
    ulRead[3] = calculate_tempture(pRead);
    memunmap(pRead);

    for(i = 0; i <= 3; i++){
        sum += ulRead[i];
    }

    return (sum /= 8);
}

#else
     #error YOU MUST DEFINE  CHIP_TYPE!
#endif

int main(int argc, char *argv[])
{		
	double ulTemp = 0;
	float delay_t = 1;
	double sum = 0;
	double max = 0;
	double min = 0;
	int num = -1;
	
	int count = 0;
    
	signal(SIGINT, signal_handler);
			
	if(argc < 2)
	{
		delay_t = 1;
	}
	else if (argc == 2)
	{
 		delay_t=atof(argv[1]);
	}
	else
	{
 		delay_t=atof(argv[1]);
 		num = atof(argv[2]);
	}

    enable_tempture();
    //ready....
	usleep(50000);
	//init paramters
    ulTemp = Read_Tempture();
//    printf("Current T: %.2f 'C\n",ulTemp);
    sum = ulTemp;
    max = min = ulTemp;
    count = 1;
    
	while(!QUIT_SIGNAL && num)
	{		
		ulTemp = Read_Tempture();
		printf("Current T: %.2f 'C\n",ulTemp);
		sum += ulTemp;
		max = T_MAX(max, ulTemp);
		min = T_MIN(min, ulTemp);
		count++;
		if (num > 0) num--;
		usleep(delay_t*1000000);
	}	
	
	printf("Average T: %.2f 'C Max: %.2f 'C Min: %.2f 'C\n", sum/count, max, min);
	close_tempture();
	return 0;
}

