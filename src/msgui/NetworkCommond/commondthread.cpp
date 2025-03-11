#include "commondthread.h"
#include <QCursor>
#include <QPoint>
#include <QtDebug>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>

CommondThread::CommondThread(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<KeyEventType>("KeyEventType");
    qRegisterMetaType<MouseEventType>("MouseEventType");
    connect(this, SIGNAL(sig_sendMouseEvent(MouseEventType)), this, SLOT(onSendMouseEvent(MouseEventType)));
    connect(this, SIGNAL(sig_sendMouseMoveEvent(int, int)), this, SLOT(onSendMouseMoveEvent(int, int)));

    moveToThread(&m_thread);
    m_thread.setObjectName("Qt-CommondThread");
    m_thread.start();

    //
    int fd = 0;
    char name[64];
    char buf[256] = { 0 };
    for (int i = 0; i < 6; i++) {
        sprintf(name, "/dev/input/event%d", i);
        if ((fd = open(name, O_RDONLY, 0)) >= 0) {
            ioctl(fd, EVIOCGNAME(sizeof(buf)), buf);
            qDebug() << QString("event%1, name:%2").arg(i).arg(buf);
            close(fd);
        }
    }
}

void CommondThread::stopThread()
{
    m_thread.quit();
    m_thread.wait();
}

void CommondThread::sendKeyEvent(int key, KeyEventType type)
{
    QMetaObject::invokeMethod(this, "onKeyEvent", Qt::QueuedConnection, Q_ARG(int, key), Q_ARG(KeyEventType, type));
}

void CommondThread::sendMouseEvent(MouseEventType type)
{
    emit sig_sendMouseEvent(type);
}

void CommondThread::sendMouseMoveEvent(int x, int y)
{
    emit sig_sendMouseMoveEvent(x, y);
}

void CommondThread::simulate_key(int fd, int kval, int value)
{
    struct input_event event;
    gettimeofday(&event.time, 0);

    //按下kval键
    event.type = EV_KEY;
    event.value = value;
    event.code = kval;
    if (write(fd, &event, sizeof(event)) < 0) {
        qWarning() << "CommondThread::simulate_key error1.";
    }

    //同步，也就是把它报告给系统
    event.type = EV_SYN;
    event.value = 0;
    event.code = SYN_REPORT;
    write(fd, &event, sizeof(event));
    if (write(fd, &event, sizeof(event)) < 0) {
        qWarning() << "CommondThread::simulate_key error2.";
    }
}

void CommondThread::mouse_move(int fd, int rel_x, int rel_y)
{
    struct input_event event;
    gettimeofday(&event.time, 0);
    //x轴坐标的相对位移
    event.type = EV_REL;
    event.value = rel_x;
    event.code = REL_X;
    write(fd, &event, sizeof(event));
    //y轴坐标的相对位移
    event.type = EV_REL;
    event.value = rel_y;
    event.code = REL_Y;
    write(fd, &event, sizeof(event));
    //同步
    event.type = EV_SYN;
    event.value = 0;
    event.code = SYN_REPORT;
    write(fd, &event, sizeof(event));
}

void CommondThread::mouse_click(int fd, int key)
{
    struct input_event event_syn;
    event_syn.type = EV_SYN;
    event_syn.value = 0;
    event_syn.code = SYN_REPORT;

    struct input_event event;
    gettimeofday(&event.time, 0);
    event.code = key;
    event.type = EV_KEY;
    //按下
    event.value = 1;
    write(fd, &event, sizeof(event));
    write(fd, &event_syn, sizeof(event_syn));
    //
    usleep(10000);
    //弹起
    event.value = 0;
    write(fd, &event, sizeof(event));
    write(fd, &event_syn, sizeof(event_syn));
}

void CommondThread::onKeyEvent(int key, KeyEventType type)
{
    qDebug() << "CommondThread::onKeyEvent, key:" << key << ", type:" << type;

    int fd_key = -1;
    int fd = 0;
    char name[64];
    char buf[256] = { 0 };
    for (int i = 0; i < 6; i++) {
        sprintf(name, "/dev/input/event%d", i);
        if ((fd = open(name, O_RDONLY, 0)) >= 0) {
            ioctl(fd, EVIOCGNAME(sizeof(buf)), buf);
            if (!strcmp(buf, "dvr_keypad")) {
                fd_key = open(name, O_RDWR);
            }
            close(fd);
        }
    }
    if (fd_key <= 0) {
        return;
    }

    switch (type) {
    case KeyPress:
        simulate_key(fd_key, key, 1);
        break;
    case KeyRelease:
        simulate_key(fd_key, key, 0);
        break;
    case KeyClick:
        simulate_key(fd_key, key, 1);
        simulate_key(fd_key, key, 0);
        break;
    }
    close(fd_key);
}

void CommondThread::onSendMouseEvent(MouseEventType type)
{
    qDebug() << "====CommondThread::onSendMouseEvent====";
    qDebug() << "----type:" << type;

    int eventIndex = 0;

    int temp_fd = 0;
    int fd = 0;
    char name[64];
    char buf[256] = { 0 };
    for (int i = 0; i < 6; i++) {
        sprintf(name, "/dev/input/event%d", i);
        if ((temp_fd = open(name, O_RDONLY, 0)) >= 0) {
            ioctl(temp_fd, EVIOCGNAME(sizeof(buf)), buf);
            if (QString(buf).contains("mouse", Qt::CaseInsensitive)) {
                if (fd > 0) {
                    close(fd);
                }
                fd = open(name, O_RDWR);
                eventIndex = i;
            }
            close(temp_fd);
        }
    }
    qDebug() << "----device:" << QString("event%1").arg(eventIndex);

    switch (type) {
    case MouseLeftButtonClick:
        simulate_key(fd, BTN_LEFT, 1);
        simulate_key(fd, BTN_LEFT, 0);
        break;
    case MouseRightButtonClick:
        //simulate_key(fd, BTN_RIGHT, 1);
        //simulate_key(fd, BTN_RIGHT, 0);
        mouse_click(fd, BTN_RIGHT);
        break;
    default:
        break;
    }
    close(fd);
}

void CommondThread::onSendMouseMoveEvent(int x, int y)
{
    qDebug() << "====CommondThread::onSendMouseMoveEvent====";
    qDebug() << "----x:" << x;
    qDebug() << "----y:" << y;

    int eventIndex = 0;

    int temp_fd = 0;
    int fd = 0;
    char name[64];
    char buf[256] = { 0 };
    for (int i = 0; i < 6; i++) {
        sprintf(name, "/dev/input/event%d", i);
        if ((temp_fd = open(name, O_RDONLY, 0)) >= 0) {
            ioctl(temp_fd, EVIOCGNAME(sizeof(buf)), buf);
            if (QString(buf).contains("mouse", Qt::CaseInsensitive)) {
                if (fd > 0) {
                    close(fd);
                }
                fd = open(name, O_RDWR);
                eventIndex = i;
            }
            close(temp_fd);
        }
    }
    qDebug() << "----device:" << QString("event%1").arg(eventIndex);

    mouse_move(fd, x, y);

    close(fd);
}
