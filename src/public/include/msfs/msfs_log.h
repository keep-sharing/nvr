#ifndef __MSFS_LOG_H__
#define __MSFS_LOG_H__

#include "msfs_disk.h"
#include "msfs_rec.h"

/////////////////////////////////////////////////////////////
///////// version 1.2 20180914 solin@milesight.com //////////
/////////////////////////////////////////////////////////////


#define	LOG_VER_			   "MF_LOG_V1.2"

#define LOG_VER_SIZE_          (32)

#define LOG_HEAD_INFO_SIZE_    (128)
#define LOG_SEG_INFO_SIZE_     (1024)
#define	LOG_INDEX_SIZE_		   (128)
#define LOG_DATA_SIZE_         (256)

#define	SEG_ONE_MAX_		   (64)       // one disk // will be erase 2000 per once
#define	SEG_ALL_MAX_		   (5000)     // include all disks


#define LOG_DETAIL_MAX_        (512)
#define LOG_DETAIL_BUF_MAX     (LOG_DETAIL_MAX_ * 8)



#define	LOG_HEAD_OFF_		   (0)


#define LOG_WR_CACHE_MAX_      (500)
#define MAX_LOG_CACHE          (500)

#if defined(_HI3536C_)
#define LOG_SEARCH_MAX_        (2000)
#else
#define LOG_SEARCH_MAX_        (10000)
#endif

#define FREE_HEAD_             ((LOG_HEAD_INFO_SIZE_ - sizeof(struct log_head_data))/sizeof(char))
#define FREE_SEG_              ((LOG_SEG_INFO_SIZE_ - sizeof(struct log_seg_data))/sizeof(char))
#define FREE_INDEX_            ((LOG_INDEX_SIZE_ - sizeof(struct log_index_data))/sizeof(char))
#define FREE_DATA_             ((LOG_DATA_SIZE_ - sizeof(struct log_data_cell) - sizeof(MF_U32))/sizeof(char))


#define REAL_HEAD_OFF(off)     ((off) + LOG_HEAD_OFF_)
#define REAL_SEG_OFF(off)      (REAL_HEAD_OFF(off) + LOG_HEAD_INFO_SIZE_)
#define REAL_INDEX_OFF(off)    (REAL_SEG_OFF(off) + (LOG_SEG_INFO_SIZE_ * SEG_ONE_MAX_))


#define HEAD_NODE_SIZE         (sizeof(struct log_head_node))
#define SEG_NODE_SIZE          (sizeof(struct log_seg_node))
#define DATA_NODE_SIZE         (sizeof(struct log_data_node))


#define LOG_MAX_SIZE           (HEADER_MAINLOGLEN + HEADER_CURLOGLEN)
#define LOG_REMAIN_SIZE        (LOG_MAX_SIZE - (LOG_HEAD_INFO_SIZE_ + (LOG_SEG_INFO_SIZE_ * SEG_ONE_MAX_)))
#define DATA_BLOCK_SIZE        (LOG_REMAIN_SIZE / SEG_ONE_MAX_)


/* nand partition */
#define LOG_NAND_SEG_MAX            (5)
#define LOG_NAND_REMAIN_SIZE        (HEADER_NANDLOG_SIZE - (LOG_HEAD_INFO_SIZE_ + (LOG_SEG_INFO_SIZE_ * LOG_NAND_SEG_MAX)))
#define LOG_NAND_DATA_BLOCK_SIZE    (LOG_NAND_REMAIN_SIZE / LOG_NAND_SEG_MAX)
#define REAL_NAND_INDEX_OFF(off)    (REAL_SEG_OFF(off) + (LOG_SEG_INFO_SIZE_ * LOG_NAND_SEG_MAX))

#define LOG_FILE_EXT(ext)           (#ext)


enum log_seg_zone{
	LOG_SEG_AVAI = 0x0,
	LOG_SEG_FULL = 0x1,
};

enum log_play{
	LOG_NO_PLAY = 0x0,
	LOG_IS_PLAY = 0x1,
};

