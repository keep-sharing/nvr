#include "MessageFilter.h"
#include <MessageObject.h>
#include "MsMessage.h"
#include "BaseWidget.h"
#include "MsGraphicsObject.h"
#include "MsGraphicsScene.h"
#include "MsObject.h"
#include "MsWidget.h"

MessageFilter::MessageFilter(QObject *parent)
    : QObject(parent)
{

}

MessageFilter &MessageFilter::instance()
{
    static MessageFilter self;
    return self;
}

void MessageFilter::processEventFilter(MessageReceive *message)
{
    auto &list = m_filterMap[message->type()];
    for (auto iter = list.begin(); iter != list.end();) {
        const auto &info = *iter;
        if (!info.objPointer) {
            iter = list.erase(iter);
            continue;
        }
        switch (info.objType) {
        case MessageObject::MSQ_OBJECT: {
            MsObject *obj = static_cast<MsObject *>(info.objPointer.data());
            obj->filterMessage(message);
            break;
        }
        case MessageObject::MSQ_WIDGET: {
            MsWidget *obj = static_cast<MsWidget *>(info.objPointer.data());
            obj->filterMessage(message);
            break;
        }
        case MessageObject::MSQ_GRAPHICS_OBJECT: {
            MsGraphicsObject *obj = static_cast<MsGraphicsObject *>(info.objPointer.data());
            obj->filterMessage(message);
            break;
        }
        case MessageObject::MSQ_GRAPHICS_SCENE: {
            MsGraphicsScene *obj = static_cast<MsGraphicsScene *>(info.objPointer.data());
            obj->filterMessage(message);
            break;
        }
        default:
            break;
        }
        iter++;
    }
}

void MessageFilter::installMessageFilter(int type, MsObject *obj)
{
    m_filterMap[type].append(ObjInfo(MessageObject::MSQ_OBJECT, obj));
}

void MessageFilter::installMessageFilter(int type, MsWidget *obj)
{
    m_filterMap[type].append(ObjInfo(MessageObject::MSQ_WIDGET, obj));
}

void MessageFilter::installMessageFilter(int type, MsGraphicsObject *obj)
{
    m_filterMap[type].append(ObjInfo(MessageObject::MSQ_GRAPHICS_OBJECT, obj));
}

void MessageFilter::installMessageFilter(int type, MsGraphicsScene *obj)
{
    m_filterMap[type].append(ObjInfo(MessageObject::MSQ_GRAPHICS_SCENE, obj));
}

void MessageFilter::removeMessageFilter(int type, QObject *obj)
{
    auto &list = m_filterMap[type];
    for (auto iter = list.begin(); iter != list.end();) {
        const auto &data = *iter;
        if (data.objPointer.data() == obj) {
            iter = list.erase(iter);
            continue;
        }
        iter++;
    }
}
