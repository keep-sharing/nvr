#include "msstd.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


int ms_string_strip(char s[], char c)
{ 
    int i = 0;
    int j = 0; 
    for (i = 0, j = 0; s[i] != '\0'; i++) 
    { 
        if (s[i] != c) 
        { 
            s[j++] = s[i]; 
        } 
    } 
    s[j] = '\0';
	
	return 0;
} 
static char base64chars[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
/*
 * Name: base64encode()
 *
 * Description: Encodes a buffer using BASE64.
 */
void ms_base64_encode(unsigned char *from, char *to, int len)
{
	int i, j;
	unsigned char current;

	for ( i = 0, j = 0 ; i < len ; i += 3 )
	{
		current = (from[i] >> 2) ;
		current &= (unsigned char)0x3F;
		to[j++] = base64chars[(int)current];

		current = ( (unsigned char)(from[i] << 4 ) ) & ( (unsigned char)0x30 ) ;
		if ( i + 1 >= len )
		{
			to[j++] = base64chars[(int)current];
			to[j++] = '=';
			to[j++] = '=';
			break;
		}
		current |= ( (unsigned char)(from[i+1] >> 4) ) & ( (unsigned char) 0x0F );
		to[j++] = base64chars[(int)current];

		current = ( (unsigned char)(from[i+1] << 2) ) & ( (unsigned char)0x3C ) ;
		if ( i + 2 >= len )
		{
			to[j++] = base64chars[(int)current];
			to[j++] = '=';
			break;
		}
		current |= ( (unsigned char)(from[i+2] >> 6) ) & ( (unsigned char) 0x03 );
		to[j++] = base64chars[(int)current];

		current = ( (unsigned char)from[i+2] ) & ( (unsigned char)0x3F ) ;
		to[j++] = base64chars[(int)current];
	}
	to[j] = '\0';
	return ;	
}

int ms_base64_decode( const unsigned char * base64, unsigned char * bindata, int bindata_len)
{
    int i, j;
    unsigned char k;
    unsigned char temp[4];
    for ( i = 0, j = 0; base64[i] != '\0' ; i += 4 )
    {
        memset( temp, 0xFF, sizeof(temp) );
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64chars[k] == base64[i] )
                temp[0]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64chars[k] == base64[i+1] )
                temp[1]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64chars[k] == base64[i+2] )
                temp[2]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64chars[k] == base64[i+3] )
                temp[3]= k;
        }
		if(j == bindata_len-1)
			break;
        bindata[j++] = ((unsigned char)(((unsigned char)(temp[0] << 2))&0xFC)) |
                ((unsigned char)((unsigned char)(temp[1]>>4)&0x03));
		if(j == bindata_len-1)
			break;
        if ( base64[i+2] == '=' )
            break;

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[1] << 4))&0xF0)) |
                ((unsigned char)((unsigned char)(temp[2]>>2)&0x0F));
		if(j == bindata_len-1)
			break;
        if ( base64[i+3] == '=' )
            break;

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[2] << 6))&0xF0)) |
                ((unsigned char)(temp[3]&0x3F));
			if(j == bindata_len-1)
			break;
    }
    return j;
}

static char BI_RM[] = "0123456789abcdefghijklmnopqrstuvwxyz";
int Base64to16( char *s,char *ret) 
{
        int i,j;
        int k = 0;
        int v,slop;
        int l=0;
        for (i = 0; i < strlen(s); ++i)
        {
            if (s[i] == '=') {
                break;
                
                }
            for(j=0;j<64;j++){
                if(base64chars[j]==s[i])
                    v=j;
            }
            if (v < 0) continue;
            if (k == 0)
            {
                ret[l]= BI_RM[(v >> 2)];
                slop = v & 3;
                k = 1;
            }
            else if (k == 1)
            {
                ret[l]=  BI_RM[((slop << 2) | (v >> 4))];
                slop = v & 0xf;
                k = 2;
            }
            else if (k == 2)
            {
                ret[l++]=  BI_RM[slop];
                ret[l]=  BI_RM[(v >> 2)];
                slop = v & 3;
                k = 3;
            }
            else
            {
                ret[l++]=  BI_RM[((slop << 2) | (v >> 4))];
                ret[l]=  BI_RM[(v & 0xf)];
                k = 0;
            }
            if(i<strlen(s))
                l++;
        }
        if (k == 1) ret[l]=  BI_RM[(slop << 2)];
        return l;
 }

