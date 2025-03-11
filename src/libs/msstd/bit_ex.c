#include "msstd.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//get bit No.p (from right) of value to see if it is on
int ms_get_bit(unsigned long long value, int p) 
{ 
    if (p < 0) 
		return -1; 
    if ((value >> p) & 1) 
		return 1; 
 	return 0; 
} 
//set bit No.p (from right) of value on/off
int ms_set_bit(unsigned long long* value, int p, int on)
{ 
	if(p < 0) 
		return -1; 
	if(on) 
		(*value) = (*value) | ((long long)(1) << p);
	else 
		(*value) = (*value) & ~((long long)(1) << (p));
	return 0;
} 
//get n bits of value at position p (from right) 
unsigned long long ms_get_bits(unsigned long long value, int p, int n) 
{ 
	//int intsize=int_size();
	if(p < 0 || n < 0) 
		return 0; 
    return (long long)((value >> p) & ~(~(long long)(0) << n));
}

void ms_printf_bits(unsigned long long value)
{
	int i = 64;
	for( ; i > 0; i--)
	{
		printf("%d", ms_get_bit(value, i-1));
	}
	printf("\n");
}

