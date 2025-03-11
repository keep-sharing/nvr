#include "centralmessage.h"
#include "LogWrite.h"
#include "MessageHelper.h"
#include "MessageObject.h"
#include "MsApplication.h"
#include "MsDevice.h"
#include "MyDebug.h"
#include "messagepool.h"
#include <QFile>

extern "C" {
#include "msoem.h"

}

void MsSendMessage(int sendto, int type, const void *data, int size)
{
    Q_UNUSED(sendto)
    Q_UNUSED(type)
    Q_UNUSED(data)
    Q_UNUSED(size)
}

void MsSendMessage(int sendto, int type, const void *data, int size, MsObject *object)
{
    if (!object) {
        qMsWarning() << "object is nullptr.";
        return;
    }
    Q_UNUSED(sendto)
    Q_UNUSED(type)
    Q_UNUSED(data)
    Q_UNUSED(size)
}

void MsSendMessage(int sendto, int type, const void *data, int size, MsWidget *widget)
{
    if (!widget) {
        qMsWarning() << "widget is nullptr.";
        return;
    }

    Q_UNUSED(sendto)
    Q_UNUSED(type)
    Q_UNUSED(data)
    Q_UNUSED(size)
}

void MsSendMessage(int sendto, int type, const void *data, int size, MsGraphicsScene *scene)
{
    if (!scene) {
        qMsWarning() << "scene is nullptr.";
        return;
    }

    Q_UNUSED(sendto)
    Q_UNUSED(type)
    Q_UNUSED(data)
    Q_UNUSED(size)
}

void MsSendMessage(int sendto, int type, const void *data, int size, MessageObject *obj)
{
    Q_UNUSED(sendto)
    Q_UNUSED(type)
    Q_UNUSED(data)
    Q_UNUSED(size)
    Q_UNUSED(obj)
}

void MessageCallback(void *arg, packet_header *header, void *data)
{
    Q_UNUSED(arg)

    gMessageHelper.responseCountAdd(header->type);
    switch (header->type) {
    case RESPONSE_FLAG_NOTIFYGUI:
    case RESPONSE_FLAG_RET_RECSTATUS:
    case RESPONSE_FLAG_PROGRESS_LOG_EXPORT:
    case RESPONSE_FLAG_PROGRESS_RETRIEVE:
    case RESPONSE_FLAG_PROGRESS_RETRIEVE_EXPORT:
        break;
    default:
        yCDebug("qt_message") << "recv:" << gMessageHelper.messageTypeString(header->type).toLocal8Bit().constData()
                              << "data:" << data
                              << "size:" << header->size
                              << "origin:" << (MessageObject *)((UInt64 *)header->clientId);
        break;
    }

    //
    MessageReceive *message = MsMessagePool->unusedMessageReceive(header->type);
    message->obj = (MessageObject *)((UInt64 *)header->clientId);

    //
    memcpy(&message->header, header, sizeof(packet_header));
    message->data = nullptr;
    if (header->size > 0) {
        if (uint(header->size) <= sizeof(message->pSmallData)) {
            memset(message->pSmallData, 0, sizeof(message->pSmallData));
            memcpy(message->pSmallData, data, header->size);
            message->data = (void *)message->pSmallData;
        } else {
            message->pBigData = new char[header->size];
            if (!message->pBigData) {
                qMsCritical() << "new error.";
                return;
            }
            memcpy(message->pBigData, data, header->size);
            message->data = message->pBigData;
        }
    }

    switch (header->type) {
    case RESPONSE_FLAG_PLAY_EVT_PICTURE:
    case RESPONSE_FLAG_PLAY_COM_PICTURE:
    case RESPONSE_FLAG_PLAY_PIC_BACKUP:
    case RESPONSE_FLAG_GET_IPC_SNAPHOST:
    case RESPONSE_FLAG_GET_INDENTATION_DIAGRAM: {
        if (message->data && message->header.size > 0) {
            qMsCDebug("qt_image_info") << "make image begin:" << message->data
                                       << ", size:" << message->header.size;
            message->image1 = QImage::fromData((uchar *)message->data, message->header.size);
            qMsCDebug("qt_image_info") << "make image end:" << message->image1.size();
        }
        break;
    }
    case RESPONSE_FLAG_SEARCH_ANPR_BACKUP:
    case RESPONSE_FLAG_SEARCH_ANPR_BACKUP_PAGE:
    case RESPONSE_FLAG_SEARCH_ANPR_BACKUP_BIGIMG: {
        resp_search_anpr_backup *anpr_backup = (resp_search_anpr_backup *)message->data;
        if (anpr_backup) {
            message->image1 = QImage::fromData((uchar *)message->data + sizeof(resp_search_anpr_backup), anpr_backup->bImageSize);
            message->image2 = QImage::fromData((uchar *)message->data + sizeof(resp_search_anpr_backup) + anpr_backup->bImageSize, anpr_backup->sImageSize);
        }
        break;
    }
    case RESPONSE_FLAG_PROGRESS_LOG_EXPORT:
    case RESPONSE_FLAG_PROGRESS_RETRIEVE:
        //日志导出进度消息太频繁，直接把队列里面未处理的消息更新成新的消息
        if (MsMessagePool->replaceMessageUsed(message)) {
            return;
        }
        break;
    default:
        break;
    }

    //
    MsMessagePool->setMessageUsed(message, true);

    gMsMessage.prepareReceivedMessage();
}