char* hex_2_base64(char *_hex)
{
  char *hex_2_bin[16] = { "0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111", "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111" };
  char *dec_2_base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  //allocating memory for binary string
  int bin_size = strlen(_hex) * 4;
  while (bin_size % 6 != 0) //add space for zero padding
    bin_size += 8;
  char *bin = malloc(bin_size + 1);
  memset(bin, 0, bin_size + 1);

  //these are for strtol, its arguments need the zero terminator
  char buf[2] = { 0 };
  char b64buf[6 + 1] = { 0 };

  //converting hex input to binary
  char *bin_end = bin;
  int i = 0;
  for ( ;i < strlen(_hex); i++)
  {
    buf[0] = _hex[i];
    memcpy(bin_end, hex_2_bin[strtol(buf, NULL, 16)], 4);
    bin_end += 4;
  }

  //pad binary string w/ zeroes
  while (strlen(bin) < bin_size)
    strcat(bin, "00000000");

  //allocating memory for b64 output
  int b64size = (strlen(bin) / 6) + 1;
  char *out = malloc(b64size);
  memset(out, 0, b64size);

  //walk through binary string, converting chunks of 6 bytes into base64 chars
  char *bin_ptr = bin;
  char *out_end = out;
  int index_b64;
  while (*bin_ptr)
  {
    strncpy(b64buf, bin_ptr, 6);
    index_b64 = strtol(b64buf, NULL, 2);
    if (index_b64 == 0)
      buf[0] = '=';
    else
      buf[0] = dec_2_base64[index_b64];
    memcpy(out_end, buf, 1);
    out_end += 1;
    bin_ptr += 6;
  }

  free(bin);
  return out;
}

int url_decode(char *dst, const char *src, int len)
{
    int a=0, b=0;
    int c;

    while (b<len)
	{
        //if (src[b]=='+')
        if (0)
		{
            dst[a++]=' ';
            b++;
        }
        else if (src[b]=='%')
		{
            if (sscanf(&src[b+1],"%2x",&c)>0)
        	{
				dst[a++]=c;
        	}
            b+=3;
        }
        else
		{
			dst[a++]=src[b++];	
		}
	}
    dst[a++]=0;
    return (a);
}
int url_encode(char *dst, const char *src, int len)
{
    int a=0, b=0;

    while (b<len)
	{
        if (src[b] == ' ')
		{
            dst[a++] = '%';  
            dst[a++] = '2';  
            dst[a++] = '0';  
            b++;
        }
        else if (src[b] == '\n')
		{
			dst[a++] = '%';  
            dst[a++] = '0';  
            dst[a++] = 'a';  
            b++;
        }
		else if(src[b] == '+')
		{
			dst[a++] = '%';  
            dst[a++] = '2';  
            dst[a++] = 'B';  
            b++;
        }
        else
		{
			dst[a++]=src[b++];	
		}
	}
    dst[a++]=0;
    return (a);
}
int sqa_encode(char *dst, const char *src, int len)
{
	int a=0, b=0;

    while (b<len)
	{
        if (src[b] == '&')
		{
            dst[a++] = '%';  
            dst[a++] = '2';  
            dst[a++] = '6';  
            b++;
        }
		else if(src[b] == '%')
		{
            dst[a++] = '%';  
            dst[a++] = '2';  
            dst[a++] = '5';  
            b++;
        }
		else if(src[b] == '<')
		{
            dst[a++] = '%';  
            dst[a++] = '3';  
            dst[a++] = 'C';  
            b++;
        }
		else if(src[b] == '>')
		{
            dst[a++] = '%';  
            dst[a++] = '3';  
            dst[a++] = 'E';  
            b++;
        }
        else
		{
			dst[a++]=src[b++];	
		}
	}
    dst[a++]=0;
    return (a);
}

static char dec2hex(short int c)
{
	if (0 <= c && c <= 9) {
		return c + '0';
	} else if (10 <= c && c <= 15) {
		return c + 'A' - 10;
	} else {
		return -1;
	}
}

