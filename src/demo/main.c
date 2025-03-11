#include <linux/input.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

#if defined(_HI3798_)
#include <execinfo.h>
#endif

#include "demo.h"
#include "msfs_bkp.h"
#include "msfs_disk.h"
#include "msfs_pb.h"
#include "msfs_rec.h"
#include "poe.h"

#include "msdb.h"
#include "msdefs.h"
#include "msg.h"
#include "network_info.h"
#include "uv.h"

#include "osa/osa_mutex.h"
#include "osa/osa_que.h"
#include "osa/osa_thr.h"

#include "linkedlists.h"
#include "pthread.h"
#include "recortsp.h"

int g_msfs_debug = MF_ALL;

static int  g_exit = 0;
static void signal_handler(int signal) {
    g_exit = 1;
}

#if defined(_HI3798_)
void print_trace(void) {
    void  *array[100];
    size_t size;
    char **strings;
    size_t i;

    size    = backtrace(array, 100);
    strings = backtrace_symbols(array, size);
    if (NULL == strings) {
        printf("backtrace_synbols");
    }

    printf("Obtained %zd stack frames.\n", size);
    for (i = 0; i < size; i++) {
        printf("backtrace : %s\n", strings[i]);
    }
    free(strings);
    strings = NULL;
}
#endif

static void signal_backtrace(int signo) {
    printf("signal_backtrace sigNo : %d\n", signo);
#if defined(_HI3798_)
    print_trace();
#endif
    exit(0);
}

#if 0
static int pb_out_func(struct pb_hdl_t *hdl, struct reco_frame *frame)
{
    send_frame_to_vdec(frame);
    return MF_SUCCESS;
}
#endif

static void set_test_signal() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGKILL, signal_handler);

    signal(SIGSEGV, signal_backtrace);   // 11
    signal(SIGBUS, signal_backtrace);    // 7
    signal(SIGILL, signal_backtrace);    // 4
    signal(SIGSTKFLT, signal_backtrace); // 16
    signal(SIGFPE, signal_backtrace);    // 8
    signal(SIGABRT, signal_backtrace);   // 6
}

#if 0
void mouse_move(int fd, int rel_x, int rel_y)
{    
    struct input_event event;
    gettimeofday(&event.time, 0);    //x轴坐标的相对位移    
    event.type = EV_REL;
    event.value = rel_x;
    event.code = REL_X;
    write(fd, &event, sizeof(event));    //y轴坐标的相对位移
    event.type = EV_REL;
    event.value = rel_y;
    event.code = REL_Y;
    write(fd, &event, sizeof(event));    //同步   
  
    event.type = EV_SYN;    
    event.value = 0;    
    event.code = SYN_REPORT;    
    write(fd, &event, sizeof(event));
     
}

void mouse_click(int fd, int key, int click)
{    
    struct input_event event;
    struct input_event event_syn;
    gettimeofday(&event.time, 0);
    
    event_syn.type = EV_SYN;    
    event_syn.value = 0;    
    event_syn.code = SYN_REPORT;

    event.code = key;
    event.type = EV_KEY;
    while(click--)
    {
        event.value = 1;
        write(fd, &event, sizeof(event));    
        write(fd, &event_syn, sizeof(event_syn));
        usleep(50000);
        event.value = 0;
        write(fd, &event, sizeof(event));
        write(fd, &event_syn, sizeof(event_syn));
    }
     
}

void mouse_usage()
{
    printf("\tusage:\n");
    printf("\tmouse [dev] [L/R/M] xx xx\n");
    printf("\tExample:\n");
    printf("\t\t[left once click]: mouse 0 L 1\n");
    printf("\t\t[left double clicks]: mouse 0 L 2\n");
    printf("\t\t[right once click]: mouse 0 R 2\n");
    printf("\t\t[move(+10,+10)]: mouse 0 M 10 10\n");
}

