#include "ptzdatamanager.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "mainwindow.h"
#include "ptzcontrol.h"

extern "C" {
#include "msg.h"
#include "ptz_public.h"

}

PtzDataManager *PtzDataManager::s_msPtz = nullptr;
PtzDataManager::PtzDataManager(QObject *parent)
    : MsObject(parent)
{
}

PtzDataManager *PtzDataManager::instance()
{
    if (!s_msPtz) {
        s_msPtz = new PtzDataManager(MainWindow::instance());
    }
    return s_msPtz;
}

void PtzDataManager::setFishCorrect(int channel, int correct)
{
    m_fishCorrectMap.insert(channel, correct);
}

int PtzDataManager::fishCorrect(int channel)
{
    return m_fishCorrectMap.value(channel);
}

void PtzDataManager::setChannel(int channel)
{
    m_channel = channel;
}

void PtzDataManager::beginGetData(int channel)
{
    m_channel = channel;
}

int PtzDataManager::waitForGetCameraModelType()
{
    sendMessage(REQUEST_FLAG_GET_IPC_MODEL_TYPE, (void *)&m_channel, sizeof(int));
    return 0;
}

int PtzDataManager::waitForGetFisheyeInfo()
{
    sendMessage(REQUEST_FLAG_GET_FISHEYE_MODE, &m_channel, sizeof(int));
    return 0;
}

int PtzDataManager::waitForGetCameraModelInfo()
{
    sendMessage(REQUEST_FLAG_GET_SINGLE_IPCTYPE, &m_channel, sizeof(int));
    return 0;
}

int PtzDataManager::waitForGetPtzSupport()
{
    sendMessage(REQUEST_FLAG_PTZ_SUPPORT, &m_channel, sizeof(int));
    return 0;
}

int PtzDataManager::waitForGetPtzOvfInfo()
{
    sendMessage(REQUEST_FLAG_PTZ_OVF_INFO, &m_channel, sizeof(int));
    return 0;
}

int PtzDataManager::waitForGetPtzHttpFishInfo(int bundleFisheyeStream)
{
    m_bundleFisheyeStream = bundleFisheyeStream;

    fisheye_ptz_preset_info info;
    info.chn = m_channel;
    if (isShowBundleStream()) {
        info.stream_id = bundleFisheyeStream;
    } else {
        info.stream_id = m_multiFisheyeStream;
    }
    sendMessage(REQUEST_FLAG_PTZ_HTTP_FISH_INFO, &info, sizeof(fisheye_ptz_preset_info));
    return 0;
}

int PtzDataManager::waitForGetPtzZoomPos()
{
    struct req_http_common_param req;
    memset(&req, 0, sizeof(struct req_http_common_param));
    req.chnid = m_channel;
    snprintf(req.info.httpname, sizeof(req.info.httpname), "%s", "vsetting.html");
    snprintf(req.info.pagename, sizeof(req.info.pagename), "%s", "video.0");
    snprintf(req.info.key, sizeof(req.info.key), "%s", "zoompos");
    qMsDebug() << "REQUEST_FLAG_GET_IPC_COMMON_PARAM, key: zoompos, channel:" << m_channel;
    sendMessage(REQUEST_FLAG_GET_IPC_COMMON_PARAM, (void *)&req, sizeof(struct req_http_common_param));
    return 0;
}

int PtzDataManager::waitForGetPtzSpeed()
{
    struct req_http_common_param req;
    memset(&req, 0, sizeof(struct req_http_common_param));
    req.chnid = m_channel;
    snprintf(req.info.httpname, sizeof(req.info.httpname), "%s", "vsetting.html");
    snprintf(req.info.pagename, sizeof(req.info.pagename), "%s", "language=ie");
    snprintf(req.info.key, sizeof(req.info.key), "%s", "ptzspeed");
    qMsDebug() << "REQUEST_FLAG_GET_IPC_COMMON_PARAM, key: ptzspeed, channel:" << m_channel;
    sendMessage(REQUEST_FLAG_GET_IPC_COMMON_PARAM, (void *)&req, sizeof(struct req_http_common_param));
    return 0;
}

