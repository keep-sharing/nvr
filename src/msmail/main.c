#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include "msstd.h"
#include "msmail.h"

// unsigned int global_debug_mask = DEBUG_LEVEL_DEFAULT; 
// hook_print global_debug_hook = 0;

static sem_t global_sem;
static void signal_handler(int signal)
{
	printf("msmail recv signal %d\n", signal);
	switch(signal){
		case SIGINT:
		case SIGTERM:
		case SIGKILL:
		sem_post(&global_sem);
		break;
	}
	
}
static void set_core_signal()
{
	signal(SIGINT,  signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGKILL, signal_handler);
}

int main()
{
	set_core_signal();
	int res = sem_init(&global_sem, 0, 0);
	if(res)
		printf("Semaphore initialization failed\n");
	
	ms_mail_start();
	sem_wait(&global_sem);
	printf("recv semaphore ...\n");
	ms_mail_stop();
	return 0;
}

