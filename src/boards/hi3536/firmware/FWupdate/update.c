#include "updateFW.h"

#define UPDATE_VERSION	xstr(UPDATE_CMD_VER)


static char update_enable = 0;
static char oem_analysis = 0;
static char update_analysis = 0;
static char update_list = 0;
static char fw_name[128] = {ANALYSIS_IMG_NAME};

static char	update_sn_data[SN_SIZE];
static char update_cmdline[CMD_LINE_SIZE];
static uint8_t	update_bootdelay = 1;

static uint8_t	update_auto_dl_data=1;
static uint8_t	update_auto_boot_data=1;
static uint32_t	update_pri_addr_data;
static char	update_pri_file_data[SN_SIZE];

#define isspace(c)		((c) == ' ' || ((c) >= '\t' && (c) <= '\r'))
#define isascii(c)		(((c) & ~0x7f) == 0)
#define isupper(c)		((c) >= 'A' && (c) <= 'Z')
#define islower(c)		((c) >= 'a' && (c) <= 'z')
#define isalpha(c)		(isupper(c) || islower(c))
#define isdigit_c(c)	((c) >= '0' && (c) <= '9')

#define ARRAY_SIZE(x) ((unsigned)(sizeof(x) / sizeof((x)[0])))

#define UPFW_LOG_EXIT(errno,errstr) \
	do { \
		UPFW_log(errno,errstr); \
		exit(errno); \
	}while(0)

#define NANDWRITE_OPTIONS_BASE		0
#define NETWORK_OPTION_BASE		20

enum numeric_short_options {
	HELP = NANDWRITE_OPTIONS_BASE,
	VERSION_INFO,
	SHOW_INFO,
	AUTO_DOWNLOAD,
	AUTO_BOOT,
	PRI_ADDR,
	PRI_FILE,	

	SERVERIP = NETWORK_OPTION_BASE,
	ETH0_MAC,
	ETH0_IP,
	ETH0_MASK,
	ETH0_GW,
	UBOOT_VER,
#if (ETH_INSTANCES >= 2)	
	ETH1_MAC,
	ETH1_IP,
	ETH1_MASK,
	ETH1_GW,
#endif	
	WIFI0_MAC,	//20120331+ for updating with the fldev_t struct
	WIFI0_IP,
	WIFI0_MASK,
	WIFI0_GW,
#if (USE_WIFI >= 2)
	WIFI1_MAC,
	WIFI1_IP,
	WIFI1_MASK,
	WIFI1_GW,
#endif
#if (USE_USBETH >= 1)
	USB_ETH0_MAC,
	USB_ETH0_IP,
	USB_ETH0_MASK,
	USB_ETH0_GW,
#endif
#if (USE_USBETH >= 2)
	USB_ETH1_MAC,
	USB_ETH1_IP,
	USB_ETH1_MASK,
	USB_ETH1_GW,
#endif	
};

static const char *short_options = "?uaelB:S:N:C:D:E:W:";
static struct option long_options[] = {
	{"help", no_argument, 0, HELP},
	{"version", no_argument, 0, VERSION_INFO},
	{"show", no_argument, 0, SHOW_INFO},

	{"upfw", no_argument, 0, 'u'},
    {"analysis", no_argument, 0, 'a'},
    {"oem", no_argument, 0, 'e'},
	{"list", no_argument, 0, 'l'},
	
    {"bootState", no_argument, 0, 'B'},
    {"system", required_argument, 0, 'S'},
	
	{"sn", required_argument, 0, 'N'},
	{"cmdline", required_argument, 0, 'C'},
	{"delay", required_argument, 0, 'D'},

	{"auto_boot", required_argument, 0, AUTO_BOOT},
	{"auto_dl", required_argument, 0, AUTO_DOWNLOAD},
	{"pri_addr", required_argument, 0, PRI_ADDR},
	{"pri_file", required_argument, 0, PRI_FILE},

	{"serverip", required_argument, 0, SERVERIP},
	
