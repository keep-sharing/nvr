/*
 * Copyright (c) 2017 HiSilicon Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


/*
 * Video output Support
 */
#include <common.h>
#include <command.h>
#include <hi3536dv100_vo.h>

extern int set_vobg(unsigned int dev, unsigned int rgb);
extern int start_vo(unsigned int dev, unsigned int type, unsigned int sync);
extern int stop_vo(unsigned int dev);
extern int start_gx(unsigned int layer, unsigned addr, unsigned int strd, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
extern int stop_gx(unsigned int layer);
extern int start_videolayer(unsigned int layer, unsigned addr, unsigned int strd,
        unsigned int x, unsigned int y, unsigned int w, unsigned int h);
extern int stop_videolayer(unsigned int layer);

extern int hdmi_display(unsigned int vosync, unsigned int input, unsigned int output);

extern void hdmi_stop(void);

#define VOU_DEV_MAX_NUM         1
static int gs_aInterfaceType[VOU_DEV_MAX_NUM] = {0};

static int atoi_checkargv(char *string)
{
	int length;
	int retval = 0;
	int i;
	int sign = 1;

	length = strlen(string);
	for (i = 0; i < length; i++) {
		if (0 == i && string[0] == '-') {
			sign = -1;
			continue;
		}
		if (string[i] > '9' || string[i] < '0')
			break;

		retval *= 10;
		retval += string[i] - '0';
	}
	retval *= sign;
	return retval;
}

int check_vo_support(unsigned int dev, unsigned int type, unsigned int sync)
{
	/* check interface type, ONLY VGA & HDMI interface is supported. */
	unsigned int enMaxIntfSync;
	unsigned int enMinIntfSync;

	int t;
	if (VOU_DEV_DHD1 == dev)
	{
		if((type & ~(VO_INTF_VGA | VO_INTF_HDMI)) || (0 == type))
		{
			printf("Hd%d only supports HDMI&VGA intftype, intf %d is illegal!\n",dev,type);
			return -1;

		}
	}
	else
	{
		printf("Unknow dev(%d)!\n", dev);
		return -1;
	}
	
	
	if (VO_INTF_HDMI & type)
	{
		if (sync < VO_OUTPUT_1080P24 || sync >= VO_OUTPUT_USER)
		{
			printf("Vo%d's intfsync %d illegal!\n", dev, sync);
			return -1;
		}
	}


	if (VO_INTF_BT1120 & type)
	{
			enMaxIntfSync = VO_OUTPUT_1680x1050_60;
			enMinIntfSync = VO_OUTPUT_1080P24;

		if (sync < enMinIntfSync || sync > enMaxIntfSync
			|| (VO_OUTPUT_2560x1440_30 == sync)
			|| (VO_OUTPUT_2560x1440_60 == sync)
			|| (VO_OUTPUT_2560x1600_60 == sync)
			)
		{
			printf("Vo%d's intfsync %d illegal!\n", dev, sync);
			return -1;
		}
	}

	if (VO_INTF_VGA & type)
	{
		if ((sync < VO_OUTPUT_1080P24)
			|| (sync > VO_OUTPUT_2560x1600_60))

		{
			t= VO_OUTPUT_2560x1600_60;
			printf("Vo%d's intfsync %d illegal!  %d\n", dev, sync, t);
			return -1;
		}
	}


	if(VO_INTF_CVBS & type)
	{
		if ((sync != VO_OUTPUT_PAL) && (sync != VO_OUTPUT_NTSC)
			&& (sync != VO_OUTPUT_960H_PAL) && (sync != VO_OUTPUT_960H_NTSC))
		{
			printf("Vo%d's intfsync %d illegal!\n", dev, sync);
			return -1;
		}
	}
	return 0;
}


int do_vobg(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    unsigned int dev, rgb;

    if (argc < 3)
    {
        printf("Insufficient parameter!\n");
        printf ("Usage:\n%s\n", cmdtp->usage);
        return -1;
    }

    if(atoi_checkargv(argv[1]) < 0)
    {
        printf("Invalid parameter!dev:%d\n", atoi_checkargv(argv[1]));
        return -1;
    }

    dev = (unsigned int)simple_strtoul(argv[1], NULL, 10);
    rgb = (unsigned int)simple_strtoul(argv[2], NULL, 10);
    if (dev > VOU_DEV_DHD1)
    {
        printf("Invalid parameter!\n");
        return -1;
    }

    set_vobg(dev, rgb);

    printf("dev %d set background color!\n", dev);

    return 0;
}

int do_startvo(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    unsigned int dev, type, sync;

    if (argc < 4)
    {
        printf("Insufficient parameter!\n");
        printf ("Usage:\n%s\n", cmdtp->usage);
        return -1;
    }

    if(atoi_checkargv(argv[1]) < 0)
    {
        printf("Invalid parameter!dev:%d\n", atoi_checkargv(argv[1]));
        return -1;
    }
    else if(atoi_checkargv(argv[2]) < 0)
    {
        printf("Invalid parameter!type:%d\n", atoi_checkargv(argv[2]));
        return -1;
    }
    else if(atoi_checkargv(argv[3]) < 0)
    {
        printf("Invalid parameter!sync:%d\n", atoi_checkargv(argv[3]));
        return -1;
    }

    dev  = (unsigned int)simple_strtoul(argv[1], NULL, 10);
    type = (unsigned int)simple_strtoul(argv[2], NULL, 10);
    sync = (unsigned int)simple_strtoul(argv[3], NULL, 10);
    if (dev > VOU_DEV_DHD1 || sync >= VO_OUTPUT_2560x1440_30 || sync == VO_OUTPUT_1920x2160_30)
    {
        printf("Invalid parameter!\n");
        return -1;
    }

    if (check_vo_support(dev, type, sync))
    {
        printf("Unsupport parameter!\n");
        return -1;
    }

    start_vo(dev, type, sync);

    if (type & VO_INTF_HDMI)
    {
        gs_aInterfaceType[dev] =  VO_INTF_HDMI;
        hdmi_display(sync, 2, 2);
    }

    printf("dev %d opened!\n", dev);

    return 0;
}

int do_stopvo(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    unsigned int dev;

    if (argc < 2)
    {
        printf("Insufficient parameter!\n");
        printf ("Usage:\n%s\n", cmdtp->usage);
        return -1;
    }

    if(atoi_checkargv(argv[1]) < 0)
    {
        printf("Invalid parameter!dev:%d\n", atoi_checkargv(argv[1]));
        return -1;
    }

    dev = (unsigned int)simple_strtoul(argv[1], NULL, 10);
    if (dev > VOU_DEV_DHD1)
    {
        printf("Invalid parameter!\n");
        return -1;
    }

    if (gs_aInterfaceType[dev] & VO_INTF_HDMI)
    {
        gs_aInterfaceType[dev] = 0;
        hdmi_stop();
    }

    stop_vo(dev);

    printf("dev %d closed!\n", dev);

    return 0;
}

int do_startgx(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    unsigned int layer, addr, strd, x, y, w, h;

    if (argc < 8)
    {
        printf("Insufficient parameter!\n");
        printf ("Usage:\n%s\n", cmdtp->usage);
        return -1;
    }

    if(atoi_checkargv(argv[1]) < 0)
    {
        printf("Invalid parameter!layer:%d\n", atoi_checkargv(argv[1]));
        return -1;
    }

    layer = (unsigned int)simple_strtoul(argv[1], NULL, 10);
    addr  = (unsigned int)simple_strtoul(argv[2], NULL, 16);
    strd  = (unsigned int)simple_strtoul(argv[3], NULL, 10);
    x     = (unsigned int)simple_strtoul(argv[4], NULL, 10);
    y     = (unsigned int)simple_strtoul(argv[5], NULL, 10);
    w     = (unsigned int)simple_strtoul(argv[6], NULL, 10);
    h     = (unsigned int)simple_strtoul(argv[7], NULL, 10);

    if (layer >= VO_GRAPHC_G1 || strd > (PIC_MAX_WIDTH*2)
        || x > PIC_MAX_WIDTH   || (x & 0x1)
        || y > PIC_MAX_HEIGHT  || (y & 0x1)
        || w > PIC_MAX_WIDTH   || (w & 0x1) || w < PIC_MIN_LENTH
        || h > PIC_MAX_HEIGHT  || (h & 0x1) || h < PIC_MIN_LENTH
        )
    {
        printf("Invalid parameter!\n");
        return -1;
    }

    start_gx(layer, addr, strd, x, y, w, h);

    printf("graphic layer %d opened!\n", layer);

    return 0;
}

int do_stopgx(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    unsigned int layer;

    if (argc < 2)
    {
        printf("Insufficient parameter!\n");
        printf ("Usage:\n%s\n", cmdtp->usage);
        return -1;
    }

    if(atoi_checkargv(argv[1]) < 0)
    {
        printf("Invalid parameter!layer:%d\n", atoi_checkargv(argv[1]));
        return -1;
    }

    layer = (unsigned int)simple_strtoul(argv[1], NULL, 10);

    if (layer >= VO_GRAPHC_G1)
    {
        printf("Invalid parameter!\n");
        return -1;
    }

    stop_gx(layer);

    printf("graphic layer %d closed!\n", layer);

    return 0;
}

int do_startvl(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    unsigned int layer, addr, strd, x, y, w, h;

    if (argc < 8)
    {
        printf("Insufficient parameter!\n");
        printf ("Usage:\n%s\n", cmdtp->usage);
        return -1;
    }

    if(atoi_checkargv(argv[1]) < 0)
    {
        printf("Invalid parameter!layer:%d\n", atoi_checkargv(argv[1]));
        return -1;
    }

    layer = (unsigned int)simple_strtoul(argv[1], NULL, 10);
    addr  = (unsigned int)simple_strtoul(argv[2], NULL, 16);
    strd  = (unsigned int)simple_strtoul(argv[3], NULL, 10);
    x     = (unsigned int)simple_strtoul(argv[4], NULL, 10);
    y     = (unsigned int)simple_strtoul(argv[5], NULL, 10);
    w     = (unsigned int)simple_strtoul(argv[6], NULL, 10);
    h     = (unsigned int)simple_strtoul(argv[7], NULL, 10);

    if (layer > VOU_LAYER_VP || layer == VOU_LAYER_VP
        || strd > (1920*2)
        || x > 1920   || (x & 0x1)
        || y > 1200  || (y & 0x1)
        || w > 1920   || (w & 0x1) || w < PIC_MIN_LENTH
        || h > 1200  || (h & 0x1) || h < PIC_MIN_LENTH
        )
    {
        printf("Invalid parameter!\n");
        return -1;
    }

    if(strd == 0 || ((strd & 0x3f) != 0))
    {
        printf("Invalid strd %u\n",strd);
        return -1;
    }

    start_videolayer(layer, addr, strd, x, y, w, h);

    printf("video layer %d opened!\n", layer);

    return 0;
}

int do_stopvl(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    unsigned int layer;

    if (argc < 2)
    {
        printf("Insufficient parameter!\n");
        printf ("Usage:\n%s\n", cmdtp->usage);
        return -1;
    }

    if(atoi_checkargv(argv[1]) < 0)
    {
        printf("Invalid parameter!layer:%d\n", atoi_checkargv(argv[1]));
        return -1;
    }

    layer = (unsigned int)simple_strtoul(argv[1], NULL, 10);

    if (layer > VOU_LAYER_VP || layer == VOU_LAYER_VP)
    {
        printf("Invalid parameter!\n");
        return -1;
    }

    stop_videolayer(layer);

    printf("layer %d closed!\n", layer);

    return 0;
}

U_BOOT_CMD(
	startvo,    CFG_MAXARGS,	1,  do_startvo,
	"startvo   - open interface of vo device.\n"
	"\t- startvo [dev type sync]",
	"\nargs: [dev, type, sync]\n"
	"\t-<dev> : 0~1(HD0~1), 2(SD0)\n"
	"\t-<type>: 1(CVBS),4(VGA),8(BT.656),16(BT.1120),32(HDMI),64(LCD)\n"
	"\t\tsupport multi type eg: 52(VGA|BT.1120|HDMI)"
	"\t-<sync>:\n"
	"\t\t0(PAL),1(NTSC),2(1080P24)\n"
	"\t\t3(1080P25),4(1080P30),5(720P50),6(720P60)\n"
	"\t\t7(1080I50),8(1080I60),9(1080P50),10(1080P60)\n"
	"\t\t11(576P50),12(480P60),13(800x600),14(1024x768)\n"
	"\t\t15(1280x1024),16(1366x768),17(1440x900),18(1280x800)\n"
	);

U_BOOT_CMD(
	stopvo,    CFG_MAXARGS,	1,  do_stopvo,
	"stopvo   - close interface of vo device.\n"
	"\t- stopvo [dev]",
	"\nargs: [dev]\n"
	"\t-<dev> : 0~1(HD0~1), 2(SD0)\n"
	);

U_BOOT_CMD(
	startgx,    CFG_MAXARGS,	1,  do_startgx,
	"startgx   - open graphics layer.\n"
	"\t- startgx [layer addr stride x y w h]\n",
	"\nargs: [layer, addr, stride, x, y, w, h]\n"
	"\t-<layer>   : 0(G0), 1(G1), 2(G2), 3(G3)\n"
	"\t-<addr>    : picture address\n"
	"\t-<stride>  : picture stride\n"
	"\t-<x,y,w,h> : display area\n"
	);

U_BOOT_CMD(
	stopgx,    CFG_MAXARGS,	1,  do_stopgx,
	"stopgx   - close graphics layer.\n"
	"\t- stopgx [layer]",
	"\nargs: [layer]\n"
	"\t-<layer> : 0(G0), 1(G1), 2(G2), 3(G3)\n"
	);

U_BOOT_CMD(
	startvl,    CFG_MAXARGS,	1,  do_startvl,
	"startvl   - open video layer.\n"
	"\t- startvl [layer addr stride x y w h]\n",
	"\nargs: [layer, addr, stride, x, y, w, h]\n"
	"\t-<layer>   : 0(V0), 1(V3), 3(V4)\n"
	"\t-<addr>    : picture address\n"
	"\t-<stride>  : picture stride\n"
	"\t-<x,y,w,h> : display area\n"
	);

U_BOOT_CMD(
	stopvl,    CFG_MAXARGS,	1,  do_stopvl,
	"stopvl   - close video layer.\n"
	"\t- stopvl [layer]",
	"\nargs: [layer]\n"
	"\t-<layer> : 0(V0), 1(V3), 3(V4)\n"
	);


U_BOOT_CMD(
	setvobg,    CFG_MAXARGS,	1,  do_vobg,
	"setvobg   - set vo backgroud color.\n"
	"\t- setvobg [dev color]",
	"\nargs: [dev, color]\n"
	"\t-<dev> : 0~1(HD0~1), 2(SD0)\n"
	"\t-<color>: rgb color space\n"
	);


