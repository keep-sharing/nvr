/* 
 * Filename:        tsar_socket
 * Created at:      2022.05.10
 * Description:     tsar socket
 *             
 * Author:          hrz
 * Copyright (C)    milesight
 * *******************************************************/


#ifndef __TSAR_SOCKET_H__
#define __TSAR_SOCKET_H__

#include "list.h"
#include "msstd.h"


#define SOCK_TSAR_PATH     "/var/run/tsarsocket"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
    TSAR_SOCK_IDEL      = 0,
    TSAR_SOCK_MSOP      = 1,//server
    TSAR_SOCK_MSCORE    = 2,
    TSAR_SOCK_BOA       = 3,
    TSAR_SOCK_MAX       = 4,
} TSAR_SOCK_TYPE;

enum {
    TSAR_CTRL_NORMAL_CMD    = 1000,
    //以上保留
    
    
        
    //以下保留
    TSAR_CTRL_SPECIAL_CMD   = 9000,
    TSAR_CTRL_CONNECT       = 9001,
    TSAR_CTRL_DISCONNECT    = 9002,
    TSAR_CTRL_DEBUG_MSG     = 9003,
};

/*
    |--HEADER--|--DATA--|
        header + data
*/
typedef struct TsarSocketHeader {
    int condition;
    int type;
    int from;
    TSAR_DEBUG_LEVEL level;
    time_t timestamp;
    char fileName[64];
    char funcName[64];
    int fileLine;
    int size;
} TSAR_SOCKET_HEADER;

#define TSAR_CONDITION_CODE		(0x20220510)
#define TSAR_DEBUG_MAX_LEN      1024
#define MS_COMMON_SOCKET_MAX    (10)
#define MS_COMMON_LIST_MAX      (100*1024)


typedef void (*COMM_CALLBACK_RECV)(void* arg, TSAR_SOCKET_HEADER *header, void * data);
void *creat_tsar_socket(TSAR_SOCK_TYPE type, TSAR_DEBUG_LEVEL level, COMM_CALLBACK_RECV callback, void *arg);
void destroy_tsar_socket(void *handle);
int  tsar_socket_server_send_msg(void *handle, TSAR_SOCK_TYPE sendto, void *data, int len);
int  tsar_socket_client_send_msg(void *handle, void *data, int len);

//TODO:手动采集用注册接口的方式
int init_tsar_debuger(TSAR_DEBUG_LEVEL level, TSAR_SOCK_TYPE type);
void uninit_tsar_debuger();
int tsar_debuger_support(TSAR_DEBUG_LEVEL level);
int tsar_debuger_output(TSAR_DEBUG_LEVEL level, const char *file, const char *func, int line, char *msg);
void tsar_log(int level, const char *file, const char *func, int line, const char* format, ...);


#define ms_log(level, format, ...) tsar_log(level, __FILE__, __func__, __LINE__, format, ## __VA_ARGS__)

//list
typedef struct CommonListNode {
    void *data;
    int size;
    struct list_head node;
} COMM_LIST_NODE;

void *creat_common_list(int maxSize);
void destroy_common_list(void *list);
int insert_common_list_node(void *handle, COMM_LIST_NODE *node);
COMM_LIST_NODE *popfront_common_list_node(void *handle);
int get_common_list_node_count(void *handle);


#ifdef __cplusplus
}
#endif

#endif