enum op_main {    ///主要类型 main type
    MAIN_ALL    = 0x1,
    MAIN_EVENT  = 0x2,
    MAIN_OP     = 0x4,
    MAIN_INFO   = 0x8,
    // 0x10 reserve for SUB_AUDIODETECT
    MAIN_EXCEPT = 0x20,
    MAIN_DEBUG  = 0x40, // for system infomation
    MAIN_MAX = MAIN_DEBUG,
    //max bits: 32bit
};

enum op_sub {            ////
    SUB_ALL = 10,
    ///////EVENT START////////11-16
    SUB_EVENT_MIN = 11,
    SUB_ALARM_IN = SUB_EVENT_MIN,   //11   A1.Event报警-Alarm in 报警输入
    SUB_ALARM_OUT,                  //12   A2.Event报警-Alarm out报警输出
    SUB_MOTION_START,               //13   A3.Event报警-Start Motion Detection开始移动检测
    SUB_MOTION_END,                 //14   A4.Event报警-Stop Motion Detection停止移动检测
    SUB_CAMERAIO,                   //15
    SUB_AUDIODETECT,                //16
    SUB_REGION_ENTRANCE_ALARM,		//17  Region Entrance Alarm 触发区域入侵警报
    SUB_STOP_REGION_ENTRANCE_ALARM,	//18  Region Entrance Alarm 停止区域入侵警报
    SUB_REGION_EXITING_ALARM,		//19  Region Exiting Alarm  触发区域离开警报
    SUB_STOP_REGION_EXITING_ALARM,	//20  Region Exiting Alarm  停止区域离开警报
    SUB_START_ADVANCED_MOTION,		//21  Start Advanced Motion Detection 触发高级移动检测
    SUB_STOP_ADVANCED_MOTION,		//22  Stop Advanced Motion Detection 停止高级移动检测
    SUB_TAMPERING_ALARM,			//23  Tampering Alarm 触发遮挡警报
    SUB_STOP_TAMPERING_ALARM,		//24  Tampering Alarm 停止遮挡警报
    SUB_LINE_CROSSING_ALARM,		//25  Line Crossing Alarm 触发过线警报
    SUB_STOP_LINE_CROSSING_ALARM,	//26  Line Crossing Alarm 停止过线警报
    SUB_LOITERING_ALARM,			//27  Loitering Alarm 触发徘徊警报
	SUB_STOP_LOITERING_ALARM,		//28  Loitering Alarm 停止徘徊警报
	SUB_HUMAN_DETECTION_ALARM,		//29  Human Detection Alarm 触发人体识别警报
	SUB_STOP_HUMAN_DETECTION_ALARM,	//30  Human Detection Alarm 停止人体识别警报
    SUB_PEOPLE_COUNTING_IN,			//31  People Counting In 人数统计进入计数
    SUB_PEOPLE_COUNTING_OUT,		//32  People Counting Out 人数统计离开计数
    SUB_OBJECT_LEFT_START,			//33  object left start
    SUB_OBJECT_LEFT_END,			//34  object left end
    SUB_OBJECT_REMOVE_START,		//35  object remove start
    SUB_OBJECT_REMOVE_END,			//36  object remove end
    SUB_ANPR,						//37 ANPR
    SUB_IPC_PEOPLE_COUNT_START,		//38
    SUB_IPC_PEOPLE_COUNT_END,		//39
    SUB_GROUP_PEOPLE_COUNT_START,	//40
    SUB_GROUP_PEOPLE_COUNT_END,		//41
    SUB_FACE,                       //42 Face Detection
    SUB_POS_CONNECT,                //43 POS connect
    SUB_POS_DISCONNECT,             //44 POS disconnect
    SUB_EVENT_MAX = SUB_POS_DISCONNECT,
//  SUB_EVENT_MAX = SUB_BLOCK_END,
    ///////EVENT END//////////

