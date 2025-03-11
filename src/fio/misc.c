#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include "misc.h"

#define FB_CTRL_CMD_FBGP1_SET		_IOW('v', 1, int)
#define FB_CTRL_CMD_FBGP1_CLEAN		_IOW('v', 2, int)
#define FB_CTRL_CMD_INIT		    _IOW('v', 3, int)
#define FB_CTRL_CMD_BUZZER_ON		_IOW('v', 4, int)
#define FB_CTRL_CMD_BUZZER_OFF		_IOW('v', 5, int)
#define FB_CTRL_CMD_RS485_SEND		_IOW('v', 6, int)
#define FB_CTRL_CMD_RS485_RECV		_IOW('v', 7, int)
#define FB_CTRL_CMD_FBGP0_SET		_IOW('v', 8, int)
#define FB_CTRL_CMD_FBGP0_CLEAN		_IOW('v', 9, int)

struct misc_ctrl {
	int fd;
	int ring_type;
	int enable;
	int delay;
	int stop;
	pthread_t tid;
//	pthread_mutex_t lock;
};

static struct misc_ctrl g_misc_ctrl;

static int fb_ctrl_open(void)
{
	int fd;
	int flag = 0;
	fd = open("/dev/fb_ctrl", O_RDWR);
	if(fd > 0){
		ioctl(fd, FB_CTRL_CMD_INIT, &flag);
	}
	return fd;
}

static int fb_ctrl_run(int fd)
{
	int flag = 0;
	ioctl(fd, FB_CTRL_CMD_FBGP1_SET, &flag);
	return 0;
}

int fb_ctrl_poweroff(int fd)
{
	int flag = 0;
	ioctl(fd, FB_CTRL_CMD_FBGP1_CLEAN, &flag);
	sleep(1);
	return 0;
}

int fb_ctrl_poweroff_end(int fd)
{
	int flag = 0;
	ioctl(fd, FB_CTRL_CMD_FBGP0_SET, &flag);
	return 0;	
}

void set_rs485_send(int fd)
{
	int flag = 0;
	ioctl(fd, FB_CTRL_CMD_RS485_SEND, &flag);
}

void set_rs485_recv(int fd)
{
	int flag = 0;
	ioctl(fd, FB_CTRL_CMD_RS485_RECV, &flag);
}


static void buzzer_on(int fd)
{
	int flag = 0;
	ioctl(fd, FB_CTRL_CMD_BUZZER_ON, &flag);
}

static void buzzer_off(int fd)
{
	int flag = 0;
	ioctl(fd, FB_CTRL_CMD_BUZZER_OFF, &flag);
}

static void misc_close(int fd)
{
	if (fd > 0) {
		close(fd);
	}
}

static int misc_write(int fd, int usec)
{
	if (fd < 0)
		return -1;
	buzzer_on(fd);
	usleep(usec);
	buzzer_off(fd);
	return 0;
}

/**
 * @todo
 */
