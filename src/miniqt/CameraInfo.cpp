#include "CameraInfo.h"
#include "CameraInfoManagement.h"
#include "VapiLayout.h"
#include <QtDebug>

QMutex                  s_mutex;
QMap<int, CameraInfo *> CameraInfo::s_mapCameraInfo;

CameraInfo::CameraInfo(QObject *parent) : QObject(parent) {
    initialize();
}

CameraInfo::CameraInfo(const QString &ip, const QString &user, const QString &password, QObject *parent) : QObject(parent), m_ip(ip), m_user(user), m_password(password) {
    initialize();
}

CameraInfo *CameraInfo::fromChannel(int channel) {
    QMutexLocker locker(&s_mutex);

    CameraInfo *info = s_mapCameraInfo.value(channel);
    if (!info) {
        info = new CameraInfo(nullptr);
        info->setChannel(channel);
        s_mapCameraInfo.insert(channel, info);
    }
    return info;
}

void CameraInfo::cleanup() {
    QMutexLocker locker(&s_mutex);

    for (auto iter = s_mapCameraInfo.constBegin(); iter != s_mapCameraInfo.constEnd(); ++iter) {
        auto *info = iter.value();
        delete info;
    }
    s_mapCameraInfo.clear();
}

int CameraInfo::count() {
    return s_mapCameraInfo.count();
}

int CameraInfo::validCount() {
    int c = 0;
    for (auto iter = s_mapCameraInfo.constBegin(); iter != s_mapCameraInfo.constEnd(); ++iter) {
        auto *info = iter.value();
        if (info->isValid()) {
            c++;
        }
    }
    return c;
}

void CameraInfo::initialize()
{
    if (!m_timerReconnect) {
        m_timerReconnect = new QTimer(this);
        connect(m_timerReconnect, SIGNAL(timeout()), this, SLOT(onTimerReconnect()));
    }
    m_timerReconnect->start(1000 * 10);
}

bool CameraInfo::isValid() const {
    if (m_ip.isEmpty()) {
        return false;
    }
    return true;
}

void CameraInfo::clear()
{
    m_ip.clear();
}

const QString &CameraInfo::ip() const {
    return m_ip;
}

void CameraInfo::setIp(const QString &newIp) {
    m_ip = newIp;
}

const QString &CameraInfo::user() const {
    return m_user;
}

void CameraInfo::setUser(const QString &newUser) {
    m_user = newUser;
}

const QString &CameraInfo::password() const {
    return m_password;
}

void CameraInfo::setPassword(const QString &newPassword) {
    m_password = newPassword;
}

int CameraInfo::channel() const {
    return m_channel;
}

void CameraInfo::setChannel(int newChannel) {
    m_channel = newChannel;
}

QString CameraInfo::channelString() const {
    return QString("%1").arg(m_channel + 1);
}

int CameraInfo::protocol() const {
    return m_protocol;
}

void CameraInfo::setProtocol(int newProtocol) {
    m_protocol = newProtocol;
}

QString CameraInfo::protocolString() const {
    switch (m_protocol) {
    case TRANSPROTOCOL_UDP:
        return QString("UDP");
    case TRANSPROTOCOL_TCP:
        return QString("TCP");
    default:
        break;
    }
    return QString();
}

int CameraInfo::codec() const {
    return m_codec;
}
void CameraInfo::setCodec(int newCodec) {
    m_codec = newCodec;
}

QString CameraInfo::codecString() const {
    switch (m_codec) {
    case ENC_TYPE_H264:
        return "H264";
    case ENC_TYPE_H265:
        return "H265";
    default:
        return "Unknow";
    }
}

QString CameraInfo::frameSize() const
{
    return m_frameSize;
}

void CameraInfo::setFrameSize(const QString &newFrameSize)
{
    m_frameSize = newFrameSize;
}

QString CameraInfo::stateString() const {
    switch (m_rtspClientState) {
    case RTSP_CLIENT_CODEC_DIFF:
        return "RTSP_CLIENT_CODEC_DIFF";
    case RTSP_CLIENT_CODEC_INIT:
        return "RTSP_CLIENT_CODEC_INIT";
    case RTSP_CLIENT_CODEC_CHANED:
        return "RTSP_CLIENT_CODEC_CHANED";
    case RTSP_CLIENT_NORESOURCE:
        return "RTSP_CLIENT_NORESOURCE";
    case RTSP_CLIENT_DISCONNECT:
        return "RTSP_CLIENT_DISCONNECT";
    case RTSP_CLIENT_NULL:
        return "RTSP_CLIENT_NULL";
    case RTSP_CLIENT_WAITING:
        return "RTSP_CLIENT_WAITING";
    case RTSP_CLIENT_CONNECT:
        return "RTSP_CLIENT_CONNECT";
    case RTSP_CLIENT_RESOLUTION_CHANGE:
        return "RTSP_CLIENT_RESOLUTION_CHANGE";
    case RTSP_CLIENT_BANDWIDTH:
        return "RTSP_CLIENT_BANDWIDTH";
    case RTSP_CLIENT_UNAUTH:
        return "RTSP_CLIENT_UNAUTH";
    case RTSP_CLIENT_NETWORK:
        return "RTSP_CLIENT_NETWORK";
    default:
        break;
    }
    return "Unknow";
}

QString CameraInfo::rtspUrl() const {
    return QString("rtsp://nvrip/ch_%1").arg(100 + m_channel);
}

bool CameraInfo::isDecodeEnable() {
    QMutexLocker locker(&m_mutex);
    return m_bDecode;
}

void CameraInfo::setDecodeEnable(bool enable) {
    QMutexLocker locker(&m_mutex);
    m_bDecode = enable;
}

void *CameraInfo::rtspServerHandle() const {
    return m_rtspServerHandle;
}

