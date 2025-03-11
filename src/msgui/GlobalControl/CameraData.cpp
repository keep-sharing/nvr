#include "CameraData.h"
#include "MsApplication.h"
#include "MsDevice.h"
#include "MsMessage.h"
#include "MyDebug.h"
#include <QTimer>

void callback_ipc_state(int chanid, int mainsub, int state, int livepb, void *param)
{
    Q_UNUSED(param)

    if (!qMsApp->isInitializeFinished()) {
        return;
    }

    CameraData::State cameraState;
    cameraState.chanid = chanid;
    cameraState.mainsub = mainsub;
    cameraState.state = state;
    cameraState.livepb = livepb;
    QMetaObject::invokeMethod(&gCameraData, "updateCameraState", Qt::QueuedConnection, Q_ARG(CameraData::State, cameraState));
}

CameraData::CameraData()
    : MsObject(nullptr)
{
    qMsDebug();
    qRegisterMetaType<CameraData::State>("CameraData::State");

    m_timerGetCameraList = new QTimer(this);
    m_timerGetCameraList->setSingleShot(true);
    m_timerGetCameraList->setInterval(3000);
    connect(m_timerGetCameraList, SIGNAL(timeout()), this, SLOT(onTimerGetCameraList()));

    //
}

CameraData::~CameraData()
{
    qMsDebug();
}

CameraData &CameraData::instance()
{
    static CameraData self;
    return self;
}

void CameraData::readyToQuit()
{
    m_timerGetCameraList->stop();
}

int CameraData::maxCameraCount()
{
    return qMsNvr->maxChannel();
}

QString CameraData::cameraIPv4Address(int channel)
{
    if (channel < 0 || channel >= maxCameraCount()) {
        qMsCritical() << "invalid channel:" << channel;
        return QString();
    }

    char buf[MAX_LEN_64];
    return QString(buf);
}

QString CameraData::cameraName(int channel)
{
    if (channel < 0 || channel >= maxCameraCount()) {
        qMsCritical() << "invalid channel:" << channel;
        return QString();
    }

    QString strName;

    return strName;
}

QString CameraData::cameraDDNS(int channel)
{
    if (channel < 0 || channel >= maxCameraCount()) {
        qMsCritical() << "invalid channel:" << channel;
        return QString();
    }

    struct camera camera;
    memset(&camera, 0, sizeof (struct camera));
    camera.id = channel;
    read_camera(SQLITE_FILE_NAME, &camera, channel);
    if (camera.camera_protocol != IPC_PROTOCOL_MSDOMAIN){
        return QString();
    }
    return QString(camera.ddns);
}

bool CameraData::isCameraEnable(int channel)
{
    Q_UNUSED(channel)
    return false;
}

bool CameraData::isCameraConnected(int channel)
{
    return isCameraMainStreamConnected(channel) || isCameraSubStreamConnected(channel);
}

bool CameraData::isCameraMainStreamConnected(int channel)
{
    Q_UNUSED(channel)
    return false;
}

bool CameraData::isCameraSubStreamConnected(int channel)
{
    Q_UNUSED(channel)
    return false;
}

bool CameraData::isCameraFacePrivacyMode(int channel)
{
    return m_cameraFacePrivacyMap.value(channel) == 1;
}

void CameraData::updateCameraState(CameraData::State state)
{
    //
    emit cameraStateChanged(state);

    //
    if (state.isMainStream() && state.isLiveView()) {
        if (state.isConnected()) {
            sendMessage(REQUEST_FLAG_GET_FACE_CONFIG, &state.chanid, sizeof(int));
        } else {
            m_cameraFacePrivacyMap.remove(state.chanid);
            emit cameraFacePrivacyState(state.chanid, 0);
        }
        //
        if (!m_cameraConnectionStateMap.contains(state.chanid) || m_cameraConnectionStateMap.value(state.chanid) != state.state) {
            m_cameraConnectionStateMap.insert(state.chanid, state.state);
            m_timerGetCameraList->start();
            emit cameraConnectionStateChanged(state.chanid, state.state);
        }
    }
}

void CameraData::getAllIpcData()
{
    sendMessage(REQUEST_FLAG_GET_IPCLIST, nullptr, 0);
    sendMessage(REQUEST_FLAG_GET_ALL_IPCTYPE, nullptr, 0);
}

void CameraData::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPCLIST:
        ON_RESPONSE_FLAG_GET_IPCLIST(message);
        break;
    case RESPONSE_FLAG_GET_ALL_IPCTYPE:
        ON_RESPONSE_FLAG_GET_ALL_IPCTYPE(message);
        break;
    case RESPONSE_FLAG_GET_FACE_CONFIG:
        ON_RESPONSE_FLAG_GET_FACE_CONFIG(message);
        break;
    }
}

void CameraData::ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message)
{
    resq_get_ipcdev *ipcdev_array = (resq_get_ipcdev *)message->data;
    int count = 0;
    if (ipcdev_array) {
        count = message->header.size / sizeof(resq_get_ipcdev);
        qMsNvr->updateCameraInfo(ipcdev_array, count);
    }
}

void CameraData::ON_RESPONSE_FLAG_GET_ALL_IPCTYPE(MessageReceive *message)
{
    resp_cam_model_info *model_info = static_cast<resp_cam_model_info *>(message->data);
    qMsNvr->updateCameraRealModel(model_info);
}

void CameraData::ON_RESPONSE_FLAG_GET_FACE_CONFIG(MessageReceive *message)
{
    MsFaceConfig *faceConfig = static_cast<MsFaceConfig *>(message->data);
    if (!faceConfig) {
        qMsWarning() << message;
        return;
    }
    qMsDebug() << QString("channel:%1, enable:%2, mosaicEnable:%3").arg(faceConfig->chnId).arg(faceConfig->enable).arg(faceConfig->mosaicEnable);
    bool enable = faceConfig->enable && faceConfig->mosaicEnable;
    m_cameraFacePrivacyMap.insert(faceConfig->chnId, enable);
    emit cameraFacePrivacyState(faceConfig->chnId, enable);
}

void CameraData::onTimerGetCameraList()
{
    getAllIpcData();
}