int PtzDataManager::waitForGetAutoScan()
{
    struct req_http_common_param req;
    memset(&req, 0, sizeof(struct req_http_common_param));
    req.chnid = m_channel;
    snprintf(req.info.httpname, sizeof(req.info.httpname), "%s", "vsetting.html");
    snprintf(req.info.pagename, sizeof(req.info.pagename), "%s", "language=ie");
    snprintf(req.info.key, sizeof(req.info.key), "%s", "autoscan");
    qMsDebug() << "REQUEST_FLAG_GET_IPC_COMMON_PARAM, key: autoscan, channel:" << m_channel;
    sendMessage(REQUEST_FLAG_GET_IPC_COMMON_PARAM, (void *)&req, sizeof(struct req_http_common_param));
    return 0;
}

int PtzDataManager::waitForGetLedParams()
{
    sendMessage(REQUEST_FLAG_GET_IPC_LED_PARAMS, &m_channel, sizeof(int));
    return 0;
}

int PtzDataManager::waitForGetLedStatus()
{
    sendMessage(REQUEST_FLAG_GET_IPC_LED_STATUS, &m_channel, sizeof(int));
    return 0;
}

int PtzDataManager::waitForGetPtzManualTracking()
{
    sendMessage(REQUEST_FLAG_GET_AUTO_TRACKING, &m_channel, sizeof(int));
    return 0;
}

void PtzDataManager::endGetData()
{
}

void PtzDataManager::waitForGetSystemInfo()
{
    struct ipc_system_info system_info;
    memset(&system_info, 0, sizeof(system_info));
    //get_ipc_system_info(m_channel, &system_info);
    m_supportWiper = system_info.wiperSupport;
}

CAM_MODEL_INFO PtzDataManager::cameraModelInfo() const
{
    return m_cameraModelInfo;
}

QString PtzDataManager::cameraModelString() const
{
    return QString(m_cameraModelInfo.model);
}

int PtzDataManager::modelType() const
{
    return m_modeType;
}

int PtzDataManager::ptzSupport() const
{
    return m_ptzSupport;
}

bool PtzDataManager::isPtzSupport() const
{
    if (m_ptzSupport & PTZ_SUPPORT_YES) {
        return true;
    }
    return false;
}

bool PtzDataManager::hasPtzInfo() const
{
    return m_hasPtzInfo;
}

resp_ptz_ovf_info *PtzDataManager::ptzOvfInfo()
{
    return &m_ptzInfo;
}

resp_ptz_ovf_info *PtzDataManager::ptzHttpFishInfo()
{
    return &m_ptzInfo;
}

int PtzDataManager::zoomPos() const
{
    return m_zoompos;
}

int PtzDataManager::ptzSpeed() const
{
    return m_speed;
}

int PtzDataManager::autoScan() const
{
    return m_autoScan;
}

ms_ipc_led_params_resp PtzDataManager::ledParams() const
{
    return m_led_params;
}

int PtzDataManager::ledStatus() const
{
    return m_ledStatus;
}

int PtzDataManager::autoTrackingStatus() const
{
    return m_auto_tracking;
}

int PtzDataManager::manualTrackingStatus() const
{
    return m_manual_tracking;
}

int PtzDataManager::maxPresetCount(int channel)
{
    int count = 0;
    MsCameraVersion cameraVersion = MsCameraVersion::fromChannel(channel);
    if (cameraVersion >= MsCameraVersion(7, 78)) {
        count = MAX_PRESET_CNT;
    } else {
        count = 255;
    }
    return count;
}

bool PtzDataManager::isFisheye() const
{
    return qMsNvr->isFisheye(m_channel);
}

bool PtzDataManager::isBundleFisheye()
{
    return m_fishmode_param.fishcorrect == MsQt::FISHEYE_BUNDLE;
}

bool PtzDataManager::isShowBundleStream()
{
    if (m_fishmode_param.fishcorrect == MsQt::FISHEYE_BUNDLE) {
        switch (m_fishmode_param.fishdisplay) {
        case MsQt::FISHEYE_DISPLAY_4R:
        case MsQt::FISHEYE_DISPLAY_1O3R:
        case MsQt::FISHEYE_DISPLAY_1P3R:
            return true;
        }
    }
    return false;
}

