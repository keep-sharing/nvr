#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define true 1
#define false 0

#define RST0 0xd0       /* Restart  0           */
#define SOI  0xd8       /* Start of Image       */
#define APP0 0xe0       /* application segments */
#define DQT  0xdb       /* quantization table   */
#define SOF0 0xc0       /* Start of Frame       */
#define DHT  0xc4       /* Huffman table        */
#define SOS  0xda       /* Start of Scan        */
#define EOI  0xd9       /* End of Image         */

#define EOB 0           /*  End of Block   */
#define ZRL 240         /* Zero Run Length */

#define SECTION_MARKER 	0xff
#define NO_OF_HUF_TBLS 	4
#define MAX_NODES 		256
#define MAX_COMPONENTS 	3
#define WORD_LEN 			8

#define ic1  				251
#define is1  				50
#define ic3  				213
#define is3  				142
#define ir2c6  			277
#define ir2s6  			669
#define ir2  				181

#define PXL_FMT_YUV420		0
#define PXL_FMT_YUV422		1
#define PXL_FMT_ARGB8888	2
#define PXL_FMT_ARGB4444	3
#define PXL_FMT_ARGB1555	4

typedef struct
{
   short app0_len;
   char identifier[5];
   short version;
   unsigned char units;
   short x_density;
   short y_density;
   short x_thumbnail;
   short y_thumbnail;
} JPGHEADER;

typedef struct
{
   unsigned char id;
   unsigned char samp_factor;
   unsigned char v_samp_factor;
   unsigned char h_samp_factor;
   unsigned char quant_tbl_no;
} COMPONENT;

typedef struct
{
   unsigned char precision;
   short height;
   short width;
   unsigned char components;
   COMPONENT component[3];
} SOFINFO;

typedef struct
{
   unsigned char id;
   unsigned char tbl_no;
   unsigned char ac_tbl_no;
   unsigned char dc_tbl_no;
} SCANCOMPONENT;

typedef struct
{
   unsigned char components;
   SCANCOMPONENT component[3];
} SOSINFO;

typedef struct
{
   unsigned char bits[16];
   unsigned char val[256];
} HUFTBL;

typedef struct
{
   unsigned short code;
   short size;
   short value;
   unsigned int tree_index;
} huf_node;

//unsigned short videobuf[38016];	//QCIF: 176*144*3=76032 bytes
//static unsigned short videobuf[4];	//QCIF: 176*144*3=76032 bytes
static unsigned char* videobuf;

static short reset_mcu;
static short image_started=false;
static JPGHEADER jpg_header;
static SOFINFO sof_info;
static short image_height;
static short image_width;
static SOSINFO sos_info;
static HUFTBL ac_huf_tbl[NO_OF_HUF_TBLS];
static HUFTBL dc_huf_tbl[NO_OF_HUF_TBLS];
static short quant_tbl[2][64];
static char identifier_mark[5];
static short thumbnail_size;
static short jfif_found;

static huf_node ac_node[MAX_COMPONENTS][MAX_NODES];
static huf_node dc_node[MAX_COMPONENTS][MAX_NODES];

static short h_div[MAX_COMPONENTS];
static short v_div[MAX_COMPONENTS];

static short total_nodes;
static short dc_total_nodes[MAX_COMPONENTS];
static short ac_total_nodes[MAX_COMPONENTS];
static short size,amplitude,symbol,run;
static short prev_dc_amplitude[MAX_COMPONENTS];

static short component[4][4][MAX_COMPONENTS][8][8];
static short transform[8][8];
static short block[64];
static short coefficient[64];
static short zigzag_array[64];

static unsigned char max_v_samp_factor;
static unsigned char max_h_samp_factor;

static short mcu_ctr;
static short mcu_x;
static short mcu_y;

static short mcu_width;
static short mcu_height;

static unsigned char current_word;
static short current_bit;

static unsigned short word_var;
static unsigned char byte_var;
static unsigned char three_byte_var[3];

static short done;
static int iter = 0;
static unsigned char* in_stream;

static int x0, x1, x2, x3, x4, x5, x6, x7, x8;

static unsigned int bg_width;
static unsigned int bg_height;
static unsigned int offset_x;
static unsigned int offset_y;
static unsigned int out_format =0;

static unsigned int layout=0; //0:central,1:left_up

void jpeg_dec(unsigned char* inbuf, unsigned char* outbuf);

static unsigned char stream_getc()
{
	unsigned char r = in_stream[iter];
	iter++;
	return r;
}

static void stream_read(unsigned char* buf, short len)
{
	short i;
	for(i=0; i<len; i++){
	   buf[i] = stream_getc();
	}
}