/**
 * @brief CentralMessage::CentralMessage
 * @param parent
 */
CentralMessage::CentralMessage(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<QMap<int, resp_camera_status>>("QMap<int, resp_camera_status>");

    //8M
    //m_thread.setStackSize((1 << 20) * 8);
    moveToThread(&m_thread);
    connect(&m_thread, SIGNAL(started()), this, SLOT(threadStarted()));
    connect(&m_thread, SIGNAL(finished()), this, SLOT(threadFinished()));
    m_thread.setObjectName("Qt-CentralMessage");
    m_thread.start();
}

CentralMessage::~CentralMessage()
{
    qMsDebug();
}

CentralMessage &CentralMessage::instance()
{
    static CentralMessage self;
    return self;
}

void CentralMessage::stopThread()
{
    m_thread.quit();
    m_thread.wait();
}

void CentralMessage::sendMessage(int sendto, int type, const void *data, uint size, MessageObject *origin)
{
    Q_UNUSED(sendto)
    gMessageHelper.requestCountAdd(type);
    yCDebug("qt_message") << "send:" << gMessageHelper.messageTypeString(type).toLocal8Bit().constData()
                          << "data:" << data
                          << "size:" << size
                          << "origin:" << origin;

}

void CentralMessage::prepareSendMessage()
{
    QMetaObject::invokeMethod(this, "onSendMessage", Qt::QueuedConnection);
}

void CentralMessage::prepareReceivedMessage()
{
    emit sig_message();
}

void CentralMessage::syncFile()
{
    QMetaObject::invokeMethod(this, "onSyncFile", Qt::QueuedConnection);
}

void CentralMessage::refreshAllCameraStatus(int delay_ms)
{
    QTimer::singleShot(delay_ms, this, SLOT(onGetIpcStatus()));
}

void CentralMessage::threadStarted()
{
    gMessageHelper.initialize();

    //
    m_timerGetIpcStatus = new QTimer(this);
    connect(m_timerGetIpcStatus, SIGNAL(timeout()), this, SLOT(onGetIpcStatus()));
    m_timerGetIpcStatus->start(5000);
}

void CentralMessage::threadFinished()
{
    m_timerGetIpcStatus->stop();
}

void CentralMessage::onSyncFile()
{
    sync();
}

void CentralMessage::onSendMessage()
{
    MessageSend *message = MsMessagePool->usedMessageSend();
    if (message) {
        //
        switch (message->type) {
        case REQUEST_FLAG_EXPORT_PICTURE_BACKUP:
            qMsDebug() << "REQUEST_FLAG_EXPORT_PICTURE_BACKUP";
            break;
        default:
            break;
        }
        //
        MsMessagePool->setMessageUsed(message, false);
    }
    //
    if (MsMessagePool->hasUnprocessedMessageSend()) {
        QMetaObject::invokeMethod(this, "onSendMessage", Qt::QueuedConnection);
    }
}

void CentralMessage::onGetIpcStatus()
{
    int channelCount = qMsNvr->maxChannel();
    resp_camera_status status_array[channelCount];
    //get_ipc_status(status_array, channelCount * sizeof(resp_camera_status));

    QMap<int, resp_camera_status> cameraStatusMap;
    for (int i = 0; i < channelCount; ++i) {
        resp_camera_status &status = status_array[i];
        cameraStatusMap.insert(i, status);

        yCDebug("qt_camera_status") << QString("channel: %1, record: %2, stream: %3, main connection: %4, sub connection: %5")
                                           .arg(status.chnid)
                                           .arg(status.record)
                                           .arg(status.stream_type)
                                           .arg(status.status[STREAM_TYPE_MAINSTREAM].connection)
                                           .arg(status.status[STREAM_TYPE_SUBSTREAM].connection);
    }
    emit sig_cameraStatus(cameraStatusMap);
}

void MsWriteLog(log_data &log)
{
    if (SUB_EVENT_MIN <= log.log_data_info.subType && log.log_data_info.subType <= SUB_EVENT_MAX) {
        log.log_data_info.mainType = MAIN_EVENT;
    } else if (SUB_OP_LOCAL_MIN <= log.log_data_info.subType && log.log_data_info.subType <= SUB_OP_REMOTE_MAX) {
        log.log_data_info.mainType = MAIN_OP;
    } else if (SUB_EXCEPT_MIN <= log.log_data_info.subType && log.log_data_info.subType <= SUB_EXCEPT_MAX) {
        log.log_data_info.mainType = MAIN_EXCEPT;
    } else if (SUB_INFO_MIN <= log.log_data_info.subType && log.log_data_info.subType <= SUB_INFO_MAX) {
        log.log_data_info.mainType = MAIN_INFO;
    }
    snprintf(log.log_data_info.ip, sizeof(log.log_data_info.ip), "%s", "0.0.0.0");
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_LOG_WRITE, (void *)&log, sizeof(struct log_data));
}