int PtzDataManager::fisheyeStream()
{
    if (isBundleFisheye()) {
        return m_bundleFisheyeStream;
    } else {
        return m_multiFisheyeStream;
    }
}

int PtzDataManager::fisheyeDisplayMode()
{
    return m_fishmode_param.fishdisplay;
}

bool PtzDataManager::isFisheyeAutoScan(int stream)
{
    return m_ptzInfo.auto_scan_status[stream];
}

bool PtzDataManager::isFisheyeSupportAutoScan()
{
    return false;
}

bool PtzDataManager::isFisheyeSupportPreset() const
{
   return false;
}

bool PtzDataManager::isFisheyeSupportPath()
{
    return false;
}

void PtzDataManager::writePtzSpeed(int speed)
{
    struct req_http_common_param common_param;
    memset(&common_param, 0, sizeof(struct req_http_common_param));

    common_param.chnid = m_channel;
    snprintf(common_param.info.httpname, sizeof(common_param.info.httpname), "%s", "vsetting.html");
    snprintf(common_param.info.key, sizeof(common_param.info.key), "%s", "ptzspeed");
    snprintf(common_param.info.value, sizeof(common_param.info.value), "%d", (speed > 0) ? (speed - 1) : 1);

    sendMessage(REQUEST_FLAG_SET_IPC_COMMON_PARAM, &common_param, sizeof(struct req_http_common_param));
}

void PtzDataManager::writePtzZoomPos(int zoom)
{
    struct req_http_common_param common_param;
    memset(&common_param, 0, sizeof(struct req_http_common_param));
    common_param.chnid = m_channel;
    snprintf(common_param.info.httpname, sizeof(common_param.info.httpname), "%s", "vsetting.html");
    snprintf(common_param.info.key, sizeof(common_param.info.key), "%s", "ipncptz=zoompos");
    snprintf(common_param.info.value, sizeof(common_param.info.value), "%d", zoom);

    sendMessage(REQUEST_FLAG_SET_IPC_COMMON_PARAM, &common_param, sizeof(struct req_http_common_param));
}

/**
 * @brief PtzDataManager::sendPtzControl
 * @param action
 * @param speed
 */
void PtzDataManager::sendPtzControl(int action, int speed)
{
    struct req_ptz_action ptz_action;
    memset(&ptz_action, 0, sizeof(req_ptz_action));

    ptz_action.chn = m_channel;
    ptz_action.action = action;

    ptz_action.speed.pan = speed;
    ptz_action.speed.tilt = speed;
    ptz_action.speed.zoom = speed;
    ptz_action.speed.focus = speed;

    ptz_action.type = 0;

    sendMessageOnly(REQUEST_FLAG_PTZ_ACTION, (void *)&ptz_action, sizeof(req_ptz_action));
}

/**
 * @brief PtzDataManager::sendPresetControl
 * @param action
 * @param presetid: 1-255
 * @param speed
 * @param name
 */
void PtzDataManager::sendPresetControl(int action, int presetid, int speed, const QString &name)
{
    struct req_ptz_action ptz_action;
    memset(&ptz_action, 0, sizeof(req_ptz_action));

    ptz_action.chn = m_channel;
    ptz_action.action = action;
    ptz_action.param = presetid;
    snprintf(ptz_action.name, sizeof(ptz_action.name), "%s", name.toStdString().c_str());

    ptz_action.speed.pan = speed;
    ptz_action.speed.tilt = speed;
    ptz_action.speed.zoom = speed;
    ptz_action.speed.focus = speed;

    ptz_action.type = 0;

    qMsDebug() << QString("action: %1, preset id: %2, speed: %3, name: %4").arg(action).arg(presetid).arg(speed).arg(name);
    sendMessageOnly(REQUEST_FLAG_PTZ_PRESET_ACTION, (void *)&ptz_action, sizeof(req_ptz_action));
}

/**
 * @brief PtzDataManager::controlPatrol
 * 调用巡航
 * @param action: REQUEST_FLAG_PTZ_TOUR_RUN, REQUEST_FLAG_PTZ_TOUR_STOP, REQUEST_FLAG_PTZ_TOUR_CLEAR
 * @param tourid: 1-8
 */
