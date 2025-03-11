#include "MsDevice.h"
#include "LiveView.h"
#include "LogWrite.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "msuser.h"
#include "screencontroller.h"
#include <QApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QTimer>
#include <QWSMouseHandler>
#include <QWSServer>
#include <QtDebug>

#ifdef MS_FISHEYE_SOFT_DEWARP
#include "FisheyePanel.h"
FisheyePlayer *MsDevice::s_fisheyeHandle = nullptr;
QMutex MsDevice::s_fisheyeMutex;
#endif

extern "C" {

#include "msg.h"
}

#ifndef _HI3536C_
#include <malloc.h>
#endif

MsDevice *MsDevice::s_msDevice = nullptr;
QPixmap *MsDevice::s_backgroundPixmap = nullptr;

QMap<int, bool> MsDevice::s_noResourceMap;

MsDevice::MsDevice(QObject *parent)
    : MsObject(parent)
{
    s_msDevice = this;

    memset(&m_cameraRealModel, 0, sizeof(m_cameraRealModel));

    //device_info
    QElapsedTimer timer;
    timer.start();
    int res = db_get_device(SQLITE_FILE_NAME, &m_deviceInfo);
    m_deviceInfo.isSupportTalk = vapi_audio_is_support_talk();
    qDebug() << "function db_get_device took" << timer.elapsed() << "ms.";
    if (res) {
        qWarning() << "db_get_device failed.";
    }
    if (m_deviceInfo.oem_type != OEM_TYPE_STANDARD) {
        db_get_device_oem(SQLITE_FILE_NAME, &m_deviceInfoOEM);
    }
    qDebug() << QString("model: %1, softver: %2, oem_type: %3").arg(m_deviceInfo.model).arg(m_deviceInfo.softver).arg(m_deviceInfo.oem_type);

    //
    readProtocolList();

    //display
    readDisplayInfo();

    //poe
    if (!check_param_key(SQLITE_FILE_NAME, PARAM_POE_NUM)) {
        m_poeCount = get_param_int(SQLITE_FILE_NAME, PARAM_POE_NUM, 0);
    }

    //network
    read_network(SQLITE_FILE_NAME, &m_networkInfo);

    //
    initializeMouseAccel();

    //
    s_backgroundPixmap = new QPixmap(":/wizard/wizard/wizard_background.jpg");

    //

    // 定时释放malloc缓存
#if defined(_HI3536A_) || defined(_NT98633_)
    QTimer *mallocTimer = new QTimer(this);
    connect(mallocTimer, SIGNAL(timeout()), this, SLOT(onMallocTrim()));
    mallocTimer->start(1000 * 60 * 5);
#endif
}

MsDevice::~MsDevice()
{
    qDebug() << QString("MsDevice::~MsDevice");

    s_msDevice = nullptr;

    if (m_camera_array) {
        delete[] m_camera_array;
        m_camera_array = nullptr;
    }
}

MsDevice *MsDevice::instance()
{
    //qDebug() << QString("MsDevice::instance()");
    return s_msDevice;
}

void MsDevice::deleteInstance()
{
    if (s_msDevice) {
        delete s_msDevice;
        s_msDevice = nullptr;
    }
}

void MsDevice::no_resource_callback(int winid, int streamid, int bNorsc)
{
    Q_UNUSED(streamid)

    qMsCDebug("qt_noresource") << QString("Show No Resource: %1, %2").arg(VapiWinIdString(winid)).arg(bNorsc ? "true" : "false");

    s_noResourceMap.insert(winid, bNorsc);

    QMetaObject::invokeMethod(qMsNvr, "noResourceChanged", Qt::QueuedConnection, Q_ARG(int, winid), Q_ARG(int, bNorsc));
}

void MsDevice::readyToQuit()
{

}

#ifdef MS_FISHEYE_SOFT_DEWARP
void MsDevice::fisheye_handle_callback(void *handle)
{
    fisheyeHandleLock();
    qDebug() << "MsDevice::fisheye_handle_callback, handle:" << handle;
    s_fisheyeHandle = static_cast<FisheyePlayer *>(handle);
    fisheyeHandleUnlock();

    QMetaObject::invokeMethod(qMsNvr, "fisheyeHandleChanged", Qt::QueuedConnection);
}

void MsDevice::fisheyeHandleLock()
{
    //qWarning() << "lock";
    s_fisheyeMutex.lock();
}

void MsDevice::fisheyeHandleUnlock()
{
    //qWarning() << "unlock";
    s_fisheyeMutex.unlock();
}
#endif

bool MsDevice::isSupportResolution_4k_60Hz() const
{
    switch (m_deviceInfo.softver[1]) {
    case '1': // 3536g
    case '7': // 3536a
        return true;
    default:
        return false;
    }
}

bool MsDevice::isSupportRaid() const
{
    switch (m_deviceInfo.softver[1]) {
    case '1': // 3536g
    case '7': // 3536a
        return true;
    default:
        return false;
    }
}

bool MsDevice::isSupportBestDecoding() const
{
    switch (m_deviceInfo.softver[1]) {
    case '1': // 3536g
    case '7': // 3536a
        return true;
    default:
        return false;
    }
}

bool MsDevice::isSupportDualStreamVideoRecording() const
{
    // 3536g平台双码流录像可能会内存不足，3536a一样
    switch (m_deviceInfo.softver[1]) {
    case '1': // 3536g
        return false;
    default:
        return true;
    }
}

bool MsDevice::isSupportPhysicalShutdown() const
{
    switch (m_deviceInfo.softver[1]) {
    case '1': // 3536g
    case '7': // 3536a
        return true;
    default:
        return false;
    }
}

bool MsDevice::isSupportESataFunction() const
{
    QString model(m_deviceInfo.model);
    if (model == "MS-N8032-UH") {
        return true;
    }
    if (model == "MS-N8064-UH") {
        return true;
    }
    if (model == "MS-N8064-G") {
        return true;
    }
    if (model == "MS-N8032-G") {
        return true;
    }
    return false;
}

device_info MsDevice::deviceInfo() const
{
    return m_deviceInfo;
}