void get_url_encode(const char *url, const int strSize, char *res, const int resultSize)
{
	int i = 0, j = 0;
	int res_len = 0;
    char c = '\0';
    
	for (i = 0; i < strSize; ++i) 
    {
		c = url[i];
		if (('0' <= c && c <= '9') ||
				('a' <= c && c <= 'z') ||
				('A' <= c && c <= 'Z')) 
        {
            if (res_len < resultSize-1)
            {
                res[res_len++] = c;
            }
            else
            {
                break;
            }
		}
        else
        {
			j = (short int)c;
			if (j < 0)
				j += 256;
			int i1, i0;
			i1 = j / 16;
			i0 = j - i1 * 16;
            if (res_len+3 < resultSize-1)
            {
                res[res_len++] = '%';
                res[res_len++] = dec2hex(i1);
                res[res_len++] = dec2hex(i0);
            }
            else
            {
                break;
            }
		}
	}
    
	res[res_len] = '\0';
    return;
}

void get_url_decode(char *url)
{
    int i = 0, j = 0;
	unsigned int high;
    while (url[j] != 0x0)
    {
        if (url[j] == '%')
        {
            sscanf(url+j, "%%%2X", &high);
			j += 3;
			url[i++] = (char)high;
        }
        else
        {
        	url[i++] = url[j];
			j++;
        }
    }
	url[i++] = 0x0;
}


//////////////////////////////////////////////////////////////////
#if 0
struct ms_uri_note
{
	char key[128];
	char value[128];
	AST_LIST_ENTRY(ms_uri_note) list;
};
static AST_LIST_HEAD_NOLOCK_STATIC(uri_list, ms_uri_note);

static char hex_ascii[256] = {
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
	0x08, 0x09,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1
};

static int ascii_to_hex( const unsigned char *data )
{
	return ( (int)(hex_ascii[*data]<<4) | (int)hex_ascii[*(data+1)] );
}

int ms_uri_remove()
{
	
}

int ms_uri_parse(char *data)
{
	int nTemp = 0;
	static char *zero = "\0";
	char *buf, *buf_end;
	register char chbuf;
	int cmd_count = 0;

	if (data == NULL)
		return -1;
	printf("req->query = %s\n", data);
	buf = (char *) malloc(MAX_CMD_LENGTH);
	if (buf == NULL)
		return -1;
	buf_end = buf+MAX_CMD_LENGTH-1;
	
	do  {
		int hex;
		chbuf = *data++;
		
		switch ( chbuf )
		{
		case '+':
			chbuf = ' ';
			break;
		case '%':
			hex = ascii_to_hex((unsigned char*)data);
			if ( hex > 0 ) {
				chbuf = (char)hex;
				data += 2;
			}
			break;
		case '\0':
		case '\n':
		case '\r':
		case ' ':
			*buf = '\0';
			cmd_count++;
			//end
			return 0;
		}
		
		switch ( chbuf )
		{
		case '&':
			cmd_count++;
			*buf++ = '\0';
			req->cmd_arg[cmd_count].name = buf;
			req->cmd_arg[cmd_count].value = zero;
			nTemp = 0;
			break;
		case '=':
			if(nTemp == 1){
				*buf++ = chbuf;
			}else{
				*buf++ = '\0';
				req->cmd_arg[cmd_count].value = buf;
			}
			nTemp = 1;
			break;
		default:
			*buf++ = chbuf;
			break;
		}
	} while (buf < buf_end);
	*buf = '\0';
	cmd_count++;

	return 0;
}
#endif


