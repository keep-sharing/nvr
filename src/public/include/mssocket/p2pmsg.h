#ifndef _P2P_MSG_H_
#define _P2P_MSG_H_

enum version
{
	P2PVER_V1 = 1,
};

enum msg_from 
{
	FROM_SERV = 0,
	FROM_GUI,
	FROM_WEB,
	FROM_SDK,
	FROM_NVR
};

struct msg_hdr
{
	size_t from;
	size_t type;
	size_t size;
	size_t errcode;
	size_t version;
	size_t crc;
	char reserved[16];
};

struct req_uid 
{
	char mac[24];
	char model[32];
	char sn[64];
	char company[128];
	char email[128];
	char dealer[128];
	char ipc[64];
};

struct resp_msg 
{
	char uid[128];
};

enum req_type
{
	REQ_UID = 0,
};

enum resp_type 
{
	RESP_UID = 0
};

#endif // _P2P_MSG_H_
