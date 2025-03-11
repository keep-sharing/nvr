
#include <common.h>
#include <command.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <part.h>
#include <asm/hardware.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/nvt-common/shm_info.h>
#include <stdlib.h>
#include <linux/arm-smccc.h>
//#include "cmd_bootlogo.h"
//#include <asm/arch/display.h>
//#include <asm/arch/top.h>
#include <linux/libfdt.h>
#include "asm/arch/lcd310.h"
#include "asm/arch/lcd210.h"
#include "asm/arch/hdmi_if.h"


//#include "logo_hdmi.dat"   //jpg bitstream binary
//#include "logo_hdmi2.dat"   //jpg bitstream binary
//#include "logo.dat"


#define DEMO_BOOT_LOGO_JPEG

#define EMU_FPGA 1

#ifdef DEMO_BOOT_LOGO_JPEG
//extern void jpeg_setfmt(unsigned int fmt);
extern void jpeg_decode_central(unsigned int bg_w, unsigned int bg_h, unsigned char* inbuf, unsigned char* outbuf);
extern void jpeg_decode_left_up(unsigned int bg_w, unsigned int bg_h, unsigned char* inbuf, unsigned char* outbuf);
extern void jpeg_decode(unsigned char* inbuf, unsigned char* outbuf);
extern void jpeg_getdim(unsigned int* width, unsigned int* height);
#endif

/*---------------------------------------------------------------
 * Configurable
 *---------------------------------------------------------------
 */
/* logo base */
#define FRAME_BUFFER_BASE	     0x10000000   //configurable
#define FRAME_BUFFER_BASE_LCD210 0x10800000   //configurable
#define FRAME_BUFFER_SIZE	     (8 << 20)    //8M, configurable
#define FRAME_BUFFER_SIZE_LCD210 (1 << 20)    //1M, configurable
#define FRAME_BUFFER_BASE_JPEG   0x10900000
#define FRAME_BUFFER_BASE_TMP1   0x20000000
#define FRAME_BUFFER_BASE_TMP2   0x21000000
#define FRAME_BUFFER_BASE_TMP3   0x22000000

#define JPEGFILE_BUFFER_ADDR_HDMI   0x10600000     //for hdmi/vga JPEG file by ceg
//#define JPEGFILE_BUFFER_ADDR_LCD210 0x10700000     //for lcd210 JPEG file by ceg

#ifdef DEMO_BOOT_LOGO_JPEG
static lcd_output_t g_output[3] = {OUTPUT_1920x1080,OUTPUT_1920x1080,OUTPUT_1920x1080};   //configurable
static lcd_vin_t    g_vin[3] = {VIN_1920x1080,VIN_1920x1080,VIN_1920x1080};         //configurable
//static lcd_output_t g_output[1] = {OUTPUT_1024x768};   //configurable
//static lcd_vin_t    g_vin[1] = {VIN_1024x768};         //configurable
static input_fmt_t  g_infmt[3] = {INPUT_FMT_YUV422,INPUT_FMT_YUV422,INPUT_FMT_YUV422 /* INPUT_FMT_RGB565 */};    //configurable
#else
static lcd_output_t g_output[3] = {OUTPUT_1280x720,OUTPUT_1280x720,OUTPUT_1280x720};   //configurable
static lcd_vin_t    g_vin[3] = {VIN_1024x768,VIN_1024x768,VIN_1024x768};         //configurable
static input_fmt_t  g_infmt[3] = {INPUT_FMT_ARGB1555,INPUT_FMT_ARGB1555,INPUT_FMT_ARGB1555 /* INPUT_FMT_RGB565 */};    //configurable
#endif

static u32 lcd_frame_base[3] = {FRAME_BUFFER_BASE,FRAME_BUFFER_BASE,FRAME_BUFFER_BASE};
static u32 lcd210_frame_base[3] = {FRAME_BUFFER_BASE_LCD210,FRAME_BUFFER_BASE_LCD210,FRAME_BUFFER_BASE_LCD210};


extern UINT32        otp_key_manager(UINT32 rowAddress);
extern BOOL  extract_trim_valid(UINT32 code, UINT32 *pData);