static short get_word()
{
   unsigned char byte1,byte2;
   short word;

   stream_read(&byte1,1);
   stream_read(&byte2,1);

   word=(byte1<<8)+byte2;
   return word;
}

static void new_idct(short k,short l,short m)
{
	short i;

	for (i = 0; i < 8; i ++)
	{
		x0 = (int) transform[i][0] << 9;
		x1 = (int) transform[i][1] << 7;
		x2 = (int) transform[i][2];
		x3 = (int) transform[i][3] * ir2;
		x4 = (int) transform[i][4] << 9;
		x5 = (int) transform[i][5] * ir2;
		x6 = (int) transform[i][6];
		x7 = (int) transform[i][7] << 7;
		x8 = x7 + x1;
		x1 -= x7;

		x7 = x0 + x4;
		x0 -= x4;
		x4 = x1 + x5;
		x1 -= x5;
		x5 = x3 + x8;
		x8 -= x3;
		x3 = ir2c6 * (x2 + x6);
		x6 = x3 + (- ir2c6 - ir2s6) * x6;
		x2 = x3 + (- ir2c6 + ir2s6) * x2;

		x3 = x7 + x2;
		x7 -= x2;
		x2 = x0 + x6;
		x0 -= x6;
		x6 = ic3 * (x4 + x5);
		x5 = (x6 + (- ic3 - is3) * x5) >> 6;
		x4 = (x6 + (- ic3 + is3) * x4) >> 6;
		x6 = ic1 * (x1 + x8);
		x1 = (x6 + (- ic1 - is1) * x1) >> 6;
		x8 = (x6 + (- ic1 + is1) * x8) >> 6;

		x7 += 512;
		x2 += 512;
		x0 += 512;
		x3 += 512;
		component[l][m][k][i][0] = (short) ((x3 + x4) >> 10);
		component[l][m][k][i][1] = (short) ((x2 + x8) >> 10);
		component[l][m][k][i][2] = (short) ((x0 + x1) >> 10);
		component[l][m][k][i][3] = (short) ((x7 + x5) >> 10);
		component[l][m][k][i][4] = (short) ((x7 - x5) >> 10);
		component[l][m][k][i][5] = (short) ((x0 - x1) >> 10);
		component[l][m][k][i][6] = (short) ((x2 - x8) >> 10);
		component[l][m][k][i][7] = (short) ((x3 - x4) >> 10);
	}


	for (i = 0; i < 8; i ++)
	{
		x0 = (int) component[l][m][k][0][i]<< 9;
		x1 = (int) component[l][m][k][1][i] << 7;
		x2 = (int) component[l][m][k][2][i];
		x3 = (int) component[l][m][k][3][i] * ir2;
		x4 = (int) component[l][m][k][4][i] << 9;
		x5 = (int) component[l][m][k][5][i] * ir2;
		x6 = (int) component[l][m][k][6][i];
		x7 = (int) component[l][m][k][7][i] << 7;
		x8 = x7 + x1;
		x1 -= x7;

		x7 = x0 + x4;
		x0 -= x4;
		x4 = x1 + x5;
		x1 -= x5;
		x5 = x3 + x8;
		x8 -= x3;
		x3 = ir2c6 * (x2 + x6);
		x6 = x3 + (- ir2c6 - ir2s6) * x6;
		x2 = x3 + (- ir2c6 + ir2s6) * x2;

		x3 = x7 + x2;
		x7 -= x2;
		x2 = x0 + x6;
		x0 -= x6;
		x4 >>= 6;
		x5 >>= 6;
		x1 >>= 6;
		x8 >>= 6;
		x6 = ic3 * (x4 + x5);
		x5 = (x6 + (- ic3 - is3) * x5);
		x4 = (x6 + (- ic3 + is3) * x4);
		x6 = ic1 * (x1 + x8);
		x1 = (x6 + (- ic1 - is1) * x1);
		x8 = (x6 + (- ic1 + is1) * x8);

		x7 += 1024;
		x2 += 1024;
		x0 += 1024;
		x3 += 1024;
		component[l][m][k][0][i] = (short) ((x3 + x4) >> 11);
		component[l][m][k][1][i] = (short) ((x2 + x8) >> 11);
		component[l][m][k][2][i] = (short) ((x0 + x1) >> 11);
		component[l][m][k][3][i] = (short) ((x7 + x5) >> 11);
		component[l][m][k][4][i] = (short) ((x7 - x5) >> 11);
		component[l][m][k][5][i] = (short) ((x0 - x1) >> 11);
		component[l][m][k][6][i] = (short) ((x2 - x8) >> 11);
		component[l][m][k][7][i] = (short) ((x3 - x4) >> 11);
	}
}