    //////OPERATION START/////////////100-140
    SUB_OP_LOCAL_MIN = 100,
    SUB_OP_BOOT = SUB_OP_LOCAL_MIN,
    //////LOCAL START/////////
    SUP_OP_POWER_ON,                  //b101{}【新增】
    SUP_OP_ABNORMAL_SHUTDOWN,         //B102.Operation操作-Abnormal Shutdown异常关机【新增】
    SUB_OP_SHUTDOWN_LOCAL,            //B103 Local:Shut Down本地:关机
    SUB_OP_REBOOT_LOCAL,              //B104 Local:Reboot本地:重启[GUI log
    SUB_OP_LOGIN_LOCAL,               //B105 -Local:Login本地:登录
    SUB_OP_LOGOUT_LOCAL,              //B106 Local:Logout本地:注销
    SUB_OP_CONFIG_LOCAL,              //B107.Operation操作-Local:Configure Parameters本地:参数配置
    //SUB_OP_EMERGENCY_REC_START_LOCAL, //B108.Operation操作-Local:Start Record本地:开始录像
    //SUB_OP_EMERGENCY_REC_STOP_LOCAL,  //B109.Operation操作-Local:Stop Record本地:停止录像
    SUB_OP_SNAPSHOT_LOCAL,          //  B110.Operation操作-Local:Capture本地:抓图
    SUB_OP_PLAYBACK_SNAPSHOT_LOCAL   ,//B111.Operation操作-Local:Playback Capture本地:回放抓图
    SUB_OP_SET_VIDEO_PARAM_LOCAL,     //B112.Operation操作-Local:Image Configuration本地:图像配置
    SUB_OP_PTZ_CONTROL_LOCAL,         //B113.Operation操作-Local:PTZ Control本地:PTZ控制
    SUP_OP_LOCK_FILE,                 //B114.Operation操作-Local:Lock File本地:锁定文件【新增】
    SUP_OP_UNLOCK_FILE,               //B115.Operation操作-Local:Unlock File本地:解锁文件【新增】
    SUB_OP_DISK_INIT_LOCAL,           //B116.Operation操作-Local:Initialize HDD本地:硬盘初始化
    SUB_OP_PLAYBACK_LOCAL,            //B117.Operation操作-Local:Playback本地:回放
    SUP_OP_ADD_IP_CHANNEL_LOCK,       //B118.Operation操作-Local:Add IP Channel本地:添加IP通道【新增】
    SUP_OP_DELETE_IP_CHANNEL_LOCK,    //B119.Operation操作-Local:Delete IP Channel本地:删除IP通道【新增】
    SUP_OP_EDIT_IP_CHANNEL_LOCK,      //B120.Operation操作-Local:Edit IP Channel本地:修改IP通道【新增】
    SUB_OP_PROFILE_EXPORT_LOCAL,      //B121.Operation操作-Local:Export Config File本地:导出配置文件
    SUB_OP_PROFILE_IMPORT_LOCAL,      //B122.Operation操作-Local:Import Config File本地:导入配置文件
    SUB_OP_PROFILE_RESET_LOCAL ,      //B123.Operation操作-Local:Restore to Defult本地:恢复默认参数【原“重置配置”】
    SUP_OP_UPGRADE_IP_CHANNEL_LOCK,   //B124.Operation操作-Local:Upgrade IP Channel本地:升级IP通道固件【新增预留】
    SUB_OP_UPGRADE_LOCAL,             //B125.Operation操作-Local:Upgrade本地:升级
    SUP_OP_PLAYBACK_BY_FILE_LOCK,     //B126.Operation操作-Local:Playback by File本地:按文件回放【新增】
    SUP_OP_PLAYBACK_BY_TIME_LOCK,     //B127.Operation操作-Local:Playback by Time本地:按时间回放【新增】
    SUB_OP_VIDEO_EXPORT_LOCAL,        //B128.Operation操作-Local:Export Record File本地:导出录像文件
    SUB_OP_PICTURE_EXPORT_LOCAL,      //B129.Operation操作-Local:Export Image File本地:导出图片文件  
    SUP_OP_ADD_NETWORK_DISK_LOCK,     //B130.Operation操作-Local:Add Network Disk本地:添加网络硬盘【新增】
    SUP_OP_DELETE_NETWORK_DISK_LOCK,  //B131.Operation操作-Local:Delete Network Disk本地:删除网络硬盘【新增】
    SUP_OP_CONFIG_NETWORK_DISK_LOCK,  //B132.Operation操作-Local:Config Network Disk本地:设置网络硬盘【新增】
    SUP_OP_TAG_OPERATION_LOCK,        //B133.Operation操作-Local:Tag Operation本地:标签操作【新增】
    SUP_OP_MAIN_SUP_MONITOR_SWITCH_LOCK,  //B134.Operation操作-Local:Main/Sub Monitor Switch本地:主次屏切换【新增】
    SUP_OP_PHYSICAL_DISK_SELF_CHECK_LOCK, //B135.Operation操作-Local:Physical Disk Self Check本地:物理硬盘自检【新增】
    SUP_OP_MOUNT_DISK_LOCK,               //B136.Operation操作-Local:Mount Disk本地:加载硬盘【新增】
    SUP_OP_UNMOUNT_DISK_LOCK,             //B137.Operation操作-Local:Unmount Disk本地:卸载硬盘【新增】
    SUP_OP_DELETE_ABNORMAL_LOCK,          //B138.Operation操作-Local:Delete Abnormal Disk本地:删除异常硬盘【新增】
	SUP_OP_START_EMERGENCY_RECORD_LOCK,   //B139.Operation操作-Local:Start Emergency Record本地:开始紧急录像【新增】--r8
    SUP_OP_STOP_EMERGENCY_RECORD_LOCK,    //B140.Operation操作-Local:stop Emergency Record本地:jies录像【新增】--r8
	SUP_OP_SYSTEM_TIME_SYNC_LOCAL,        //141  B141.Information提示-System Time Sync系统校时【新增
	