int main(int argc, char **argv)
{
    set_test_signal();

    if (argc < 4)
    {
        mouse_usage();
        return -1;
    }
    char dev[32];
    sprintf(dev, "/dev/input/event%s", argv[1]);
    
    int fd = open(dev, O_RDWR);
    if (fd < 0)
    {
        printf("mouse disconnected !!!\n");
        return -1;
    }
    switch(*argv[2])
    {
        case 'L':  mouse_click(fd, BTN_LEFT, atoi(argv[3]));
            break;
        case 'R':  mouse_click(fd, BTN_RIGHT, atoi(argv[3]));
            break;
        case 'M':  mouse_move(fd, atoi(argv[3]), atoi(argv[4]));
            break;
        default:   mouse_usage();
            break;
    }
    close(fd);
	return 0;
}

#else
struct test_node {
    int   id;
    char  pname[MAX_LEN_64];
    void *ptr;

    AST_LIST_ENTRY(test_node) list;
};

AST_LIST_HEAD_NOLOCK_STATIC(test_list, test_node);
static MUTEX_OBJECT test_mutex;

int main(int argc, char **argv) {
    int               flag = 0, icnt = 0;
    struct test_node *c = NULL;
    struct test_node *p = NULL;

    set_test_signal();
    ms_mutex_init(&test_mutex);

    msprintf("test_list:%p icnt:%d start.", &test_list, icnt);
    if (AST_LIST_EMPTY(&test_list)) {
        msprintf("the test_list is null now.");
    }

    struct test_node *node1 = (struct test_node *)ms_malloc(sizeof(struct test_node));
    struct test_node *node2 = (struct test_node *)ms_malloc(sizeof(struct test_node));

    node1->id  = 1;
    node1->ptr = (void *)node2;
    snprintf(node1->pname, sizeof(node1->pname), "%s", "node1");
    msprintf("The test_list add node1:%p.", node1);

    node2->id  = 2;
    node2->ptr = NULL;
    snprintf(node2->pname, sizeof(node2->pname), "%s", "node1");
    msprintf("The test_list add node2:%p.", node2);

    ms_mutex_lock(&test_mutex);
    AST_LIST_INSERT_TAIL(&test_list, node1, list);
    ms_mutex_unlock(&test_mutex);

    ms_mutex_lock(&test_mutex);
    AST_LIST_INSERT_TAIL(&test_list, node2, list);
    ms_mutex_unlock(&test_mutex);

    ms_mutex_lock(&test_mutex);
#if 1
    AST_LIST_TRAVERSE_SAFE_BEGIN(&test_list, c, list) {
        icnt++;
        if (c)
            msprintf("c:%p ptr:%p flag:%d", c, c->ptr, flag);
        if (flag)
            break;
        if (c->ptr) {
            p = (struct test_node *)(c->ptr);
            msprintf("The test_list del node:%p.", p);
            AST_LIST_REMOVE(&test_list, p, list);
            ms_free(p);
            p    = NULL;
            flag = 1;
        }
        if (c) {
            msprintf("The test_list del node:%p.", c);
            AST_LIST_REMOVE(&test_list, c, list);
            ms_free(c);
            flag = 1;
            c    = NULL;
        }
        break;
    }
    AST_LIST_TRAVERSE_SAFE_END;
    if (AST_LIST_EMPTY(&test_list))
        msprintf("The test_list is null now.");
#else
    AST_LIST_TRAVERSE(&test_list, c, list) {
        icnt++;
        if (c)
            msprintf("c:%p ptr:%p flag:%d", c, c->ptr, flag);
        if (AST_LIST_EMPTY(&test_list))
            break;
        if (c->ptr) {
            p = (struct test_node *)(c->ptr);
            msprintf("The test_list del node:%p.", p);
            AST_LIST_REMOVE(&test_list, p, list);
            ms_free(p);
            p    = NULL;
            flag = 1;
        }
        if (c) {
            msprintf("The test_list del node:%p.", c);
            AST_LIST_REMOVE(&test_list, c, list);
            ms_free(c);
            flag = 1;
            c    = NULL;
        }
        break;
    }
    if (AST_LIST_EMPTY(&test_list))
        msprintf("the test_list is null now.");
#endif

    ms_mutex_unlock(&test_mutex);

    ms_mutex_uninit(&test_mutex);
    msprintf("test_list:%p icnt:%d end.", &test_list, icnt);

    return 0;
}
#endif
