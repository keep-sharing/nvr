#ifndef MSOBJECT_H
#define MSOBJECT_H

#include <QObject>

class MessageReceive;

class MsObject : public QObject {
    Q_OBJECT

public:
    explicit MsObject(QObject *parent = nullptr);

    virtual void processMessage(MessageReceive *message);
    virtual void filterMessage(MessageReceive *message);

protected:
    void sendMessage(int type, const void *data, int size);
    void sendMessageOnly(int type, const void *data, int size);

signals:
};

#endif // MSOBJECT_H