	{"eth0_mac", required_argument, 0, 'E'},
	{"eth0_ip", required_argument, 0, ETH0_IP},
	{"eth0_mask", required_argument, 0, ETH0_MASK},
	{"eth0_gw", required_argument, 0, ETH0_GW},
    {"uboot_ver", required_argument, 0, UBOOT_VER},
#if (ETH_INSTANCES >= 2)
	{"eth1_mac", required_argument, 0, ETH1_MAC},
	{"eth1_ip", required_argument, 0, ETH1_IP},
	{"eth1_mask", required_argument, 0, ETH1_MASK},
	{"eth1_gw", required_argument, 0, ETH1_GW},
#endif
	{"wifi0_mac", required_argument, 0, 'W'},
	{"wifi0_ip", required_argument, 0, WIFI0_IP},
	{"wifi0_mask", required_argument, 0, WIFI0_MASK},
	{"wifi0_gw", required_argument, 0, WIFI0_GW},
#if (USE_WIFI >= 2)
	{"wifi1_mac", required_argument, 0, WIFI1_MAC},
	{"wifi1_ip", required_argument, 0, WIFI1_IP},
	{"wifi1_mask", required_argument, 0, WIFI1_MASK},
	{"wifi1_gw", required_argument, 0, WIFI1_GW},
#endif
#if (USE_USBETH >= 1)
	{"usbeth0_mac", required_argument, 0, USB_ETH0_MAC},
	{"usbeth0_ip", required_argument, 0, USB_ETH0_IP},
	{"usbeth0_mask", required_argument, 0, USB_ETH0_MASK},
	{"usbeth0_gw", required_argument, 0, USB_ETH0_GW},
#endif	
#if (USE_USBETH >= 2)
	{"usbeth1_mac", required_argument, 0, USB_ETH1_MAC},
	{"usbeth1_ip", required_argument, 0, USB_ETH1_IP},
	{"usbeth1_mask", required_argument, 0, USB_ETH1_MASK},
	{"usbeth1_gw", required_argument, 0, USB_ETH1_GW},
#endif	

	{0, 0, 0, 0},
};

struct hint_s {
	const char *arg;
	const char *str;
};

static const struct hint_s hint[] = {

	{"", "display this help and exit"},
	{"", "output version information and exit"},
	{"",  "Show Milesight AMBOOT information "},

	{"", "Update Milesight upgrade images"},
    {"", "Analysis Milesight upgrade images"},
    {"", "Analysis Milesight upgrade OEM images"},
	{"", "List upgrade image information"},
	
    {"state", "set boot state"},
	{"id", "Switch Milesight System"},
	
	{"sn", "Update Milesight AMBOOT SN "},	
	{"str", "Update Milesight AMBOOT cmdline "},	
	{"digit", "Update Milesight AMBOOT bootdelay "},
	
	{"digit", "Update Milesight AMBOOT load linux from nandflash or no "},
	{"digit", "Update Milesight AMBOOT load linux from network or no "},	
	{"hex", "Update Milesight AMBOOT pri_addr "},
	{"str", "Update Milesight updated file name(32bytes) "},

	{"ip", "Update Milesight AMBOOT tftpd server ip "},
	
	{"MAC", "Update Milesight AMBOOT eth0 MAC "},
	{"ip", "Update Milesight AMBOOT eth0 lan ip "},
	{"ip", "Update Milesight AMBOOT eth0 lan mask "},
	{"ip", "Update Milesight AMBOOT eth0 lan gw "},
    {"ver", "set uboot version , eg: xx.xx.xx.xx"},
#if (ETH_INSTANCES >= 2)	
	{"MAC", "Update Milesight AMBOOT eth1 MAC "},
	{"ip", "Update Milesight AMBOOT eth1 lan ip "},
	{"ip", "Update Milesight AMBOOT eth1 lan mask "},
	{"ip", "Update Milesight AMBOOT eth1 lan gw "},
#endif	
	{"MAC", "Update Milesight AMBOOT wifi0 MAC "},
	{"ip", "Update Milesight AMBOOT wifi0 ip "},
	{"ip", "Update Milesight AMBOOT wifi0 mask"},
	{"ip", "Update Milesight AMBOOT wifi0 gw "},
#if (USE_WIFI >= 2)
	{"MAC", "Update Milesight AMBOOT wifi1 MAC "},
	{"ip", "Update Milesight AMBOOT wifi1 ip "},
	{"ip", "Update Milesight AMBOOT wifi1 mask"},
	{"ip", "Update Milesight AMBOOT wifi1 gw "},
#endif
#if (USE_USBETH >= 1)	
	{"MAC", "Update Milesight AMBOOT usb_eth0 MAC "},
	{"ip", "Update Milesight AMBOOT usb_eth0 ip "},
	{"ip", "Update Milesight AMBOOT usb_eth0 mask"},
	{"ip", "Update Milesight AMBOOT usb_eth0 gw "},
#endif	
#if (USE_USBETH >= 2)
	{"MAC", "Update Milesight AMBOOT usb_eth1 MAC "},
	{"ip", "Update Milesight AMBOOT usb_eth1 ip "},
	{"ip", "Update Milesight AMBOOT usb_eth1 mask"},
	{"ip", "Update Milesight AMBOOT usb_eth1 gw "},
#endif

};


