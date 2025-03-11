#include <common.h>
#include <command.h>
#include <config.h>
#include <i2c.h>
#include "environment.h"

#define CHIPINFO				"chipinfo"
#define CHIPINFO_LEN			64
#define CHIPINFO_ARR_LEN		CHIPINFO_LEN + 1

#define CHIPINFO				"chipinfo"
#define MSKEY					"mskey"
#define MSKEY_VALUE				"ms1234"
#define MSRAN					"msran"
#define MSAUTHCHECK				"msauthcheck"

int mstype_write(const char *value, uchar off, uchar cnt);
int mstype_read(uchar off, uchar cnt, char *output);
int do_mstype( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

int mstype_write(const char *value, uchar off, uchar cnt)
{
	uchar i;
	if (off < 0 || cnt < 0 || (off + cnt) > CHIPINFO_LEN)
	{
		printf("Check off and cnt!\n");
		return -1;
	}
	
	char chipinfo[CHIPINFO_ARR_LEN] = {0};
	char *chipinfo_temp = getenv(CHIPINFO);
	if (chipinfo_temp != NULL)
		strncpy(chipinfo, chipinfo_temp, CHIPINFO_LEN);
	if (value == NULL)
	{
		printf("Value can't be NULL.\n");
		return -1;
	}
	if (chipinfo == NULL)
		return -1;
	for (i = 0; i < cnt;i++)
	{
		uchar j = (*(value + i));
		*((uchar*)chipinfo + off + i) = j ? j : ' ';
	}
	chipinfo[CHIPINFO_LEN] = '\0';
	setenv(CHIPINFO, chipinfo);
	env_crc_update();
	if (strcmp(getenv(CHIPINFO), chipinfo))
	{
		printf("Write env error.\n");
		return -1;
	}
	saveenv();
	return 0;

}

int mstype_read(uchar off, uchar cnt, char *output)
{
	
	if ((off + cnt) > CHIPINFO_LEN)
	{
		printf("Check off and cnt!\n");
		return -1;
	}

	char *chipinfo = getenv(CHIPINFO);
	if (!chipinfo)
	    return -1;
	    
	strncpy(output, chipinfo + off, cnt);
	output[cnt] = '\0';
	printf("Read value from env %s\n", output);
	return 0;

}

int read_encrypted_param(char *name)
{
	int state;
	int i = 0, j = 0;
	char c, buf[17];
	
	if (strcmp(name, MSKEY) != 0 && 
		strcmp(name, MSRAN) != 0 &&
		strcmp(name, MSAUTHCHECK) != 0) //goli add to hide out of mskey_env when cmd is "printenv mskey"
		return -1;
	
	state = 1;
	buf[16] = '\0';
	while (state && env_get_char(i) != '\0') {
		if (envmatch((uchar *)name, i) >= 0)
		{
			state = 0;
		}
		
		j = 0;
		do {
			buf[j++] = c = env_get_char(i++);
			if (j == sizeof(buf) - 1) {
				if (state < 1)
					puts(buf);
				j = 0;
			}
		} while (c != '\0');

		if (state < 1) {
			if (j)
				puts(buf);
			putc('\n');
		}
		
		if (ctrlc())
			return -1;
	}

	return 0;
}


int do_mstype( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	const char *const fmt1 = "%s: offset %04x count %d ... \n";
	const char *const fmt2 = "%s: values %s offset %04x count %d ... \n";
	char output[CHIPINFO_ARR_LEN] = {0};
	
	if ( argc == 5 )
	{	
		if (strcmp (argv[1], "write") == 0)
		{
		    uchar off  = (uchar)simple_strtoul (argv[3], NULL, 10);
		    uchar cnt  = (uchar)simple_strtoul (argv[4], NULL, 10);
			int rcode;
			ulong addr = 0;
			
			if (off != 0 && cnt != CHIPINFO_LEN)
			{
				char space[CHIPINFO_ARR_LEN] = {0};
				uchar i = 0;
				for (; i < CHIPINFO_LEN; i++)
					space[i] = ' ';
				setenv(CHIPINFO, space);
				env_crc_update();
			}

			if (argv[2][0] == '0' && (argv[2][1] == 'x' || argv[2][1] == 'X')) {
				
				addr = simple_strtoul (argv[2], NULL, 16);
				printf (fmt1, argv[1], off, cnt);
				rcode = mstype_write ((char *)addr, off, cnt);
				strncpy(output, (char *)addr,cnt);
			} else {
				if (strlen(argv[2]) < cnt)
					cnt = strlen(argv[2]);
				printf (fmt2, argv[1], argv[2], off, cnt);
				rcode = mstype_write (argv[2], off, cnt);
				strncpy(output, argv[2],cnt);
			}
				
			if( rcode )
			{
				printf("Write Failed.\n");
			}
			else
			{
				output[cnt] = '\0';
				printf("Read value from env %s\n", output);
			}
			return rcode;
		}
	}
	else if( argc == 4 )
	{
        if (strcmp (argv[1], "read") == 0) {
            int rcode;
            
            uchar off  = (uchar)simple_strtoul (argv[2], NULL, 10);
		    uchar cnt  = (uchar)simple_strtoul (argv[3], NULL, 10);
            
            printf (fmt1, argv[1], off, cnt);
			rcode = mstype_read(off, cnt, output);
			if(rcode)
			{
				printf("Read Failed.\n");
			}
            return rcode;
        }
	}

	if (strcmp (argv[1], "check") == 0 && strcmp (argv[2], "encrypted") == 0)
	{
		int rcode = -1;
		int i;
		for (i = 3; i < argc; i++)
		{
			char *name = argv[i];
			rcode = read_encrypted_param(name);
			if (rcode)
			{
				printf("## Error: \"%s\" not defined\n", name);
			}
		}
		return rcode;
	}
	 
	printf ("Usage:\n%s\n", cmdtp->usage);
	return 1;
}

U_BOOT_CMD(
	mstype,	6, 0, do_mstype,
	"mstype  - MSTYPE sub-system\n\t- read  offset cnt\n\t- write value/address offset cnt\n",
	"mstype read offset cnt\n"
	"mstype write value/address offset cnt\n"
	"       - read/write `cnt' bytes at DM2016 offset `offset'\n"
);


