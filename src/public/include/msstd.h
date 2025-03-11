#ifndef __MS_STD_H__
#define __MS_STD_H__
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

//#define MS_LEAK_DETECT

/*----------------------------------------------*
 * The common data type, will be used in the whole project.*
 *----------------------------------------------*/

typedef unsigned char           MS_U8;
typedef unsigned short          MS_U16;
typedef unsigned int            MS_U32;

typedef char                    MS_S8;
typedef short                   MS_S16;
typedef int                     MS_S32;

typedef float                   MS_FLOAT;
typedef double                  MS_DOUBLE;

#ifndef _M_IX86
typedef unsigned long long  MS_U64;
typedef long long           MS_S64;
#else
typedef __int64             MS_U64;
typedef __int64             MS_S64;
#endif

typedef char                    MS_CHAR;
#define MS_VOID                 void

typedef pthread_cond_t          MS_COND_T;
typedef pthread_mutex_t         MS_MUTEX_T;

#define MS_NULL     0L
#define MS_SUCCESS  0
#define MS_FAILURE      (-1)
#define MS_UNKNOWN      (-2)

#define THREAD_STACK_MIN_SIZE   (16*1024)
#define THREAD_STACK_32K_SIZE   (32*1024)
#define THREAD_STACK_64K_SIZE   (64*1024)
#define THREAD_STACK_128K_SIZE  (128*1024)
#define THREAD_STACK_256K_SIZE  (256*1024)
#define THREAD_STACK_512K_SIZE  (512*1024)
#define THREAD_STACK_1M_SIZE    (1024*1024)
#define THREAD_STACK_2M_SIZE    (2*1024*1024)

#define PROCESS_EXECUTABLE_MSCORE       "mscore"
#define PROCESS_EXECUTABLE_WEBSEVER     "boa"
#define PROCESS_EXECUTABLE_WEBCGI       "main"
#define PROCESS_EXECUTABLE_SYSCONF      "sysconf"
#define PROCESS_EXECUTABLE_MSSP         "mssp"
#define PROCESS_EXECUTABLE_DAEMON       "daemon"
#define PROCESS_EXECUTABLE_FIO          "fio"
#define PROCESS_EXECUTABLE_CLI          "mscli"
#define PROCESS_EXECUTABLE_MSJS         "msjs"
#define PROCESS_EXECUTABLE_MSKB         "mskb"
#define PROCESS_EXECUTABLE_MSMAIL       "msmail"
#define PROCESS_EXECUTABLE_PROFILE      "msprofile"
#define PROCESS_EXECUTABLE_UPDATE       "msupdate"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#endif

#ifndef HAVE_SPLICE
#define HAVE_SPLICE
#endif

#ifndef SPLICE_F_MOVE
#define SPLICE_F_MOVE           0x01
#endif

#ifndef SPLICE_F_NONBLOCK
#define SPLICE_F_NONBLOCK       0x02
#endif

#ifndef SPLICE_F_MORE
#define SPLICE_F_MORE           0x04
#endif

#ifndef SPLICE_F_GIFT
#define SPLICE_F_GIFT           0x08
#endif

#define __IO_BUFSIZE (4096)     /*reasonable buffer size based on pipe buffers*/

typedef enum {
    MS_MSCORE = 1,
    MS_BOA,
    MS_WEBCGI,
    MS_SYSCONF,
    MS_MSSP,
    MS_DAEMON,
    MS_FIO,
    MS_MSCLI,
    MS_MSJS,
    MS_MSKB,
    MS_MAIL,
    MS_PROFILE,
    MS_UPDATE
} MS_EXE;

typedef enum {
    MS_FALSE = 0,
    MS_TRUE  = 1,
    MS_NO    = 0,
    MS_YES   = 1,
} MS_BOOL;

#define RECORD_PATH_MMAP    "/tmp/rdmmap"

//memory manager
#define MEM_LEAK_OUTPUT_FILE "/tmp/mem_leak"
void *__ms_realloc(void *p, unsigned int size, const char *file, int lineno);
void *__ms_malloc(unsigned int size, const char *file, int lineno);
void  __ms_free(void *p, const char *file, int lineno);
void  __ms_scope_free(void **p);
void *__ms_calloc(unsigned int n, unsigned int size, const char *file, int lineno);
void *__ms_valloc(unsigned int size, const char *file, int lineno);
void *__ms_map(unsigned int phy_addr, unsigned int size);
int  __ms_unmap(void *addr_mapped);
void ms_memcpy(void *dest, void *src, unsigned int size);