	SUP_OP_FAILOVER_SLAVE_ADD_MASTER,     //140 本地添加工作机（热备机）
	SUP_OP_FAILOVER_SLAVE_DEL_MASTER,     //141 本地删除工作机（热备机）
	SUP_OP_FAILOVER_SLAVE_ADD_IPC,        //142 操作-本地添加IP通道（热备机开始热备时）
    SUP_OP_FAILOVER_SLAVE_DEL_IPC,        //143 操作-本地删除IP通道（热备机，工作机上线后）
    
    SUB_OP_LOCAL_MAX = SUP_OP_FAILOVER_SLAVE_DEL_IPC,
    //////LOCAL END////////////

    //////REMOTE START/////////
    SUB_OP_REMOTE_MIN = 200,
    //SUB_OP_REBOOT_REMOTE = SUB_OP_REMOTE_MIN,//200
    SUB_OP_REBOOT_REMOTE = SUB_OP_REMOTE_MIN,          //200  B201.Operation操作-Remote:Reboot远程:重启
    SUB_OP_LOGIN_REMOTE,           //201  B202.Operation操作-Remote:Login远程:登录
    SUB_OP_LOGOUT_REMOTE,          //202  B203.Operation操作-Remote:Logout远程:注销
    SUB_OP_UPGRADE_REMOTE,         //203  B204.Operation操作-Remote:Upgrade远程:升级
    SUB_OP_CONFIG_REMOTE,          //204  B205.Operation操作-Remote:Config Parameters远程:参数配置
    //SUB_OP_RECORD_START_REMOTE,    //205  B206.Operation操作-Remote:Start Record远程:开始录像
    //SUB_OP_RECORD_STOP_REMOTE,     //206  B207.Operation操作-Remote:Stop Record远程:停止录像
    SUB_OP_SNAPSHOT_REMOTE,        //207  B208.Operation操作-Remote:Capture远程:抓图
    SUB_OP_PTZ_CONTROL_REMOTE,     //208  B209.Operation操作-Remote:PTZ Control远程:PTZ控制
    SUB_OP_DISK_INIT_REMOTE,       //209  B210.Operation操作-Remote:Initialize HDD远程:硬盘初始化
    SUB_OP_PREVIEW_REMOTE,         //210  B211.Operation操作-Remote:Live View远程:预览
    SUB_OP_PLAYBACK_REMOTE      ,  //211  B212.Operation操作-Remote:Playback远程:回放/下载
    SUP_OP_ADD_IP_CHANNEL_REMOTE,    //212  B213.Operation操作-Remote:Add IP Channel远程:添加IP通道【新增】
    SUP_OP_DELETE_IP_CHANNEL_REMOTE, //213  B214.Operation操作-Remote:Delete IP Channel远程:删除IP通道【新增】
    SUP_OP_EDIT_IP_CHANNEL_REMOTE,   //214  B215.Operation操作-Remote:Edit IP Channel远程:修改IP通道【新增】
    SUB_OP_PROFILE_EXPORT_REMOTE,    //215  B216.Operation操作-Remote:Export Config File远程:导出配置文件
    SUB_OP_PROFILE_IMPORT_REMOTE,    //216  B217.Operation操作-Remote:Import Config File远程:导入配置文件
    SUB_OP_ADMIN_RESET_REMOTE,       //217  B218.Operation操作-Remote:Restore to Defult远程:恢复默认参数【原“重置配置”】
    SUP_OP_UPGRADE_IP_CHANNEL_REMOTE,    //218  B219.Operation操作-Remote:Upgrade IP Channel远程:升级IP通道固件【新增预留】
    SUB_OP_SET_VIDEO_PARAM,              //219  B220.Operation操作-Remote:Image Configuration远程:图像配置
    SUB_OP_PLAYBACK_SNAPSHOT_REMOTE,     //220  B221.Operation操作-Remote:Playback Capture远程:回放截图
    SUP_OP_ADD_NETWORK_DISK_REMOTE,      //221  B222.Operation操作-Remote:Add Network Disk远程:添加网络硬盘【新增】
    SUP_OP_DELETE_NETWORK_DISK_REMOTE,   //222  B223.Operation操作-Remote:Delete Network Disk远程:删除网络硬盘【新增】
    SUP_OP_CONFIG_NETWORK_DISK_REMOTE,   //223  B224.Operation操作-Remote:Config Network Disk远程:设置网络硬盘【新增】
    SUP_OP_MOUNT_DISK_REMOTE,            //224  B225.Operation操作-Remote:Mount Disk远程:加载硬盘【新增】
    SUP_OP_UNMOUNT_DISK_REMOTE,          //225  B226.Operation操作-Remote:Unmount Disk远程:卸载硬盘【新增】
    SUP_OP_DELETE_ABNORMAL_DISK_REMOTE,  //226  B227.Operation操作-Remote:Delete Abnormal Disk远程:删除异常硬盘【新增】
    SUP_OP_SYSTEM_TIME_SYNC_REMOTE,    //227  B228.Information提示-System Time Sync系统校时【新增】
    SUB_OP_VIDEO_EXPORT_REMOTE,			//
    SUB_OP_UPGRADE_OEM,					//
    SUB_OP_REMOTE_MAX = SUP_OP_SYSTEM_TIME_SYNC_REMOTE,
    //////REMOTE END/////////////

