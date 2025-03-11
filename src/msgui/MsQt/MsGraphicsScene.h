#ifndef MSGRAPHICSSCENE_H
#define MSGRAPHICSSCENE_H

#include <QGraphicsScene>

class MessageReceive;

class MsGraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit MsGraphicsScene(QObject *parent = nullptr);

    virtual void processMessage(MessageReceive *message);
    virtual void filterMessage(MessageReceive *message);

protected:
    void sendMessage(int type, const void *data, int size);
    void sendMessageOnly(int type, const void *data, int size);
};

#endif // MSGRAPHICSSCENE_H
