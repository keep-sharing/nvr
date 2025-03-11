#include "MsGraphicsScene.h"
#include "MessageHelper.h"
#include "MsMessage.h"
#include "MyDebug.h"

MsGraphicsScene::MsGraphicsScene(QObject *parent)
    : QGraphicsScene(parent)
{
}

void MsGraphicsScene::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void MsGraphicsScene::filterMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void MsGraphicsScene::sendMessage(int type, const void *data, int size)
{
    Q_UNUSED(type)
    Q_UNUSED(data)
    Q_UNUSED(size)
}

void MsGraphicsScene::sendMessageOnly(int type, const void *data, int size)
{
    Q_UNUSED(type)
    Q_UNUSED(data)
    Q_UNUSED(size)
}
