/**
 * @file log.h
 *
 * @date Created on: 2013-12-3
 * @author: chimmu
 */

#ifndef LOG_H_
#define LOG_H_

#include "cellular/cel_public.h"
#include "msfs/msfs_log.h"


#define VERSION_MAIN	0
#define VERSION_SUB		1
#define VERSION_PATCH	0

#define LOG_MAX_CONNECT   10

#define LOCK_PREFIX	"/tmp"

enum log_level {
	LOG_LEVEL_COMM = 0x0, // be writed to disk/esata/nas
	LOG_LEVEL_CRIT = 0x1, // be writed to /mnt/nand/xxxx
};

enum _operator {
    OP_LOCAL = 0,
    OP_REMOTE,
};

enum sub_op_param {
    SUB_PARAM_NONE = 900,
    SUB_PARAM_ADD_IPC,//901
    SUB_PARAM_DEL_IPC,//902
    SUB_PARAM_EDIT_IPC,//903
    SUB_PARAM_PTZ_CONFIG,//904
    SUB_PARAM_RECORD_MODE,//905
    SUB_PARAM_RECORD_CFG,//906
    SUB_PARAM_RECORD_SCHED,//907
    SUB_PARAM_VIDEOLOSS,//908
    SUB_PARAM_MOTION,//909
    SUB_PARAM_ALARMINPUT,//910
    SUB_PARAM_ALARMOUTPUT,//911
    SUB_PARAM_LAYOUTCONFIG,//912
    SUB_PARAM_SYSTEM_GENERAL,//913
    SUB_PARAM_NETWORK,//914
    SUB_PARAM_NETWORK_PPPOE,//915
    SUB_PARAM_NETWORK_DDNS,//916
    SUB_PARAM_NETWORK_MAIL,//917
    SUB_PARAM_NETWORK_MORE,//918
    SUB_PARAM_DISK_RECYCLE_MODE,//919
    SUB_PARAM_HOLIDAY,//920
    SUB_PARAM_ADD_USER,//921
    SUB_PARAM_DEL_USER,//922
    SUB_PARAM_CAMERAIO,//923
    SUB_PARAM_VOLUMEDETECT,//924
    SUB_PARAM_AUDIO_FILE,//925
    SUB_PARAM_MAX = SUB_PARAM_AUDIO_FILE,
};

enum log_title{
    LOG_TITLE_MIN = 10000,
    LOG_TITLE_NO = LOG_TITLE_MIN,
    LOG_TITLE_MAINTYPE,
    LOG_TITLE_TIME,
    LOG_TITLE_SUBTYPE,
    LOG_TITLE_PARAM,
    LOG_TITLE_CHANNEL,
    LOG_TITLE_USER,
    LOG_TITLE_IP,
    LOG_TITLE_MAX = LOG_TITLE_IP,
};

enum log_export_lang{
	LOG_LANG_CHINESE = 1,
	LOG_LANG_ENGLISH = 2,
	LOG_LANG_DEFAULT = LOG_LANG_ENGLISH,
	//... ...
};

enum op_tag_action{
	OP_TAG_SET,
	OP_TAG_REMOVE,
	OP_TAG_EDIT,
};

struct log_version {
    int main;
    int sub;
    int patch;
};

struct log_struct {
    long time; ///< current time, leave it with blank
    int tzone; ///< time zone, if > 0, west, else east, leave it with blank
    int main_type; ///< main type of this operation
    int sub_type; ///< sub type of this operation
    int sub_param; ///< sub type parameter
    //long long channel; ///< EVENT or camera channel mask
    CH_MASK log_mask;	///david add 
    int remote; ///< whether controlled by remote
    char user[64]; ///< who did this
    char ip[64]; ///< if local, filled it with 0.0.0.0 hrz.milesight 24-->64 for ipv6
    char detail[256]; ///< reserved
};

struct query_struct {
    long time_start;
    long time_end;
    int main_type;
    int sub_type;
    int limit; ///< max records
};

struct query_result {
    struct log_struct *node;
    struct query_result *next;
};

///------------------ MSFS LOG detail Solin 18.5.3----------------------
///  enum - struct
enum LOG_DETAIL_BODY{
	LOG_DETAIL_BODY_MIN = 1,

	// EVENT
	EV_EXCEPT_DISK_FULL = LOG_DETAIL_BODY_MIN,
	EV_EXCEPT_DISK_FAIL,
	EV_SMART_EVENT_COUNT,
	EV_IPC_EVENT,
	EV_ANPR_EVENT,
	EV_IPC_ALARM_IN_EVENT,
	EV_IPC_VCA_EVENT,
	EV_PEOPLECNT_EVENT,
	EV_POS_EVENT,
	EV_EXCEPT_DISK_OFFLINE,
	EV_EXCEPT_DISK_HEAT,
	EV_EXCEPT_DISK_MICROTHERM,
	EV_EXCEPT_DISK_CONNECTION_EXCEPTION,
	EV_EXCEPT_DISK_STRIKE,