    /////OPERATION END////////////////

    //////EXCEPTION_START/////////////
    SUB_EXCEPT_MIN = 400,
    SUB_EXCEPT_VIDEO_LOSS = SUB_EXCEPT_MIN,  //400  D1.Exception异常-Video Loss视频丢失
    SUB_EXCEPT_DISK_FULL,                    //401  D2.Exception异常-HDD Full硬盘满
    SUB_EXCEPT_DISK_FAILURE,                 //402  D3.Exception异常-HDD Error硬盘错误
    SUB_EXCEPT_NETWORK_DISCONNECT,           //403  D4.Exception异常-Network Disconnect网络断开
    SUB_EXCEPT_RECORD_FAIL,                  //404  D5.Exception异常-Record Failed录像失败
    SUB_EXCEPT_SNAPSHOT_FAIL,                //405  D6.Exception异常-Capture Failed抓图失败 
    //	SUB_EXCEPT_CAMERA_DISCONNECT,
    SUB_EXCEPT_IPC_CONFLICT,                 //406  D7.Exception异常-NVR IP Conflict通道IP冲突 
    SUB_EXCEPT_DISK_UNINITIALIZED,			 //407  D8.Exception异常-DISK Uninitialized 硬盘未初始化
    SUB_EXCEPT_PLAYBACK_SNAPSHOT_FAIL,		 //408  D9.Exception异常-Playback Capture Failed抓图失败 