unsigned char *get_base64_decode(char *code)  
{  
//根据base64表，以字符找到对应的十进制数据  
    int table[]={0,0,0,0,0,0,0,0,0,0,0,0,
    		 0,0,0,0,0,0,0,0,0,0,0,0,
    		 0,0,0,0,0,0,0,0,0,0,0,0,
    		 0,0,0,0,0,0,0,62,0,0,0,
    		 63,52,53,54,55,56,57,58,
    		 59,60,61,0,0,0,0,0,0,0,0,
    		 1,2,3,4,5,6,7,8,9,10,11,12,
    		 13,14,15,16,17,18,19,20,21,
    		 22,23,24,25,0,0,0,0,0,0,26,
    		 27,28,29,30,31,32,33,34,35,
    		 36,37,38,39,40,41,42,43,44,
    		 45,46,47,48,49,50,51
    	       };  
    long len;  
    long str_len;  
    unsigned char *res;  
    int i,j;  
  
//计算解码后的字符串长度  
    len = strlen(code);  
//判断编码后的字符串后是否有=  
    if (strstr(code,"=="))  
        str_len = len/4*3-2;  
    else if (strstr(code,"="))  
        str_len = len/4*3-1;  
    else  
        str_len = len/4*3;  
  
    res = ms_malloc(str_len+1);  
    res[str_len] = '\0';  
  
	//以4个字符为一位进行解码  
    for (i = 0,j = 0; i < len-2; j += 3,i += 4)  
    {  
    	int id0 =  code[i];
		int id1 =  code[i+1];
		int id2 =  code[i+2];
		int id3 =  code[i+3];
        res[j]   = ((unsigned char)table[id0])<<2 | (((unsigned char)table[id1])>>4); 	//取出第一个字符对应base64表的十进制数的前6位与第二个字符对应base64表的十进制数的后2位进行组合  
        res[j+1] = (((unsigned char)table[id1])<<4) | (((unsigned char)table[id2])>>2); //取出第二个字符对应base64表的十进制数的后4位与第三个字符对应bas464表的十进制数的后4位进行组合  
        res[j+2] = (((unsigned char)table[id2])<<6) | ((unsigned char)table[id3]); 		//取出第三个字符对应base64表的十进制数的后2位与第4个字符进行组合  
    }  
  
    return res;  
}


//need ms_free
char *get_base64_encode(char *str, int str_len)  
{  
    long len;  
    char *res = NULL;  
    int i = 0,j = 0;  
	//定义base64编码表  
    char *base64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";  

    if (!str || !str_len) {
        return NULL;
    }
    
	//计算经过base64编码后的字符串长度  
    if(str_len % 3 == 0)  
        len = str_len/3*4;  
    else  
        len = (str_len/3+1)*4;  
  
    res = ms_malloc(len+1);  
    res[len] = '\0';
  
	//以3个8位字符为一组进行编码  
    for(i = 0,j = 0; i < len-2; j += 3,i += 4)  
    {  
        res[i]   = base64_table[str[j]>>2]; 						//取出第一个字符的前6位并找出对应的结果字符  
        res[i+1] = base64_table[(str[j]&0x3)<<4 | (str[j+1]>>4)]; 	//将第一个字符的后位与第二个字符的前4位进行组合并找到对应的结果字符  
        res[i+2] = base64_table[(str[j+1]&0xf)<<2 | (str[j+2]>>6)];	//将第二个字符的后4位与第三个字符的前2位组合并找出对应的结果字符  
        res[i+3] = base64_table[str[j+2]&0x3f];						//取出第三个字符的后6位并找出结果字符  
    }  
  
    switch(str_len % 3)  
    {  
        case 1:  
            res[i-2] = '=';  
            res[i-1] = '=';  
            break;  
        case 2:  
            res[i-1] = '=';  
            break;  
    }  
  
    return res;  
}


/*************************************************************************/

#ifndef u8
    #define u8                          MS_U8
#endif
#ifndef s8
    #define s8                          MS_S8
#endif
#ifndef s32
    #define s32                         MS_S32
#endif


#define B64_TB_ORG                      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
#define B64_TB_URL                      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"
#define B64_END_CHAR                    '='
#define STOP_CHAR                       '\0'
#define B64_ENCODE                      (MS_TRUE)
#define B64_DECODE                      (!B64_ENCODE)
#define BASE64_GET_CHAR(str, i, size)   ((i) < (size) ? (str[(i)]) : (STOP_CHAR))


static s32 base64Encode(u8 *dst, s32 dstSize,
                      u8 *src, s32 srcSize,
                      u8 *table, s32 tableSize,
                      s8 endChar)
{
    s32 i = 0;
    s32 j = 0;
    s32 m = 0;
    s32 dstLen = 0;
    s32 index[4] = {0};
    if (!dst || dstSize <= 0 || !src || srcSize <= 0 || !table || tableSize <= 0 || (s32)endChar < 0) {
        return -1;
    }

    dstLen = (srcSize + 2) / 3 * 4;
    if (dstLen >= dstSize) {
        return -2;
    }
    dst[dstLen] = STOP_CHAR;

    for (i = 0, j = 0; i < dstLen - 3 && j < srcSize; i += 4, j += 3) {
        index[0] = src[j] >> 2;
        index[1] = ((src[j] & 0x3) << 4) | (BASE64_GET_CHAR(src, j + 1, srcSize) >> 4);
        index[2] = ((BASE64_GET_CHAR(src, j + 1, srcSize) & 0xf) << 2) | (BASE64_GET_CHAR(src, j + 2, srcSize) >> 6);
        index[3] = BASE64_GET_CHAR(src, j + 2, srcSize) & 0x3f;
        if (index[0] >= tableSize
            || index[1] >= tableSize
            || index[2] >= tableSize
            || index[3] >= tableSize) {
            return -3;
        }
        for (m = 0; m < 4; m++) {
            dst[i + m] = table[index[m]];
        }
    }

    if (srcSize % 3 == 1) {
        dst[i - 2] = endChar;
        dst[i - 1] = endChar;
    } else if (srcSize % 3 == 2) {
        dst[i - 1] = endChar;
    }

    return (s32)strlen((s8 *)dst);
}