void CameraInfo::setRtspServerHandle(void *newRtspServerHandle) {
    m_rtspServerHandle = newRtspServerHandle;
}

int CameraInfo::rtspServerState() const {
    return m_rtspServerState;
}

void CameraInfo::setRtspServerState(int newRtspServerState) {
    m_rtspServerState = newRtspServerState;
}

int CameraInfo::rtspClientState() const {
    return m_rtspClientState;
}

void CameraInfo::setRtspClientState(int newRtspClientState) {
    m_rtspClientState = newRtspClientState;

    emit stateChanged(m_channel, m_rtspClientState);
}

void CameraInfo::updateFrameInfo(reco_frame *frame)
{
    m_width = frame->width;
    m_height = frame->height;
}

int CameraInfo::createRtspClient() {
    struct rtsp_client_args param;
    memset(&param, 0, sizeof(param));

    param.recvcb          = callbackRtspClientReceive;
    param.arg             = this;
    param.trans_protocol  = (TRANSPROTOCOL)m_protocol;
    param.cb_client_state = callbackRtspClientState;
    snprintf(param.hostname, sizeof(param.hostname), "%s", m_ip.toStdString().c_str());
    snprintf(param.username, sizeof(param.username), "%s", m_user.toStdString().c_str());
    snprintf(param.password, sizeof(param.password), "%s", m_password.toStdString().c_str());
    param.port         = 554;
    param.chanid       = m_channel;
    param.mainsub      = 1;
    static int tindex  = 0;
    param.tindex       = tindex++;
    param.streamfmt    = STREAM_TYPE_MAINSTREAM;
    param.fps          = 25;
    param.bitrate      = 4096;
    param.nvr_code     = CODECTYPE_H264 | CODECTYPE_H265;
    param.ipc_model    = 0;
    param.add_protocol = IPC_PROTOCOL_ONVIF;
    m_rtspClientHandle = reco_rtsp_client_create(&param);
    if (!m_rtspClientHandle) {
        qCritical() << QString("reco_rtsp_client_create(channel: %1, ip: %2) failed!").arg(m_channel).arg(m_ip);
    }

    gVapiLayout.updateLayout(m_channel, true);
    return 0;
}

int CameraInfo::deleteRtspClient() {
    setRtspClientState(RTSP_CLIENT_NULL);
    if (m_rtspClientHandle) {
        reco_rtsp_client_destroy(m_rtspClientHandle);
        m_rtspClientHandle = nullptr;
    }

    gVapiLayout.updateLayout(m_channel, false);
    return 0;
}

int CameraInfo::createRtspServer() {
    struct rtsp_server_args param;
    memset(&param, 0, sizeof(param));

    snprintf(param.name, sizeof(param.name), "ch_%d", m_channel + 100);
    param.chnid           = m_channel;
    param.codec           = m_codec;
    param.cb_server_state = callbackRtspServerState;
    param.arg             = this;
    param.timeout         = -1;
    param.audioon         = 1;

    struct rtsp_user_info user_info[RTSP_MAX_USER];
    memset(user_info, 0, sizeof(user_info));
    for (int i = 0; i < RTSP_MAX_USER; i++) {
        strcpy(user_info[i].username, "admin");
        strcpy(user_info[i].password, "");
    }
    reco_rtsp_server_set_user(user_info);

    m_rtspServerHandle = reco_rtsp_server_create(&param);
    qDebug() << QString("reco_rtsp_server_create, path: %1").arg(param.name);
    if (!m_rtspServerHandle) {
        qCritical() << QString("reco_rtsp_server_create(%1) failed!").arg(param.name);
        return -1;
    }

    return 0;
}

int CameraInfo::deleteRtspServer() {
    if (m_rtspServerHandle) {
        reco_rtsp_server_destroy(m_rtspServerHandle);
        m_rtspServerHandle = nullptr;
    }

    return 0;
}

int CameraInfo::callbackRtspClientReceive(void *arg, reco_frame *frame) {
    CameraInfo *info = static_cast<CameraInfo *>(arg);
    info->updateFrameInfo(frame);

    frame->ch          = info->channel();
    frame->stream_from = SST_IPC_STREAM;

    if (info->rtspServerHandle()) {
        switch (frame->strm_type) {
        case ST_VIDEO:
        case ST_AUDIO:
            reco_rtsp_server_senddata(info->rtspServerHandle(), frame);
            break;
        default:
            break;
        }
    }

    if (frame->strm_type == ST_VIDEO) {
        if (info->isDecodeEnable()) {
            vapi_send_frame(frame->ch, (unsigned char *)frame->data, frame->size, frame->time_usec, 1);
        }
    }

    return 0;
}

int CameraInfo::callbackRtspClientState(int state, void *arg, rtsp_client_state_info *client_state_info) {
    Q_UNUSED(client_state_info)

    CameraInfo *info = static_cast<CameraInfo *>(arg);
    if (info) {
        info->setRtspClientState(state);
        qDebug() << QString("rtsp client, channel: %1, ip: %2, state: %3").arg(info->channel()).arg(info->ip()).arg(info->stateString());

        CameraInfoManagement::instance().updateData(info->channel());
    }

    return 0;
}

int CameraInfo::callbackRtspServerState(int state, void *arg) {
    CameraInfo *info = static_cast<CameraInfo *>(arg);
    if (info) {
        info->setRtspServerState(state);
        qDebug() << QString("rtsp server, channel: %1, ip: %2, state: %3").arg(info->channel()).arg(info->ip()).arg(state);
    }

    return 0;
}

void CameraInfo::onTimerReconnect()
{
    if (!isValid()) {
        return;
    }
    if (rtspClientState() != RTSP_CLIENT_CONNECT) {
        deleteRtspClient();
        createRtspClient();
    }
}