static void *misc_task(void *prm)
{
	struct misc_ctrl *mctrl = (struct misc_ctrl *)prm;

	prctl(PR_SET_NAME, (unsigned long)"misc_task");

	while (!mctrl->stop) {
//		pthread_mutex_lock(&mctrl->lock);
		if (mctrl->enable) {
			switch (mctrl->ring_type) {
			case RING_KEY:
//				printf("key ring 222222222222, fd: %d\n", mctrl->fd);
				misc_write(mctrl->fd, 50000);
				mctrl->enable = 0;
				break;
			case RING_ALARM:
				if (mctrl->delay > 0) {
					mctrl->delay--;
				} else{
					mctrl->enable = 0;
				}
				misc_write(mctrl->fd, 200000);
				usleep(130000);
				misc_write(mctrl->fd, 200000);
				usleep(130000);
				misc_write(mctrl->fd, 200000);
				usleep(100000);
				misc_write(mctrl->fd, 100000);
				usleep(100000);
				misc_write(mctrl->fd, 100000);
				usleep(1000000);
				break;
			case RING_NETWORK_DISCONN: {
				if (mctrl->delay > 0) {
					mctrl->delay--;
				} else{
					mctrl->enable = 0;
				}
				misc_write(mctrl->fd, 160000);
				usleep(50000);
				misc_write(mctrl->fd, 120000);
				usleep(50000);
				misc_write(mctrl->fd, 100000);
#if 0
				misc_write(mctrl->fd, 80000);
				usleep(50000);
				misc_write(mctrl->fd, 70000);
#endif
				usleep(1000000);
			};
			break;
			case RING_VLOSS: {
				if (mctrl->delay > 0) {
					mctrl->delay--;
				} else{
					mctrl->enable = 0;
				}
				misc_write(mctrl->fd, 160000);
				usleep(80000);
				misc_write(mctrl->fd, 160000);
				usleep(80000);
				misc_write(mctrl->fd, 160000);
				usleep(80000);
				misc_write(mctrl->fd, 160000);
				usleep(800000);
			};
			break;
			case RING_DISK_FULL: {
				if (mctrl->delay > 0) {
					mctrl->delay--;
				} else{
					mctrl->enable = 0;
				}
				misc_write(mctrl->fd, 100000);
				usleep(100000);
				misc_write(mctrl->fd, 120000);
				usleep(800000);
			};
			break;
			case RING_RECORD_FAIL: {
				if (mctrl->delay > 0) {
					mctrl->delay--;
				} else{
					mctrl->enable = 0;
				}
				misc_write(mctrl->fd, 60000);
				usleep(80000);
				misc_write(mctrl->fd, 80000);
				usleep(80000);
				misc_write(mctrl->fd, 60000);
				usleep(80000);
				misc_write(mctrl->fd, 80000);
				usleep(80000);
				misc_write(mctrl->fd, 100000);
				usleep(800000);
			};
			break;
			case RING_MOTION_DETECT: {
				if (mctrl->delay > 0) {
					mctrl->delay--;
				} else{
					mctrl->enable = 0;
				}
				misc_write(mctrl->fd, 50000);
				usleep(50000);
				misc_write(mctrl->fd, 100000);
				usleep(50000);
				misc_write(mctrl->fd, 200000);
				usleep(800000);
			}
			break;
			case RING_DISK_FAIL: {
				if (mctrl->delay > 0) {
					mctrl->delay--;
				} else{
					mctrl->enable = 0;
				}
				misc_write(mctrl->fd, 300000);
				usleep(70000);
				misc_write(mctrl->fd, 170000);
				usleep(700000);
			};break;
            case RING_CAMERAIO: {
				if (mctrl->delay > 0) {
					mctrl->delay--;
				} else{
					mctrl->enable = 0;
				}
				misc_write(mctrl->fd, 100000);
				usleep(100000);
				misc_write(mctrl->fd, 120000);
				usleep(800000);
            }
            break;
            case RING_VOLUMEDETECT: {
				if (mctrl->delay > 0) {
					mctrl->delay--;
				} else{
					mctrl->enable = 0;
				}
				misc_write(mctrl->fd, 100000);
				usleep(100000);
				misc_write(mctrl->fd, 120000);
				usleep(800000);
            }
            break;
			default:
				break;
			}
//			mctrl->enable = 0;
		}
//		pthread_mutex_unlock(&mctrl->lock);
		usleep(3000);
	}
	return NULL;
}

/**
 * @todo buzzer control
 */
int misc_ctrl(int type, int on, int delay)
{
	struct misc_ctrl *mctrl = &g_misc_ctrl;

//	pthread_mutex_lock(&mctrl->lock);
	mctrl->ring_type = type;
	mctrl->enable = on;
	mctrl->delay = delay;
//	pthread_mutex_unlock(&mctrl->lock);
	return 0;
}

int misc_init(int *dupfd) ///< @todo dup fd
{
	struct misc_ctrl *mctrl = &g_misc_ctrl;

	memset(mctrl, 0, sizeof(struct misc_ctrl));
	if ((mctrl->fd = fb_ctrl_open()) < 0)
		return -1;
	*dupfd = dup(mctrl->fd);
	printf("fd: %d,dupfd: %d\n",mctrl->fd, *dupfd);
	fb_ctrl_run(mctrl->fd);
	printf("fb_ctrl_run done\n");
	set_rs485_recv(mctrl->fd);
	printf("set_rs485_recv done\n");
//	pthread_mutex_init(&mctrl->lock, NULL);
	if (pthread_create(&mctrl->tid, NULL, misc_task, mctrl)) {
		perror("pthread create");
		misc_close(mctrl->fd);
		return -1;
	}
	printf("misc init done\n");
	return 0;
}

int misc_deinit(void)
{
	struct misc_ctrl *mctrl = &g_misc_ctrl;
	mctrl->stop = 1;
	pthread_join(mctrl->tid, NULL);
	misc_close(mctrl->fd);
//	pthread_mutex_destroy(&mctrl->lock);
	return 0;
}

#if 0
int main(void)
{
	int i ;

	misc_init();
	misc_ctrl(RING_ALARM, 1);
	sleep(5);
	printf("sleep 5s done\n");
	misc_ctrl(RING_ALARM, 0);
	printf("turn alarm off\n");
	sleep(5);
	misc_deinit();
	return 0;
}
#endif

#if 0
int main(int argc, char *argv[])
{
	int fd;
	int cmd;
	int res;
	int val;

	if(argc != 2){
		printf("front 1  ----set state_run\n");
		printf("front 2  ----set powerof\n");
		printf("front 3  ----get GP0\n");
		return -1;
	}

	fd = fb_ctrl_open();
	if(fd < 0){
		printf("open FB failed\n");
		return -1;
	}

	cmd = atoi(argv[1]);
	switch(cmd){
	case 1:
		set_state_run(fd);
		break;

	case 2:
		set_state_run(fd);
		sleep(1);
		set_poweroff(fd);
		break;

	case 3:
		val = 0;
		set_state_run(fd);
		res = read(fd, &val, 4);
		printf("read %x\n", val);
		break;

	case 4:
		buzzer_on(fd);
		break;

	case 5:
		buzzer_off(fd);
		break;
	}
	
	close(fd);

	return 0;
}
#endif