s32 base64GetDecodeLen(u8 *src, s8 endChar)
{
    s32 dstLen = -1;
    s8 endStr[3] = {0};
    s32 srcSize = strlen((s8 *)src);
    if (endChar == STOP_CHAR) {
        if (srcSize % 4 == 1) {
            return -2;
        }
        dstLen = srcSize / 4 * 3;
        dstLen += srcSize % 4 ? srcSize % 4 - 1 : 0;
    } else {
        dstLen = srcSize / 4 * 3;
        snprintf(endStr, sizeof(endStr), "%c%c", endChar, endChar);
        if (strstr((s8 *)src, endStr)) {
            dstLen -= 2;
        } else {
            snprintf(endStr, sizeof(endStr), "%c", endChar);
            if (strstr((s8 *)src, endStr)) {
                dstLen--;
            }
        }
    }

    return dstLen;
}

static s32 base64Decode(u8 *dst, s32 dstSize,
                      u8 *src, s32 srcSize,
                      u8 *table, s32 tableSize,
                      s8 endChar)
{
    s32 i = 0;
    s32 j = 0;
    s32 m = 0;
    s32 dstLen = 0;
    s32 index[4] = {0};
    s32 decodeTable[128] = {0};
    if (!dst || dstSize <= 0 || !src || srcSize <= 0 || !table || tableSize <= 0 || (s32)endChar < 0) {
        return -1;
    }

    memset(decodeTable, 0, sizeof(decodeTable));
    for (i = 0; i < tableSize; i++) {
       decodeTable[(s32)table[i]] = i;
    }

    dstLen = base64GetDecodeLen(src, endChar);
    if (dstLen <= 0 || dstLen >= dstSize) {
        return -2;
    }
    dst[dstLen] = STOP_CHAR;

    for (i = 0, j = 0; i < srcSize && j < dstLen; i += 4, j += 3) {
        for (m = 0; m < 4; m++) {
            if ((index[m] = BASE64_GET_CHAR(src, i + m, srcSize)) >= sizeof(decodeTable)) {
                return -3;
            }
        }

        dst[j] = (((u8)decodeTable[index[0]]) << 2) | (((u8)decodeTable[index[1]]) >> 4);
        if (j + 1 < dstLen) {
            dst[j + 1] = (((u8)decodeTable[index[1]]) << 4) | (((u8)decodeTable[index[2]]) >> 2);
        }
        if (j + 2 < dstLen) {
            dst[j + 2] = (((u8)decodeTable[src[i + 2]]) << 6) | ((u8)decodeTable[index[3]]);
        }
    }

    return dstLen;
}

s32 base64Codec(s32 codec,
                   u8 *dst, s32 dstSize,
                   u8 *src, s32 srcSize,
                   u8 *table, s32 tableSize,
                   s8 endChar)
{
    if (codec == B64_ENCODE) {
        return base64Encode(dst, dstSize, src, srcSize, table, tableSize, endChar);
    } else {
        return base64Decode(dst, dstSize, src, srcSize, table, tableSize, endChar);
    }
}

s32 base64EncodeOrg(u8 *dst, s32 dstSize, u8 *src, s32 srcSize)
{
    return base64Codec(B64_ENCODE, dst, dstSize, src, srcSize, (u8 *)B64_TB_ORG, strlen(B64_TB_ORG), B64_END_CHAR);
}

s32 base64DecodeOrg(u8 *dst, s32 dstSize, u8 *src, s32 srcSize)
{
    return base64Codec(B64_DECODE, dst, dstSize, src, srcSize, (u8 *)B64_TB_ORG, strlen(B64_TB_ORG), B64_END_CHAR);
}