static void set_pixel_yuv422(unsigned x, unsigned y, unsigned char y1, unsigned char u, unsigned char v)
{
	int idx;
	unsigned char* videobuf2= videobuf+ sof_info.width*sof_info.height;
	idx = (int)y;
	idx *= sof_info.width;
	idx += x;
//YUV422 pack
	if(x & 1){
		videobuf[idx] = y1;
		videobuf2[idx] = v;
	}
	else{
		videobuf[idx] = y1;
		videobuf2[idx] = u;

	}	


}
static void set_pixel_vyuy(unsigned x, unsigned y, unsigned char y1, unsigned char u, unsigned char v)
{
	int idx;

#if 0	
	idx = (int)y;
	idx *= sof_info.width;
	idx += x;
	idx *= 2;
#else
	idx = (int)(y+offset_y);
	idx *= bg_width;
	idx += x+offset_x;
	idx *= 2;
#endif	
	
	if(x & 1){
		videobuf[idx] = v;
		videobuf[idx+1] = y1;
	}
	else{
		videobuf[idx] = u;
		videobuf[idx+1] = y1;
	}
}
#undef FMT_ARGB1555
#undef FMT_ARGB8888
#undef FMT_ARGB1565
#define FMT_ARGB4444

static void set_pixel(unsigned x,unsigned y, unsigned char red,unsigned char green,unsigned char blue)
{
	int idx;
	unsigned int tmp = 0;
	//idx = (y * sof_info.width + x) * 2;

	idx = (int)y;
	idx *= sof_info.width;
	idx += x;
#if 0
	if(idx % 2)
	{
		idx /= 2;
		videobuf[idx] += red;
		videobuf[idx+1] = (green<<8) + blue;
	}
	else
	{
		idx /= 2;
		videobuf[idx] = (red<<8) + green;
		videobuf[idx+1] = (blue<<8);
	}
#else
	switch(out_format)
	{
		case PXL_FMT_ARGB8888:
			idx *= 4;
		    videobuf[idx+3] = 0xff;
		    videobuf[idx+2] = red;
		    videobuf[idx+1] = green;
		    videobuf[idx] = blue;
			break;
		case PXL_FMT_ARGB4444:
		    idx *= 2;
		    tmp = 0xF;
		    tmp <<= 4;
		    tmp |= (unsigned char)((red >> 4) & 0xf);
		    tmp <<= 4;
		    tmp |= (unsigned char)((green >> 4) & 0xf);
		    tmp <<= 4;
		    tmp |= (unsigned char)((blue >> 4) & 0xf);
		    videobuf[idx+1] = (tmp >> 8)&0xff;
		    videobuf[idx] = (tmp&0xff);
			break;
		case PXL_FMT_ARGB1555:
		    idx *= 2;
		    tmp = 1;
		    tmp <<= 5;
		    tmp |= (unsigned char)((red >> 3) & 0x1f);
		    tmp <<= 5;
		    tmp |= (unsigned char)((green >> 3) & 0x1f);
		    tmp <<= 5;
		    tmp |= (unsigned char)((blue >> 3) & 0x1f);
		    videobuf[idx+1] = (tmp >> 8)&0xff;
		    videobuf[idx] = (tmp&0xff);
			break;
	}
//	printf("[r,g,b] = [%x,%x,%x],argb=%x\r\n",red,green,blue,tmp);
#endif
}


static void check_for_reset_marker(unsigned char buf)
{
   short i;

   for (i=0;i<8;i++)
   {
      if (buf==(RST0+i))
      {
         reset_mcu=true;
         break;
      }
   }

   if (reset_mcu)
   {
      /* zero out the previous dc values */
      for (i=0;i<MAX_COMPONENTS;i++)
         prev_dc_amplitude[i]=0;
   }
}

static unsigned char getbit()
{
   unsigned char mask;
   unsigned char buf;

   if (current_bit==0)
   {
      current_word=stream_getc();
      if (current_word==SECTION_MARKER)
      {
         /* Process section markers found in image data stream */
         buf=stream_getc();
         if (buf==EOI){
            done=true;
			/* Fix bug:avoid read more data, decode unknown data */
            reset_mcu = true; 
         }
         else if (buf!=0 && buf!=SECTION_MARKER)
         {
            check_for_reset_marker(buf);
            if (reset_mcu)
               return 0;
         }
      }
   }
   mask=1<<(WORD_LEN-current_bit-1);
   current_bit++;
   if (current_bit==WORD_LEN) current_bit=0;
   if ((current_word & mask)==mask)
      return 1;
   else
      return 0;
}


