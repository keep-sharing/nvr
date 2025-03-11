#include "MsObject.h"
#include "MessageHelper.h"
#include "MsMessage.h"
#include "MyDebug.h"

MsObject::MsObject(QObject *parent)
    : QObject(parent)
{
}

void MsObject::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void MsObject::filterMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void MsObject::sendMessage(int type, const void *data, int size)
{

    Q_UNUSED(type)
    Q_UNUSED(data)
    Q_UNUSED(size)
}

void MsObject::sendMessageOnly(int type, const void *data, int size)
{
    Q_UNUSED(type)
    Q_UNUSED(data)
    Q_UNUSED(size)
}