	// LOCAL / REMOTE OPTION
	OP_REBOOT     = 100,
	OP_ADD_NETWORK_DISK,
	OP_INIT_DISK,
	OP_REC_START_USER,
	OP_REC_STOP_USER,
	OP_REC_START_SCHE,
	OP_REC_STOP_SCHE,
	OP_SET_VIDEO_PARA,
	OP_VIDEO_EXPORT,
	OP_PICTURE_EXPORT,
	OP_CONF_NETWORK_DISK,
	OP_A_D_E_IP_CHANNEL,
	OP_TAG_OPERATION,
	OP_FAILOVER_OPERATION,
	OP_LOCK_OR_UNLOCK_FILE,

	INFO_DISK_NETWORK,
	INFO_DISK_SMART,
	INFO_FAILOVER,
	INFO_RECORD_STATUS,
	INFO_CHANNEL_ANR,
	//....

    EXCEPT_FAILOVER,
    EXCEPT_EVENT,
    EXCEPT_NETWORK,
	EXCEPT_RECORD_FAILED,
	EXCEPT_SNAPSHOT_FAILED,

	EXCEPT_DETAIL_DATE_ERROR,

	OP_MS_DEBUG         = 2000,         // for msfs debug
	LOG_DETAIL_BODY_MAX = OP_MS_DEBUG,
};

// ->event
struct ev_except_disk_full{ // SUB_EXCEPT_DISK_FULL
	int port;
	char info[MAX_LEN_32]; // for ipv4 / ipv6
};

struct ev_smart_event_cnt{ // SUB_EXCEPT_DISK_FULL
	char pInfo[MAX_LEN_128];
};


// ->option
struct op_lr_reboot{ // OP_LOCAL_REBOOT
	char model[MAX_LEN_32];
	char mac[MAX_LEN_32];
	char softver[MAX_LEN_32];
};

struct op_lr_add_del_network_disk{ // OP_LOCAL_REBOOT
	char type[MAX_LEN_16];
	char ip[MAX_LEN_64];
	char path[MAX_LEN_128];
};

struct op_lr_rec_start_stop_user{ // operate by user
	char user[MAX_LEN_64];
	int chanid;
};

struct op_lr_rec_start_stop_sche{ // operate by schedule
	char user[MAX_LEN_64];
	int chanid;
};

struct op_lr_init_disk{
	char user[MAX_LEN_64];
	int port;
	int state;
};

struct op_lr_setvideo_para{
	int brightness;
	int contrast;
	int saturation;
	int sharpness;
	int nflevel;
};

struct op_video_export{
	char chan[MAX_LEN_128];
	char start[MAX_LEN_32];
	char end[MAX_LEN_32];
};

struct op_picture_export{
	char chan[MAX_LEN_128];
	char ptime[MAX_LEN_32];
};

struct op_tag_operation{
	int action;
	char tname[MAX_LEN_32];
	char ptime[MAX_LEN_32];
};

struct op_lr_a_d_e_ip_channel{
	int action;
	char ip[MAX_LEN_64];
	int port;
	int channel;
	char domain[MAX_LEN_64];
	char protocal[MAX_LEN_16];
	char tran_protocal[MAX_LEN_16];
	unsigned char timeset;
};

struct info_disk_network{
	unsigned char port;
	char vendor[DEV_NAME_LEN];
	int stat;
	int type;
	int rw;
	int opt;
};

//record
struct info_record_status{
    int ch;
    int stream; // @see STREAM_TYPE
    int event;  // @see REC_EVENT_EN
};

struct detail_record_except{
    int ch;
    int stream;//
};

struct detail_network_except{
    int lan;
};

struct detail_anpr_info{
    int ch;
    int stream;//
    char plate[32];
    char pmode[16];
    char pregion[16];
    int direction;
    int roiId;
    int plateColor;
    int vehicleType;
    int vehicleColor;
    int vehicleSpeed;
};

struct detail_ipc_alarm_info{
    int ch;
    char alarm_name[64];
};

//日志数据存入硬盘，结构体大小需前后兼容，后续新增数据需加在struct log_data中
struct detail_ipc_vca_info{
    int ch;
    int stream;//
    int objtype;
    int region;
};

struct detail_peoplecnt_info{
	int type;  //0: ipc 1:NVR
    int id;
    int stream; 
    int incnt;
    int outcnt;
    int thresholds;
    int line;
};

//failover
enum op_failover_action{
	OP_FAILOVER_ADD_MASTER,
	OP_FAILOVER_DEL_MASTER,
	OP_FAILOVER_ADD_IPC,
	OP_FAILOVER_DEL_IPC
};