static void setup_zigzag_array()
{
   short i,j,x,y,k=0;

   for (j=0;j<8;j++)
   {
      for (i=0;i<=j;i++)
      {
         if (j%2==0)
         {
            x=i;
            y=j-i;
         }
         else
         {
            x=j-i;
            y=i;
         }
         zigzag_array[k]=(y<<3)+x;
         k++;
      }
   }

   for (j=1;j<8;j++)
   {
      for (i=0;i<=8-1-j;i++)
      {
         if (j%2==0)
         {
            x=8-1-i;
            y=j+i;
         }
         else
         {
            x=j+i;
            y=8-1-i;
         }
         zigzag_array[k]=(y<<3)+x;
         k++;
      }
   }
}


static void dec_unknown_marker()
{
   short length;
   short i;

   if (!image_started)
      done=true;
   else
   {
      length=get_word();
      length-=2;  /* Subtract length of length word */

      /* Skip over the rest of the unknown marker */
      for (i=0;i<length;i++)
         stream_read(&byte_var,1);
   }
}


static void dec_application_segment()
{
   short i;

   if (!image_started)
      done=true;
   else
   {
      jpg_header.app0_len=get_word();

      stream_read((unsigned char*)identifier_mark, 5);
      if (strcmp(identifier_mark,"JFIF")==0)
      {
         jfif_found=true;
      }
      else
      {
         if (jfif_found)
         {
            /* application extension found */
            for (i=0;i<jpg_header.app0_len-2;i++)
               stream_read(&byte_var, 1);
            return;
         }
      }

      strcpy(jpg_header.identifier,identifier_mark);

      stream_read((unsigned char*)&word_var,2);
      jpg_header.version=word_var;

      stream_read(&byte_var,1);
      jpg_header.units=byte_var;

      stream_read((unsigned char*)&word_var,2);
      jpg_header.x_density=word_var;

      stream_read((unsigned char*)&word_var,2);
      jpg_header.y_density=word_var;

      stream_read(&byte_var,1);
      jpg_header.x_thumbnail=byte_var;

      stream_read(&byte_var,1);
      jpg_header.y_thumbnail=byte_var;

      thumbnail_size=jpg_header.x_thumbnail*jpg_header.y_thumbnail;

      /* Skip over thumbnail */
      for (i=0;i<thumbnail_size;i++)
         stream_read((unsigned char*)three_byte_var,3);

      /* Skip over the rest of the app0 marker */
      for (i=0;i<jpg_header.app0_len-16-3*thumbnail_size;i++)
         stream_read(&byte_var,1);
   }
}

static void dec_quantization_table()
{
   short length,number,no_of_tables;
   short i,j;

   if (!image_started)
      done=true;
   else
   {
      length=get_word();
      no_of_tables=(length-2)/65;
      for (i=0;i<no_of_tables;i++)
      {
         stream_read(&byte_var,1);
         number=byte_var;
         for (j=0;j<64;j++)
         {
            stream_read(&byte_var,1);
            quant_tbl[number][j]=byte_var;
         }
      }
   }
}

static void dec_start_of_frame()
{
   short length;
   short i;

   if (!image_started)
      done=true;
   else
   {
      length=get_word();

      stream_read(&byte_var,1);
      sof_info.precision=byte_var;
      if (sof_info.precision!=8)
      {
         done=true;
         return;
      }

      sof_info.height=get_word();
      image_height = sof_info.height;
      sof_info.width=get_word();
      image_width = sof_info.width;

      if((bg_width == 0) && (bg_height == 0)){
      	offset_x = 0;
      	offset_y = 0;
      	bg_width = image_width;
      	bg_height = image_height;
      }
      else{
      	if((bg_width < image_width) || (bg_height < image_height)){
//      		printf("image width/height > bg_width/bg_height!!\n");
      		offset_x = 0;
      		offset_y = 0;
      		bg_width = image_width;
      		bg_height = image_height;
      	}
      	else{
			if(layout == 0) // central
			{
				offset_x = (bg_width - image_width)/2;
				offset_y = (bg_height - image_height)/2;
			}
			else  //left_up
			{
      			offset_x = 0; //(bg_width - image_width)/2;
      			offset_y = 0; //(bg_height - image_height)/2;
			}
      	}
      }

      stream_read(&byte_var,1);
      sof_info.components=byte_var;
      if (sof_info.components!=3)
      {
         done=true;
         return;
      }

      max_v_samp_factor=0;
      max_h_samp_factor=0;
      for (i=0;i<sof_info.components;i++)
      {
         stream_read(&byte_var,1);
         sof_info.component[i].id=byte_var;

         stream_read(&byte_var,1);
         sof_info.component[i].samp_factor=byte_var;
         sof_info.component[i].v_samp_factor=             /* Lo Nibble */
         sof_info.component[i].samp_factor & 0x0f;
         sof_info.component[i].h_samp_factor=             /* Hi Nibble */
         sof_info.component[i].samp_factor>>4;

         if (sof_info.component[i].v_samp_factor>max_v_samp_factor)
            max_v_samp_factor=sof_info.component[i].v_samp_factor;

         if (sof_info.component[i].h_samp_factor>max_h_samp_factor)
            max_h_samp_factor=sof_info.component[i].h_samp_factor;

         stream_read(&byte_var,1);
         sof_info.component[i].quant_tbl_no=byte_var;
      }

      /* Get divisors used for upsampling the chrominance components */
      for (i=0;i<sof_info.components;i++)
      {
         h_div[i]=max_h_samp_factor/sof_info.component[i].h_samp_factor;
         v_div[i]=max_v_samp_factor/sof_info.component[i].v_samp_factor;
      }

      /* Get number of MCUs across and down in the image */
      mcu_width=(sof_info.width-1)/(max_h_samp_factor*8)+1;
      mcu_height=(sof_info.height-1)/(max_v_samp_factor*8)+1;

      /* Skip over the rest of the start of frame */
      for (i=0;i<length-8-3*sof_info.components;i++)
         stream_read(&byte_var,1);
   }
}