static void display_update_version(void)
{
    mtd_table_t table;
    int i;
    
	printf("Milesight update version: %s\n", UPDATE_VERSION);
	get_env_mtd_table(&table);
	for(i=0; i<PART_MAX; i++)
	{	    
	    if(table.part[i].version == 0)
	    {
            continue;
	    }
	    if((strstr(table.bootargs, table.part[i].name) != NULL) 
	        || (strstr(table.bootcmd, table.part[i].name) != NULL ))
        {
            printf("%s:(current)\n", table.part[i].name);
        }
        else if((strstr(table.bootargs, "nfs") != NULL) 
                && (strstr(table.part[i].name, "fs") != NULL)
                )
        {
            printf("%s:(nfs)\n", table.part[i].name);
        }
        else
        {
            printf("%s:\n", table.part[i].name);

        }
        printf("\tversion = %d.%d.%d.%d\n",
                (table.part[i].version >> 24) & 0xff,
                (table.part[i].version  >> 16) & 0xff,
                (table.part[i].version  >> 8) & 0xff,
                (table.part[i].version  ) & 0xff);
        printf("\tsize    = %d K\n", table.part[i].size/1024);
        printf("\tdate    = %d/%d/%d\n",
                (table.part[i].date >> 16) & 0xffff,
                (table.part[i].date >> 8) & 0xff,
                (table.part[i].date  ) & 0xff);
	}
}

static void display_help (void)
{
	int i;
	printf("Usage:\n"
			"  update [OPTION] upfw_file \n"
			"  Writes to the specified MTD device.\n"
			"\n");
	for (i = 0; i < ARRAY_SIZE(long_options) - 1; i++)
	{
		if (isalpha(long_options[i].val))
		{
			printf("-%c ", long_options[i].val);
		}
		else
		{
			printf("   ");
		}
			
		printf("--%s", long_options[i].name);
		if (hint[i].arg[0] != 0)
		{
			printf(" [%s]", hint[i].arg);
		}
		printf("\t%s\n", hint[i].str);
	}
	printf("\n");			
}