struct detail_failover_operation{
	int action;
    char ipaddr[MAX_LEN_64];
    int chnid;
    char ddns[MAX_LEN_64];
    char protocol[MAX_LEN_64];
    char transmit[MAX_LEN_16];
    char port[MAX_LEN_16];
};

enum info_failover_action{
	INFO_FAILOVER_MASTER_LOGIN,
	INFO_FAILOVER_MASTER_LOGOUT,
	INFO_FAILOVER_MASTER_SYNC,
	INFO_FAILOVER_SLAVE_START_WORK,
	INFO_FAILOVER_SLAVE_STOP_WORK,
	INFO_FAILOVER_IPC_START_RECORD,
	INFO_FAILOVER_IPC_STOP_RECORD,
	INFO_FAILOVER_MASTER_ANR_START,
	INFO_FAILOVER_MASTER_ANR_END
};

struct detail_failover_info{
	int action;
    char ipaddr[MAX_LEN_64];//主机地址/热备机地址
    int master_id;//工作机序号
    int slave_port;//服务端口
    int master_port;//热备端口
    char start_time[MAX_LEN_32];
    char end_time[MAX_LEN_32];
    int anr_type;//传回类型：0=热备机ANR录像
    float anr_percent;//传回进度
    int error_type;//错误码
};

enum except_failover_action{
	INFO_FAILOVER_MASTER_ANR_ERROR,
	INFO_FAILOVER_SLAVE_WORK_ERROR
};

struct detail_failover_except{
	int action;
    int anr_type;//传回类型：0=热备机ANR录像
    char start_time[MAX_LEN_32];
    char end_time[MAX_LEN_32];
    char cur_time[MAX_LEN_32];
    int error_type;//错误码
    float anr_percent;//传回进度
    int master_id;
    char ipaddr[MAX_LEN_64];
};


//anr
enum info_channel_anr_action
{
	INFO_CHANNEL_ANR_RETURN_START = 0,
	INFO_CHANNEL_ANR_RETURN_FINISH = 1
};

struct detail_channel_anr_info
{
	int action;
	int chnid;
	int res;//0=success, -1=failed
	int retry;
	char ipaddr[MAX_LEN_64];
	char start_time[MAX_LEN_32];
	char end_time[MAX_LEN_32];
};

enum except_record_failed_action
{
	ACTION_COMMON_FAILED 			= 0,
	ACTION_STREAM_UNAVAILABLE		= 1
};

//record failed
struct detail_record_failed_except
{
	int action;
	int ch;
	int type;
};

enum except_snapshot_failover_action
{
	ACTION_SNAP_FAILED		= 0,
	ACTION_OVF_FAILED		= 1
};

struct detail_snapshot_failed_excepy
{
	int action;
	int ch;
	int type;//@see INFO_MAJOR_EN
};

struct detail_lockor_unlock_file
{
	int chnid;
	char start_time[MAX_LEN_32];
	char end_time[MAX_LEN_32];
};

struct detail_date_error_except
{
	int action;
};


#define INFO_DISK_SMART_MAX ((LOG_PKG_DATA_MAX - DEV_NAME_LEN - sizeof(unsigned char))/sizeof(struct disk_smart_attr))
struct disk_smart_attr{
	unsigned char id;
	int value;
	int worst;
	int thres;
	unsigned char statu;
}__attribute__((__packed__));
struct info_disk_smart{
	char vendor[DEV_NAME_LEN];
	struct disk_smart_attr attr[INFO_DISK_SMART_MAX];
	unsigned char result;
};
///-----------------------------


////////////////////////////////////
//for formatting disk
int lock_file(const char *file);

int unlock_file(int fd);
////////////////////////////////////

/**
 * @brief write current message to sqlite
 * @param log_struct: message to write
 * @param disk: current record disk
 * @retval 0: success; -1: failure
 */
int log_write(struct log_struct *log_struct/*, const char *disk*/);

/**
 * @brief query log that matches the conditions of "query"
 * @param query: query conditions
 * @param res: linklist's head, when function returned, log message was stored in it
 * @param count: the length of the linklist
 * @retval 0: success; -1: failure
 */
int log_query(struct query_struct *query, struct query_result **res, int *count);

/**
 * @brief destroy the linklist allocate by log_query
 * @param res: linklist's head
 * @retval 0: success; -1: failure
 */
int log_query_destroy(struct query_result **res);

/**
 * @brief get current log version
 * @param disk: mounted disk
 * @param version: when function returned, version info will be stored in it
 * @retval 0:success; -1: failure */
int log_get_version(const char *disk, struct log_version *version);
/**
 * @brief check log version
 * @param disk: mounted disk
 * @retval 0:version matched; -1:failure
 */
int log_check_version(const char *disk);

//translate
int trans_init(const char *lang);
void trans_deinit(void);
int trans_get_value(int key, char *value, int len);

int disk_log_write(struct log_struct *log);

int get_log_except_network_disconnected(struct log_data *log, int lan);

#endif /* LOG_H_ */