void PtzDataManager::sendPatrolControl(int action, int tourid)
{
    struct req_ptz_tour ptz_tour;
    memset(&ptz_tour, 0, sizeof(req_ptz_tour));
    ptz_tour.chn = m_channel;
    ptz_tour.tourid = tourid;

    qDebug() << QString("PtzDataManager::sendPatrolControl, channel: %1, action: %2, tourid: %3").arg(m_channel).arg(action).arg(tourid);
    sendMessageOnly(action, (void *)&ptz_tour, sizeof(req_ptz_tour));
}

/**
 * @brief PtzDataManager::setPatrol
 * 设置巡航
 * @param tourid: 1-8
 * @param list
 */
void PtzDataManager::sendPatrolData(int tourid, const QList<ptz_path> &list)
{
    struct req_ptz_tour_all ptz_tour_all;
    memset(&ptz_tour_all, 0, sizeof(req_ptz_tour_all));
    ptz_tour_all.chn = m_channel;
    ptz_tour_all.tourid = tourid;
    ptz_tour_all.rowsize = list.size();
    for (int i = 0; i < list.size(); ++i) {
        ptz_path &path = ptz_tour_all.path[i];
        path = list.at(i);
    }
    sendMessage(REQUEST_FLAG_PTZ_SET, &ptz_tour_all, sizeof(req_ptz_tour_all));
}

/**
 * @brief PtzDataManager::sendPatternControl
 * 调用花式扫描
 * @param operation
 * @param id: 1-4
 * @param speed
 */
void PtzDataManager::sendPatternControl(const PatternOperation &operation, int id, int speed)
{
    struct req_ptz_action ptz_action;
    memset(&ptz_action, 0, sizeof(req_ptz_action));

    ptz_action.chn = m_channel;
    ptz_action.param = id;

    ptz_action.speed.pan = speed;
    ptz_action.speed.tilt = speed;
    ptz_action.speed.zoom = speed;
    ptz_action.speed.focus = speed;

    ptz_action.type = 0;

    switch (operation) {
    case PatternStartRecord:
        ptz_action.action = PTZ_PATTERN_START;
        break;
    case PatternStopRecord:
        ptz_action.action = PTZ_PATTERN_STOP;
        break;
    case PatternRun:
        ptz_action.action = PTZ_PATTERN_RUN;
        break;
    case PatternStop:
        ptz_action.action = PTZ_STOP_ALL;
        break;
    case PatternDelete:
        ptz_action.action = PTZ_PATTERN_DEL;
        break;
    default:
        break;
    }

    sendMessageOnly(REQUEST_FLAG_PTZ_TRACK_ACTION, (void *)&ptz_action, sizeof(req_ptz_action));
}

void PtzDataManager::sendPresetControlJson(IPC_PTZ_CONTORL_TYPE cmd, int value)
{
    IpcPtzControl control;
    memset(&control, 0, sizeof(IpcPtzControl));
    control.chnId = m_channel;
    control.controlType = cmd;
    control.controlAction = value;
    sendMessageOnly(REQUEST_FLAG_SET_IPC_PTZ_CONTROL_JSON, &control, sizeof(IpcPtzControl));
}

bool PtzDataManager::isAutoZoomModel() const
{
    QString strMode(m_cameraModelInfo.model);
    QString strTail;
    QRegExp rx("MS-C(.*)-(.*)");
    if (rx.indexIn(strMode) != -1) {
        strTail = rx.cap(2);
        if (strTail.contains("F")) {
            return true;
        }
    }
    return false;
}

bool PtzDataManager::isAbfProBox() const
{
    //MSHN-6176 QT-Live View：（MS-CXX5X-REPB）枪机不支持preset，path，pattern功能，建议置灰PTZ控制面板上的控件按钮
    return false;
}

bool PtzDataManager::isPtzBullet() const
{
    QString strMode(m_cameraModelInfo.model);
    //
    QRegExp rx1("MS-C..61");
    QRegExp rx2("MS-C..67");
    if (strMode.contains(rx1) || strMode.contains(rx2)) {
        return true;
    }

    //
    return false;
}