s32 base64EncodeUrl(u8 *dst, s32 dstSize, u8 *src, s32 srcSize)
{
    return base64Codec(B64_ENCODE, dst, dstSize, src, srcSize, (u8 *)B64_TB_URL, strlen(B64_TB_URL), STOP_CHAR);
}

s32 base64DecodeUrl(u8 *dst, s32 dstSize, u8 *src, s32 srcSize)
{
    return base64Codec(B64_DECODE, dst, dstSize, src, srcSize, (u8 *)B64_TB_URL, strlen(B64_TB_URL), STOP_CHAR);
}

int check_mac_code(char *code)
{
    int i;
    int offset = 3;
    int codeSize = 17;
    char tmp;
    
    if(strlen(code) != codeSize) {
        return -1;
    }
    
    for (i = 0; i < codeSize; ) {
        if (!isxdigit(code[i])) {
            return -1;
        }
        if (!isxdigit(code[i+1])) {;
            return -1;
        }

        if (i == 0) {
            tmp = code[1];
            if (tmp == '1' || tmp == '3' || tmp == '5' || tmp == '7' || tmp == '9'
                || tmp == 'B' || tmp == 'D' || tmp == 'F'
                || tmp == 'b' || tmp == 'd' || tmp == 'f') {
                //mac地址为单播地址，第8位必须为0，即第一个字节必须是偶数
                return -1;
            }
        }
        
        if (i < 15 && code[i+2] != ':') {
            return -1;
        }
        i += offset;
    }

    return 0;
}

int check_sn_code(char *code)
{
    int i;
    char tmp;
    int len = strlen(code);

    if (len == 0) {
        return -1;
    }

    for (i = 0; i < len; i++) {
        tmp = code[i];
        if (isalnum(tmp)) {
            continue;
        }
        return -1;
    }

    return 0;
}

char *ms_strstr(char *str, char *element, const char *fileName, int line, const char *func)
{
    char *p = NULL;
    if (!str || !element) {
        msprintf("strstr warning str:[%p], element:[%p], fileName:[%s], line:[%d], func:[%s]",
            str, element, fileName, line, func);
        return p;
    }

    p = strstr(str, element);

    return p;
}

int ms_strcasecmp(const char *s1, const char *s2)
{
    if (!s1 && !s2) {
        return 0;
    }
    if (!s1) {
        return -1;
    }
    if (!s2) {
        return 1;
    }
    return strcasecmp(s1, s2);
}

int ms_get_text_section_int(const char *buff, char *keyWord, int *pOutValue)
{
    char tmp[65] = {0};

    if (ms_get_text_section_string(buff, keyWord, tmp, sizeof(tmp))) {
        return -1;
    }

    *pOutValue = atoi(tmp);
    return 0;
}

int ms_get_text_section_long(const char *buff, char *keyWord, long *pOutValue)
{
    char tmp[65] = {0};

    if (ms_get_text_section_string(buff, keyWord, tmp, sizeof(tmp))) {
        return -1;
    }

    *pOutValue = atol(tmp);
    return 0;
}

int ms_get_text_section_float(const char *buff, char *keyWord, float *pOutValue)
{
    char tmp[65] = {0};

    if (ms_get_text_section_string(buff, keyWord, tmp, sizeof(tmp))) {
        return -1;
    }

    *pOutValue = atof(tmp);
    return 0;
}


int ms_get_text_section_string(const char *buff, char *keyWord, char *pOutValue, int pOutLen)
{
    int i = 0;
    unsigned int keyLen;
    char *pstr = strstr(buff, keyWord);

    if (!pstr) {
        return -1;
    }
    keyLen = strlen(keyWord);
    while ((pstr[i + keyLen] != '\r') && (pstr[i + keyLen] != '\n') && (pstr[i + keyLen] != '\0')) {
        pOutValue[i] = pstr[i + keyLen];
        if (i++ >= pOutLen - 1) {
            break;
        }
    }

    pOutValue[i] = '\0';

    return 0;
}

int _ms_strcmp(const char *s1, const char *s2, const char *file, const char *func, int line)
{
    if (!s1 || !s2) {
        msprintf("strcmp warning s1:%p s2:%p [%s %s:%d]", s1, s2, file, func, line);
        return -1;
    }
    
    return strcmp(s1, s2);
}

