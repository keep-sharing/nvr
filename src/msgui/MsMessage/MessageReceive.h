#ifndef MESSAGERECEIVE_H
#define MESSAGERECEIVE_H

#include <QImage>
#include <QPointer>
#include "MessageObject.h"

extern "C" {
#include "mssocket/mssocket.h"
}

class MessageReceive {
public:
    MessageReceive();
    virtual ~MessageReceive();

    packet_header header; //have data real size
    void *data = nullptr; //for data
    char pSmallData[1024] = {}; //for data real size <= 1024
    char *pBigData = nullptr; //for data real size > 1024

    MessageObject *obj = nullptr;

    QImage image1;
    QImage image2;

    int type() const;
    int size() const;

    bool isAccepted() const;
    bool isNull() const;
    void accept();
    void ignore();
    void clear();

private:
    bool accepted = false;
};

QDebug operator<<(QDebug dbg, MessageReceive *message);

#endif // MESSAGERECEIVE_H
