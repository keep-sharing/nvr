#include "MessageReceive.h"
#include <QtDebug>

QDebug operator<<(QDebug dbg, MessageReceive *message)
{
    if (message->obj) {
        dbg.nospace() << "MessageReceive(type: " << message->type() << ", size: " << message->header.size << ", count: "
                      << message->header.count << ", data: " << message->data << ", object: " << message->obj->originatingObject() << ")";
    } else {
        dbg.nospace() << "MessageReceive(type: " << message->type() << ", size: " << message->header.size << ", count: "
                      << message->header.count << ", data: " << message->data << ", object: " << message->obj << ")";
    }
    return dbg.space();
}

MessageReceive::MessageReceive()
{
}

MessageReceive::~MessageReceive()
{
}

int MessageReceive::type() const
{
    return header.type;
}

int MessageReceive::size() const
{
    return header.size;
}

bool MessageReceive::isAccepted() const
{
    return accepted;
}

bool MessageReceive::isNull() const
{
    return data == nullptr;
}

void MessageReceive::accept()
{
    accepted = true;
}

void MessageReceive::ignore()
{
    accepted = false;
}

void MessageReceive::clear()
{
    if (obj) {
        delete obj;
        obj = nullptr;
    }

    if (pBigData) {
        delete[] pBigData;
        pBigData = nullptr;
    }

    data = nullptr;
    accepted = false;
    if (!image1.isNull()) {
        image1 = QImage();
    }
    if (!image2.isNull()) {
        image2 = QImage();
    }
}
