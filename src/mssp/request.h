#ifndef _MSSP_REQ_H_
#define _MSSP_REQ_H_

#include "list.h"
#include "mssp_msg.h"

struct request
{
	int req_from;
	int (*notify)(struct mssp_header* phdr, void *arg, void *data);
	struct list_head list;
};

int req_register(const struct request *req, int len);

int req_unregister(const struct request *req, int len);

int req_dispatch(struct mssp_header *phdr, void *args, void *data);

typedef int (*hook_sendmsg)(struct mssp_header*, int, void *, void *);

int req_sendmsg_init(hook_sendmsg);

int req_sendmsg(struct mssp_header *phdr, int ret, void *buff, void *args);

#endif
