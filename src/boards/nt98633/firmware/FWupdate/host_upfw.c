
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "updateFW.h"


static int g_cflag = 0;
static int g_lflag = 0;
static int g_vflag = 0;
void usage (char* cmdname)
{
	fprintf (stderr, "Usage: %s -c -l -v [datafile]\n"
			"		-h ==> show help\n"
			"		-c ==> create imager \n"
			"		-l ==> list image header information\n"
			"		-v ==> verify image infomation\n",
			cmdname);

	exit (EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
	char ch = 0;
	int ret;
	char* datafile = OUTPUT_UPFW_NAME;
	
	while ((ch = getopt(argc, argv, "l:chv?")) > 0)
	{
		switch (ch) {
		case 'l':
			g_lflag = 1;
			datafile = optarg;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			break;
		case 'c':
			g_cflag = 1;
			break;
		case 'v':
			g_vflag = 1;
			break;
		default:
			fprintf(stderr, "ERROR: unkown option '%c'\n", ch);
			usage(argv[0]);
			break;
		}
	}	

	if(g_cflag)
	{
		UPFW_creat_img();
	}
	if(g_lflag)
	{
		if (UPFW_list_intfo(datafile) == -1)
		{
			return -1;
		}
	}
	if(g_vflag)
	{
		ret = UPFW_verify(datafile, NULL);
		if ( ret != 0)
		{
			fprintf(stderr, "Verify image failed!\n");
			return -1;
		}
		else
		{
			fprintf(stdout, "Verify image success!\n");
			return 0;
		}
	}
	
	return 0;
}
