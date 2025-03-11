#include <stdio.h>
#include <linux/input.h>
#include <fcntl.h>

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