void MsDevice::readDisplayInfo()
{
    memset(&m_displayInfo, 0, sizeof(struct display));
    read_display(SQLITE_FILE_NAME, &m_displayInfo);
}

void MsDevice::writeDisplayInfo(struct display *info)
{
    if (m_displayInfo.event_region != info->event_region) {
        emit displayEventDetectionChanged(info->event_region);
    }
    //
    memcpy(&m_displayInfo, info, sizeof(m_displayInfo));
    write_display(SQLITE_FILE_NAME, info);
}

const display &MsDevice::displayInfo() const
{
    return m_displayInfo;
}

void MsDevice::writeDisplayColor(int index, const QColor &color)
{
    set_param_int(SQLITE_FILE_NAME, PARAM_GUI_CHAN_COLOR, index);

    emit displayColorChanged(color);
}

network MsDevice::networkInfo() const
{
    return m_networkInfo;
}

QString MsDevice::deviceName() const
{
    return QString(m_networkInfo.host_name);
}

/**
 * @brief MsDevice::platform
 * 预留接口，目前只有这一个平台的
 * @return
 */
MsDevice::Type MsDevice::platform()
{
    return HI_NVR;
}

bool MsDevice::is3798() const
{
    if (m_deviceInfo.softver[1] == '2') {
        return true;
    } else {
        return false;
    }
}

bool MsDevice::is3536() const
{
    if (m_deviceInfo.softver[1] == '1') {
        return true;
    } else {
        return false;
    }
}

bool MsDevice::is3536g() const
{
    if (m_deviceInfo.softver[1] == '1') {
        return true;
    } else {
        return false;
    }
}

bool MsDevice::is3536c() const
{
    if (m_deviceInfo.softver[1] == '3') {
        return true;
    } else {
        return false;
    }
}

bool MsDevice::is3536a() const
{
    if (m_deviceInfo.softver[1] == '7') {
        return true;
    } else {
        return false;
    }
}

bool MsDevice::is5016() const
{
    return QString(m_deviceInfo.model).contains("5016");
}

bool MsDevice::is1004() const
{
    return QString(m_deviceInfo.model).contains("1004");
}

bool MsDevice::isnt98323() const
{
#ifdef _NT98323_
    return true;
#else
    return false;
#endif
}

bool MsDevice::isnt98633() const
{
#ifdef _NT98633_
    return true;
#else
    return false;
#endif
}

bool MsDevice::is8000Series() const
{
    return QString(m_deviceInfo.model).startsWith("MS-N8");
}

bool MsDevice::is7000Series() const
{
    return QString(m_deviceInfo.model).startsWith("MS-N7");
}

bool MsDevice::is5000Series() const
{
    return QString(m_deviceInfo.model).startsWith("MS-N5");
}

bool MsDevice::is1000Series() const
{
    return QString(m_deviceInfo.model).startsWith("MS-N1");
}

int MsDevice::maxChannel() const
{
    return m_deviceInfo.max_cameras;
}

int MsDevice::maxAlarmOutput() const
{
    return m_deviceInfo.max_alarm_out;
}

int MsDevice::maxAlarmInput() const
{
    return m_deviceInfo.max_alarm_in;
}

quint64 MsDevice::channelMaskValue() const
{
    quint64 value = 0;
    for (int i = 0; i < m_deviceInfo.max_cameras; ++i) {
        value |= ((quint64)1 << i);
    }
    return value;
}

quint64 MsDevice::channelMaskValue(int channel) const
{
    quint64 value = 0;

    value |= ((quint64)1 << channel);

    return value;
}

bool MsDevice::isSupportEventSnapshot() const
{
    if (is3536c()) {
        return false;
    }
    return true;
}

bool MsDevice::isSupportContinuousSnapshot() const
{
    if (is3536c()) {
        return false;
    }
    return true;
}

DisplayMode MsDevice::displayMode()
{
    char value[20] = { 0 };
    get_param_value(SQLITE_FILE_NAME, PARAM_GUI_DISPLAY_MODE, value, sizeof(value), "");
    return static_cast<DisplayMode>(QString(value).toInt());
}

bool MsDevice::isTargetMode()
{
    char value[20] = { 0 };
    get_param_value(SQLITE_FILE_NAME, PARAM_GUI_ANPR_MODE, value, sizeof(value), "");
    return QString(value).toInt();
}

void MsDevice::setTargetMode(bool mode)
{
    char value[20] = { 0 };
    snprintf(value, sizeof(value), "%d", mode);
    set_param_value(SQLITE_FILE_NAME, PARAM_GUI_ANPR_MODE, value);
}

bool MsDevice::isOccupancyMode()
{
    char value[20] = { 0 };
    get_param_value(SQLITE_FILE_NAME, PARAM_GUI_OCCUPANCY_MODE, value, sizeof(value), "");
    return QString(value).toInt();
}

void MsDevice::setOccupancyMode(bool mode)
{
    char value[20] = { 0 };
    snprintf(value, sizeof(value), "%d", mode);
    set_param_value(SQLITE_FILE_NAME, PARAM_GUI_OCCUPANCY_MODE, value);
}

bool MsDevice::isBootWizardEnable()
{
    char value[20] = { 0 };
    get_param_value(SQLITE_FILE_NAME, PARAM_GUI_WIZARD_ENABLE, value, sizeof(value), "");
    return QString(value).toInt();
}

void MsDevice::setBootWizardEnable(bool enable)
{
    char value[20] = { 0 };
    snprintf(value, sizeof(value), "%d", enable);
    set_param_value(SQLITE_FILE_NAME, PARAM_GUI_WIZARD_ENABLE, value);
}

bool MsDevice::isLocalAuthentication()
{
    char value[20] = { 0 };
    get_param_value(SQLITE_FILE_NAME, PARAM_GUI_AUTH, value, sizeof(value), "");
    return QString(value).toInt();
}

void MsDevice::setLocalAuthentication(bool enable)
{
    char value[20] = { 0 };
    snprintf(value, sizeof(value), "%d", enable);
    set_param_value(SQLITE_FILE_NAME, PARAM_GUI_AUTH, value);
}

