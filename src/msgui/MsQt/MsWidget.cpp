#include "MsWidget.h"
#include "MessageHelper.h"
#include "MsMessage.h"
#include "MyDebug.h"

MsWidget::MsWidget(QWidget *parent)
    : QWidget(parent)
{
}

void MsWidget::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void MsWidget::filterMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void MsWidget::sendMessage(int type, const void *data, int size)
{
    Q_UNUSED(type)
    Q_UNUSED(data)
    Q_UNUSED(size)
}

void MsWidget::sendMessageOnly(int type, const void *data, int size)
{
    Q_UNUSED(type)
    Q_UNUSED(data)
    Q_UNUSED(size)
}
