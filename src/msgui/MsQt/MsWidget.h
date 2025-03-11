#ifndef MSWIDGET_H
#define MSWIDGET_H

#include <QWidget>

class MessageReceive;

class MsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MsWidget(QWidget *parent = nullptr);

    virtual void processMessage(MessageReceive *message);
    virtual void filterMessage(MessageReceive *message);

protected:
    void sendMessage(int type, const void *data, int size);
    void sendMessageOnly(int type, const void *data, int size);

signals:

};

#endif // MSWIDGET_H