bool MsDevice::isMenuAuthentication()
{
    char value[20] = { 0 };
    get_param_value(SQLITE_FILE_NAME, PARAM_GUI_MENU_AUTH, value, sizeof(value), "");
    return QString(value).toInt();
}

void MsDevice::setMenuAuthentication(bool enable)
{
    char value[20] = { 0 };
    snprintf(value, sizeof(value), "%d", enable);
    set_param_value(SQLITE_FILE_NAME, PARAM_GUI_MENU_AUTH, value);
}

QList<int> MsDevice::bitRateList() const
{
    static QList<int> list;
    if (list.isEmpty()) {
        list << 16 << 32 << 64 << 128 << 256 << 384 << 512 << 640 << 768 << 1024 << 1536 << 2048 << 3072 << 4096 << 5120 << 6144 << 7168 << 8192 << 10240 << 12288 << 14336 << 16384;
    }
    return list;
}

bool MsDevice::isOEM() const
{
    if (m_deviceInfo.oem_type != OEM_TYPE_STANDARD) {
        return true;
    } else {
        return false;
    }
}

OEM_TYPE MsDevice::OEMType() const
{
    return static_cast<OEM_TYPE>(m_deviceInfo.oem_type);
}

bool MsDevice::isMilesight() const
{
    if (m_deviceInfo.oem_type == OEM_TYPE_STANDARD || (QString(m_deviceInfo.company) == QString("Milesight"))) {
        return true;
    } else {
        return false;
    }
}

QString MsDevice::model() const
{
    if (isOEM()) {
        return QString(m_deviceInfoOEM.model);
    } else {
        return QString(m_deviceInfo.model);
    }
}

QString MsDevice::hardwareVersion() const
{
    return QString(m_deviceInfo.hardver);
}

double MsDevice::hardwareVersionEx() const
{
    QString hdVersion = QString(m_deviceInfo.hardver).mid(1);
    return hdVersion.toDouble();
}

QString MsDevice::softwareVersion() const
{
    return QString(m_deviceInfo.softver);
}

QMap<int, camera> MsDevice::cameraMap()
{
    camera cam[MAX_CAMERA];
    memset(&cam, 0, sizeof(camera) * MAX_CAMERA);
    int camCount = 0;
    read_cameras(SQLITE_FILE_NAME, cam, &camCount);

    QMap<int, camera> map;
    for (int i = 0; i < maxChannel(); ++i) {
        map.insert(i, cam[i]);
    }
    return map;
}

QList<int> MsDevice::disabledCameraList()
{
    const QMap<int, camera> &map = cameraMap();

    QList<int> channelList;
    QMap<int, camera>::const_iterator iter = map.constBegin();
    for (; iter != map.constEnd(); ++iter) {
        const camera &cam = iter.value();
        if (cam.enable == 0 && cam.id < maxChannel()) {
            channelList.append(cam.id);
        }
    }
    return channelList;
}

void MsDevice::readDatabaseCameras()
{
    if (!m_camera_array) {
        m_camera_array = new struct camera[MAX_CAMERA];
    }
    memset(m_camera_array, 0, sizeof(struct camera) * MAX_CAMERA);
    int camCount = 0;
    read_cameras(SQLITE_FILE_NAME, m_camera_array, &camCount);
}

void MsDevice::readDatabaseCamera(camera *cam, int channel)
{
    if (!cam) {
        cam = new struct camera;
    }
    memset(cam, 0, sizeof(struct camera));
    read_camera(SQLITE_FILE_NAME, cam, channel);
}

void MsDevice::writeDatabaseCamera(camera *cam)
{
    if (cam->type != CAMERA_TYPE_IPNC) {
        qMsWarning() << "invalid type:" << cam->type;
        //cam->type = CAMERA_TYPE_IPNC;
    }
    write_camera(SQLITE_FILE_NAME, cam);
}

camera MsDevice::databaseCamera(int channel)
{
    if (!m_camera_array) {
        readDatabaseCameras();
    }
    Q_ASSERT(channel >= 0 && channel < MAX_CAMERA && channel < maxChannel());
    return m_camera_array[channel];
}

int MsDevice::nextUnusedChannel()
{
    readDatabaseCameras();
    for (int i = 0; i < MAX_CAMERA && i < maxChannel(); ++i) {
        const camera &cam = m_camera_array[i];
        if (cam.enable == 0 && cam.id < MAX_CAMERA) {
            Q_ASSERT(i == cam.id);
            return cam.id;
        }
    }
    return -1;
}

/**
 * @brief MsDevice::checkSameChannelExist
 * @param channel
 * @param ip
 * @param channel_id
 * @return 0:可以继续添加，-1:已经被添加，-2:poe不能重复添加
 */
int MsDevice::checkSameChannelExist(int channel, const QString &ip, int port, int channel_id)
{
    Q_UNUSED(channel)
    Q_UNUSED(ip)
    Q_UNUSED(port)
    Q_UNUSED(channel_id)
    return 0;
}

int MsDevice::checkSameDDNSExist(int channel, const QString &ddns, int channel_id)
{
    Q_UNUSED(channel)
    Q_UNUSED(ddns)
    Q_UNUSED(channel_id)
    return 0;
}

int MsDevice::checkSameRTSPExist(int channel, const QString &rtsp, int port)
{
    qMsDebug() << QString("check, channel: %1, ip: %2,port:%3").arg(channel).arg(rtsp).arg(port);
    for (int i = 0; i < MAX_CAMERA && i < qMsNvr->maxChannel(); ++i) {
        const camera &cam = qMsNvr->databaseCamera(i);
        if (cam.camera_protocol != IPC_PROTOCOL_RTSP) {
            continue;
        }
        if (cam.id == channel) {
            continue;
        }
        if (!cam.enable) {
            continue;
        }
        if (QString(cam.ip_addr) == rtsp && cam.main_rtsp_port == port) {
            qMsDebug() << QString("same, channel: %1, ip: %2").arg(cam.id).arg(cam.ip_addr);
            return -1;
        }
    }
    return 0;
}