void mem_leak_init();
unsigned int mem_get_cur_list();
void mem_leak_report();
void mem_leak_show();
#define ms_realloc(p, size) __ms_realloc(p, size, __FILE__, __LINE__)
#define ms_malloc(len)  __ms_malloc((len), __FILE__, __LINE__)
#define ms_calloc(n, size)  __ms_calloc(n, size, __FILE__, __LINE__)
#define ms_valloc(len) __ms_valloc((len), __FILE__, __LINE__)
#define ms_map(phyAddr, size) __ms_map(phyAddr, size)
#define ms_unmap(virAddr) __ms_unmap(virAddr)
#define ms_free(p)       \
    do{\
        __ms_free(p, __FILE__, __LINE__);\
        p = NULL;\
    }while(0);
#define ms_scope_free __attribute__ ((__cleanup__(__ms_scope_free)))
#define ms_scope_malloc(type, var, size) \
    ms_scope_free void *_##var = ms_malloc(size); \
    type var = (type)_##var;

//
typedef enum {
    QT_MEM_ADD,
    QT_MEM_DEL,
    QT_MEM_REP
} QT_MEM_TYPE_E;
typedef void (*QT_CALLBACK_MEM)(QT_MEM_TYPE_E type, void *mem_old, void *mem_new, int size, const char *file, int lineno);
void set_qt_callback_mem(QT_CALLBACK_MEM callback);

//bit operation
int  ms_get_bit(unsigned long long value, int p);   //get bit No.p (from right) of value
int  ms_set_bit(unsigned long long *value, int p, int on); //set bit No.p (from right) of value on/off
unsigned long long ms_get_bits(unsigned long long value, int p,
                               int n); //get n bits of value at position p (from right)
void ms_printf_bits(unsigned long long value); //printf bits

//string and time
int ms_string_strip(char s[], char c);//delete char 'c' from string s
void time_to_string(time_t ntime, char stime[32]); //time change to string(fmt:year-month-day hour:minute:second)
void time_to_string_ms(char stime[32]);
void get_current_time_to_compact_string(char stime[32]);    //20210331123456
MS_U64 ms_get_current_time_to_msec(void);
MS_U64 ms_get_current_time_to_usec(void);
int ms_get_current_date_cnt(void);
int ms_get_current_hours_cnt(void);
int ms_get_current_date_cnt_ex(char *localTime);
int ms_get_current_hours_cnt_ex(char *localTime);
time_t ms_get_time_from_string(char *localTime);
int ms_get_text_section_int(const char *buff, char *keyWord, int *pOutValue);
int ms_get_text_section_long(const char *buff, char *keyWord, long *pOutValue);
int ms_get_text_section_float(const char *buff, char *keyWord, float *pOutValue);
int ms_get_text_section_string(const char *buff, char *keyWord, char *pOutValue, int pOutLen);



//file manager
int ms_create_dir(const char *dir);
int ms_remove_dir(const char *dir);
int ms_file_existed(const char *file);
enum MEM_TYPE {
    MEM_TYPE_TOTAL,
    MEM_TYPE_FREE,
    MEM_TYPE_PERCENT,
};
int ms_is_dir_existed(const char* dir);
int mu_proc_exist(char *proc_path);
int ms_save_proc_id(char *path);




//system
int ms_system(const char *cmd);
int ms_vsystem(const char *cmd);
int ms_system_closefd(const char *cmd);
FILE *ms_vpopen(const char *program, const char *type);
int ms_vpclose(FILE *iop);

//lock mutex
#define MUTEX_OBJECT pthread_mutex_t
#define COND_OBJECT pthread_cond_t

int ms_mutex_init(MUTEX_OBJECT *mutex);
void ms_mutex_lock(MUTEX_OBJECT *mutex);
int ms_mutex_trylock(MUTEX_OBJECT *mutex);
int ms_mutex_timelock(MUTEX_OBJECT *mutex, int us);
void ms_mutex_unlock(MUTEX_OBJECT *mutex);
void ms_mutex_uninit(MUTEX_OBJECT *mutex);

