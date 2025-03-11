#include "mssockreq.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "msstd.h"
#include "msg.h"
#include "linkedlists.h"
#include "mssocket.h"

//list node
struct sock_req_node
{
	int req;
	void (*handler)(int reqfrom, void* param, int size, UInt64 clientId);
	AST_LIST_ENTRY(sock_req_node) list;
};

static AST_LIST_HEAD_NOLOCK_STATIC(reqlist, sock_req_node);

void sock_request_register(struct sock_request_info* req, int count)
{
	int i = 0;
	for ( ; i < count; i++)
	{
		struct sock_req_node* node = ms_malloc(sizeof(struct sock_req_node));
		if (!node) continue;
		node->req     = req[i].req;
		node->handler = req[i].handler;

		AST_LIST_INSERT_TAIL(&reqlist, node, list);
	}
}

void sock_request_unregister(struct sock_request_info* req, int count)
{
	int i = 0;
	for ( ; i < count; i++)
	{
		struct sock_req_node* node = NULL;
		if (AST_LIST_EMPTY(&reqlist)) 
			break;
		AST_LIST_TRAVERSE(&reqlist, node, list)
		{
			if (!node) continue;
			if (node->req == req[i].req) break;
		}
		if (!node)
			continue;

		AST_LIST_REMOVE(&reqlist, node, list);
		ms_free(node);
	}

}

void dispatch_request(int req, int reqfrom, void* param, int size, UInt64 clientId)
{
	int noReqsFlag = 0;
	struct sock_req_node *node = NULL, *noReqNode = NULL;
	AST_LIST_TRAVERSE(&reqlist, node, list)
	{
		if(node->req == REQUEST_FLAG_DEAL_NO_REQS)
		{
			noReqNode = node;
		}
		
		if (node->req != req)
			continue;
		(node->handler)(reqfrom, param, size, clientId);
		noReqsFlag = 1;
	}
	if(!noReqsFlag && reqfrom != SOCKET_TYPE_CORE)
		(noReqNode->handler)(reqfrom, (void *)&req, size, clientId);
}