bool MsDevice::isChannelConnected(int channel)
{
    bool connect = false;
    if (LiveView::instance()) {
        connect = LiveView::instance()->isChannelConnected(channel);
    }
    return connect;
}

void MsDevice::updateCameraRealModel(resp_cam_model_info *model)
{
    memset(&m_cameraRealModel, 0, sizeof(m_cameraRealModel));
    if (model) {
        memcpy(&m_cameraRealModel, model, sizeof(m_cameraRealModel));
    }
}

void MsDevice::updateSingleCameraRealModel(int channel, CAM_MODEL_INFO *model)
{
    if (channel < 0 || channel > maxChannel()) {
        qMsWarning() << "invalid channel:" << channel;
    } else {
        CAM_MODEL_INFO &tempModel = m_cameraRealModel.modelInfo[channel];
        memset(&tempModel, 0, sizeof(tempModel));
        memcpy(&tempModel, model, sizeof(tempModel));
    }
}

MsCameraModel MsDevice::cameraRealModel(int channel)
{
    QString realModel;
    if (channel < 0 || channel > maxChannel()) {
        qMsWarning() << "invalid channel:" << channel;
    } else {
        const CAM_MODEL_INFO &model = m_cameraRealModel.modelInfo[channel];
        if (model.chnid == channel) {
            realModel = QString(model.model);
        }
    }
    return MsCameraModel(realModel);
}

bool MsDevice::isSigma(int channel)
{
    if (channel < 0 || channel > maxChannel()) {
        qMsWarning() << "invalid channel:" << channel;
        return false;
    }
    //
    QString realModel = cameraRealModel(channel).model();
    //型号以D结尾机型为sigma
    QRegExp rx("MS-C.*D$");
    if (realModel.contains(rx)) {
        return true;
    } else {
        return false;
    }
}
//61或67 且 是安霸ipc或加密芯片12位是Y
bool MsDevice::isPtzBullet(int channel)
{
    Q_UNUSED(channel)
    return false;
}

bool MsDevice::isAmba(int channel)
{
    Q_UNUSED(channel)
    return false;
}

bool MsDevice::isNT(int channel)
{
    if (channel < 0 || channel > maxChannel()) {
        qMsWarning() << "invalid channel:" << channel;
        return false;
    }
    struct ipc_system_info system_info;
    memset(&system_info, 0, sizeof(system_info));
    //get_ipc_system_info(channel, &system_info);
    return system_info.globalSupportinfo.isNtPlatform == 1;
}

bool MsDevice::isHisi(int channel)
{
    Q_UNUSED(channel)
    return false;
}

QMap<int, osd> MsDevice::osdMap()
{
    osd osd_list[MAX_CAMERA];
    int count = 0;
    read_osds(SQLITE_FILE_NAME, osd_list, &count);
    QMap<int, osd> osdMap;
    for (int i = 0; i < count; ++i) {
        osdMap.insert(i, osd_list[i]);
    }
    return osdMap;
}

QString MsDevice::osdName(int channel) const
{
    char name[64] = { 0 };
    read_osd_name(SQLITE_FILE_NAME, name, channel);
    return QString(name);
}

void MsDevice::setOSDName(int channel, const QString &name)
{
    osd osdInfo;
    memset(&osdInfo, 0, sizeof(osdInfo));
    osdInfo.id = channel;
    snprintf(osdInfo.name, sizeof(osdInfo.name), "%s", name.toStdString().c_str());
    write_osd(SQLITE_FILE_NAME, &osdInfo);
}

void MsDevice::updateChannelsName()
{
    m_channelNameMap.clear();

    QMap<int, osd> osdInfoMap = osdMap();
    for (auto iter = osdInfoMap.constBegin(); iter != osdInfoMap.constEnd(); ++iter) {
        const osd &osdInfo = iter.value();
        m_channelNameMap.insert(osdInfo.id, QString(osdInfo.name));
    }
}

QString MsDevice::channelName(int channel) const
{
    Q_UNUSED(channel)
    QString strName;

    return strName;
}

void MsDevice::readProtocolList()
{
    m_protocolMap.clear();

    ipc_protocol *protocolList = NULL;
    int count = 0;
    read_ipc_protocols(SQLITE_FILE_NAME, &protocolList, &count);
    for (int i = 0; i < count; ++i) {
        const ipc_protocol &protocol = protocolList[i];
        m_protocolMap.insert(protocol.pro_id, protocol);
    }
    release_ipc_protocol(&protocolList);
}

QString MsDevice::protocolName(int id)
{
    QString name;
    if (m_protocolMap.contains(id)) {
        name = m_protocolMap.value(id).pro_name;
    }
    return name;
}

QString MsDevice::protocolTypeString(int type)
{
    QString text;
    switch (type) {
    case IPC_PROTOCOL_ONVIF:
        text = QString("IPC_PROTOCOL_ONVIF(%1)").arg(type);
        break;
    case IPC_PROTOCOL_RTSP:
        text = QString("IPC_PROTOCOL_RTSP(%1)").arg(type);
        break;
    case IPC_PROTOCOL_MILESIGHT:
        text = QString("IPC_PROTOCOL_MILESIGHT(%1)").arg(type);
        break;
    default:
        text = QString("IPC_PROTOCOL(%1)").arg(type);
        break;
    }
    return text;
}

QMap<int, ipc_protocol> MsDevice::protocolMap()
{
    return m_protocolMap;
}

QStringList MsDevice::macList() const
{
    QStringList macList;
    QFile file("/tmp/mac");
    if (file.open(QFile::ReadOnly)) {
        QString text = file.readAll();
        QRegExp rx("([A-Fa-f0-9]{2}:){5}[A-Fa-f0-9]{2}");
        int pos = 0;
        while ((pos = rx.indexIn(text, pos)) != -1) {
            macList << rx.cap(0);
            pos += rx.matchedLength();
        }
    }
    //
    if (macList.isEmpty()) {
        qWarning() << "Get mac failed.";
        macList << QString("1C:C3:16:0A:19:C8");
        macList << QString("1C:C3:16:0A:19:C9");
    }
    return macList;
}