bool PtzDataManager::isPresetEnable() const
{
    if (isFisheye()) {
        return isFisheyeSupportPreset();
    } else {
        QString strMode(QString(m_cameraModelInfo.model));

        //
        if (isAutoZoomModel() || isAbfProBox()) {
            return false;
        }

        //MS-C2962-RELPB
        QRegExp rx1("MS-C..(..)-(.*)");
        if (rx1.indexIn(strMode) != -1) {
            QString str1 = rx1.cap(1);
            QString str2 = rx1.cap(2);
            if (str1 == QString("62") && str2.contains(QString("E"))) {
                return false;
            }
            if (str1 == QString("75") && str2.contains(QString("E"))) {
                return false;
            }
        }
    }

    //
    return true;
}

bool PtzDataManager::isSupportWiper() const
{
    return m_supportWiper;
}

bool PtzDataManager::modelDetect(const QRegExp &rx)
{
    int index = rx.indexIn(QString(m_cameraModelInfo.model));
    return index != -1;
}

void PtzDataManager::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void PtzDataManager::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPC_MODEL_TYPE:
        ON_RESPONSE_FLAG_GET_IPC_MODEL_TYPE(message);
        break;
    case RESPONSE_FLAG_GET_FISHEYE_MODE:
        ON_RESPONSE_FLAG_GET_FISHEYE_MODE(message);
        break;
    case RESPONSE_FLAG_GET_SINGLE_IPCTYPE:
        ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(message);
        break;
    case RESPONSE_FLAG_PTZ_SUPPORT:
        ON_RESPONSE_FLAG_PTZ_SUPPORT(message);
        break;
    case RESPONSE_FLAG_PTZ_OVF_INFO:
        ON_RESPONSE_FLAG_PTZ_OVF_INFO(message);
        break;
    case RESPONSE_FLAG_PTZ_HTTP_FISH_INFO:
        ON_RESPONSE_FLAG_PTZ_HTTP_FISH_INFO(message);
        break;
    case RESPONSE_FLAG_GET_IPC_COMMON_PARAM:
        ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(message);
        break;
    case RESPONSE_FLAG_GET_IPC_LED_PARAMS:
        ON_RESPONSE_FLAG_GET_IPC_LED_PARAMS(message);
        break;
    case RESPONSE_FLAG_GET_IPC_LED_STATUS:
        ON_RESPONSE_FLAG_GET_IPC_LED_STATUS(message);
        break;
    case RESPONSE_FLAG_GET_AUTO_TRACKING:
        ON_RESPONSE_FLAG_GET_AUTO_TRACKING(message);
        break;
    }
}

void PtzDataManager::ON_RESPONSE_FLAG_GET_IPC_MODEL_TYPE(MessageReceive *message)
{
    if (message->data) {
        m_modeType = *((int *)message->data);
        qMsDebug() << QString("channel: %1, mode type: %2").arg(m_channel).arg(m_modeType);
    } else {
        qMsWarning() << message;
    }

    m_eventLoop.exit();
}

void PtzDataManager::ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message)
{
    Q_UNUSED(message)
}

void PtzDataManager::ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(MessageReceive *message)
{
    CAM_MODEL_INFO *cam_model_info = (CAM_MODEL_INFO *)message->data;
    memset(&m_cameraModelInfo, 0, sizeof(CAM_MODEL_INFO));
    if (cam_model_info) {
        memcpy(&m_cameraModelInfo, cam_model_info, sizeof(CAM_MODEL_INFO));
        qMsDebug() << QString("channel: %1, ip: %2, model: %3, fwversion: %4")
                          .arg(cam_model_info->chnid)
                          .arg(cam_model_info->ipaddr)
                          .arg(cam_model_info->model)
                          .arg(cam_model_info->fwversion);
    } else {
        qMsWarning() << message;
    }

    m_eventLoop.exit();
}

void PtzDataManager::ON_RESPONSE_FLAG_PTZ_SUPPORT(MessageReceive *message)
{
    if (message->data) {
        m_ptzSupport = *((int *)message->data);
        qMsDebug() << QString("channel: %1, ptz support: %2").arg(m_channel).arg(m_ptzSupport);
    } else {
        qMsWarning() << message;
    }

    m_eventLoop.exit();
}