static void dec_huffman_table()
{
   short length;
   short i,count,len;
   unsigned char type,ndx;
   HUFTBL *huf_tbl_ptr;

   if (!image_started)
      done=true;
   else
   {
      length=get_word();
      len=length-2; /* Subtract length of length word */

      while (len>0)
      {
         stream_read(&byte_var,1);

         type=byte_var>>4;                               /* Hi Nibble */
         ndx=byte_var&0x0f;                              /* Lo Nibble */

         /* Choose Huffman Table for AC or DC components */
         if (type==1)
            huf_tbl_ptr=&ac_huf_tbl[ndx];
         else
            huf_tbl_ptr=&dc_huf_tbl[ndx];

         count=0;
         for (i=0;i<16;i++)
         {
            stream_read(&byte_var,1);
            huf_tbl_ptr->bits[i]=byte_var;
            count+=byte_var;
         }

         if (count>256)
         {
            done=true;
            return;
         }

         for (i=0;i<count;i++)
         {
            stream_read(&byte_var,1);
            huf_tbl_ptr->val[i]=byte_var;
         }
         len-=(1+16+count); /* subtract for type, index, bits & values */
      }
   }
}

static void dec_mcu(void);
static void build_huf_nodes();
static void dec_image_data()
{
   short i;

   if (!image_started)
      done=true;
   else
   {
      build_huf_nodes();

      /* Initialize previous DC amplitudes to zero for each component */
      for (i=0;i<MAX_COMPONENTS;i++)
         prev_dc_amplitude[i]=0;
      mcu_ctr=0;
      current_bit=0;
      while (!done)
         dec_mcu();
   }
}

static void dec_start_of_scan()
{
   short length,len;
   short i;

   if (!image_started)
      done=true;
   else
   {
      length=get_word();

      len=length;

      stream_read(&byte_var,1);
      sos_info.components=byte_var;

      len-=3; /* Subtract for length & components */

      for (i=0;i<sos_info.components;i++)
      {
         stream_read(&byte_var,1);
         sos_info.component[i].id=byte_var;

         stream_read(&byte_var,1);
         sos_info.component[i].id=byte_var;
         sos_info.component[i].tbl_no=byte_var;
         sos_info.component[i].ac_tbl_no=              /* Lo Nibble */
         sos_info.component[i].tbl_no & 0x0f;
         sos_info.component[i].dc_tbl_no=              /* Hi Nibble */
         sos_info.component[i].tbl_no>>4;
         len-=2;
      }

      /* Skip over the rest of the start of scan */
      for (i=0;i<len;i++)
         stream_read(&byte_var,1);
   }

   dec_image_data();
}

void jpeg_decode_central(unsigned int bg_w, unsigned int bg_h, unsigned char* inbuf, unsigned char* outbuf)
{
	int i;
	unsigned int* tmpbuf = (unsigned int*)outbuf;
	bg_width = bg_w;
	bg_height = bg_h;

	for(i=0; i<(bg_width*bg_height)*2/4; i++){
		tmpbuf[i] = 0x00800080L;
	}
	layout = 0;
	jpeg_dec(inbuf, outbuf);
}