bool MsDevice::isPoe() const
{
    return m_poeCount > 0;
}

int MsDevice::poeCount() const
{
    return m_poeCount;
}

QString MsDevice::currentUserName() const
{
    return m_currentUserName;
}

void MsDevice::openLiveviewAudio(int channel)
{
    m_liveviewAudioChannel = channel;

    struct req_set_audiochn audio;
    audio.chn = channel;
    audio.pb_or_live = 0;
    qDebug() << QString("REQUEST_FALG_SET_AUDIOCHAN, chn: %1, pb_or_live: %2").arg(audio.chn).arg(audio.pb_or_live);
    sendMessageOnly(REQUEST_FALG_SET_AUDIOCHAN, (void *)&audio, sizeof(req_set_audiochn));
}

void MsDevice::openPlaybackAudio(int channel)
{
    m_playbackAudioChannel = channel;

    struct req_set_audiochn audio;
    audio.chn = channel;
    audio.pb_or_live = 1;
    qDebug() << QString("REQUEST_FALG_SET_AUDIOCHAN, chn: %1, pb_or_live: %2").arg(audio.chn).arg(audio.pb_or_live);
    sendMessageOnly(REQUEST_FALG_SET_AUDIOCHAN, (void *)&audio, sizeof(req_set_audiochn));
}

void MsDevice::closeLiveviewAudio()
{
    openLiveviewAudio(-1);
}

void MsDevice::closeLiveviewAudio(int channel)
{
    if (liveviewAudioChannel() == channel && channel != -1) {
        closeLiveviewAudio();
    }
}

void MsDevice::closePlaybackAudio()
{
    openPlaybackAudio(-1);
}

int MsDevice::liveviewAudioChannel() const
{
    return m_liveviewAudioChannel;
}

int MsDevice::playbackAudioChannel() const
{
    return m_playbackAudioChannel;
}

bool MsDevice::isAudioOpen() const
{
    return liveviewAudioChannel() != -1 || playbackAudioChannel() != -1;
}

bool MsDevice::isLiveviewAudioOpen() const
{
    return liveviewAudioChannel() != -1;
}

void MsDevice::closeAudio()
{
    if (liveviewAudioChannel() != -1) {
        closeLiveviewAudio();
    }
    if (playbackAudioChannel() != -1) {
        closePlaybackAudio();
    }
}

void MsDevice::closeAudio(int channel)
{
    if (liveviewAudioChannel() == channel) {
        closeLiveviewAudio();
    }
}

bool MsDevice::isSupportTalkback() const
{
    return m_deviceInfo.isSupportTalk && maxAlarmInput() > 0 && maxAlarmOutput() > 0;
}

bool MsDevice::isCamTalkbackOpen() const
{
    return m_talkbackChannel != -1;
}

int MsDevice::talkbackChannel() const
{
    return m_talkbackChannel;
}

void MsDevice::openTalkback(int channel)
{
    m_talkbackChannel = channel;
}

void MsDevice::closeTalkback()
{
    //Audio: 关闭对讲

    if (!isCamTalkbackOpen()) {
        return;
    }


    m_talkbackChannel = -1;
}

void MsDevice::closeTalkback(int channel)
{
    if (talkbackChannel() == channel) {
        closeTalkback();
    }
}

int MsDevice::get_mac_addr(char mac0[], char mac1[])
{
    //bruce.milesight debug notyet
    FILE *fp = fopen("/tmp/mac", "rb");
    if (!fp) {
        snprintf(mac0, 32, "%s", "1C:C3:16:0A:19:C8");
        snprintf(mac1, 32, "%s", "1C:C3:16:0A:19:C9");
        return -1;
    }
    fgets(mac0, 32, fp);
    fgets(mac1, 32, fp);
    fclose(fp);
    mac0[17] = '\0';
    mac1[17] = '\0';
    //end debug

    return 0;
}

bool MsDevice::hasRs485() const
{
    return m_deviceInfo.max_rs485 > 0;
}

int MsDevice::maxDiskPort() const
{
    return m_deviceInfo.max_disk;
}

bool MsDevice::isSlaveMode()
{
    int mode = get_param_int(SQLITE_FILE_NAME, PARAM_FAILOVER_MODE, 0);
    return mode == FAILOVER_MODE_SLAVE;
}

bool MsDevice::isSupportTargetMode() const
{
    if (QString(MS_PLATFORM) == QString("3536C")) {
        return false;
    }
    return true;
}

bool MsDevice::isSupportSnapshot() const
{
    if (QString(MS_PLATFORM) == QString("3536C")) {
        return false;
    }
    return true;
}

bool MsDevice::isSupportAudio() const
{
    if (m_deviceInfo.max_audio_out != 0) {
        return true;
    }
    return false;
}

bool MsDevice::isSupportFaceDetection() const
{
    if (QString(MS_PLATFORM) == QString("3536C")) {
        return false;
    }
    return true;
}