void PtzDataManager::ON_RESPONSE_FLAG_PTZ_OVF_INFO(MessageReceive *message)
{
    struct resp_ptz_ovf_info *ptz_ovf_info = (struct resp_ptz_ovf_info *)message->data;
    memset(&m_ptzInfo, 0, sizeof(resp_ptz_ovf_info));
    if (ptz_ovf_info) {
        memcpy(&m_ptzInfo, ptz_ovf_info, sizeof(resp_ptz_ovf_info));
        m_hasPtzInfo = true;
    } else {
        m_hasPtzInfo = false;
        qMsWarning() << QString("PtzDataManager::ON_RESPONSE_FLAG_PTZ_OVF_INFO, channel: %1, data is null.").arg(m_channel);
    }

    m_eventLoop.exit();
}

void PtzDataManager::ON_RESPONSE_FLAG_PTZ_HTTP_FISH_INFO(MessageReceive *message)
{
    ON_RESPONSE_FLAG_PTZ_OVF_INFO(message);
}

void PtzDataManager::ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(MessageReceive *message)
{
    struct resp_http_common_param *param = (struct resp_http_common_param *)message->data;
    if (param) {
        if (QString(param->info.key) == QString("zoompos")) {
            m_zoompos = QString(param->info.value).toInt();
            qMsDebug() << QString("channel: %1, zoompos: %2").arg(m_channel).arg(m_zoompos);
        } else if (QString(param->info.key) == QString("ptzspeed")) {
            m_speed = QString(param->info.value).toInt() + 1;
            qMsDebug() << QString("channel: %1, ptzspeed: %2").arg(m_channel).arg(m_speed);
        }
        if (QString(param->info.key) == QString("autoscan")) {
            m_autoScan = QString(param->info.value).toInt();
            qMsDebug() << QString("channel: %1, autoscan: %2").arg(m_channel).arg(m_autoScan);
        }
    }

    m_eventLoop.exit();
}

void PtzDataManager::ON_RESPONSE_FLAG_GET_IPC_LED_PARAMS(MessageReceive *message)
{
    ms_ipc_led_params_resp *led_params = (ms_ipc_led_params_resp *)message->data;

    qDebug() << "====PtzDataManager::ON_RESPONSE_FLAG_GET_IPC_LED_PARAMS====";

    memset(&m_led_params, 0, sizeof(m_led_params));
    if (led_params) {
        memcpy(&m_led_params, led_params, sizeof(m_led_params));

        qDebug() << "----channel:" << m_led_params.chnid;
        qDebug() << "----led_alarm:" << m_led_params.led_alarm;
        qDebug() << "----led_number:" << m_led_params.led_number;
        qDebug() << "----led_manual:" << m_led_params.led_manual;
    }

    m_eventLoop.exit();
}

void PtzDataManager::ON_RESPONSE_FLAG_GET_IPC_LED_STATUS(MessageReceive *message)
{
    ms_ipc_led_status_resp *led_status = (ms_ipc_led_status_resp *)message->data;
    if (led_status) {
        m_ledStatus = led_status->led_status;
        qDebug() << QString("PtzDataManager::ON_RESPONSE_FLAG_GET_IPC_LED_STATUS, channel: %1, status: %2").arg(led_status->chnid).arg(led_status->led_status);
    }

    m_eventLoop.exit();
}

void PtzDataManager::ON_RESPONSE_FLAG_GET_AUTO_TRACKING(MessageReceive *message)
{
    ms_auto_tracking *auto_tracking = (ms_auto_tracking *)message->data;
    if (auto_tracking) {
        m_auto_tracking = auto_tracking->enable;
        m_manual_tracking = auto_tracking->manual_tracking;
        qDebug() << QString("PtzDataManager::ON_RESPONSE_FLAG_GET_AUTO_TRACKING, channel: %1, manual_tracking: %2").arg(auto_tracking->chanid).arg(auto_tracking->manual_tracking);
    }

    m_eventLoop.exit();
}
