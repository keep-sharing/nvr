#include "FaceData.h"
#include "MsMessage.h"
#include "MyDebug.h"
#include "centralmessage.h"
FaceData::FaceData()
    : MsObject(nullptr)
{
}

FaceData::~FaceData()
{
}

FaceData &FaceData::instance()
{
    static FaceData self;
    return self;
}

void FaceData::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_FACE_SUPPORT:
        ON_RESPONSE_FLAG_GET_FACE_SUPPORT(message);
        break;
    case RESPONSE_FLAG_GET_FACE_CONFIG:
        ON_RESPONSE_FLAG_GET_FACE_CONFIG(message);
        break;
    case RESPONSE_FLAG_SET_FACE_CONFIG:
        ON_RESPONSE_FLAG_SET_FACE_CONFIG(message);
        break;
    default:
        break;
    }
}

void FaceData::ON_RESPONSE_FLAG_GET_FACE_SUPPORT(MessageReceive *message)
{
    if (message->data) {
        m_isFaceSupport = *(static_cast<bool *>(message->data));
    }
    qDebug() << QString("FaceData::ON_RESPONSE_FLAG_GET_FACE_SUPPORT, support: %1").arg(m_isFaceSupport);
    m_eventLoop.exit();
}

void FaceData::ON_RESPONSE_FLAG_GET_FACE_CONFIG(MessageReceive *message)
{
    qDebug() << "====FaceCapture::ON_RESPONSE_FLAG_GET_FACE_CONFIG, begin====";
    memset(&m_faceConfig, 0, sizeof(MsFaceConfig));
    struct MsFaceConfig *faceConfig = static_cast<MsFaceConfig *>(message->data);
    if (!faceConfig) {
        qWarning() << "data is null.";
        m_eventLoop.exit();
        return;
    }
    memcpy(&m_faceConfig, faceConfig, sizeof(MsFaceConfig));
    qDebug() << QString("FaceData::ON_RESPONSE_FLAG_GET_FACE_CONFIG, channel: %1, enable: %2, min size: %3, captureMode: %4,")
                    .arg(m_faceConfig.chnId)
                    .arg(m_faceConfig.enable)
                    .arg(m_faceConfig.minPixel)
                    .arg(m_faceConfig.captureMode);
    m_eventLoop.exit();
}

void FaceData::ON_RESPONSE_FLAG_SET_FACE_CONFIG(MessageReceive *message)
{
    Q_UNUSED(message)
}

void FaceData::getFaceConfig(int channel)
{
    Q_UNUSED(channel)
}

bool FaceData::isFaceConflict()
{
    if (m_isFaceSupport && m_faceConfig.enable) {
        return true;
    }
    return false;
}

void FaceData::setFaceDisable()
{
    m_faceConfig.enable = false;
    m_faceConfig.mutuallyExclusive = 1;
    sendMessage(REQUEST_FLAG_SET_FACE_CONFIG, &m_faceConfig, sizeof(MsFaceConfig));
}
