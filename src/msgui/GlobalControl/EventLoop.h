#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <QObject>
#include <QEventLoop>
#include "MyDebug.h"

#define gEventLoop EventLoop::instance()

#define gEventLoopExec() ({ \
    qMsDebug() << "gEventLoopExec"; \
    EventLoop::instance().exec(); \
})

#define gEventLoopExit(a) ({ \
    qMsDebug() << "gEventLoopExit" << a; \
    EventLoop::instance().exit(a); \
})

class EventLoop : public QObject
{
    Q_OBJECT
public:
    explicit EventLoop(QObject *parent = nullptr);

    static EventLoop &instance();

    int exec();
    void exit(int returnCode = 0);

    bool isRunning() const;

signals:

private:
    QEventLoop m_eventLoop;
};

#endif // EVENTLOOP_H