void jpeg_decode_left_up(unsigned int bg_w, unsigned int bg_h, unsigned char* inbuf, unsigned char* outbuf)
{
    int i;
    unsigned int* tmpbuf = (unsigned int*)outbuf;
    bg_width = bg_w;
    bg_height = bg_h;

    for(i=0; i<(bg_width*bg_height)*2/4; i++){
        tmpbuf[i] = 0x00800080L;
    }
    layout = 1;
    jpeg_dec(inbuf, outbuf);
}
void jpeg_setfmt(unsigned int fmt)
{
	out_format = fmt;
}
void jpeg_decode(unsigned char* inbuf, unsigned char* outbuf)
{
	bg_width = 0;
	bg_height = 0;
	jpeg_dec(inbuf, outbuf);
}

void jpeg_getdim(unsigned int* width, unsigned int* height)
{
	*width = image_width;
	*height = image_height;
}

void jpeg_dec(unsigned char* inbuf, unsigned char* outbuf)
{
   short c;

   in_stream = inbuf;
   videobuf = outbuf;

   iter = 0;

   setup_zigzag_array();

   done=false;
   jfif_found=false;
   while (!done)
   {
      c=stream_getc();
      {
         if (c!=SECTION_MARKER)
            done=true;
         else
         {
            c=stream_getc();
            {
                  if(c== SOI){
                     image_started=true;
                  }
                  else if(c== APP0)
                     dec_application_segment();
                  else if(c== DQT)
                     dec_quantization_table();
                  else if(c== SOF0)
                     dec_start_of_frame();
                  else if(c== DHT)
                     dec_huffman_table();
                  else if(c== SOS)
                     dec_start_of_scan();
                  else if(c== EOI){
                     done=true;
                  }
                  else
                     dec_unknown_marker();
            }
         }
      }
   }
}


static void create_index(huf_node *node,short total_nodes)
{
   short i;

   for (i=0;i<total_nodes;i++)
       node[i].tree_index=(1L<<node[i].size)-1+node[i].code;
}


static void sort(huf_node *node,short total_nodes)
{
   short i,j;
   huf_node t;

   for (i=0;i<total_nodes;i++)
      for (j=i;j<total_nodes-1;j++)
         if (node[j+1].tree_index<node[i].tree_index)
         {
            t=node[i];
            node[i]=node[j+1];
            node[j+1]=t;
         }
}


static void generate_codes(unsigned char *hufval,huf_node *node)
{
   short k,
   code,si;

   k=0;
   code=0;
   si=node[k].size;
   while (node[k].size>0)
   {
      while (node[k].size==si)
      {
         node[k].code=code;
         node[k].value=hufval[k];
         code++;
         k++;
      }

      if (node[k].size>0)
      {
         while (node[k].size!=si)
         {
            code<<=1;
            si++;
         }
      }
   }
}


static void generate_code_size(unsigned char *bits,huf_node *node)
{
   short i,j,k;

   i=0;j=1;k=0;
   while (i<16)
   {
      while (j<=bits[i])
      {
         node[k].size=i+1;
         k++;
         j++;
      }
      i++;
      j=1;
   }
   node[k].size=0;
   total_nodes=k;
}

static void build_huf_nodes()
{
   short k;

   for (k=0;k<sof_info.components;k++)
   {
      generate_code_size(dc_huf_tbl[sos_info.component[k].dc_tbl_no].bits,
          dc_node[k]);
      dc_total_nodes[k]=total_nodes;
      generate_codes(dc_huf_tbl[sos_info.component[k].dc_tbl_no].val,
          dc_node[k]);
      create_index(dc_node[k],dc_total_nodes[k]);
      sort(dc_node[k],dc_total_nodes[k]);

      generate_code_size(ac_huf_tbl[sos_info.component[k].ac_tbl_no].bits,
          ac_node[k]);
      ac_total_nodes[k]=total_nodes;
      generate_codes(ac_huf_tbl[sos_info.component[k].ac_tbl_no].val,
          ac_node[k]);
      create_index(ac_node[k],ac_total_nodes[k]);
      sort(ac_node[k],ac_total_nodes[k]);
   }
}


static void build_transform()
{
   short x,y,k=0;

   for (y=0;y<8;y++)
      for (x=0;x<8;x++)
      {
         transform[x][y]=block[k];
         k++;
      }
}

static void de_zigzag(short k)
{
   short i;
   for (i=0;i<64;i++)
      block[zigzag_array[i]]=coefficient[i]*
       quant_tbl[sof_info.component[k].quant_tbl_no][i];
}

static void dequantize_block(short k)
{
   short i;

   for (i=0;i<64;i++)
      block[i]*=quant_tbl[sof_info.component[k].quant_tbl_no][i];
}


