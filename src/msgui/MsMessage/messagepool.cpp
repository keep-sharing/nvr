#include "messagepool.h"
#include "MyDebug.h"
#include "centralmessage.h"

const int Max_MessageSend = 100;
const int Max_MessageReceive = 100;

MessagePool *MessagePool::s_self = nullptr;

MessagePool::MessagePool(QObject *parent)
    : QObject(parent)
    , m_mutexUnusedSend(QMutex::Recursive)
    , m_mutexUsedSend(QMutex::Recursive)
    , m_mutexUnusedReceive(QMutex::Recursive)
    , m_mutexUsedReceive(QMutex::Recursive)
{
    s_self = this;

    for (int i = 0; i < 100; ++i) {
        MessageSend *send = new MessageSend;
        m_unusedSend.enqueue(send);

        MessageReceive *receive = new MessageReceive;
        m_unusedReceive.enqueue(receive);
    }
}

MessagePool::~MessagePool()
{
    qMsDebug() << "~MessagePool, begin";
    s_self = nullptr;

    qDeleteAll(m_unusedSend.begin(), m_unusedSend.end());
    m_unusedSend.clear();
    qDeleteAll(m_usedSend.begin(), m_usedSend.end());
    m_usedSend.clear();

    qDeleteAll(m_unusedReceive.begin(), m_unusedReceive.end());
    m_unusedReceive.clear();
    qDeleteAll(m_usedReceive.begin(), m_usedReceive.end());
    m_usedSend.clear();

    qMsDebug() << "~MessagePool, end";
}

MessagePool *MessagePool::instance()
{
    return s_self;
}

MessageSend *MessagePool::unusedMessageSend(int type)
{
    m_mutexUnusedSend.lock();
    MessageSend *message = nullptr;
    if (!m_unusedSend.isEmpty()) {
        message = m_unusedSend.dequeue();
    } else {
        m_sendCount++;
        qMsDebug() << QString("make MessageSend, type: %1, all count: %2, used count: %3")
                          .arg(type)
                          .arg(m_sendCount)
                          .arg(usedSendCount());
        showUsedMessageSendQueue();
        message = new MessageSend;
    }
    m_mutexUnusedSend.unlock();
    return message;
}

MessageSend *MessagePool::usedMessageSend()
{
    m_mutexUsedSend.lock();
    MessageSend *message = nullptr;
    if (!m_usedSend.isEmpty()) {
        message = m_usedSend.dequeue();
    }
    m_mutexUsedSend.unlock();
    return message;
}

bool MessagePool::hasUnprocessedMessageSend()
{
    QMutexLocker locker(&m_mutexUsedSend);
    return !m_usedSend.isEmpty();
}

void MessagePool::setMessageUsed(MessageSend *message, bool used)
{
    if (used) {
        m_mutexUsedSend.lock();
        m_usedSend.enqueue(message);
        m_mutexUsedSend.unlock();
    } else {
        m_mutexUnusedSend.lock();
        message->clear();
        m_unusedSend.enqueue(message);
        m_mutexUnusedSend.unlock();
    }
}

int MessagePool::usedSendCount()
{
    QMutexLocker locker(&m_mutexUsedSend);
    return m_usedSend.count();
}

int MessagePool::unusedSendCount()
{
    QMutexLocker locker(&m_mutexUnusedReceive);
    return m_unusedSend.count();
}

MessageReceive *MessagePool::unusedMessageReceive(int type)
{
    m_mutexUnusedReceive.lock();
    MessageReceive *message = nullptr;
    if (!m_unusedReceive.isEmpty()) {
        message = m_unusedReceive.dequeue();
    } else {
        m_receiveCount++;
        qMsDebug() << QString("make MessageReceive, type: %1, all count: %2, used count: %3")
                          .arg(type)
                          .arg(m_receiveCount)
                          .arg(usedReceiveCount());
        showUsedMessageReceiveQueue();
        message = new MessageReceive;
    }
    m_mutexUnusedReceive.unlock();
    return message;
}

MessageReceive *MessagePool::usedMessageReceive()
{
    m_mutexUsedReceive.lock();
    MessageReceive *message = nullptr;
    if (!m_usedReceive.isEmpty()) {
        message = m_usedReceive.dequeue();
    }
    m_mutexUsedReceive.unlock();
    return message;
}

bool MessagePool::hasUnprocessedMessageReceive()
{
    QMutexLocker locker(&m_mutexUsedReceive);
    return !m_usedReceive.isEmpty();
}

void MessagePool::setMessageUsed(MessageReceive *message, bool used)
{
    if (used) {
        m_mutexUsedReceive.lock();
        m_usedReceive.enqueue(message);
        m_mutexUsedReceive.unlock();
    } else {
        m_mutexUnusedReceive.lock();
        message->clear();
        m_unusedReceive.enqueue(message);
        m_mutexUnusedReceive.unlock();
    }
}

bool MessagePool::replaceMessageUsed(MessageReceive *message)
{
    QMutexLocker locker(&m_mutexUsedReceive);
    for (auto iter = m_usedReceive.begin(); iter != m_usedReceive.end(); ++iter) {
        MessageReceive *receive = *iter;
        if (receive->header.type == message->type()) {
            setMessageUsed(receive, false);
            *iter = message;
            return true;
        }
    }
    return false;
}

int MessagePool::usedReceiveCount()
{
    QMutexLocker locker(&m_mutexUsedReceive);
    return m_usedReceive.count();
}

int MessagePool::unusedReceiveCount()
{
    QMutexLocker locker(&m_mutexUnusedReceive);
    return m_unusedReceive.count();
}

void MessagePool::mutexProcessingReceiveLock()
{
    m_mutexProcessingReceive.lock();
}

void MessagePool::mutexProcessingReceiveUnlock()
{
    m_mutexProcessingReceive.unlock();
}

bool MessagePool::isProcessingReceive()
{
    return m_isProcessingReceive;
}

void MessagePool::setProcessingReceive(bool value)
{
    m_isProcessingReceive = value;
}

void MessagePool::mutexProcessingSendLock()
{
    m_mutexProcessingSend.lock();
}

void MessagePool::mutexProcessingSendUnlock()
{
    m_mutexProcessingSend.unlock();
}

bool MessagePool::isProcessingSend()
{
    return m_isProcessingSend;
}

void MessagePool::setProcessingSend(bool value)
{
    m_isProcessingSend = value;
}

void MessagePool::showUsedMessageReceiveQueue()
{
    m_mutexUsedReceive.lock();
    QString text;
    text.append(QString("showUsedMessageReceiveQueue, count: %1\n").arg(m_usedReceive.size()));
    for (int i = 0; i < m_usedReceive.size(); ++i) {
        MessageReceive *message = m_usedReceive.at(i);
        text.append(QString("--index: %1, type: %2\n").arg(i).arg(message->type()));
    }
    qMsDebug() << qPrintable(text);
    m_mutexUsedReceive.unlock();
}

void MessagePool::showUsedMessageSendQueue()
{
    m_mutexUsedSend.lock();
    QString text;
    text.append(QString("showUsedMessageSendQueue, count: %1\n").arg(m_usedSend.size()));
    for (int i = 0; i < m_usedSend.size(); ++i) {
        MessageSend *message = m_usedSend.at(i);
        text.append(QString("--index: %1, type: %2\n").arg(i).arg(message->type));
    }
    qMsDebug() << qPrintable(text);
    m_mutexUsedSend.unlock();
}