static int process_options (int argc, char *argv[])
{
	int error = 0;
	int ch;
	int option_index = 0;
	while ((ch = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) 
	{
		switch (ch) 
		{
		case HELP:
			display_help();
			break;
		case VERSION_INFO:
			display_update_version();
			break;
		case SHOW_INFO:
			display_info();
			break;
		case 'u':
			update_enable = 1;
			break;
		case 'a':
			update_analysis = 1;
			break;
		case 'l':
			update_list = 1;
			break;
		case 'e':
		    oem_analysis = 1;
		    break;
	    case 'B':
		    if (set_boot_state(strtoul (optarg, NULL, 0))!= UPFW_SUCCESS)
			{
				fprintf(stderr, "set boot state faild.\n");
				error = 1;	
			}
		    break;
		case 'S':			
		    if (set_switch_filesys(strtoul (optarg, NULL, 0))!= UPFW_SUCCESS)
			{
				fprintf(stderr, "switch system faild.\n");
				error = 1;	
			}
		    break;
		case 'N':
			strncpy(update_sn_data, optarg, SN_SIZE);
			if( set_ptb_sn(update_sn_data) != UPFW_SUCCESS )
			{
				fprintf(stderr, "update sn faild.\n");
				error = 1;	
			}
			break;
		case 'C':
			strncpy(update_cmdline, optarg, CMD_LINE_SIZE);
			if (set_ptb_cmdline(update_cmdline) != UPFW_SUCCESS)
			{
				fprintf(stderr, "update cmdline faild.\n");
				error = 1;	
			}
			break;			
		case 'D':
			update_bootdelay = strtoul (optarg, NULL, 0);
			if( set_ptb_bootdelay(update_bootdelay) != UPFW_SUCCESS)
			{
				fprintf(stderr, "update bootdelay faild.\n");
				error = 1;	
			}
			break;
		case AUTO_BOOT:
			update_auto_boot_data = strtoul (optarg, NULL, 0);
			if(set_ptb_autoboot(update_auto_boot_data) != UPFW_SUCCESS)
			{
				fprintf(stderr, "update auto_boot faild.\n");
				error = 1;	
			}
			break;
		case AUTO_DOWNLOAD:
			update_auto_dl_data = strtoul (optarg, NULL, 0);
			if(set_ptb_autodownload(update_auto_dl_data) != UPFW_SUCCESS)
			{
				fprintf(stderr, "update auto_dl faild.\n");
				error = 1;	
			}			
			break;
		case PRI_ADDR:
			update_pri_addr_data = strtoul (optarg, NULL, 0);
			if(set_ptb_pri_addr(update_pri_addr_data) != UPFW_SUCCESS)
			{
				fprintf(stderr, "update pri_addr faild.\n");
				error = 1;	
			}
			break;
		case PRI_FILE:
			strncpy(update_pri_file_data, optarg, SN_SIZE);
			if (set_ptb_pri_file(update_pri_file_data) != UPFW_SUCCESS)
			{
				fprintf(stderr, "update pri_file faild.\n");
				error = 1;	
			}
			break;
		case SERVERIP:
			if (set_ptb_tftpd_ip(optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update tftpd ip address faild.\n");
				error = 1;	
			}		
			break;
		case 'E':
			if (set_ptb_eth_mac(0, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update eth0 mac faild.\n");
				error = 1;	
			}
			break;
		case ETH0_IP:
			if (set_ptb_eth_ip(0, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update eth0 ip address faild.\n");
				error = 1;	
			}
			break;
		case ETH0_MASK:
			if (set_ptb_eth_netmask(0, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update eth0 netmask faild.\n");
				error = 1;	
			}
			break;
		case ETH0_GW:
			if (set_ptb_eth_gateway(0, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update eth0 gateway faild.\n");
				error = 1;	
			}
			break;
        case UBOOT_VER:
            if (set_ptb_uboot_version(optarg)!= UPFW_SUCCESS)
            {
                fprintf(stderr, "update uboot version faild.\n");
                error = 1;  
            }
            break;
#if ( ETH_INSTANCES >= 2 )			
		case ETH1_MAC:
			if (set_ptb_eth_mac(1, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update eth1 mac faild.\n");
				error = 1;	
			}
			break;
		case ETH1_IP:
			if (set_ptb_eth_ip(1, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update eth1 ip address faild.\n");
				error = 1;	
			}
			break;
		case ETH1_MASK:
			if (set_ptb_eth_netmask(1, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update eth1 netmask faild.\n");
				error = 1;	
			}
			break;
		case ETH1_GW:
			if (set_ptb_eth_gateway(1, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update eth1 gateway faild.\n");
				error = 1;	
			}
			break;	
#endif /* ETH_INSTANCES >= 2 */			
		case 'W':
			if (set_ptb_wifi_mac(0, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update wifi0 mac faild.\n");
				error = 1;	
			}
			break;
		case WIFI0_IP:
			if (set_ptb_wifi_ip(0, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update wifi0 ip address faild.\n");
				error = 1;	
			}		
			break;
		case WIFI0_MASK:
			if (set_ptb_wifi_netmask(0, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update wifi0 netmask faild.\n");
				error = 1;	
			}		
		 	break;
		case WIFI0_GW:
			if (set_ptb_wifi_gateway(0, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update wifi0 gateway faild.\n");
				error = 1;	
			}		
			break;	
#if (USE_WIFI >=2)			
		case WIFI1_MAC:
			if (set_ptb_wifi_mac(1, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update wifi1 mac faild.\n");
				error = 1;	
			}
			break;			
		case WIFI1_IP:
			if (set_ptb_wifi_ip(1, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update wifi1 ip address faild.\n");
				error = 1;	
			}
			break;
		case WIFI1_MASK:
			if (set_ptb_wifi_netmask(1, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update wifi1 netmask faild.\n");
				error = 1;	
			}		
		 	break;
		case WIFI1_GW:
			if (set_ptb_wifi_gateway(1, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update wifi1 gateway faild.\n");
				error = 1;	
			}		
			break;	
#endif /* USE_WIFI >=2 */	

#if (USE_USBETH >= 1)
		case USB_ETH0_MAC:
			if (set_ptb_usbeth_mac(0, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update usb_eth0 mac faild.\n");
				error = 1;	
			}
			break;
		case USB_ETH0_IP:
			if (set_ptb_usbeth_ip(0, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update usb_eth0 ip address faild.\n");
				error = 1;	
			}		
			break;
		case USB_ETH0_MASK:
			if (set_ptb_usbeth_netmask(0, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update usb_eth0 netmask faild.\n");
				error = 1;	
			}		
			break;
		case USB_ETH0_GW:
			if (set_ptb_usbeth_gateway(0, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update usb_eth0 gateway faild.\n");
				error = 1;	
			}		
			break;	
#endif /* USE_USBETH >= 1*/			
#if (USE_USBETH >=2)			
		case USB_ETH1_MAC:
			if (set_ptb_usbeth_mac(1, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update usb_eth1 mac faild.\n");
				error = 1;	
			}
			break;
		case USB_ETH1_IP:
			if (set_ptb_usbeth_ip(1, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update usb_eth1 ip address faild.\n");
				error = 1;	
			}		
			break;
		case USB_ETH1_MASK:
			if (set_ptb_usbeth_netmask(1, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update usb_eth1 netmask faild.\n");
				error = 1;	
			}		
			break;
		case USB_ETH1_GW:
			if (set_ptb_usbeth_gateway(1, optarg)!= UPFW_SUCCESS)
			{
				fprintf(stderr, "update usb_eth1 gateway faild.\n");
				error = 1;	
			}		
			break;	
#endif /* USE_USBETH >=2 */		
		case '?':
			fprintf(stderr, "Error: Unknow option\n");
			error = 1;
			goto ERR_EXIT;
		}
	}

ERR_EXIT:
	if(error)
	{
		display_help();
		exit(UPFW_FAILD);
	}

	if((argc - optind) == 1)
	{
		memset(fw_name, 0, sizeof(fw_name));
		strcpy(fw_name, argv[optind]);
	}
	return UPFW_SUCCESS;
}

int main(int argc, char* argv[])
{
	char errbuf[256];
	int err_no;

    env_init();
	process_options(argc, argv);
	if (!oem_analysis && (update_analysis || update_enable) && (UPFW_version_check(fw_name) != 0))  
	{
		err_no = UPFW_UPDATE_VER_LIMIT;
		snprintf(errbuf, sizeof(errbuf),
				UPFW_ERR_ERRSTR_FMT, "firmware mismatch!");
		UPFW_LOG_EXIT(err_no, errbuf);
	}

	if(update_list)
	{
		debug("List upfw %s infomation.\n", fw_name);
		err_no = UPFW_list_intfo(fw_name);
		if( err_no != UPFW_SUCCESS)
		{
			fprintf(stderr, "List upfw %s infomation failed!\n", fw_name);
			exit(err_no);
		}
	}

	if(oem_analysis || update_analysis)
	{
		debug("Analysis image %s \n", fw_name);
		err_no = UPFW_analysis_img(fw_name, 0);
		if( err_no != UPFW_SUCCESS)
		{
			snprintf(errbuf, sizeof(errbuf), "Analysis %s faild!\n", fw_name);
			UPFW_LOG_EXIT(err_no, errbuf);
		}
	}
	
	if(update_enable)
	{
		debug("Begin update %s \n", fw_name);
		err_no = UPFW_run_update(fw_name);
		switch(err_no)
		{
		case UPFW_SUCCESS:
			snprintf(errbuf, sizeof(errbuf),
				UPFW_ERR_ERRSTR_FMT, "update success!");
			break;
		case UPFW_FAILD:
			snprintf(errbuf, sizeof(errbuf),
				UPFW_ERR_ERRSTR_FMT, "update faild!");
			break;
		case UFPW_UPDATE_BLD_ERR:
			snprintf(errbuf, sizeof(errbuf),
				UPFW_ERR_ERRSTR_FMT, "update bld partition faild!");
			break;
		case UPFW_UPDATE_HAL_ERR:
			snprintf(errbuf, sizeof(errbuf),
				UPFW_ERR_ERRSTR_FMT, "update hal partition faild!");
			break;
		case UPFW_UPDATE_PTB_ERR:
			snprintf(errbuf, sizeof(errbuf),
				UPFW_ERR_ERRSTR_FMT, "update ptb partition faild!");
			break;
		case UPFW_UPDATE_DSP_ERR:
			snprintf(errbuf, sizeof(errbuf),
				UPFW_ERR_ERRSTR_FMT, "update dsp partition faild!");
			break;
		case UPFW_UPDATE_KER_ERR:
			snprintf(errbuf, sizeof(errbuf),
				UPFW_ERR_ERRSTR_FMT, "update kernel partition faild!");
			break;
		case UPFW_UPDATE_FS_ERR:
			snprintf(errbuf, sizeof(errbuf),
				UPFW_ERR_ERRSTR_FMT, "update filesys partition faild!");
			break;
		case UPFW_UPDATE_CMDLINE:
			snprintf(errbuf, sizeof(errbuf),
				UPFW_ERR_ERRSTR_FMT, "update cmdline faild!");
			break;
		case UPFW_KER1_MISMATCH:
			snprintf(errbuf, sizeof(errbuf),
				UPFW_ERR_ERRSTR_FMT, "kernel 1 version mismatch!");
			break;
		case UPFW_KER2_MISMATCH:
			snprintf(errbuf, sizeof(errbuf),
				UPFW_ERR_ERRSTR_FMT, "kernel 2 version mismatch!");
			break;
		case UPFW_VERIFY_FAILD:
			snprintf(errbuf, sizeof(errbuf),
				UPFW_ERR_ERRSTR_FMT, "verify update firmware faild!");
			break;
		case UPFW_ANALYSIS_FAILD:
			snprintf(errbuf, sizeof(errbuf),
				UPFW_ERR_ERRSTR_FMT, "analysis update firmware faild!");
			break;	
		default:
			snprintf(errbuf, sizeof(errbuf),
				UPFW_ERR_ERRSTR_FMT, "unknow error!");
			break;
		}
		
		UPFW_LOG_EXIT(err_no, errbuf);
	}
    env_deinit();
	exit(UPFW_SUCCESS);
}