    SUB_EXCEPT_FAILOVER_MASTER_ANR_ERROR,    //409  ANR传回异常（工作机）
    SUB_EXCEPT_FAILOVER_SLAVE_WORK_ERROR,    //410  异常-热备异常（热备机开始热备时）//缺少配置文件？通道数为0？
    SUB_EXCEPT_DATE_ERROR,    				 //411  日期错误 错误年份1970
    SUB_EXCEPT_NVR_IP_CONFLICT,              //412  Exception异常-IP Address Conflict 本机IP冲突
    SUB_EXCEPT_DISK_OFFLINE,                 //413  Exception异常-硬盘掉线
    SUB_EXCEPT_DISK_HEAT,                    //414  Exception异常-硬盘高温
    SUB_EXCEPT_DISK_MICROTHERM,              //415  Exception异常-硬盘低温
    SUB_EXCEPT_DISK_CONNECTION_EXCEPTION,    //416  Exception异常-硬盘连接异常
    SUB_EXCEPT_DISK_DISK_STRIKE,             //417  Exception异常-硬盘震动
#if defined(_HI3536A_)
    SUB_EXCEPT_MAX = SUB_EXCEPT_DISK_DISK_STRIKE,
#else
    SUB_EXCEPT_MAX = SUB_EXCEPT_DISK_OFFLINE,
#endif
    //////EXCEPTION END///////////////

    //////INFORMATION START///////////
    SUB_INFO_MIN = 500,
    SUB_INFO_DISK_INFO = SUB_INFO_MIN, //500Information提示-HDD Information本地硬盘信息【原“硬盘信息”】
    SUP_INFO_NETWORK_DISK_INFORMATION,   //501  C2.Information提示-Network Disk Information网络硬盘信息【新增】
    SUB_INFO_DISK_SMART,                //502  C3.Information提示-S.M.A.R.T Information S.M.A.R.T硬盘信息
    SUB_INFO_RECORD_START,              //503  C4.Information提示-Start Record开始录像
    SUB_INFO_RECORD_STOP,               //504  C5.Information提示-Stop Record停止录像
    SUP_INFO_DELETE_EXPRIRED_VIDEO,      //505  C6.Information提示-Delete Expired Video删除过期录像【新增】
    SUP_INFO_SYSTEM_OPERATION_STATUS,    //506  C7.Information提示-System operation status系统运行状态【新增】
    SUP_INFO_START_ANR_RECORD,           //507  C8.Information提示-Start ANR Record 开始ANR录像【新增】
    SUP_INFO_STOP_ANR_RECORD,            //508  C9.Information提示-Stop ANR Record 停止ANR录像【新增】
    SUP_INFO_ADD_IP_CHANNEL_ANR_TIME,    //509  C10.Information提示-Add IP Channel ANR Time添加通道ANR时间 【新增】
    SUP_INFO_DELETE_IP_CHANNEL_ANR_TIME, //510  C11.Information提示-Delete IP Channel ANR Time删除通道ANR时间【新增】

    SUP_INFO_FAILOVER_MASTER_INFO,      //511  工作机信息（工作机）
    SUP_INFO_FAILOVER_MASTER_ANR_START, //512  ANR传回开始（工作机）
    SUP_INFO_FAILOVER_MASTER_ANR_END,   //513  ANR传回录像完成（工作机）
    SUP_INFO_FAILOVER_SLAVE_START_WORK, //514  信息-热备机开始备份工作机（热备机开始热备时)
    SUP_INFO_FAILOVER_SLAVE_STOP_WORK,  //515  信息-热备机停止备份工作机（热备机，工作机上线后）
    SUP_INFO_FAILOVER_SLAVE_IPC_START,  //516  信息-开始录像（热备机开始热备时）
    SUP_INFO_FAILOVER_SLAVE_IPC_STOP,   //517  信息-结束录像（热备机，工作机上线后）
    SUP_INFO_IP_CHANNEL_ANR_START,   	//518信息-IP通道ANR录像传回开始
    SUP_INFO_IP_CHANNEL_ANR_FINISH,   	//519  信息-IP通道ANR录像传回结束
    
