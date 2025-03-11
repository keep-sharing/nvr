#include <stdio.h>
#include "dm2016.h"
#include "hisi_dm2016_eeprom.h"
#include <stdlib.h>
#include "dlfcn.h"
#include "chipinfo.h"
//#include "fw_env.h"
#include "msstd.h"

int main(int argc, char **argv)
{
	//char sMsg[128] = {0};
	unsigned char sChipInfo[CHIPINFO_LEN + 1] = {0};
	char chipinfo[CHIPINFO_LEN + 1] = {0};
	int keylen = 0;

	if(argc == 1 || (argc >= 2 && (strcmp(argv[1], "-r") == 0)))
	{
		env_init();
		
		if(mschip_read(chipinfo) == -1)
		{
			printf("Read Failed.\n");
            env_deinit();
			return -1;
		}
		else
		{
			printf("Read old:%s\n", chipinfo);
		}
		env_deinit();
		return 0;
	}	
	snprintf((char*)sChipInfo, sizeof(sChipInfo), "%s", argv[1]);
	//Check ChipInfo validation
	
	keylen = strlen(argv[1]);
	if( keylen < 10 || keylen > CHIPINFO_LEN){
		//snprintf(sMsg, sizeof(sMsg), "The Chip Info Len[%d] invalid!\n", strlen(argv[1]));
		printf("The Chip Info Len[%d] invalid!\n", strlen(argv[1]));
		return -1;
	}
	else if( 
		sChipInfo[0] != 'M' || sChipInfo[1] != 'S' || sChipInfo[2] != 'N' )
	{
		//snprintf(sMsg, sizeof(sMsg), "The Chip Info %s != MSC..H..S..E..N\n", sChipInfo);
		printf("The Chip Info %s != MSC..H..S..E..N\n", sChipInfo);
		return -1;
	}
	env_init();
	printf("Writing...\n");
	if(mschip_write(argv[1]))
	{
		printf("Write Error.\n");
	}
	else
	{
		printf("Read old:%s\n", argv[1]);
	}
	env_deinit();
	return 0;
}