static int argb1555_to_yuv422(unsigned short *input, unsigned char *output, const unsigned int width, const unsigned int height)
{
#define cst_prom0   21
#define cst_prom1   79
#define cst_prom2   29
#define cst_prom3   43
#define RGB_GET_Y(r,g,b)    (((int)g) + ((cst_prom1 * (((int)r)-((int)g))) >> 8) + ((cst_prom2 * (((int)b)-((int)g))) >> 8) )
#define RGB_GET_U(r,g,b)    (128 + ((cst_prom3 * (((int)g)-((int)r))) >> 8) + ((((int)b)-((int)g)) >> 1) )
#define RGB_GET_V(r,g,b)    (128 + ((cst_prom0 * (((int)g)-((int)b))) >> 8) + ((((int)r)-((int)g)) >> 1) )

	int ret, i, j, k, unmap_input = 0, unmap_output = 0;
	int a, r, g, b, y, u, v, c;

	flush_dcache_range((uintptr_t)input, (uintptr_t)(input + width * height * 2));
	
	k = 0;
	for(i = 0 ; i < height ; ++i){
		for(j = 0 ; j < width ; j+=2){
			c = input[i * width + j];
			a = ((c & 0x8000)>>15);
			r = ((c & 0x7C00)>>10);
			g = ((c & 0x03E0)>>5);
			b = ((c & 0x001F));
			r = ((r * 255)/31);
			g = ((g * 255)/31);
			b = ((b * 255)/31);
			y = RGB_GET_Y(r, g, b);
			u = RGB_GET_U(r, g, b);
			v = RGB_GET_V(r, g, b);
			output[k++] = u;
			output[k++] = y;
			output[k++] = v;
			output[k++] = y;
		}
	}

	ret = 0;


	flush_dcache_range((uintptr_t)output, (uintptr_t)(output + width * height * 2));


	return ret;
}

static int yuv422_to_argb1555(unsigned char *input, unsigned short *output, const unsigned int sourceWidth, const unsigned int sourceHeight, const unsigned int targetWidth, const unsigned int targetHeight)
{
#define icst_prom0  103
#define icst_prom1  88
#define icst_prom2  183
#define icst_prom3  198
#define YUV_GET_R(y,u,v)    ( ((int)y) + ((int)(v)-128) + ((icst_prom0*((int)(v)-128))>>8) )
#define YUV_GET_G(y,u,v)    ( ((int)y) - ((icst_prom1*((int)(u)-128))>>8) - ((icst_prom2*((int)(v)-128))>>8) )
#define YUV_GET_B(y,u,v)    ( ((int)y) + ((int)(u)-128) + ((icst_prom3*((int)(u)-128))>>8) )

	int unmap_input = 0, unmap_output = 0;
	unsigned int widthCoefficient, heightCoefficient, x, y;
	unsigned int hc1, hc2, wc1, wc2, offsetX, offsetY;
	unsigned short *out;
	int r, g, b, y1, y2, u, v, c1, c2;

	const unsigned int wStepFixed16b = (targetWidth << 16)  / (unsigned int)targetWidth;
	const unsigned int hStepFixed16b = (targetHeight << 16) / (unsigned int)targetHeight;

	int offsetPixelY;
	int offsetPixelY1;


	out = output;

	flush_dcache_range((uintptr_t)input, (uintptr_t)(input + sourceWidth * sourceHeight * 2));

	heightCoefficient = 0x8000;//0;

	for (y = 0; y < targetHeight; y++){

		offsetY = (heightCoefficient >> 16);
		hc2 = (heightCoefficient >> 9) & (unsigned char)127;
		hc1 = 128 - hc2;

		widthCoefficient = 0x8000;//0;

		offsetPixelY  = offsetY * sourceWidth;
		offsetPixelY1 = (offsetY + 1) * sourceWidth;

		//convert 2 pixels at a time
		//so input and output can point to the same address without data corruption
		for (x = 0; x < targetWidth; x += 2){
			//printf("[yuv422_to_argb1555] x=%u\n", x);
			offsetX = (widthCoefficient >> 16);
			wc2 = (widthCoefficient >> 9) & (unsigned char)127;
			wc1 = 128 - wc2;

			widthCoefficient += (wStepFixed16b << 1);

			if((offsetX + 1) >= sourceWidth && 
					offsetY >= sourceHeight && 
					(x + 1) >= targetWidth)
				continue;

			y1 = input[(offsetPixelY + offsetX)*2 + 1];
			u = input[((offsetPixelY + offsetX)>>1)*4];
			v = input[((offsetPixelY + offsetX)>>1)*4 + 2];
			y2 = input[(offsetPixelY + offsetX)*2 + 3];

			//1st argb1555 pixel
			r = YUV_GET_R(y1,u,v);
			g = YUV_GET_G(y1,u,v);
			b = YUV_GET_B(y1,u,v);
			if(r < 0)
				r = 0;
			else if(r > 255)
				r = 255;
			if(g < 0)
				g = 0;
			else if(g > 255)
				g = 255;
			if(b < 0)
				b = 0;
			else if(b > 255)
				b = 255;
			r *= 31;
			r /= 255;
			r &= 0x01F;
			g *= 31;
			g /= 255;
			g &= 0x01F;
			b *= 31;
			b /= 255;
			b &= 0x01F;
			c1 = ((1<<15) | (r<<10) | (g<<5) | b);

			//2nd argb1555 pixel
			r = YUV_GET_R(y2,u,v);
			g = YUV_GET_G(y2,u,v);
			b = YUV_GET_B(y2,u,v);
			if(r < 0)
				r = 0;
			else if(r > 255)
				r = 255;
			if(g < 0)
				g = 0;
			else if(g > 255)
				g = 255;
			if(b < 0)
				b = 0;
			else if(b > 255)
				b = 255;
			r *= 31;
			r /= 255;
			r &= 0x01F;
			g *= 31;
			g /= 255;
			g &= 0x01F;
			b *= 31;
			b /= 255;
			b &= 0x01F;
			c2 = ((1<<15) | (r<<10) | (g<<5) | b);

			*output++ = (c1 & 0xFFFF);
			*output++ = (c2 & 0xFFFF);
		}

		heightCoefficient += hStepFixed16b;
	}

	flush_dcache_range((uintptr_t)output, (uintptr_t)(output + targetWidth * targetHeight * 2));


	return 0;
}

