#include "DiskSmartTest.h"
#include "centralmessage.h"
#include <QtDebug>

DiskSmartTest::DiskSmartTest(QObject *parent)
    : MsObject(parent)
{
    m_progressTimer = new QTimer(this);
    connect(m_progressTimer, SIGNAL(timeout()), this, SLOT(onProcessTimer()));
    m_progressTimer->setInterval(5000);
}

DiskSmartTest::~DiskSmartTest()
{

}

DiskSmartTest &DiskSmartTest::instance()
{
    static DiskSmartTest self;
    return self;
}

void DiskSmartTest::setTestPort(int port)
{
    m_port = port;
}

void DiskSmartTest::clearTestPort()
{
    m_port = -1;
    m_progress = 0;
}

int DiskSmartTest::testPort()
{
    return m_port;
}

int DiskSmartTest::testProgress()
{
    return m_progress;
}

void DiskSmartTest::startGetProcess()
{
    m_progressTimer->start();
    onProcessTimer();
}

bool DiskSmartTest::isTesting()
{
    return m_progressTimer->isActive();
}

void DiskSmartTest::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_SMART_PROCESS:
        ON_RESPONSE_FLAG_GET_SMART_PROCESS(message);
        break;
    }
}

void DiskSmartTest::ON_RESPONSE_FLAG_GET_SMART_PROCESS(MessageReceive *message)
{
    if (!message->data) {
        qWarning() << "DiskSmartTest::ON_RESPONSE_FLAG_GET_SMART_PROCESS, data is null.";
        return;
    }
    int value = (*(int *)(message->data));
    qDebug() << QString("PageSmartStatus::ON_RESPONSE_FLAG_GET_SMART_PROCESS, value: %1").arg(value);
    m_progress = value;
    if (value >= 100) {
        m_progressTimer->stop();
    }
    emit progressChanged(m_port, m_progress);
}

void DiskSmartTest::onProcessTimer()
{
    sendMessage(REQUEST_FLAG_GET_SMART_PROCESS, (void *)&m_port, sizeof(int));
}