//rwlock
#define RWLOCK_BOJECT pthread_rwlock_t
void ms_rwlock_init(RWLOCK_BOJECT *rwlock);
void ms_rwlock_uninit(RWLOCK_BOJECT *rwlock);
int ms_rwlock_rdlock(RWLOCK_BOJECT *rwlock);
int ms_rwlock_wrlock(RWLOCK_BOJECT *rwlock);
int ms_rwlock_tryrdlock(RWLOCK_BOJECT *rwlock);
int ms_rwlock_trywrlock(RWLOCK_BOJECT *rwlock);
int ms_rwlock_unlock(RWLOCK_BOJECT *rwlock);

#define BARRIER_OBJECT pthread_barrier_t
void ms_barrier_init(BARRIER_OBJECT *barrier, unsigned int count);
void ms_barrier_uninit(BARRIER_OBJECT *barrier);
int ms_barrier_wait(BARRIER_OBJECT *barrier);



//thread
#define TASK_OK 0
#define TASK_ERROR -1
int ms_task_set_name(const char *name);
#define TASK_HANDLE pthread_t
int ms_task_create(TASK_HANDLE *handle, int size, void *(*func)(void *), void *arg);//detached
int ms_task_create_join(TASK_HANDLE *handle, void *(*func)(void *), void *arg);//join
int ms_task_create_join_stack_size(TASK_HANDLE *handle, int size, void *(*func)(void *), void *arg);

int ms_task_join(TASK_HANDLE *handle);
int ms_task_detach(TASK_HANDLE *handle);
int ms_task_cancel(TASK_HANDLE *handle);
#define ms_task_quit(ptr) pthread_exit(ptr)

// dup standard output to telnet.
int dup_task_init(void);
int dup_task_deinit(void);

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int UINT;
#define MAX_PATH    256
#define ERR_LOG_PATH    "/mnt/nand/err_log.txt"

#define ms_max(a,b)            (((a) > (b)) ? (a) : (b))
#define ms_min(a,b)            (((a) < (b)) ? (a) : (b))

/* Word color define */
#define YELLOW          "\033[0;33m"    // yellow
#define GRAY            "\033[2;37m"
#define DGREEN          "\033[1;32m"
#define GREEN           "\033[0;32m"
#define DARKGRAY        "\033[0;30m"
//#define BLACK         "\033[0;39m"
#define BLACK           "\033[2;30m"    // not faint black
#define NOCOLOR         "\033[0;39m \n"
#define CLRCOLOR        "\033[0;39m "
//#define NOCOLOR       "\033[0m \n"
#define BLUE            "\033[1;34m"
#define DBLUE           "\033[2;34m"
#define RED             "\033[0;31m"
#define BOLD            "\033[1m"
#define UNDERLINE       "\033[4m"
#define CLS             "\014"
#define NEWLINE         "\r\n"
//#define CR                "\r\0"
#define MAX_THREAD_POOL_NUM 10
#define MAX_THREAD_POOL_TASK 200

#define MS_SERIAL_POE_BIT (15) // is poe ?
#define MS_SERIAL_HDD_BIT (7)  // a number of hdd
#define MS_SERIAL_HDD_LEN (2)  // a number of hdd

//debug print
enum DEBUG_LEVEL {
    DEBUG_ERR = 0x01,
    DEBUG_WRN = 0x02,
    DEBUG_INF = 0x04,
    DEBUG_GUI = 0x08,
    DEBUG_POE = 0X10,
};
#define DEBUG_LEVEL_DEFAULT 0xffffffff
extern unsigned int global_debug_mask;
typedef void (*hook_print)(const char *format, ...);
extern hook_print global_debug_hook;


typedef enum
{
    TSAR_INFO   = 0,
    TSAR_DEBUG  = 1,
    TSAR_WARN   = 2,
    TSAR_ERR    = 3,
    TSAR_FATAL  = 4
} TSAR_DEBUG_LEVEL;
    
typedef void (*MS_FUNC_PRINT)(int level, const char *file, const char *func, int line, char *msg);

