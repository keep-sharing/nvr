#include "list.h"
#include "request.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msstd.h"
#include "mssp.h"

static hook_sendmsg g_sendmsg;
static LIST_HEAD(g_req_list);

int req_register(const struct request *req, int len)
{
	if (!req || !len)
		return -1;
	struct request *preqs = ms_calloc(len, sizeof(struct request));
	if(!preqs)
	{
		perror("ms_calloc");
		return -1;
	}
	memcpy(preqs, req, len * sizeof(struct request));
	int i = 0;

	for (; i < len; i++) 
	{
		list_add_tail(&preqs[i].list, &g_req_list);
	}
	return 0;
}

int req_unregister(const struct request *req, int len)
{
	if (!req || !len)
		return -1;
	int i = 0;

	struct request *pos, *tmp;
	for (i = 0; i < len; i++) 
	{
		list_for_each_entry_safe(pos, tmp, &g_req_list, list)
		{
			if (pos->req_from == req->req_from) 
			{
				list_del(&pos->list);
				ms_free(pos);
			}
		}
	}
	return 0;
}

int req_dispatch(struct mssp_header *phdr, void *args, void *data)
{
	struct request *pos = NULL;
	list_for_each_entry(pos, &g_req_list, list)
	{
		if (pos->req_from == phdr->req)
		{
			//search_device *dev = (search_device *)args;
			//msprintf("===[david debug] req_dispatch:%p req_from:%d device_name:%s===", (void*)args, pos->req_from, dev->device_name);
			pos->notify(phdr, args, data);
		}
	}
	return 0;
}

int req_sendmsg_init(hook_sendmsg hook)
{
	g_sendmsg = hook;
	return 0;
}

int req_sendmsg(struct mssp_header *phdr, int ret, void *buff, void *args)
{
	if (g_sendmsg)
	{
		return g_sendmsg(phdr, ret, buff, args);
	}
	return 0;
}
