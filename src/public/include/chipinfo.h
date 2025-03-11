#ifndef __CHIPINFO_H_
#define __CHIPINFO_H_


#define CHIPINFO                "chipinfo"
#define CHIPINFO_DEFAULT        "MSN100402H10EU000000S13155100010"
#define CHIPINFO_LEN            32
#define DM2016_VAL_LEN          32

#define INTERNAL_MSKEY_VALUE    "1"
#define MSKEY                   "mskey"
#define MSKEY_LEN               32
#define MSAUTHCHECK             "msauthcheck"
#define MSAUTHCHECK_VALUE       "1"
#define MSAUTHCHECK_LEN         32
#define MSRAN                   "msran"
#define MSRAN_LEN               32

#define ENV_SNCODE              "sncode"
#define ENV_ETHADDR             "ethaddr"
#define ENV_ETHADDR2            "ethaddr2"

int ms_dm2016_open();
void ms_dm2016_close(int fd);
int ms_dm2016_eeprom_read(int fd, int offset, int len, unsigned char *data);
int ms_env_init(void);
int ms_env_deinit(void);
int mschip_write(char *value);
int mschip_read(char *chipinfo);

char *ms_getenv(char *name);
int ms_setenv(char *name, char *value);

//use for exe
int   fw_printenv(int argc, char *argv[]);
char *fw_getenv(char *name);
int fw_setenv(int argc, char *argv[]);
unsigned    long  crc32(unsigned long, const unsigned char *, unsigned);

//use for api
int env_init(void);
int env_deinit(void);
char *hs_getenv(char *name);
int hs_setenv(char *name, char *value);
int hs_printenv(void);

#endif
