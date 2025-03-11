#include "msstd.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int ms_openfifo(const char *path, int mode)
{
	int fd = -1;

	do
	{
		if (mode == O_WRONLY)
		{
			unlink(path);
			if(mkfifo(path, S_IFIFO | 0666))
			{
				perror("mkfifo");
				break;
			}
		}
		if ((fd = open(path, mode)) < 0)
		{
			perror("open");
			break;
		}
	}while (0);
	
	return fd;
}

int ms_closefifo(int fd, const char *path)
{
	close(fd);
	return unlink(path);
}

#if 0
int ms_readfifo(int fd, void *data)
{
	struct reco_frame head = {0}, *frame = (struct reco_frame *)data;
	if (read(fd, &head, sizeof(head)) <= 0)
	{
		perror("read");
		return -1;
	}

	void *tmp = frame->data;
	if (head.size > frame->size)
	{
		if ((frame->data = realloc(frame->data, head.size)) == NULL)
		{
			frame->data = tmp;
			return -1;
		}
	}
	tmp = frame->data;
	memcpy(frame, &head, sizeof(head));
	frame->data = tmp;

	if (read(fd, frame->data, head.size) <= 0) 
	{
		perror("read");
		return -1;
	}
	
	return 0;
}

int ms_writefifo(int fd, void *data)
{
	struct reco_frame *frame = (struct reco_frame *)data;
	if (write(fd, frame, sizeof(struct reco_frame)) <= 0) 
	{
		perror("write");
		return -1;
	}
	
	if (write(fd, frame->data, frame->size) <= 0) 
	{
		perror("write");
		return -1;
	}
	
	return 0;
}
#endif

