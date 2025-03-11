/****
 * 
 * dup2_ex.c
 * 
 * Des:
 *   Define output API.
 * 
 * History:
 *  2015/12/01  - [eric@milesight.cn] Create file.
 * 
 * Copyright (C) 2011-2015, Milesight, Inc.
 * 
 * All rights reserved. No Part of this file may be reproduced, stored
 * in a retrieval system, or transmitted, in any form, or by any means,
 * electronic, mechanical, photocopying, recording, or otherwise,
 * without the prior consent of Milesight, Inc.
 * 
 */


#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __UCLIBC__
#include <bits/getopt.h>
#else
#include <getopt.h>
#endif
#include <ctype.h>
#include "msstd.h"


#define OUT2TELNET_0	"/tmp/dup2pts0"
#define OUT2TELNET_1	"/tmp/dup2pts1"
#define TELNET_0		"/dev/pts/0"
#define TELNET_1		"/dev/pts/1"

typedef struct std_out_s
{
	char dup_name[64];
	int stdin_fd;
	int	stdout_fd;
	int stderr_fd;
	int dup_stdin_fd;
	int dup_stdout_fd;
	int dup_stderr_fd;

	// for pthread control
	TASK_HANDLE dup_handle;
	int	dup_exit;

}std_out_t;

static std_out_t g_output = 
{
	.stdin_fd		= STDIN_FILENO,
	.stdout_fd		= STDOUT_FILENO,
	.stderr_fd		= STDERR_FILENO,
	.dup_stdin_fd	= -1,
	.dup_stdout_fd	= -1,
	.dup_stderr_fd	= -1,

	.dup_handle 	= -1,
	.dup_exit		= 1,
};

static int dup_stdout_stderr(char* destfile)
{
	int dupfd;
	
	if(destfile == NULL)
	{
		fprintf(stderr, "filename is NULL! \n");
		return -1;
	}
	printf("stdout and stderr dup2 %s", destfile);

	dupfd = open(destfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if(dupfd < 0) 
	{
		perror("open");
		return -1;
	}

	snprintf(g_output.dup_name, sizeof(g_output.dup_name), "%s", destfile);

	/* create a duplicate handle for standard output */
	g_output.dup_stdout_fd = dup(STDOUT_FILENO);
	g_output.dup_stderr_fd = dup(STDERR_FILENO);
	fflush(stdout);
	fflush(stderr);
		
	/*	redirect standard output to destfile
		by duplicating the file handle onto the
		file handle for standard output. 	*/
	dup2(dupfd, STDOUT_FILENO);
	dup2(dupfd, STDERR_FILENO);

	/* close the handle for destfile */
	close(dupfd);
	
	return 0;

}

static int restore_stdout_stderr()
{
	if(g_output.dup_stdout_fd >= 0 && g_output.dup_stderr_fd >= 0)
	{
		printf("restore stdout stderr");
		fflush(stdout);
		fflush(stderr);
		
		/* restore original standard output handle */
		dup2(g_output.dup_stdout_fd, STDOUT_FILENO);
		dup2(g_output.dup_stderr_fd, STDERR_FILENO);
		printf("restore stdout stderr success!");
		
		/* close duplicate handle for STDOUT */
		close(g_output.dup_stdout_fd);
		g_output.dup_stdout_fd = -1;
		close(g_output.dup_stderr_fd);
		g_output.dup_stderr_fd = -1;
	}

	if(g_output.dup_name[0] != '\0')
	{
		g_output.dup_name[0] = '\0';
	}

	return 0;
}

static void* dup_task(void* params)
{
	ms_task_set_name("DUP2");
	
	while(!g_output.dup_exit)
	{
		if(access(OUT2TELNET_0, F_OK) == 0) // telnet pts/0 exit
		{
			if(access(TELNET_0, F_OK) == 0)
			{
				if(strcmp(TELNET_0, g_output.dup_name) != 0)
				{
					// restore to standard
					restore_stdout_stderr();
				}
				
				// change to telnet-0
				if(g_output.dup_stderr_fd == -1 &&
					g_output.dup_stdout_fd == -1)
				{
					dup_stdout_stderr(TELNET_0);
				}
			}
			else
			{
				// restore to standard
				restore_stdout_stderr();
				ms_system("rm -f "OUT2TELNET_0);
			}
		}
		else if(access(OUT2TELNET_1, F_OK) == 0)
		{
			if(access(TELNET_1, F_OK) == 0)
			{
				if(strcmp(TELNET_1, g_output.dup_name) != 0)
				{
					// restore to standard
					restore_stdout_stderr();
				}
				
				// change to telnet-1
				if(g_output.dup_stderr_fd == -1 &&
					g_output.dup_stdout_fd == -1)
				{
					dup_stdout_stderr(TELNET_1);
				}
			}
			else
			{
				// restore to standard
				restore_stdout_stderr();
				ms_system("rm -f "OUT2TELNET_1);
			}
		}
		else
		{
			// restore to standard
			restore_stdout_stderr();
		}

		sleep(1);
	}
	restore_stdout_stderr();
	ms_task_quit("DUP");
}

int dup_task_init()
{
	g_output.dup_exit = 0;
	if(g_output.dup_handle == -1)
	{
		if(ms_task_create(&g_output.dup_handle, THREAD_STACK_512K_SIZE, dup_task, NULL) < 0)
		{
			printf("create dup task");
			return -1;
		}
	}

	return 0;
}

int dup_task_deinit()
{
	g_output.dup_exit = 1;
	g_output.dup_handle = -1;

	return 0;
}


