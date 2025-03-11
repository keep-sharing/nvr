#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <signal.h>
#if defined (_HI3798_)
#include <execinfo.h>
#endif
#include "msdefs.h"
#include "msjs.h"

static int g_msjs_stop = 0;

#if defined (_HI3798_)
void print_trace (void)
{
    void *array[100];
    size_t size;
    char **strings;
    size_t i;
    
    size = backtrace (array, 100);
    strings = backtrace_symbols (array, size);
    if (NULL == strings)
    {
        msprintf( "backtrace_synbols");
    }
    
    msprintf( "Obtained %zd stack frames.\n", size);
    for (i = 0; i < size; i++)
    {
        msprintf("backtrace : %s\n", strings[i]);
    }
    free (strings);
    strings = NULL;
}
#endif 
static void signal_backtrace(int signo)
{
#if defined (_HI3798_)
    print_trace();
    exit(0);
#endif
}

static void signal_handler(int signal)
{	
	g_msjs_stop = 1;
}

static void set_signal()
{
	signal(SIGINT,  signal_handler);	
	signal(SIGTERM, signal_handler);
	signal(SIGKILL, signal_handler);
	signal(SIGSEGV, signal_backtrace);
	signal(SIGPIPE, SIG_IGN);
}

int main(int argc, char **argv)  
{  
	int ret = 0;
	set_signal();
	
	ret = ms_js_init();
	if (ret < 0)
	{
		printf("ms_js_init failed !\n");
	}
	
	while(!g_msjs_stop)
	{
		usleep(100*1000);
	}
	
	printf("exit msjs\n");
	
	ms_js_uninit();
	
	return 0;  
}  

