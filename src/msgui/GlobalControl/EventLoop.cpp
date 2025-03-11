#include "EventLoop.h"
#include "qglobal.h"

EventLoop::EventLoop(QObject *parent)
    : QObject(parent)
{

}

EventLoop &EventLoop::instance()
{
    static EventLoop self;
    return self;
}

int EventLoop::exec()
{
    // if (m_eventLoop.isRunning()) {
    //     qMsWarning() << "eventloop is running";
    // }
    return 1;
}

void EventLoop::exit(int returnCode)
{
    Q_UNUSED(returnCode)
    // if (!m_eventLoop.isRunning()) {
    //     qMsWarning() << "eventloop is not running";
    //     return;
    // }
    // m_eventLoop.exit(returnCode);
}

bool EventLoop::isRunning() const
{
    return 0;
}