static void bilinear(const unsigned short *input, unsigned short *output, const unsigned int sourceWidth, const unsigned int sourceHeight, const unsigned int targetWidth, const unsigned int targetHeight)
{
    unsigned int widthCoefficient, heightCoefficient, x, y;
    unsigned short pixel1, pixel2, pixel3, pixel4;
    unsigned int hc1, hc2, wc1, wc2, offsetX, offsetY;
    unsigned int r, g, b, a;

    const unsigned int wStepFixed16b = (sourceWidth << 16)  / (unsigned int)targetWidth;
    const unsigned int hStepFixed16b = (sourceHeight << 16) / (unsigned int)targetHeight;

    int offsetPixelY;
    int offsetPixelY1;
    int offsetX1;
	//printf("%s source=[%dx%d] , target=[%dx%d]\r\n",__func__,sourceWidth,sourceHeight,targetWidth,targetHeight);
    heightCoefficient = 0x8000;//0;

	for (y = 0; y < targetHeight; y++)
    {
        offsetY = (heightCoefficient >> 16);
        hc2 = (heightCoefficient >> 9) & (unsigned char)127;
        hc1 = 128 - hc2;

        widthCoefficient = 0x8000;//0;

        offsetPixelY  = offsetY * sourceWidth;
        offsetPixelY1 = (offsetY + 1) * sourceWidth;

		for (x = 0; x < targetWidth; x++)
        {
            offsetX = (widthCoefficient >> 16);
            wc2 = (widthCoefficient >> 9) & (unsigned char)127;
            wc1 = 128 - wc2;
            offsetX1 = offsetX + 1;

			if(offsetX < sourceWidth && offsetY < sourceHeight)
				pixel1 = *(input + (offsetPixelY  + offsetX));
			else
				pixel1 = 0;

			if(offsetX < sourceWidth && (offsetY + 1) < sourceHeight)
				pixel2 = *(input + (offsetPixelY1 + offsetX));
			else
				pixel2 = 0;

			if(offsetX1 < sourceWidth && offsetY < sourceHeight)
				pixel3 = *(input + (offsetPixelY  + offsetX1));
			else
				pixel3 = 0;

			if(offsetX1 < sourceWidth && (offsetY + 1) < sourceHeight)
				pixel4 = *(input + (offsetPixelY1 + offsetX1));
			else
				pixel4 = 0;

			//a = ((((pixel1 >> 15) & 0x1F) * hc1 + ((pixel2 >> 15) & 0x1F) * hc2) * wc1 +
            //    (((pixel3  >> 15) & 0x1F) * hc1 + ((pixel4 >> 15) & 0x1F) * hc2) * wc2) >> 14;
			a = (((pixel1 >> 15) & 0x01) + ((pixel2 >> 15) & 0x01) +
                 ((pixel3 >> 15) & 0x01) + ((pixel4 >> 15) & 0x01));

			if(a){
				r = ((((pixel1 >> 10) & 0x1F) * hc1 + ((pixel2 >> 10) & 0x1F) * hc2) * wc1 +
					(((pixel3  >> 10) & 0x1F) * hc1 + ((pixel4 >> 10) & 0x1F) * hc2) * wc2) >> 14;
				g = ((((pixel1 >> 5) & 0x1F) * hc1 + ((pixel2 >> 5) & 0x1F) * hc2) * wc1 +
					(((pixel3  >> 5) & 0x1F) * hc1 + ((pixel4 >> 5) & 0x1F) * hc2) * wc2) >> 14;
				b = ((((pixel1) & 0x1F) * hc1 + ((pixel2) & 0x1F) * hc2) * wc1 +
					(((pixel3)  & 0x1F) * hc1 + ((pixel4) & 0x1F) * hc2) * wc2) >> 14;
				*output = (1 << 15) | (r << 10) | (g << 5) | b;
				output++;
			}
			else {
				*output = 0x00;//(r << 10) | (g << 5) | b;
				output++;
			}

            widthCoefficient += wStepFixed16b;
        }

        heightCoefficient += hStepFixed16b;
    }
}

