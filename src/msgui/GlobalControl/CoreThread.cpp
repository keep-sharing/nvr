#include "CoreThread.h"

#include "mainwindow.h"

int isCoreRun = 0;

void callback_core_signal(int signal)
{
    fprintf(stderr, "\n\n----callback_core_signal, signal: %d----\n\n", signal);

}

CoreThread::CoreThread(QObject *parent) :
    QObject(parent)
{
    //8M
    //setStackSize((1 << 20) * 8);

    moveToThread(&m_thread);
    m_thread.start();
}

CoreThread &CoreThread::instance()
{
    static CoreThread self;
    return self;
}

void CoreThread::startCore()
{
    QMetaObject::invokeMethod(this, "onStartCore", Qt::QueuedConnection);
}

void CoreThread::stopCore()
{
    QMetaObject::invokeMethod(this, "onStopCore", Qt::QueuedConnection);
}

void CoreThread::prepareQuit()
{
    QMetaObject::invokeMethod(this, "onPrepareQuit", Qt::QueuedConnection);
}

void CoreThread::stopThread()
{
    m_thread.quit();
    m_thread.wait();
}

void CoreThread::onStartCore()
{
    //中心初始化
    isCoreRun = 1;

    emit coreStarted();
}

void CoreThread::onStopCore()
{
    onPrepareQuit();
}

void CoreThread::onPrepareQuit()
{
    if (!m_timerCheck) {
        m_timerCheck = new QTimer(this);
        connect(m_timerCheck, SIGNAL(timeout()), this, SLOT(onTimerCheck()));
    }
    m_timerCheck->start(1000);
}

void CoreThread::onTimerCheck()
{
    if (isCoreRun) {
        return;
    }

    emit coreStoped();
}