    SUB_INFO_MAX = SUP_INFO_IP_CHANNEL_ANR_FINISH,
    //////INFORMATION END/////////////

	//////DEBUG START/////
	SUB_DEBUG_MIN  = 600,
    SUB_DEBUG_WAR = SUB_DEBUG_MIN,
    SUB_DEBUG_INF,
    SUB_DEBUG_DBG,
    SUB_DEBUG_ERR,
    SUB_DEBUG_MAX = SUB_DEBUG_ERR,
    //////DEBUG END//////
};

struct log_head_data {
	MF_S8 logVer[LOG_VER_SIZE_];
	//MF_PTS startTime;
	//MF_PTS endTime;
}__attribute__((packed));
struct log_head_info_n {
	struct log_head_data headData;
	MF_S8 free[FREE_HEAD_];
};
struct log_head_node
{
	MF_U8 port;
	TYPE_EN type;
	MF_U8 errNum;
	MF_S32 fd;
	MF_BOOL buninit;

	MF_U64 headOff;
	struct log_head_info_n headInfo;

	MF_U64 segOff;
	MF_U32 sector;
	//MF_U32 segNum;                      // max segs
	//struct list_head segList;           // log_seg_node
	//MUTEX_OBJECT segMutex;

	MF_U64 indOff;
	struct diskObj *pDev;

	struct list_head node;
};


struct log_seg_data {
	MF_S32  segNo;
	MF_U32  indNum;     // index count
	MF_U8   full;
    MF_U32  mType;
	MF_PTS  startTime;
	MF_PTS  endTime;
    MF_U64  startOff;
	MF_U64  iCurrOff;   // index off real-time
	MF_U64  dCurrOff;   // detail off real-time
	MF_U64  endOff;
	MF_BOOL isBadBlk;   // segemnt occurs some badblocks.
}__attribute__((packed));
struct log_seg_info_n {
	struct log_seg_data segData;
	MF_S8 free[FREE_SEG_];
};
struct log_seg_node
{
	struct log_head_node *head;
	struct log_seg_info_n  segInfo;

	struct list_head node;
};

#define LOG_INDEX_POS(member) ((unsigned long)(&((struct log_index_data *)0)->member))
struct log_index_data {
	MF_PTS pts;
    MF_U32 mType;

	MF_U32 dataSize;
    MF_U64 dataOff;
	MF_U32 detailSize;
	MF_U64 detailOff;
	MF_U32 crc;
}__attribute__((packed));
struct log_index_info_n
{
	struct log_index_data indexData;
	MF_S8 free[FREE_INDEX_];
};

#define LOG_DATA_POS(member) ((unsigned long)(&((struct log_data_data *)0)->member))
struct log_data_cell{
	MF_U32 mType;
    MF_U64 sType;
	DETEC_OBJ_EN enDtc;
	MF_PTS pts;
	//MF_S8 ptc[32];  // real time string
	MF_U32 paraType;
	MF_S32 remote;  ///< whether controlled by remote
	MF_S32 chanNo;
    MF_S8 ip[46];   ///< if local, filled it with 0.0.0.0 compat ipv6
    MF_S8 user[64]; ///< who did this
    MF_U8 streamType;
	MF_U8 isPlay;
};
struct log_data_data
{
	struct log_data_cell cell;
	MF_S8 reserved[FREE_DATA_];
    MF_U32 crc;
};
struct log_data_node
{
	MF_U32 logLevel;

	MF_S8 detail[LOG_DETAIL_MAX_];
	MF_U32 detailSize;
	struct log_data_data data;

	struct list_head node;
};


struct log_disk_info_n
{
	MF_S32 diskNum;
	MF_U64 diskStat;

	MUTEX_OBJECT	headMutex;
	struct list_head headList;
	//struct list_head nandList; // for nand