static void shift_block(short k,short l,short m)
{
   short x,y;

   for (x=0;x<8;x++)
      for (y=0;y<8;y++)
         component[l][m][k][x][y]+=128;
}

static short find_node_index(unsigned int index,short *value,huf_node *node,short total_nodes)
{
   short found=false;
   short first=0;
   short last=total_nodes-1;
   short middle;

   while (last>=first && !found)
   {
      middle=(first+last)>>1;
      if (index<node[middle].tree_index)
         last=middle-1;
      else if (index>node[middle].tree_index)
         first=middle+1;
      else
      {
         found=true;
         *value=node[middle].value;
      }
   }
   return found;
}

static short get_huf_code(huf_node *node,short total_nodes)
{
   char bit;
   short found=false;
   unsigned int index=0;
   short value,ctr=0;

   while (!found && ctr<16)
   {
      bit=getbit();
      if (reset_mcu) return 0;
      if (bit==0)
         index=(index<<1)+1;  /* take  left branch */
      else
         index=(index<<1)+2;  /* take right branch */
      found=find_node_index(index,&value,node,total_nodes);
      if (found) return value;
      ctr++;
   }

   return 0;
}


static unsigned short get_binary_number(short length)
{
   short i;
   char bit;
   unsigned short mask,number;

   number=0;
   mask=1<<(length-1);
   for (i=0;i<length;i++)
   {
      bit=getbit();
      if (reset_mcu) return 0;
      if (bit==1)
      {
         number|=mask;
      }
      mask>>=1;
   }
   if (number<1<<(length-1))
      /* negative */
      return number+((-1)<<length)+1;
   else
      /* positive */
      return number;
}


static void get_component_block(short k,short l,short m)
{
   short done_with_dat_unit=false;
   short i,index=0;

   size=get_huf_code(dc_node[k],dc_total_nodes[k]);
   if (reset_mcu) return;
   amplitude=get_binary_number(size);

   coefficient[index]=amplitude+prev_dc_amplitude[k];
   prev_dc_amplitude[k]=coefficient[index];
   index++;

   while (!done_with_dat_unit)
   {
      symbol=get_huf_code(ac_node[k],ac_total_nodes[k]);
      if (reset_mcu) return;
      if (symbol==EOB)
      {
         for (i=index;i<64;i++)
            coefficient[i]=0;
         done_with_dat_unit=true;
      }
      else
      {
         run=symbol>>4;                                  /* Hi Nibble */
         size=symbol & 0x0f;                             /* Lo Nibble */
         if (symbol==ZRL) run++;
         for (i=0;i<run;i++)
         {
            coefficient[index]=0;
            index++;
         }
         if (size>0)
         {
            amplitude=get_binary_number(size);
            coefficient[index]=amplitude;
            index++;
         }
         if (index==64) done_with_dat_unit=true;
      }
   }

   de_zigzag(k);
   build_transform();
   new_idct(k,l,m);
   shift_block(k,l,m);
}


static void dec_dat_unit(short k)
{
   short i,j;

   for (j=0;j<sof_info.component[k].v_samp_factor;j++)
      for (i=0;i<sof_info.component[k].h_samp_factor;i++)
      {
         get_component_block(k,i,j);
         if (reset_mcu) return;
      }
}


static void convert_yuv_to_rgb(short y,short u,short v,
        unsigned char *r,unsigned char *g,unsigned char *b)
{
   float red,green,blue;

   red=y+1.402*(v-128);
   if (red<0) red=0;
   if (red>255) red=255;

   green=y-0.34414*(u-128)-0.71414*(v-128);
   if (green<0) green=0;
   if (green>255) green=255;

   blue=y+1.772*(u-128);
   if (blue<0) blue=0;
   if (blue>255) blue=255;

   *r=(unsigned char)red;
   *g=(unsigned char)green;
   *b=(unsigned char)blue;
}


