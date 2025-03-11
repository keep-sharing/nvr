#ifndef MESSAGEPOOL_H
#define MESSAGEPOOL_H

#include <QMutex>
#include <QObject>
#include <QQueue>

class MessageReceive;
class MessageSend;

#define MsMessagePool (static_cast<MessagePool *>(MessagePool::instance()))

class MessagePool : public QObject {
    Q_OBJECT

public:
    explicit MessagePool(QObject *parent = 0);
    ~MessagePool();

    static MessagePool *instance();

    MessageSend *unusedMessageSend(int type);
    MessageSend *usedMessageSend();
    bool hasUnprocessedMessageSend();
    void setMessageUsed(MessageSend *message, bool used);
    int usedSendCount();
    int unusedSendCount();

    MessageReceive *unusedMessageReceive(int type);
    MessageReceive *usedMessageReceive();
    bool hasUnprocessedMessageReceive();
    void setMessageUsed(MessageReceive *message, bool used);
    bool replaceMessageUsed(MessageReceive *message);
    int usedReceiveCount();
    int unusedReceiveCount();

    //
    void mutexProcessingReceiveLock();
    void mutexProcessingReceiveUnlock();
    bool isProcessingReceive();
    void setProcessingReceive(bool value);

    void mutexProcessingSendLock();
    void mutexProcessingSendUnlock();
    bool isProcessingSend();
    void setProcessingSend(bool value);

private:
    void showUsedMessageReceiveQueue();
    void showUsedMessageSendQueue();

signals:

public slots:

private:
    static MessagePool *s_self;

    int m_sendCount = 0;
    QMutex m_mutexUnusedSend;
    QMutex m_mutexUsedSend;
    QQueue<MessageSend *> m_unusedSend;
    QQueue<MessageSend *> m_usedSend;

    int m_receiveCount = 0;
    QMutex m_mutexUnusedReceive;
    QMutex m_mutexUsedReceive;
    QQueue<MessageReceive *> m_unusedReceive;
    QQueue<MessageReceive *> m_usedReceive;

    //
    QMutex m_mutexProcessingReceive;
    bool m_isProcessingReceive = false;
    QMutex m_mutexProcessingSend;
    bool m_isProcessingSend = false;
};

#endif // MESSAGEPOOL_H