void MsDevice::reboot()
{
    char mac[MAX_LEN_32] = { 0 };
    char mac0[MAX_LEN_32] = { 0 };
    char mac1[MAX_LEN_32] = { 0 };
    //CH_MASK ch_mask = {0};
    struct log_data log_data;
    struct op_lr_reboot olr;

    memset(&olr, 0, sizeof(olr));
    memset(&log_data, 0, sizeof(struct log_data));
    log_data.log_data_info.subType = SUB_OP_REBOOT_LOCAL;
    log_data.log_data_info.parameter_type = SUB_PARAM_NONE;
    snprintf(log_data.log_data_info.user, sizeof(log_data.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
    log_data.log_data_info.chan_no = 0;

    // full detail info
    if (m_deviceInfo.oem_type != OEM_TYPE_STANDARD) {
        snprintf(olr.model, sizeof(olr.model), "%s", m_deviceInfoOEM.model); // model
    } else {
        snprintf(olr.model, sizeof(olr.model), "%s", m_deviceInfo.model);
    }
    read_mac_conf(mac);
    if (mac[0] == '\0') {
        get_mac_addr(mac0, mac1);
    } else {
        snprintf(mac0, sizeof(mac0), "%s", mac);
    }
    snprintf(olr.mac, sizeof(olr.mac), "%s", mac0);
    snprintf(olr.softver, sizeof(olr.softver), "%s", m_deviceInfo.softver);
    msfs_log_pack_detail(&log_data, OP_REBOOT, &olr, sizeof(olr));
    MsWriteLog(log_data);

    sendMessageOnly(REQUEST_FLAG_SYSTEM_REBOOT, nullptr, 0);

    //
    //QTimer::singleShot(0, qApp, SLOT(quit()));
}

void MsDevice::rebootLater(int msec)
{
    QTimer::singleShot(msec, this, SLOT(reboot()));
}

void MsDevice::shutdown()
{
    qDebug() << "MsDevice::shutdown";

    char mac[MAX_LEN_32] = { 0 };
    char mac0[MAX_LEN_32] = { 0 };
    char mac1[MAX_LEN_32] = { 0 };
    struct op_lr_reboot olr;
    struct log_data log_data;
    memset(&olr, 0, sizeof(olr));
    memset(&log_data, 0, sizeof(struct log_data));
    log_data.log_data_info.subType = SUB_OP_SHUTDOWN_LOCAL;
    log_data.log_data_info.parameter_type = SUB_PARAM_NONE;
    snprintf(log_data.log_data_info.user, sizeof(log_data.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
    log_data.log_data_info.chan_no = 0;

    // full detail info
    if (m_deviceInfo.oem_type != OEM_TYPE_STANDARD) {
        snprintf(olr.model, sizeof(olr.model), "%s", m_deviceInfoOEM.model); // model
    } else {
        snprintf(olr.model, sizeof(olr.model), "%s", m_deviceInfo.model);
    }
    read_mac_conf(mac);
    if (mac[0] == '\0') {
        get_mac_addr(mac0, mac1);
    } else {
        snprintf(mac0, sizeof(mac0), "%s", mac);
    }
    snprintf(olr.mac, sizeof(olr.mac), "%s", mac0);
    snprintf(olr.softver, sizeof(olr.softver), "%s", m_deviceInfo.softver);
    msfs_log_pack_detail(&log_data, OP_REBOOT, &olr, sizeof(olr));
    MsWriteLog(log_data);

    sendMessageOnly(REQUEST_FLAG_SYSTEM_SHUTDOWN, nullptr, 0);

    //
    //QTimer::singleShot(0, qApp, SLOT(quit()));
}

void MsDevice::shutdownLater(int msec)
{
    qDebug() << "MsDevice::shutdownLater";

    QTimer::singleShot(msec, this, SLOT(shutdown()));
}

void MsDevice::onMallocTrim()
{
#ifndef _HI3536C_
    malloc_trim(0);
#endif
}

bool MsDevice::isVaildValue(int value)
{
    return value != InvalidValue;
}

bool MsDevice::isSameChannelCanExist()
{
    int same_channel_exist = get_param_int(SQLITE_FILE_NAME, PARAM_SAME_CHANNEL, 0);
    return same_channel_exist == 2;
}

bool MsDevice::isQuickSwitchScreenEnable() const
{
    int value = get_param_int(SQLITE_FILE_NAME, PARAM_QUICK_SCREEN_SWITCH, 0);
    return value == 1;
}

void MsDevice::setQuickSwitchScreenEnable(bool enable)
{
    char value[20] = { 0 };
    snprintf(value, sizeof(value), "%d", enable);
    set_param_value(SQLITE_FILE_NAME, PARAM_QUICK_SCREEN_SWITCH, value);
}
//SubControl初始化在MsDevice前，无法调用，有修改记得一起改
int MsDevice::multiScreenSupport() const
{
    QString strModel(m_deviceInfo.model);
    QString strPrefix(m_deviceInfo.prefix);
    if (strModel == QString("MS-N7016-UH") || strModel == QString("MS-N7032-UH") || strModel == QString("MS-N7016-UPH") || strModel == QString("MS-N7032-UPH") || strModel == QString("MS-N7048-UPH") || strModel == QString("MS-N5032-UH") || strModel == QString("MS-N7016-G") || strModel == QString("MS-N7016-PG") || strModel == QString("MS-N7032-G") || strModel == QString("MS-N7048-PG")) {
        return 2;
    } else if (strPrefix == QString("8")) {
        return 1;
    } else {
        return 0;
    }
}

QMap<AlarmKey, QString> MsDevice::allAlarmoutNames() const
{
    QMap<AlarmKey, QString> map = allChannelAlarmoutNames();

    alarm_out alarms[MAX_ALARM_OUT];
    int alarmCount = 0;
    read_alarm_outs(SQLITE_FILE_NAME, alarms, &alarmCount);
    for (int i = 0; i < alarmCount && i < qMsNvr->maxAlarmOutput(); ++i) {
        const alarm_out &alarm = alarms[i];
        AlarmKey key(alarm.id, alarm.name);
        map.insert(key, alarm.name);
    }
    return map;
}

QMap<AlarmKey, QString> MsDevice::allAlarminNames() const
{
    QMap<AlarmKey, QString> map = allChannelAlarminNames();

    alarm_in alarms[MAX_ALARM_IN];
    int alarmCount = 0;
    read_alarm_ins(SQLITE_FILE_NAME, alarms, &alarmCount);
    for (int i = 0; i < alarmCount && i < qMsNvr->maxAlarmInput(); ++i) {
        const alarm_in &alarm = alarms[i];
        AlarmKey key(alarm.id, alarm.name);
        map.insert(key, alarm.name);
    }
    return map;
}

QMap<AlarmKey, QString> MsDevice::allChannelAlarmoutNames() const
{
    QMap<AlarmKey, QString> map;

    alarm_chn_out_name name_array_1[MAX_REAL_CAMERA];
    memset(&name_array_1, 0, sizeof(alarm_chn_out_name) * MAX_REAL_CAMERA);
    read_alarm_chnOut_event_names(SQLITE_FILE_NAME, name_array_1, MAX_REAL_CAMERA, 0);

    for (int i = 0; i < MAX_REAL_CAMERA && i < maxChannel(); ++i) {
        const alarm_chn_out_name &name = name_array_1[i];
        AlarmKey key(name.chnid, 0, name.delay_time, name.name);
        map.insert(key, name.name);
    }

    //
    alarm_chn_out_name name_array_2[MAX_REAL_CAMERA];
    memset(&name_array_2, 0, sizeof(alarm_chn_out_name) * MAX_REAL_CAMERA);
    read_alarm_chnOut_event_names(SQLITE_FILE_NAME, name_array_2, MAX_REAL_CAMERA, 1);

    for (int i = 0; i < MAX_REAL_CAMERA && i < maxChannel(); ++i) {
        const alarm_chn_out_name &name = name_array_2[i];
        AlarmKey key(name.chnid, 1, name.delay_time, name.name);
        map.insert(key, name.name);
    }

    return map;
}

QMap<AlarmKey, QString> MsDevice::allChannelAlarminNames() const
{
    QMap<AlarmKey, QString> map;

    alarm_chn_name name_array_1[MAX_REAL_CAMERA];
    memset(&name_array_1, 0, sizeof(alarm_chn_name) * MAX_REAL_CAMERA);
    read_alarm_chnIn_event_names(SQLITE_FILE_NAME, name_array_1, MAX_REAL_CAMERA, 0);

    for (int i = 0; i < MAX_REAL_CAMERA && i < maxChannel(); ++i) {
        const alarm_chn_name &name = name_array_1[i];
        AlarmKey key(name.chnid, 0, name.name);
        map.insert(key, name.name);
    }

    //
    alarm_chn_name name_array_2[MAX_REAL_CAMERA];
    memset(&name_array_2, 0, sizeof(alarm_chn_name) * MAX_REAL_CAMERA);
    read_alarm_chnIn_event_names(SQLITE_FILE_NAME, name_array_2, MAX_REAL_CAMERA, 1);

    for (int i = 0; i < MAX_REAL_CAMERA && i < maxChannel(); ++i) {
        const alarm_chn_name &name = name_array_2[i];
        AlarmKey key(name.chnid, 1, name.name);
        map.insert(key, name.name);
    }

    return map;
}

void MsDevice::updateCameraInfo(resq_get_ipcdev *ipc_array, int count)
{
    m_cameraInfoMap.clear();
    for (int i = 0; i < count; ++i) {
        const resq_get_ipcdev &ipcdev = ipc_array[i];
        m_cameraInfoMap.insert(ipcdev.chanid, ipcdev);
    }
}

resq_get_ipcdev MsDevice::cameraInfo(int channel)
{
    resq_get_ipcdev ipcdev;
    memset(&ipcdev, 0, sizeof(resq_get_ipcdev));
    ipcdev.chanid = -1;
    if (m_cameraInfoMap.contains(channel)) {
        const resq_get_ipcdev &temp_ipcdev = m_cameraInfoMap.value(channel);
        memcpy(&ipcdev, &temp_ipcdev, sizeof(resq_get_ipcdev));
    }
    return ipcdev;
}

bool MsDevice::isFisheye(int channel)
{
    if (channel < 0 || channel > maxChannel()) {
        qMsWarning() << "invalid channel:" << channel;
        return false;
    }
    //
    QString realModel;
    const CAM_MODEL_INFO &model = m_cameraRealModel.modelInfo[channel];
    if (model.chnid == channel) {
        realModel = QString(model.model);
    }
    //
    QString displayModel;
    const resq_get_ipcdev &ipcdev = cameraInfo(channel);
    if (ipcdev.chanid == channel) {
        displayModel = QString(ipcdev.model);
    }
    //qMsDebug() << QString("channel: %1, displayModel: %2, realModel: %3").arg(channel).arg(displayModel).arg(realModel);
    QRegExp rx("MS-C..74");
    if (realModel.contains(rx)) {
        return true;
    } else {
        return false;
    }
}

bool MsDevice::isMsCamera(int channel)
{
    if (channel < 0 || channel > maxChannel()) {
        qMsWarning() << "invalid channel:" << channel;
        return false;
    }
    const resq_get_ipcdev &ipcdev = cameraInfo(channel);
    if (ipcdev.chanid == channel) {
        if (QString(ipcdev.sn) == QString("TX_MS20119_X01201112250001000100010")) {
            return true;
        }
    }
    return false;
}

MsCameraVersion MsDevice::cameraVersion(int channel)
{
    MsCameraVersion version;

    const resq_get_ipcdev &ipcdev = cameraInfo(channel);
    if (ipcdev.chanid < 0) {

    } else {
        version.setVersionString(ipcdev.fwversion);
    }

    return version;
}

bool MsDevice::isCameraVersionEqualOrGreaterThan(int channel, const MsCameraVersion &version)
{
    char ver[50];
    Q_UNUSED(channel)
    MsCameraVersion cameraVersion(ver);
    qDebug() << cameraVersion;
    if (!cameraVersion.isVaild()) {
        qMsWarning() << "cameraVersion is invalid.";
        return false;
    }
    if (!version.isVaild()) {
        qMsWarning() << "version is invalid.";
        return false;
    }
    return cameraVersion >= version;
}

void MsDevice::initializeMouseAccel()
{
    char mouseAccel[10] = { 0 };
    get_param_value(SQLITE_FILE_NAME, "mouse_accel", mouseAccel, sizeof(mouseAccel), "2");
    double accel = QString(mouseAccel).toDouble();

    ScreenController::instance()->setMouseSpeed(accel);
}

bool MsDevice::isNoResource(int winid)
{
    if (s_noResourceMap.contains(winid)) {
        if (s_noResourceMap.value(winid)) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool MsDevice::hasNoResource(SCREEN_E screen)
{
    qDebug() << "MsDevice::hasNoRecource, screen:" << screen;
    bool hasNoResource = false;
    for (auto iter = s_noResourceMap.constBegin(); iter != s_noResourceMap.constEnd(); ++iter) {
        int id = iter.key();
        bool noResource = iter.value();
        if (noResource) {
            int temp_screen = id >> 16;
            if (temp_screen == screen) {
                hasNoResource = true;
                qDebug() << "----" << VapiWinIdString(id);
                break;
            }
        }
    }
    return hasNoResource;
}

void MsDevice::clearNoResource()
{
    s_noResourceMap.clear();
}

void MsDevice::clearNoResource(SCREEN_E screen)
{
    for (auto iter = s_noResourceMap.begin(); iter != s_noResourceMap.end();) {
        int id = iter.key();
        bool noResource = iter.value();
        if (noResource) {
            int temp_screen = id >> 16;
            if (temp_screen == screen) {
                iter = s_noResourceMap.erase(iter);
                continue;
            }
        }
        iter++;
    }
}

void MsDevice::writeLog(const camera &cam, int sub_type, int parameter_type)
{
    struct log_data log;
    memset(&log, 0, sizeof(struct log_data));
    //
    if (SUB_EVENT_MIN <= sub_type && sub_type <= SUB_EVENT_MAX) {
        log.log_data_info.mainType = MAIN_EVENT;
    } else if (SUB_OP_LOCAL_MIN <= sub_type && sub_type <= SUB_OP_REMOTE_MAX) {
        log.log_data_info.mainType = MAIN_OP;
    } else if (SUB_EXCEPT_MIN <= sub_type && sub_type <= SUB_EXCEPT_MAX) {
        log.log_data_info.mainType = MAIN_EXCEPT;
    } else if (SUB_INFO_MIN <= sub_type && sub_type <= SUB_INFO_MAX) {
        log.log_data_info.mainType = MAIN_INFO;
    }
    snprintf(log.log_data_info.ip, sizeof(log.log_data_info.ip), "%s", "0.0.0.0");
    //
    log.log_data_info.subType = sub_type;
    log.log_data_info.parameter_type = parameter_type;
    log.log_data_info.chan_no = cam.id + 1;
    snprintf(log.log_data_info.user, sizeof(log.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
    //
    struct op_lr_a_d_e_ip_channel olic;
    memset(&olic, 0, sizeof(struct op_lr_a_d_e_ip_channel));
    olic.action = parameter_type;
    olic.port = cam.manage_port;
    olic.channel = cam.id + 1;
    olic.timeset = cam.sync_time;
    snprintf(olic.ip, sizeof(olic.ip), "%s", cam.ip_addr);
    if (cam.camera_protocol == IPC_PROTOCOL_ONVIF) {
        snprintf(olic.protocal, sizeof(olic.protocal), "%s", "ONVIF");
    } else if (cam.camera_protocol == IPC_PROTOCOL_RTSP) {
        snprintf(olic.protocal, sizeof(olic.protocal), "%s", "RTSP");
    } else {
        snprintf(olic.protocal, sizeof(olic.protocal), "%s", "MSSF");
    }
    if (cam.transmit_protocol == TRANSPROTOCOL_UDP) {
        snprintf(olic.tran_protocal, sizeof(olic.tran_protocal), "%s", "UDP");
    } else if (cam.transmit_protocol == TRANSPROTOCOL_TCP) {
        snprintf(olic.tran_protocal, sizeof(olic.tran_protocal), "%s", "TCP");
    } else {
        snprintf(olic.tran_protocal, sizeof(olic.tran_protocal), "%s", "Auto");
    }
    msfs_log_pack_detail(&log, OP_A_D_E_IP_CHANNEL, &olic, sizeof(olic));
    //
    qMsDebug() << QString("write log, channel: %1, mainType: %2, sub_type: %3, parameter_type: %4").arg(cam.id).arg(log.log_data_info.mainType).arg(sub_type).arg(parameter_type);
    sendMessageOnly(REQUEST_FLAG_LOG_WRITE, (void *)&log, sizeof(struct log_data));
}

void MsDevice::writeLog(int sub_type)
{
    struct log_data log_data;
    memset(&log_data, 0, sizeof(struct log_data));
    log_data.log_data_info.subType = sub_type;
    snprintf(log_data.log_data_info.user, sizeof(log_data.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
    MsWriteLog(log_data);
}

bool MsDevice::isPortUsed(int port) const
{
    FILE *fp;
    int result = 0;
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "netstat -an | grep -E \"LISTEN\" | grep \"0.0.0.0:%d \" > /tmp/port", port);
    ms_system(cmd);
    fp = fopen("/tmp/port", "rb");
    if (!fp)
        return 0;

    while (fgets(cmd, sizeof(cmd), fp)) {
        if (cmd[0] != '\0') {
            result = 1;
            break;
        }
    }
    fclose(fp);
    if (result) {
        return result;
    }

    snprintf(cmd, sizeof(cmd), "netstat -an | grep -E \"ESTABLISHED|WAITTING|FIN_WAIT\" | grep \":%d \" > /tmp/port", port);
    ms_system(cmd);
    fp = fopen("/tmp/port", "rb");
    if (!fp)
        return 0;

    while (fgets(cmd, sizeof(cmd), fp)) {
        if (cmd[0] != '\0') {
            result = 1;
            ms_system("rm -rf /tmp/test");
            ms_system("cp /tmp/port /tmp/test");
            break;
        }
    }
    fclose(fp);

    ms_system("rm -f /tmp/port");
    if (result) {
        snprintf(cmd, sizeof(cmd), "%s", "grep \"::ffff\" /tmp/test > /tmp/port");
        ms_system(cmd);
        fp = fopen("/tmp/port", "rb");
        if (!fp)
            return 0;

        while (fgets(cmd, sizeof(cmd), fp)) {
            if (cmd[0] != '\0') {
                result = 0;
                break;
            }
        }
        fclose(fp);
        ms_system("rm -f /tmp/port");
    }

    return result;
}

void MsDevice::mallocTrimLater(int msec)
{
    Q_UNUSED(msec)
    //3536C平台调用这个概率导致mscore进程直接消失，无任何打印，先屏蔽
    //QTimer::singleShot(msec, this, SLOT(onMallocTrim()));
}
