#include "MessageObject.h"
#include "MessageHelper.h"
#include "MsGraphicsObject.h"
#include "MsGraphicsScene.h"
#include "MsObject.h"
#include "MsWidget.h"
#include "MyDebug.h"

uint constructCount = 0;
uint destructCount = 0;

QDebug operator<<(QDebug dbg, MessageObject *obj)
{
    if (obj) {
        dbg.nospace() << obj->originatingObject();
    } else {
        dbg.nospace() << "0x0";
    }
    return dbg.space();
}

MessageObject::MessageObject(MsObject *obj, int message)
    : QObject(nullptr)
    , m_type(MSQ_OBJECT)
    , m_pointer(obj)
    , m_messageType(message)
    , m_time(QDateTime::currentDateTime())
{
    initialize();
}

MessageObject::MessageObject(MsWidget *obj, int message)
    : QObject(nullptr)
    , m_type(MSQ_WIDGET)
    , m_pointer(obj)
    , m_messageType(message)
    , m_time(QDateTime::currentDateTime())
{
    initialize();
}

MessageObject::MessageObject(MsGraphicsObject *obj, int message)
    : QObject(nullptr)
    , m_type(MSQ_GRAPHICS_OBJECT)
    , m_pointer(obj)
    , m_messageType(message)
    , m_time(QDateTime::currentDateTime())
{
    initialize();
}

MessageObject::MessageObject(MsGraphicsScene *obj, int message)
    : QObject(nullptr)
    , m_type(MSQ_GRAPHICS_SCENE)
    , m_pointer(obj)
    , m_messageType(message)
    , m_time(QDateTime::currentDateTime())
{
    initialize();
}

MessageObject::~MessageObject()
{
    destructCount++;
}

void MessageObject::setArgument(void *arg)
{
    m_arg = arg;
}

void *MessageObject::argument() const
{
    return m_arg;
}

bool MessageObject::isValid() const
{
    if (m_type == MSQ_NULL) {
        qMsWarning() << "invalid type";
        return false;
    }
    if (m_pointer.isNull()) {
        return false;
    }
    return true;
}

QObject *MessageObject::originatingObject() const
{
    return m_pointer.data();
}

void MessageObject::clear()
{
    m_type = MSQ_NULL;
    m_pointer = nullptr;
    m_arg = nullptr;
}

void MessageObject::processMessage(MessageReceive *message)
{
    m_timer->stop();
    if (m_secs > (m_timeout - 1)) {
        yCritical() << "message response:" << gMessageHelper.messageTypeString(m_messageType)
                      << "over seconds:" << m_secs
                      << "origin:" << originatingObject();
    }
    //
    if (m_pointer) {
        switch (m_type) {
        case MSQ_OBJECT: {
            MsObject *obj = static_cast<MsObject *>(m_pointer.data());
            obj->processMessage(message);
            break;
        }
        case MSQ_WIDGET: {
            MsWidget *obj = static_cast<MsWidget *>(m_pointer.data());
            obj->processMessage(message);
            break;
        }
        case MSQ_GRAPHICS_OBJECT: {
            MsGraphicsObject *obj = static_cast<MsGraphicsObject *>(m_pointer.data());
            obj->processMessage(message);
            break;
        }
        case MSQ_GRAPHICS_SCENE: {
            MsGraphicsScene *obj = static_cast<MsGraphicsScene *>(m_pointer.data());
            obj->processMessage(message);
            break;
        }
        default:
            break;
        }
    }
}

void MessageObject::showCount()
{
    qMsWarning() << "MessageObject:" << constructCount << "~MessageObject:" << destructCount;
}

void MessageObject::initialize()
{
    constructCount++;
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_timer->start(1000);
}

void MessageObject::onTimeout()
{
    m_secs++;
    if (m_secs > 0 && m_secs % m_timeout == 0) {
        yCritical() << "message no response:" << gMessageHelper.messageTypeString(m_messageType)
                      << "over seconds:" << m_secs
                      << "origin:" << originatingObject();
    }
}