static void display_data_block(short i,short j)
{
   short x,y;
   short y_comp,u_comp,v_comp;
   unsigned char r,g,b;
   unsigned x_pos,y_pos;
   short v_data_index,h_data_index;

   for (y=0;y<8;y++)
      for (x=0;x<8;x++)
      {
         y_comp=component[i][j][0][x][y];

         /* Up Sample the blue chrominance */
         if (i>=sof_info.component[1].h_samp_factor)
            h_data_index=sof_info.component[1].h_samp_factor-1;
         else
            h_data_index=i;

         if (j>=sof_info.component[1].v_samp_factor)
            v_data_index=sof_info.component[1].v_samp_factor-1;
         else
            v_data_index=j;

         u_comp=component[h_data_index][v_data_index][1][(x+i*8)/h_div[1]][(y+j*8)/v_div[1]];

         /* Up Sample the red chrominance */
         if (i>=sof_info.component[2].h_samp_factor)
            h_data_index=sof_info.component[2].h_samp_factor-1;
         else
            h_data_index=i;
         if (j>=sof_info.component[2].v_samp_factor)
            v_data_index=sof_info.component[2].v_samp_factor-1;
         else
            v_data_index=j;

         v_comp=component[h_data_index][v_data_index][2][(x+i*8)/h_div[1]][(y+j*8)/v_div[1]];
 /*        //???, it should be
         //v_comp=component[h_data_index][v_data_index][2][(x+i*8)/h_div[2]][(y+j*8)/v_div[2]];
*/


		switch(out_format)
		{
			case PXL_FMT_ARGB8888:
			case PXL_FMT_ARGB4444:
			case PXL_FMT_ARGB1555:
	        	convert_yuv_to_rgb(y_comp,u_comp,v_comp,&r,&g,&b);
				break;
			case PXL_FMT_YUV420:
			case PXL_FMT_YUV422:
			default:
				break;
		}
         mcu_x=mcu_ctr % mcu_width;
         mcu_y=mcu_ctr/mcu_width;

         x_pos=mcu_x*max_h_samp_factor*8+i*8+x;
         y_pos=mcu_y*max_v_samp_factor*8+j*8+y;

         if (x_pos<sof_info.width && y_pos<sof_info.height)
#if 1		//YUV422
			{
				switch(out_format)
    	    	{
        	    	case PXL_FMT_ARGB8888:
	        	    case PXL_FMT_ARGB4444:
    	        	case PXL_FMT_ARGB1555:
						set_pixel(x_pos, y_pos, r, g, b);
						break;
					case PXL_FMT_YUV420:
					case PXL_FMT_YUV422:
						if(y_comp < 0) y_comp = 0;
						if(u_comp < 0) u_comp = 0;
						if(v_comp < 0) v_comp = 0;
						if(y_comp > 0xFF) y_comp = 0xFF;
						if(u_comp > 0xFF) u_comp = 0xFF;
						if(v_comp > 0xFF) v_comp = 0xFF;
						if(out_format==PXL_FMT_YUV422) 
							set_pixel_yuv422(x_pos, y_pos, (unsigned char)y_comp, (unsigned char)u_comp, (unsigned char)v_comp);
						else if(out_format == PXL_FMT_YUV420)
							set_pixel_vyuy(x_pos, y_pos, (unsigned char)y_comp, (unsigned char)u_comp, (unsigned char)v_comp);
				}
			}		
#else		//RGB888
            set_pixel(x_pos, y_pos, r, g, b);
#endif
      }
}


static void dec_mcu(void)
{
   short i,j,k;

   reset_mcu=false;
   for (k=0;k<sof_info.components;k++)
   {
      dec_dat_unit(k);
      if (reset_mcu) break;
   }
   if (!reset_mcu)
   {
      for (j=0;j<max_v_samp_factor;j++)
         for (i=0;i<max_h_samp_factor;i++)
            display_data_block(i,j);
      mcu_ctr++;
   }
}

#if 0
int main(int argc, char* argv[])
{
	FILE *inp, *outp;
	int filelen;
	unsigned char* inbuf;
	unsigned char* outbuf;


	if(argc != 3){
		printf("\nusage: jdec input.jpg output.rgb\n\n");
		return -1;
	}

	inp = fopen (argv[1], "rb");
  	if (inp == NULL) {
  		printf("open file %s error!\n", argv[1]);
  		return -1;
  	}

  	fseek (inp , 0 , SEEK_END);
  	filelen = ftell (inp);
  	fseek (inp , 0 , SEEK_SET);

  	inbuf = (unsigned char*) malloc (sizeof(char)*filelen);
  	if (inbuf == NULL) {
  		printf("memory alloc error!\n");
  		return -1;
  	}

  	if(fread (inbuf, 1, filelen, inp) != filelen){
  		printf("fread error!\n");
  		return -1;
  	}

  	fclose(inp);

  	outp = fopen (argv[2], "wb");
  	if (outp == NULL) {
  		printf("open file %s error!\n", argv[2]);
  		return -1;
  	}

  	outbuf = (unsigned char*) malloc (1920*1080*2);
  	if (outbuf == NULL) {
  		printf("memory alloc 2 error!\n");
  		return -1;
  	}

	iter = 0;

   jpeg_decode(inbuf, outbuf);

   fwrite (outbuf, 1, image_height*image_width*2, outp) ;

   fclose(outp);

   free(inbuf);
   free(outbuf);

   return 0;
}
#endif