	MF_U32 segNum;                      // max segs
	MUTEX_OBJECT segMutex;
	struct list_head segList;
};

struct log_search_result_n
{
	int	total;
	struct list_head list;
};

/////////////////////////////////////////


/////////////// data & detail .solin//////////////////
typedef struct log_data_info
{
    MF_U32 mainType;
    MF_U64 subType;
	DETEC_OBJ_EN enDtc;
	long pts;
	//char ptc[32];                  // real time string
	MF_U32 parameter_type;
	MF_S32 remote;                   ///< whether controlled by remote
	MF_S32 chan_no;
    MF_S8 ip[46];                   ///< if local, filled it with 0.0.0.0 compat ipv6
    MF_S8 user[64];                 ///< who did this
    MF_U8 stream_type;              // main stream or sub stream
    MF_U8 isplay;                   // own video to play

	// cut to here for copy end.solin
	MF_U32	dataSize;
	MF_S8 detail[LOG_DETAIL_MAX_];
}log_data_info;                  // sizeof(log_data_info) --> 112


#define LOG_PKG_DATA_MAX   (LOG_DETAIL_PKG_SIZE) // 384
#define LOG_PKG_HEAD_RES (LOG_DETAIL_MAX_ - LOG_PKG_DATA_MAX - sizeof(struct log_pkg_head) - sizeof(unsigned int))
#define LOG_DETAIL_POS(member) ((unsigned long)(&((struct log_detail_pkg *)0)->member))

struct log_pkg_head{
	MF_U32 type;
	MF_U32 size;
};

typedef struct log_detail_pkg{
	struct log_pkg_head head;
	MF_S8 body[LOG_PKG_DATA_MAX];
	MF_U32 crc;
	//... //add others
	MF_S8 reserved[LOG_PKG_HEAD_RES];
}log_detail_pkg;




struct log_data
{
	struct log_data_info log_data_info;
	MF_U32 logLevel;   // enum op_level
};

struct log_data_list
{
	struct log_data log_data;
	struct list_head node;
};

///////////////////////////////



typedef struct search_criteria{
	MF_PTS start_time;
	MF_PTS end_time;
	MF_U32 mainType;
	MF_U64 subType[100];
	MF_S32 count;
    MF_S32 page;
    MF_U64 chnMask;
    MF_BOOL bSeachNoChn;
}search_criteria;

typedef struct log_cache{
	struct list_head node;
	MF_S32 count;
	MUTEX_OBJECT mutex;
	TASK_HANDLE pthread;
	MF_S32 run;
}log_cache;

typedef struct log_cb_para{
	MF_U8 port;
	MF_S32 entype;
	MF_S8 vendor[DEV_NAME_LEN];
	MF_S32 stat;
	MF_S32 rw;
}log_cb_para;

typedef struct log_search_mac_type{
	int mAll;
	int sAll;
	int mDbg;
}log_search_mac_type;

typedef void (* LOG_USER_CB)(EVENT_E, log_cb_para *);

void msfs_log_set_exit(MF_BOOL bexit);

MF_S32 msfs_log_write(struct log_data *log_data);

void msfs_log_search_cancel(MF_BOOL flag);

MF_S32 msfs_log_unpack_detail(struct log_detail_pkg *pkg, struct log_data *l_data);

MF_S32 msfs_log_pack_detail(struct log_data *l_data, MF_U32 type, void *body, MF_S32 size);

void msfs_log_notify_bar(PROGRESS_BAR_E enBar, MF_S32 percent);

MF_S32 msfs_log_deinit();

MF_S32 msfs_log_init(log_search_mac_type *lmac, LOG_USER_CB user_cb);

void msfs_log_show_disks();

void msfs_log_show_seg(MF_U8 port);

void msfs_log_search_all(struct search_criteria *cond,struct log_search_result_n *result, MF_BOOL bnotify);

void msfs_log_search_release(struct log_search_result_n *result);

void msfs_log_search_dup(struct log_data_info *dst, struct log_data_node *dnode);

void msfs_log_mem_usage();

int log_channo_is_disk(int mainType, long long subType);//0=no 1=yes

#endif