#ifdef CONFIG_MS_NVR
static int load_jpeg_file(ulong upload_addr, unsigned int size)
{
    int rc = 0;
    char cmd[CONFIG_SYS_CBSIZE];
    sprintf(cmd, "mw.b 0x%lx ff %d; mmc read 0x%lx logo", upload_addr, size, upload_addr);
    rc = run_command(cmd, 0);
    return rc;
}
#endif

static int do_bootlogo(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    int hdmi = 0, cvbs = 0;
	int channel = 0;
	int pos = 0; //0 = center, 1= full
	BOOL	 is_found;
	UINT32	 code;
	UINT32	 hdmi_trim;
	unsigned char *outbuf;
	unsigned char *newbuf;
	unsigned short *tmpbuf;
	unsigned short *tmpbuf2;
	unsigned int img_width, img_height, ratio_w, ratio_h;
	unsigned int lcd_w = 1920, lcd_h = 1080;

    if(argc >= 2 && argc <= 3 )
	{
		if (argc >= 2) 
		{
        	char *p = argv[1];
	        if (!strcmp("hdmi", p))
    	    	hdmi = 1;
        	else if (!strcmp("cvbs", p))
	        	cvbs = 1;
			else if (!strcmp("exthdmi", p))
				hdmi = 2;
			else if (!strcmp("center",p))
				pos = 0;
			else if (!strcmp("full",p))
				pos = 1;
	        else {
    	        printf("Usage: bootlogo {hdmi|cvbs|exthdmi} {center|full}\n");
        	    return -1;
	        }
   		 }

		if(argc == 3)
		{
			char *p = argv[2];
			if (!strcmp("center", p))
            	pos = 0 ;
        	else if (!strcmp("full", p))
            	pos = 1;
        	else {
            	printf("Usage: bootlogo {hdmi|cvbs|exthdmi} {center|full}\n");
            	return -1;
        	}
		}
	}
	
	if (hdmi >= 1) {
#if (!EMU_FPGA)
		code = otp_key_manager(4); // PackageUID addr[4] bit [11] = 1 HDMI(X)	bit[11] = 0 HDMI(O)

		if(code == 0) {
			
		} else if(code == -33) {
			printf("Read package UID error\r\n");
			hdmi = 0;
		} else {
			hdmi_trim = 0;
			is_found = extract_trim_valid(code, &hdmi_trim);
			if(is_found) {
				if(((hdmi_trim & (1<<11)) == (1<<11))) {
					printf("Failed to init hdmi, chip no support feature\n");
					hdmi = 0;
					return -1;
				} 
			} else {
				printf("Read package UID = 0x%x(NULL)\r\n", (uint)hdmi_trim);
			}
		}
#endif
	}

#ifdef DEMO_BOOT_LOGO_JPEG
	{
		if (cvbs == 0) {
			switch (g_output[LCD0_ID]) {
			case OUTPUT_1280x720:
				lcd_w = 1280;
				lcd_h = 720;
				break;
			case OUTPUT_1920x1080:
				lcd_w = 1920;
				lcd_h = 1080;
				break;
			case OUTPUT_1024x768:
				lcd_w = 1024;
				lcd_h = 768;
				break;
			default:
				lcd_w = 1920;
				lcd_h = 1080;
				break;
			}
		}
		else
		{
			lcd_w = 720;
			lcd_h = 480;
		}
    }
	{
		unsigned char *inbuf = (unsigned char*)JPEGFILE_BUFFER_ADDR_HDMI;
#ifdef CONFIG_MS_NVR
		if (hdmi) {
			load_jpeg_file(JPEGFILE_BUFFER_ADDR_HDMI, FRAME_BUFFER_SIZE);
		}
#endif
		//inbuf = (unsigned char*)FRAME_BUFFER_BASE_JPEG;
		newbuf = (unsigned char*)FRAME_BUFFER_BASE_TMP1;
		tmpbuf = (unsigned short*)FRAME_BUFFER_BASE_TMP2;
		tmpbuf2 = (unsigned short*)FRAME_BUFFER_BASE_TMP3;
		if (cvbs)
			outbuf = (unsigned char*)FRAME_BUFFER_BASE_LCD210;
		else
			outbuf = (unsigned char*)FRAME_BUFFER_BASE;
	
		printf("start JPEG decode\n");
		//dcache_enable();
		printf("inbuf = 0x%x, newbuf = 0x%x\r\n", inbuf, newbuf);
		//jpeg_decode(inbuf, outbuf);
		if(pos == 0)
			jpeg_decode_central(lcd_w,lcd_h, inbuf, outbuf);
		else
			jpeg_decode_left_up(lcd_w,lcd_h, inbuf, newbuf);

		jpeg_getdim(&img_width, &img_height);
		printf("image size: %d x %d\n", img_width, img_height);
		//dcache_disable();
		if(pos == 0)
			flush_dcache_range((uintptr_t)outbuf, (uintptr_t)(outbuf + lcd_w * lcd_h * 2));
		else
			flush_dcache_range((uintptr_t)newbuf, (uintptr_t)(newbuf + lcd_w * lcd_h * 2));
		printf("JPEG decode end\n");
	}
	
	if(pos ==1)
	{
		if (g_infmt[LCD0_ID] == INPUT_FMT_YUV422 || cvbs == 1) {
			yuv422_to_argb1555(newbuf, tmpbuf, lcd_w, lcd_h, img_width, img_height);  //convert yuv422 to argb1555
			flush_dcache_range((uintptr_t)tmpbuf, (uintptr_t)(tmpbuf + img_width * img_height * 2));
			bilinear(tmpbuf, tmpbuf2, img_width, img_height,lcd_w,lcd_h);
			flush_dcache_range((uintptr_t)tmpbuf2, (uintptr_t)(tmpbuf2 + lcd_w * lcd_h * 2));
			argb1555_to_yuv422(tmpbuf2, outbuf, lcd_w, lcd_h);  // convert argb1555 to yuv422
		}
		flush_dcache_range((uintptr_t)outbuf, (uintptr_t)(outbuf + lcd_w * lcd_h * 2));
	
	}
#endif
    if (!cvbs) {
		if(hdmi == 2)
			flcd_main(LCD1_ID, g_vin[LCD1_ID], g_output[LCD1_ID], g_infmt[LCD1_ID], lcd_frame_base[LCD1_ID]);
		else
			flcd_main(LCD0_ID, g_vin[LCD0_ID], g_output[LCD0_ID], g_infmt[LCD0_ID], lcd_frame_base[LCD0_ID]);
	}
		
#if 1
    /* this only for lcd0 */
    if (hdmi >= 1) {
		hdmi_video_t hdmi_video;

        switch (g_output[LCD0_ID]) {
		case OUTPUT_1280x720:
			hdmi_video = HDMI_VIDEO_720P;
			break;
		case OUTPUT_1920x1080:
			hdmi_video = HDMI_VIDEO_1080P;
			break;
		case OUTPUT_1024x768:
			hdmi_video = HDMI_VIDEO_1024x768;
			break;
		default:
			return -1;
			break;
		}
		if(hdmi == 1)
			hdmi_if_init(hdmi_video);
    }
#endif
	if (cvbs) {
		flcd210_main(cvbs, FRAME_BUFFER_BASE_LCD210);
	}

	return 0;
}



U_BOOT_CMD(
	bootlogo,	3,	1,	do_bootlogo,
	"show lcd bootlogo [hdmi|cvbs]",
	"no argument means VGA output only, \n"
	"[hdmi] - VGA and HDMI output simulataneously"
);