//避免栈空间问题
#define ms_log_err(log) \
    do{\
        int filesize = -1;\
        FILE * hfile = NULL;\
        hfile = fopen("/mnt/nand/err_log.txt", "a");\
        if(hfile){\
            fseek(hfile, 0L, SEEK_END);\
            filesize = ftell(hfile);\
            fwrite(log, 1, strlen(log), hfile);\
            fflush(hfile);\
            fclose(hfile);\
            if(filesize > 1024*1024){\
                system("rm -rf /mnt/nand/err_log.txt");\
            }\
        }\
    }while(0)


//mslogerr add by mjx.milesight 2014.12.17 for Stability Test start...
#define _log_err(level, prefix, format, ...) \
    do{\
        if(global_debug_mask & level){\
            char stime[32] = {0};\
            char log[1024] = {0};\
            char cmd[128] = {0};\
            int filesize = -1;\
            FILE * hfile = NULL;\
            hfile = fopen(ERR_LOG_PATH, "a");\
            if(hfile){\
                fseek(hfile, 0L, SEEK_END);\
                filesize = ftell(hfile);\
                time_to_string(time(0), stime);\
                snprintf(log, sizeof(log), "[%s%s %d %lu==func:%s file:%s line:%d]==" format "\n", \
                         prefix, stime, getpid(), pthread_self(), __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
                fwrite(log, 1, strlen(log), hfile);\
                fflush(hfile);\
                fclose(hfile);\
                snprintf(cmd, sizeof(cmd), "rm -rf %s", ERR_LOG_PATH);\
                if(filesize > 1024*1024)\
                    system(cmd);\
            }\
        }\
    }while(0);

#define mslogerr(level, format, ...) \
    if(level == DEBUG_ERR){\
        _debug(level,  "ERR:",RED format CLRCOLOR, ##__VA_ARGS__)\
        _log_err(level, "ERR:", format, ##__VA_ARGS__)\
    }else if(level == DEBUG_WRN)\
        _log_err(level, "WRN:", format, ##__VA_ARGS__)\
        else\
            _log_err(level, "INF:", format, ##__VA_ARGS__)
//end

#define _debug(level, prefix, format, ...) \
    do{\
        if(global_debug_mask & level){\
            char stime[32] = {0};\
            time_to_string_ms(stime);\
            if(global_debug_hook)\
                global_debug_hook("[%s%s %d %lu--func:%12s file:%12s line:%6d]==" format "\n",\
                                  prefix, stime, getpid(), pthread_self(), __func__, __FILE__, __LINE__, ##__VA_ARGS__);\
            else \
                printf("[%s%s %d %lu==func:%12s file:%12s line:%6d]==" format "\n", \
                       prefix, stime, getpid(), pthread_self(), __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
        }\
    }while(0);

#define msdebug(level, format, ...) \
    do { \
        switch(level) \
        { \
            case DEBUG_ERR: \
                _debug(level,  "ERR:",RED format CLRCOLOR, ##__VA_ARGS__); \
                _log_err(level, "ERR:", format, ##__VA_ARGS__); \
                break; \
            case DEBUG_WRN: \
                _debug(level, "WRN:",YELLOW format CLRCOLOR, ##__VA_ARGS__); \
                break; \
            case DEBUG_GUI: \
                _debug(level, "GUI:",GREEN format CLRCOLOR, ##__VA_ARGS__); \
                break; \
            default: \
                _debug(level, "INF:", format, ##__VA_ARGS__); \
                break; \
        } \
    }while(0)

#define msprintf(format, ...) \
    do{\
        char stime[32] = {0};\
        time_to_string(time(0), stime);\
        printf("[%s %d %lu==func:%s file:%s line:%d]==" format "\n", \
               stime, getpid(), pthread_self(), __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
    }while(0)

#define msprintf_ms(format, ...) \
    do{\
        char stime[32] = {0};\
        time_to_string_ms(stime);\
        printf("[%s %d %lu==func:%s file:%s line:%d]==" format "\n", \
               stime, getpid(), pthread_self(), __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
    }while(0)

//link table
typedef struct ms_node_t {
    void *next;                 /**< next __node_t containing element */
    void *element;              /**< element in Current node */
} ms_node_t;

typedef struct ms_list_t {
    int nb_elt;         /**< Number of element in the list */
    ms_node_t *node;     /**< Next node containing element  */
} ms_list_t;

typedef struct {
    ms_node_t *actual;
    ms_node_t **prev;
    ms_list_t *li;
    int pos;
} ms_list_iterator_t;

typedef struct thread_pool_task {
    void *(*do_pool_task)(void *arg);
    void *arg;
    struct thread_pool_task *next;
} ms_thread_pool_task;

typedef struct {
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_ready;

    ms_thread_pool_task *queue_head;

    pthread_t *pthreadId;
    int shutdown;
    int max_thread_num;
    int cur_queue_size;
} ms_thread_pool;

int ms_list_init(ms_list_t *li);
void ms_list_special_free(ms_list_t *li, void *(*free_func)(void *));
void ms_list_ofchar_free(ms_list_t *li);
int ms_list_size(const ms_list_t *li);
int ms_list_eol(const ms_list_t *li, int pos);
int ms_list_add(ms_list_t *li, void *element, int pos);
void *ms_list_get(const ms_list_t *li, int pos);
int ms_list_remove(ms_list_t *li, int pos);

#define ms_list_iterator_has_elem( it ) ( 0 != (it).actual && (it).pos < (it).li->nb_elt )
void *ms_list_get_first(ms_list_t *li, ms_list_iterator_t *it);
void *ms_list_get_next(ms_list_iterator_t *it);
void *ms_list_iterator_remove(ms_list_iterator_t *it);
int  ms_list_iterator_add(ms_list_iterator_t *it, void *element, int pos);

//net config
int net_get_ifaddr(const char *ifname, char *addr, int length);
int net_get_netmask(const char *ifname, char *addr, int length);
int net_get_gateway(const char *ifname, char *addr, int length);
int is_port_use(int port);


int write_mac_conf(char *filename);
int read_mac_conf(char *mac);
int write_mac_conf_ex(char *filename, char *mac);


int ms_sys_get_chipinfo(char *chip_info, int size);
int ms_sys_get_cpu_temperature(float *T);

int net_get_ipv6_ifaddr(char type, const char *ifname, char *addr, int length, char *prefix,
                        int length2);//hrz.milesight
int net_get_ipv6_gateway(char type, const char *ifname, char *gateway, int length);//cgi popen failed in hi3798
int net_get_ipv6_gateway2(char type, const char *ifname, char *gateway, int length);//fopen
int net_is_validipv6(const char *hostname);
int net_is_haveipv6(void);
int net_is_dhcp6c_work(const char *ifname);
int net_get_dhcp6c_pid(const char *ifname, int *pid);
int net_ipv4_trans_ipv6(const char *source, char *dest, int length);
int net_get_dhcpv6gateway(const char *dhcpAddr, const char *dhcpPrefix, char *gateway,
                          int length);//for dhcpv6 set gateway
int net_get_dhcp_dns(const char *ifname, char *dns, int length);


//pool thread
void ms_pool_thread_init(ms_thread_pool *pool_task, int thread_num, void *(*pthread_main)(void *arg));
int ms_pool_add_task(ms_thread_pool *pool_task, ms_thread_pool_task *new_task);
int ms_pool_thread_uninit(ms_thread_pool *pool_task);

void ms_base64_encode(unsigned char *from, char *to, int len);
int ms_base64_decode(const unsigned char *base64, unsigned char *bindata, int bindata_len);
int Base64to16(char *s, char *ret);
char *hex_2_base64(char *_hex);
int url_decode(char *dst, const char *src, int len);
int url_encode(char *dst, const char *src, int len);
int sqa_encode(char *dst, const char *src, int len);
void get_url_encode(const char *url, const int strSize, char *res, const int resultSize);
void get_url_decode(char *url);

char *get_base64_encode(char *str, int str_len);
unsigned char *get_base64_decode(char *code);

MS_S32 base64Codec(MS_S32 codec,
                   MS_U8 *dst, MS_S32 dstSize,
                   MS_U8 *src, MS_S32 srcSize,
                   MS_U8 *table, MS_S32 tableSize,
                   MS_S8 endChar);
MS_S32 base64GetDecodeLen(MS_U8 *src, MS_S8 endChar);
MS_S32 base64EncodeOrg(MS_U8 *dst, MS_S32 dstSize, MS_U8 *src, MS_S32 srcSize);
MS_S32 base64DecodeOrg(MS_U8 *dst, MS_S32 dstSize, MS_U8 *src, MS_S32 srcSize);
MS_S32 base64EncodeUrl(MS_U8 *dst, MS_S32 dstSize, MS_U8 *src, MS_S32 srcSize);
MS_S32 base64DecodeUrl(MS_U8 *dst, MS_S32 dstSize, MS_U8 *src, MS_S32 srcSize);

int check_mac_code(char *code);
int check_sn_code(char *code);




//cond & mutex
int ms_cond_init(COND_OBJECT *cond, MUTEX_OBJECT *mutex);
int ms_cond_uninit(COND_OBJECT *cond, MUTEX_OBJECT *mutex);
int ms_cond_signal(COND_OBJECT *cond, MUTEX_OBJECT *mutex);
int ms_cond_wait(COND_OBJECT *cond, MUTEX_OBJECT *mutex);
int ms_cond_wait_time(COND_OBJECT *cond, MUTEX_OBJECT *mutex, int secs);

unsigned int ms_get_mem(int type);
unsigned long ms_get_file_size(const char *path);
int ms_check_file_exist(char *filename);
int ms_remove_file(char *filename);

// Message API.
int ms_msg_init(int msgKey);
int ms_msg_deinit(int msgKey);
int ms_msg_rcv_nowait(int qid, void *msgbuf, int msgsize, int msgtype);
int ms_msg_rcv_wait(int qid, void *msgbuf, int msgsize, int msgtype);
int ms_msg_rcv_wait_timeout(int qid, void *msgbuf, int msgsize, int msgtype, int ms);
int ms_msg_snd_nowait(int qid, void *msgbuf, int msgsize);
int ms_msg_snd_wait(int qid, void *msgbuf, int msgsize);

#define AUDIO_BUF_SIZE          2048
typedef struct audio_msg_s {
    long    mtype;
    int     dst;
    int     lenth;
    char    buf[AUDIO_BUF_SIZE];
} audio_msg_t;

typedef struct {
    char audio_data[AUDIO_BUF_SIZE];
    int audio_len;
    int dst;//-1:nvr(vapi) to web(boa)  or  web to nvr;     0-63:channel
} ms_audio_pkg_t;

typedef struct req_audio_talk_conf {
    int from;  //TALK_DATA_P2P or not
    int cmd;   //0:close   1:open
    int channel;
} req_audio_talk_conf;

#define MAX_LIMIT_ADDR      100

typedef enum {
    ADDRESS_TYPE_MAC = 0,
    ADDRESS_TYPE_IP_SINGLE,
    ADDRESS_TYPE_IP_RANGE,
    ADDRESS_TYPE_MAX,
} ADDRESS_TYPE;

struct access_list {
    ADDRESS_TYPE type;//0:mac;  1:ip single;  2:ip range
    char address[32];
};

struct access_filter {
    int enable;
    int filterType;
    int cnt;
    struct access_list addr_list[MAX_LIMIT_ADDR];
};

int get_client_mac_by_ip(char *ip, char *mac, int mac_size);
int isAddressAllow(char *address, struct access_filter *conf);
int check_client_allow(struct access_filter *conf, char *ip, char *mac, int mac_size);

int ms_spliced_copy(int in_fd, int out_fd);
int ms_spliced_copy_ex(int in_fd, int pipe_out_fd);
int ms_std_copy(int in_fd, int out_fd);
int ms_sendfile_copy(int in_fd, int out_fd);
int ms_file_copy(char *fileIn, char *fileOut);
int ms_file_move(char *srcPath, char *dstPath);

int ms_set_task_priority(TASK_HANDLE thread_id, int priority);

char *ms_strstr(char *str, char *element, const char *fileName, int line, const char *func);
#define MS_STRSTR(str, element) (ms_strstr(str, element, __FILE__, __LINE__, __func__))

int ms_strcasecmp(const char *s1, const char *s2);

int _ms_strcmp(const char *s1, const char *s2, const char *file, const char *func, int line);
#define ms_strcmp(s1, s2) _ms_strcmp(s1, s2, __FILE__, __func__, __LINE__)


#ifdef __cplusplus
}
#endif


#endif
