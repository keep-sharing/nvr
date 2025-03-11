/****
 * 
 * message.c
 * 
 * Des:
 *   Define message API.
 * 
 * History:
 *  2015/10/25  - [eric@milesight.cn] Create file.
 * 
 * Copyright (C) 2011-2015, Milesight, Inc.
 * 
 * All rights reserved. No Part of this file may be reproduced, stored
 * in a retrieval system, or transmitted, in any form, or by any means,
 * electronic, mechanical, photocopying, recording, or otherwise,
 * without the prior consent of Milesight, Inc.
 * 
 */

#include <sys/types.h>
#include <unistd.h>
#include <sys/msg.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msstd.h"

// Use for perror return.   
#define perror_return(errstr,errno) \
	do{ \
		perror(errstr); \
		return (errno); \
	} while(0)

int ms_msg_init( int msgKey )
{
	int qid;
	
	qid = msgget((key_t)msgKey,0);
	if(qid < 0)
	{
		qid = msgget((key_t)msgKey, IPC_CREAT | 0666);
		if(qid == -1)
		{
			perror_return("msgget", qid);
		}
//		msprintf("Creat queue id:%d", qid);
	}
	msprintf("client queue id:%d", qid);

	return qid;
}

int ms_msg_deinit(int msgKey)
{
	int qid;

	qid = msgget((key_t)msgKey, 0);
	if(qid < 0)
	{
		return 0;
	}
	else
	{
		if(msgctl(qid, IPC_RMID, NULL) < 0)
		{
			perror_return("msgctl", -1);
		}
	}
	
	return 0;
}

int ms_msg_rcv_nowait(int qid, void *msgbuf, int msgsize, int msgtype)
{
	return msgrcv(qid, msgbuf, msgsize - sizeof(long), msgtype, IPC_NOWAIT);
}

int ms_msg_rcv_wait(int qid, void *msgbuf, int msgsize, int msgtype)
{
	return msgrcv(qid, msgbuf, msgsize - sizeof(long), msgtype, 0);
}

int ms_msg_rcv_wait_timeout(int qid, void *msgbuf, int msgsize, int msgtype, int microsecond)
{
	int count = 0;
	int ret = -1;
	if(microsecond == 0)
	{
		return msgrcv(qid, msgbuf, msgsize - sizeof(long), msgtype, 0);
	}
	do
	{
		ret = ms_msg_rcv_nowait(qid, msgbuf, msgsize, msgtype);
		if(ret != -1)
			return ret;
		usleep(10000);
	}while(count++ < microsecond/10);
	return ret;
}

int ms_msg_snd_nowait(int qid, void *msgbuf, int msgsize)
{
	return msgsnd(qid, msgbuf, msgsize - sizeof(long), IPC_NOWAIT);
}

int ms_msg_snd_wait(int qid, void *msgbuf, int msgsize)
{
	return msgsnd(qid, msgbuf, msgsize - sizeof(long),0);
}

#if 0
int ms_msg_rcv_wait_timeout(int qid, void *msgbuf, int msgsize, int ms)
{
	int count = 0;
	int ret = -1;
	if(ms == 0)
	{
		return ms_msg_snd_nowait(qid, msgbuf, msgsize);
	}
	do
	{
		count++;
		ret = ms_msg_snd_nowait(qid, msgbuf, msgsize);
		if(ret != -1)
			return ret;
		usleep(10*1000);
	}while(count < ms/10);
	return ret;
}
#endif

