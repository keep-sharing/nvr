#include "CameraEditTabSettings.h"
#include "ui_CameraEditTabSettings.h"
#include "CameraEdit.h"
#include "LogWrite.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "camerachanneladd.h"
#include "centralmessage.h"
#include "myqt.h"

extern "C" {
#include "msg.h"
}

CameraEditTabSettings::CameraEditTabSettings(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::CameraEditTabSettings)
{
    ui->setupUi(this);

    m_edit = static_cast<CameraEdit *>(parent);

    ui->comboBox_protocol->clear();
    QMap<int, ipc_protocol> protocolMap = qMsNvr->protocolMap();
    for (auto iter = protocolMap.constBegin(); iter != protocolMap.constEnd(); ++iter) {
        const ipc_protocol &protocol = iter.value();
        ui->comboBox_protocol->addItem(protocol.pro_name, protocol.pro_id);
    }
    if (!MsDevice::instance()->isMilesight()) {
        ui->comboBox_protocol->removeItemFromData(IPC_PROTOCOL_MSDOMAIN);
    }
    //protocol
    ui->comboBox_protocol->setCurrentIndex(0);
    m_msddns = "ddns.milesight.com/";
    m_perProtocol = IPC_PROTOCOL_ONVIF;
    on_comboBox_protocol_activated(0);

    ui->comboBox_transferMode->clear();
    ui->comboBox_transferMode->addItem(GET_TEXT("FISHEYE/12014", "Bundle-Stream Mode"), 1);
    ui->comboBox_transferMode->addItem(GET_TEXT("FISHEYE/12015", "Multi-Channel Mode"), 0);

    ui->comboBox_transportProtocol->clear();
    ui->comboBox_transportProtocol->addItem("Auto", TRANSPROTOCOL_AUTO);
    ui->comboBox_transportProtocol->addItem("UDP", TRANSPROTOCOL_UDP);
    ui->comboBox_transportProtocol->addItem("TCP", TRANSPROTOCOL_TCP);

    ui->comboBox_fisheyeInstallMode->clear();
    ui->comboBox_fisheyeInstallMode->addItem(GET_TEXT("FISHEYE/12002", "Ceiling"), MsQt::FISHEYE_INSTALL_CEILING);
    ui->comboBox_fisheyeInstallMode->addItem(GET_TEXT("FISHEYE/12003", "Wall"), MsQt::FISHEYE_INSTALL_WALL);
    ui->comboBox_fisheyeInstallMode->addItem(GET_TEXT("FISHEYE/12004", "Flat"), MsQt::FISHEYE_INSTALL_FLAT);

    //fisheye
    ui->label_fisheyeInstallMode->hide();
    ui->label_fisheyeDisplayMode->hide();
    ui->comboBox_fisheyeInstallMode->hide();
    ui->comboBox_fisheyeDisplayMode->hide();

    //valid check
    ui->lineEdit_channelName->setCheckMode(MyLineEdit::EmptyCheck);
    ui->lineEdit_ip->setCheckMode(MyLineEdit::ServerCheck);
    ui->lineEdit_port->setCheckMode(MyLineEdit::RangeCheck, 1, 65535);
    ui->lineEditHTTPSPort->setCheckMode(MyLineEdit::RangeCheck, 1, 65535);
    ui->lineEdit_userName->setCheckMode(MyLineEdit::UserNameCheck);
    ui->lineEdit_password->setCheckMode(MyLineEdit::EmptyCheck);

    onLanguageChanged();
}

CameraEditTabSettings::~CameraEditTabSettings()
{
    delete ui;
}

void CameraEditTabSettings::initializeData()
{
    m_channel_previous = m_edit->m_channel;
    m_perProtocol = IPC_PROTOCOL_ONVIF;
    m_ipAddress = "";

    //
    qMsNvr->readDatabaseCameras();
    const camera &camera_previous = qMsNvr->databaseCamera(m_channel_previous);
    memcpy(&m_camera_previous, &camera_previous, sizeof(struct camera));
    //channel
    ui->comboBox_channel->clear();
    for (int i = 0; i < MAX_CAMERA && i < qMsNvr->maxChannel(); ++i) {
        const struct camera &cam = qMsNvr->databaseCamera(i);
        if (cam.enable == 0 || i == m_channel_previous) {
            ui->comboBox_channel->addItem(QString::number(i + 1), i);
        }
    }
    ui->comboBox_channel->setCurrentIndexFromData(m_channel_previous);
    //protocol
    ui->comboBox_protocol->setCurrentIndexFromData(camera_previous.camera_protocol);
    on_comboBox_protocol_activated(ui->comboBox_protocol->currentIndex());
    //
    ui->lineEdit_userName->setText(camera_previous.username);
    ui->lineEdit_password->setText(camera_previous.password);
    ui->comboBox_transportProtocol->setCurrentIndexFromData(camera_previous.transmit_protocol);
    on_comboBox_transportProtocol_activated(camera_previous.transmit_protocol);
    ui->checkBox_timeSetting->setChecked(camera_previous.sync_time);
    ui->lineEdit_port->setText(QString::number(camera_previous.manage_port));
    ui->lineEditHTTPSPort->setText(QString::number(camera_previous.https_port));
    if (camera_previous.camera_protocol == IPC_PROTOCOL_MSDOMAIN) {
        m_msddns = camera_previous.ddns;
        m_ipAddress = "";
        ui->lineEdit_ip->setText(camera_previous.ddns);
        ui->lineEdit_port->setText(QString::number(80));
    } else {
        m_msddns = "ddns.milesight.com/" + QString(m_edit->m_ipcdev.mac);
        ui->lineEdit_ip->setText(camera_previous.ip_addr);
        m_ipAddress = camera_previous.ip_addr;
        if (camera_previous.camera_protocol == IPC_PROTOCOL_RTSP) {
            if (QString(camera_previous.ip_addr).indexOf("ddns.milesight.com/") != -1) {
                m_ipAddress = "";
            }
            ui->lineEdit_port->setText(QString::number(80));
        }
    }

    if (QString(camera_previous.main_rtspurl).isEmpty()) {
        if (camera_previous.camera_protocol == IPC_PROTOCOL_MSDOMAIN) {
            ui->lineEdit_primary->setText("rtsp://");
        } else {
            ui->lineEdit_primary->setText("rtsp://" + QString(camera_previous.ip_addr));
        }
    } else {
        ui->lineEdit_primary->setText(camera_previous.main_rtspurl);
    }
    m_primary = ui->lineEdit_primary->text();
    if (QString(camera_previous.sub_rtspurl).isEmpty()) {
        ui->lineEdit_secondary->setText("rtsp://");
    } else {
        ui->lineEdit_secondary->setText(camera_previous.sub_rtspurl);
    }
    m_secondary = ui->lineEdit_secondary->text();
    //
    ui->lineEdit_channelName->setText(qMsNvr->channelName(m_channel_previous));
    //
    if (qMsNvr->isPoe() && camera_previous.poe_channel) {
        ui->lineEdit_ip->setEnabled(false);
        ui->lineEdit_port->setText(QString::number(camera_previous.physical_port));
        ui->lineEdit_port->setEnabled(false);
        ui->comboBox_channelId->setEnabled(false);
    } else {
        ui->lineEdit_ip->setEnabled(true);
        ui->lineEdit_port->setEnabled(true);
        ui->comboBox_channelId->setEnabled(true);
    }

    //鱼眼,先更新缓存
    //MsWaitting::showGlobalWait();
    sendMessage(REQUEST_FLAG_GET_SINGLE_IPCTYPE, &m_channel_previous, sizeof(int));
    //m_eventLoop.exec();
    sendMessage(REQUEST_FLAG_GET_FISHEYE_MODE, &m_channel_previous, sizeof(int));
    //m_eventLoop.exec();
    //MsWaitting::closeGlobalWait();
    qMsDebug() << "REQUEST_FLAG_GET_FISHEYE_MODE"
               << ", chnid:" << m_fishmode_param.chnid
               << ", result:" << m_fishmode_param.result
               << ", transfer:" << FisheyeTransferModeString(m_fishmode_param.fishcorrect)
               << ", installation:" << FisheyeInstallModeString(m_fishmode_param.fishmount)
               << ", display:" << FisheyeDisplayModeString(m_fishmode_param.fishdisplay);
    m_isFisheye_previous = qMsNvr->isFisheye(m_channel_previous);
    //
    if (!m_isFisheye_previous) {
        ui->label_transferMode->hide();
        ui->comboBox_transferMode->hide();
        ui->label_fisheyeInstallMode->hide();
        ui->comboBox_fisheyeInstallMode->hide();
        ui->label_fisheyeDisplayMode->hide();
        ui->comboBox_fisheyeDisplayMode->hide();
        ui->label_channelId->hide();
        ui->comboBox_channelId->hide();
    } else {
        ui->label_transferMode->show();
        ui->comboBox_transferMode->show();
        ui->label_fisheyeInstallMode->show();
        ui->comboBox_fisheyeInstallMode->show();
        ui->label_fisheyeDisplayMode->show();
        ui->comboBox_fisheyeDisplayMode->show();
        ui->label_channelId->show();
        ui->comboBox_channelId->show();

        ui->comboBox_transferMode->setCurrentIndexFromData(m_fishmode_param.fishcorrect);
        ui->comboBox_fisheyeInstallMode->setCurrentIndexFromData(m_fishmode_param.fishmount);
        ui->comboBox_fisheyeDisplayMode->setCurrentIndexFromData(m_fishmode_param.fishdisplay);
        //channel id
    }
}

void CameraEditTabSettings::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_TEST_IPCCONNECT:
    case RESPONSE_FLAG_TRY_TEST_IPCCONNECT:
        ON_RESPONSE_FLAG_TEST_IPCCONNECT(message);
        message->accept();
        break;
    case RESPONSE_FLAG_CHECK_IPC_FISHEYE_INFO:
        ON_RESPONSE_FLAG_CHECK_IPC_FISHEYE_INFO(message);
        message->accept();
        break;
    case RESPONSE_FLAG_GET_FISHEYE_MODE:
        ON_RESPONSE_FLAG_GET_FISHEYE_MODE(message);
        message->accept();
        break;
    case RESPONSE_FLAG_SET_FISHEYE_MODE:
        ON_RESPONSE_FLAG_SET_FISHEYE_MODE(message);
        message->accept();
        break;
    case RESPONSE_FLAG_SET_IPC_FISHEYE_INFO:
        ON_RESPONSE_FLAG_SET_IPC_FISHEYE_INFO(message);
        message->accept();
        break;
    case RESPONSE_FLAG_REMOVE_IPC:
        ON_RESPONSE_FLAG_REMOVE_IPC(message);
        message->accept();
        break;
    case RESPONSE_FLAG_GET_SINGLE_IPCTYPE:
        ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(message);
        message->accept();
        break;
    }
}

void CameraEditTabSettings::ON_RESPONSE_FLAG_TEST_IPCCONNECT(MessageReceive *message)
{
    //MsWaitting::closeGlobalWait();
    if (message->data) {
        QString strMessage;
        m_ipcTestResult = static_cast<IPC_CONN_RES>(*((int *)message->data));
        switch (m_ipcTestResult) {
        case IPC_OK:
            break;
        case IPC_NETWORK_ERR:
            strMessage = GET_TEXT("CHANNELMANAGE/30040", "Network Error.");
            break;
        case IPC_INVALID_USER:
            strMessage = GET_TEXT("CHANNELMANAGE/30041", "User Name or Password Error.");
            break;
        case IPC_UNKNOWN_ERR:
            strMessage = GET_TEXT("CHANNELMANAGE/30042", "Unknown Error.");
            break;
        case IPC_PROTO_NOT_SUPPORT:
            strMessage = GET_TEXT("CHANNELMANAGE/30043", "Protocol not support.");
            break;
        case IPC_OUT_LIMIT_BANDWIDTH:
            strMessage = "Bandwidth reaches the limit.";
            break;
        default:
            strMessage = GET_TEXT("CHANNELMANAGE/30042", "Unknown Error.");
            break;
        }

        if (m_ipcTestResult != IPC_OK) {
            ShowMessageBox(strMessage);
        }
    }
}

void CameraEditTabSettings::ON_RESPONSE_FLAG_CHECK_IPC_FISHEYE_INFO(MessageReceive *message)
{
    memset(&m_fisheye_info, 0, sizeof(m_fisheye_info));
    memcpy(&m_fisheye_info, message->data, sizeof(m_fisheye_info));
    m_eventLoop.exit();
}

void CameraEditTabSettings::ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message)
{
    memset(&m_fishmode_param, 0, sizeof(m_fishmode_param));
    memcpy(&m_fishmode_param, message->data, sizeof(m_fishmode_param));
    m_eventLoop.exit();
}

void CameraEditTabSettings::ON_RESPONSE_FLAG_SET_FISHEYE_MODE(MessageReceive *message)
{
    Q_UNUSED(message)
    m_eventLoop.exit();
}

void CameraEditTabSettings::ON_RESPONSE_FLAG_SET_IPC_FISHEYE_INFO(MessageReceive *message)
{
    int result = *(int *)(message->data);
    qMsDebug() << "result:" << result;
    m_eventLoop.exit();
}

void CameraEditTabSettings::ON_RESPONSE_FLAG_REMOVE_IPC(MessageReceive *message)
{
    Q_UNUSED(message)
    m_eventLoop.exit();
}

void CameraEditTabSettings::ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(MessageReceive *message)
{
    CAM_MODEL_INFO *cam_model_info = static_cast<CAM_MODEL_INFO *>(message->data);
    if (cam_model_info) {
        qDebug() << "channel:" << m_channel_previous
                 << "model channel:" << cam_model_info->chnid
                 << "model type:" << cam_model_info->model;
        qMsNvr->updateSingleCameraRealModel(m_channel_previous, cam_model_info);
    }
    m_eventLoop.exit();
}

void CameraEditTabSettings::testIpcConnect()
{
    struct req_try_test_ipcconnect test_ipcconnect;
    memset(&test_ipcconnect, 0, sizeof(test_ipcconnect));
    test_ipcconnect.chan_id = m_channel_previous;
    snprintf(test_ipcconnect.ip, sizeof(test_ipcconnect.ip), "%s", ui->lineEdit_ip->text().toStdString().c_str());
    test_ipcconnect.port = ui->lineEdit_port->text().toInt();
    snprintf(test_ipcconnect.user, sizeof(test_ipcconnect.user), "%s", ui->lineEdit_userName->text().toStdString().c_str());
    snprintf(test_ipcconnect.password, sizeof(test_ipcconnect.password), "%s", ui->lineEdit_password->text().toStdString().c_str());
    test_ipcconnect.protocol = ui->comboBox_protocol->currentData().toInt();

    sendMessage(REQUEST_FLAG_TRY_TEST_IPCCONNECT, (void *)&test_ipcconnect, sizeof(test_ipcconnect));
}

bool CameraEditTabSettings::isInputValid()
{
    bool valid = true;
    valid &= ui->lineEdit_channelName->checkValid();
    switch (ui->comboBox_protocol->currentIntData()) {
    case IPC_PROTOCOL_RTSP:
        valid &= ui->lineEdit_primary->checkValid();
        if (!ui->lineEdit_secondary->isEmpty()) {
            valid &= ui->lineEdit_secondary->checkValid();
        }
        break;
    case IPC_PROTOCOL_MSDOMAIN: {
        QRegExp rx("(^ddns\\.milesight\\.com\\/)([0-9a-fA-F]{12})");
        if (rx.indexIn(ui->lineEdit_ip->text()) == -1) {
            ui->lineEdit_ip->setCustomValid(false, GET_TEXT("MYLINETIP/112001", "Invalid."));
            valid = false;
        }
        break;
    }
    default:
        valid &= ui->lineEdit_ip->checkValid();
        valid &= ui->lineEdit_port->checkValid();
        break;
    }
    valid &= ui->lineEdit_userName->checkValid();
    valid &= ui->lineEdit_password->checkValid();
    if (ui->comboBox_transportProtocol->currentData() == TRANSPROTOCOL_ROH) {
        valid &= ui->lineEditHTTPSPort->checkValid();
    }
    if (!valid) {
        return false;
    }

    //
    if (ui->comboBox_transferMode->isVisible() && ui->comboBox_transferMode->currentText().isEmpty()) {
        ShowMessageBox(GET_TEXT("CHANNELMANAGE/30072", "Invalid Transfer Mode."));
        ui->comboBox_transferMode->setFocus();
        return false;
    }
    if (ui->comboBox_fisheyeInstallMode->isVisible() && ui->comboBox_fisheyeInstallMode->currentText().isEmpty()) {
        ShowMessageBox(GET_TEXT("CHANNELMANAGE/30073", "Invalid On-board Installation Mode."));
        ui->comboBox_fisheyeInstallMode->setFocus();
        return false;
    }
    if (ui->comboBox_fisheyeDisplayMode->isVisible() && ui->comboBox_fisheyeDisplayMode->currentText().isEmpty()) {
        ShowMessageBox(GET_TEXT("CHANNELMANAGE/30074", "Invalid On-board Display Mode."));
        ui->comboBox_fisheyeDisplayMode->setFocus();
        return false;
    }
    if (ui->comboBox_channelId->isVisible() && ui->comboBox_channelId->currentText().isEmpty()) {
        ShowMessageBox(GET_TEXT("CHANNELMANAGE/30075", "Invalid Channel ID."));
        ui->comboBox_channelId->setFocus();
        return false;
    }

    return true;
}

void CameraEditTabSettings::copyCameraSchedule(int channel_previous, int channel_result)
{
    qDebug()<<"[gsjt debug] write to motion effective from tab settings by channel:"<<channel_result;
    //sched
    smart_event_schedule eventSchedule;
    memset(&eventSchedule, 0, sizeof(smart_event_schedule));
    read_motion_effective_schedule(SQLITE_FILE_NAME, &eventSchedule, channel_previous);
    write_motion_effective_schedule(SQLITE_FILE_NAME, &eventSchedule, channel_result);

    memset(&eventSchedule, 0, sizeof(smart_event_schedule));
    read_smart_event_effective_schedule(SQLITE_FILE_NAME, &eventSchedule, channel_previous, REGIONIN);
    write_smart_event_effective_schedule(SQLITE_FILE_NAME, &eventSchedule, channel_result, REGIONIN);

    memset(&eventSchedule, 0, sizeof(smart_event_schedule));
    read_smart_event_effective_schedule(SQLITE_FILE_NAME, &eventSchedule, channel_previous, REGIONOUT);
    write_smart_event_effective_schedule(SQLITE_FILE_NAME, &eventSchedule, channel_result, REGIONOUT);

    memset(&eventSchedule, 0, sizeof(smart_event_schedule));
    read_smart_event_effective_schedule(SQLITE_FILE_NAME, &eventSchedule, channel_previous, ADVANCED_MOTION);
    write_smart_event_effective_schedule(SQLITE_FILE_NAME, &eventSchedule, channel_result, ADVANCED_MOTION);

    memset(&eventSchedule, 0, sizeof(smart_event_schedule));
    read_smart_event_effective_schedule(SQLITE_FILE_NAME, &eventSchedule, channel_previous, TAMPER);
    write_smart_event_effective_schedule(SQLITE_FILE_NAME, &eventSchedule, channel_result, TAMPER);

    memset(&eventSchedule, 0, sizeof(smart_event_schedule));
    read_smart_event_effective_schedule(SQLITE_FILE_NAME, &eventSchedule, channel_previous, LINECROSS);
    write_smart_event_effective_schedule(SQLITE_FILE_NAME, &eventSchedule, channel_result, LINECROSS);

    memset(&eventSchedule, 0, sizeof(smart_event_schedule));
    read_smart_event_effective_schedule(SQLITE_FILE_NAME, &eventSchedule, channel_previous, LOITERING);
    write_smart_event_effective_schedule(SQLITE_FILE_NAME, &eventSchedule, channel_result, LOITERING);

    memset(&eventSchedule, 0, sizeof(smart_event_schedule));
    read_smart_event_effective_schedule(SQLITE_FILE_NAME, &eventSchedule, channel_previous, HUMAN);
    write_smart_event_effective_schedule(SQLITE_FILE_NAME, &eventSchedule, channel_result, HUMAN);

    memset(&eventSchedule, 0, sizeof(smart_event_schedule));
    read_smart_event_effective_schedule(SQLITE_FILE_NAME, &eventSchedule, channel_previous, PEOPLE_CNT);
    write_smart_event_effective_schedule(SQLITE_FILE_NAME, &eventSchedule, channel_result, PEOPLE_CNT);
}

bool CameraEditTabSettings::isForbidSameChannelExist()
{
    if (qMsNvr->isSameChannelCanExist()) {
        return false;
    }

    QString strIP = ui->lineEdit_ip->text();
    for (int i = 0; i < MAX_CAMERA && i < qMsNvr->maxChannel(); ++i) {
        const camera &cam = qMsNvr->databaseCamera(i);
        if (cam.id == m_channel_previous)
            continue;
        if (!cam.enable)
            continue;
        if (cam.camera_protocol == IPC_PROTOCOL_RTSP)
            continue;
        if (QString(cam.ip_addr) == strIP)
            return true;
    }

    return false;
}

void CameraEditTabSettings::updateFisheyeInstallMode()
{
    ui->comboBox_fisheyeInstallMode->setCurrentIndexFromData(m_fisheye_info.installation);
    updateFisheyeDisplayMode();
}

void CameraEditTabSettings::updateFisheyeDisplayMode()
{
    ui->comboBox_fisheyeDisplayMode->clear();
    int transferMode = ui->comboBox_transferMode->currentData().toInt();
    int installationMode = ui->comboBox_fisheyeInstallMode->currentData().toInt();
    if (transferMode == 0) {
        //multi
        switch (installationMode) {
        case MsQt::FISHEYE_INSTALL_CEILING:
            ui->comboBox_fisheyeDisplayMode->addItem("1O", MsQt::FISHEYE_DISPLAY_1O);
            ui->comboBox_fisheyeDisplayMode->addItem("1P", MsQt::FISHEYE_DISPLAY_1P);
            ui->comboBox_fisheyeDisplayMode->addItem("2P", MsQt::FISHEYE_DISPLAY_2P);
            ui->comboBox_fisheyeDisplayMode->addItem("4R", MsQt::FISHEYE_DISPLAY_4R);
            ui->comboBox_fisheyeDisplayMode->addItem("1O3R", MsQt::FISHEYE_DISPLAY_1O3R);
            ui->comboBox_fisheyeDisplayMode->addItem("1P3R", MsQt::FISHEYE_DISPLAY_1P3R);
            ui->comboBox_fisheyeDisplayMode->addItem("1O1P3R", MsQt::FISHEYE_DISPLAY_1O1P3R);
            break;
        case MsQt::FISHEYE_INSTALL_WALL:
            ui->comboBox_fisheyeDisplayMode->addItem("1O", MsQt::FISHEYE_DISPLAY_1O);
            ui->comboBox_fisheyeDisplayMode->addItem("1P", MsQt::FISHEYE_DISPLAY_1P);
            ui->comboBox_fisheyeDisplayMode->addItem("4R", MsQt::FISHEYE_DISPLAY_4R);
            ui->comboBox_fisheyeDisplayMode->addItem("1O3R", MsQt::FISHEYE_DISPLAY_1O3R);
            ui->comboBox_fisheyeDisplayMode->addItem("1P3R", MsQt::FISHEYE_DISPLAY_1P3R);
            ui->comboBox_fisheyeDisplayMode->addItem("1O1P3R", MsQt::FISHEYE_DISPLAY_1O1P3R);
            break;
        case MsQt::FISHEYE_INSTALL_FLAT:
            ui->comboBox_fisheyeDisplayMode->addItem("1O", MsQt::FISHEYE_DISPLAY_1O);
            ui->comboBox_fisheyeDisplayMode->addItem("1P", MsQt::FISHEYE_DISPLAY_1P);
            ui->comboBox_fisheyeDisplayMode->addItem("2P", MsQt::FISHEYE_DISPLAY_2P);
            ui->comboBox_fisheyeDisplayMode->addItem("4R", MsQt::FISHEYE_DISPLAY_4R);
            ui->comboBox_fisheyeDisplayMode->addItem("1O3R", MsQt::FISHEYE_DISPLAY_1O3R);
            ui->comboBox_fisheyeDisplayMode->addItem("1P3R", MsQt::FISHEYE_DISPLAY_1P3R);
            ui->comboBox_fisheyeDisplayMode->addItem("1O1P3R", MsQt::FISHEYE_DISPLAY_1O1P3R);
            break;
        }
    } else if (transferMode == 1) {
        //bundle
        switch (installationMode) {
        case MsQt::FISHEYE_INSTALL_CEILING:
            ui->comboBox_fisheyeDisplayMode->addItem("1O", MsQt::FISHEYE_DISPLAY_1O);
            ui->comboBox_fisheyeDisplayMode->addItem("1P", MsQt::FISHEYE_DISPLAY_1P);
            ui->comboBox_fisheyeDisplayMode->addItem("2P", MsQt::FISHEYE_DISPLAY_2P);
            ui->comboBox_fisheyeDisplayMode->addItem("4R", MsQt::FISHEYE_DISPLAY_4R);
            ui->comboBox_fisheyeDisplayMode->addItem("1O3R", MsQt::FISHEYE_DISPLAY_1O3R);
            ui->comboBox_fisheyeDisplayMode->addItem("1P3R", MsQt::FISHEYE_DISPLAY_1P3R);
            break;
        case MsQt::FISHEYE_INSTALL_WALL:
            ui->comboBox_fisheyeDisplayMode->addItem("1O", MsQt::FISHEYE_DISPLAY_1O);
            ui->comboBox_fisheyeDisplayMode->addItem("1P", MsQt::FISHEYE_DISPLAY_1P);
            ui->comboBox_fisheyeDisplayMode->addItem("4R", MsQt::FISHEYE_DISPLAY_4R);
            ui->comboBox_fisheyeDisplayMode->addItem("1O3R", MsQt::FISHEYE_DISPLAY_1O3R);
            ui->comboBox_fisheyeDisplayMode->addItem("1P3R", MsQt::FISHEYE_DISPLAY_1P3R);
            break;
        case MsQt::FISHEYE_INSTALL_FLAT:
            ui->comboBox_fisheyeDisplayMode->addItem("1O", MsQt::FISHEYE_DISPLAY_1O);
            ui->comboBox_fisheyeDisplayMode->addItem("1P", MsQt::FISHEYE_DISPLAY_1P);
            ui->comboBox_fisheyeDisplayMode->addItem("2P", MsQt::FISHEYE_DISPLAY_2P);
            ui->comboBox_fisheyeDisplayMode->addItem("4R", MsQt::FISHEYE_DISPLAY_4R);
            ui->comboBox_fisheyeDisplayMode->addItem("1O3R", MsQt::FISHEYE_DISPLAY_1O3R);
            ui->comboBox_fisheyeDisplayMode->addItem("1P3R", MsQt::FISHEYE_DISPLAY_1P3R);
            break;
        }
    }
}

void CameraEditTabSettings::updateFisheyeChannelId()
{
    int transMode = ui->comboBox_transferMode->currentData().toInt();
    int displayMode = ui->comboBox_fisheyeDisplayMode->currentData().toInt();
    int idCount = 0;
    if (transMode == 1) {
        idCount = 1;
    } else {
        switch (displayMode) {
        case MsQt::FISHEYE_DISPLAY_1O:
        case MsQt::FISHEYE_DISPLAY_1P:
        case MsQt::FISHEYE_DISPLAY_2P:
            idCount = 1;
            break;
        case MsQt::FISHEYE_DISPLAY_4R:
        case MsQt::FISHEYE_DISPLAY_1O3R:
        case MsQt::FISHEYE_DISPLAY_1P3R:
            idCount = 4;
            break;
        case MsQt::FISHEYE_DISPLAY_1O1P3R:
            idCount = 5;
            break;
        default:
            qMsWarning() << "invalid display mode:" << m_fisheye_info.display;
            break;
        }
    }
    ui->comboBox_channelId->clear();
    for (int i = 0; i < idCount; ++i) {
        ui->comboBox_channelId->addItem(QString("%1").arg(i + 1), i + 1);
    }
}

void CameraEditTabSettings::on_comboBox_transferMode_indexSet(int index)
{
    Q_UNUSED(index)

    //MSHN-8887 QT-Camera Management：Camera Edit 鱼眼通道Transfer改变时，对应Installation Mode需变更为Ceiling
    ui->comboBox_fisheyeInstallMode->setCurrentIndexFromData(MsQt::FISHEYE_INSTALL_CEILING);
    updateFisheyeChannelId();
}

void CameraEditTabSettings::on_comboBox_fisheyeInstallMode_indexSet(int index)
{
    Q_UNUSED(index)
    updateFisheyeDisplayMode();
    updateFisheyeChannelId();
}

void CameraEditTabSettings::on_comboBox_fisheyeDisplayMode_indexSet(int index)
{
    Q_UNUSED(index)
    updateFisheyeChannelId();
}

int CameraEditTabSettings::saveSettings()
{
    return 0;
}

void CameraEditTabSettings::lineEditIPShow(bool enable)
{
    if (enable) {
        ui->label_primary->hide();
        ui->label_secondary->hide();
        ui->lineEdit_primary->hide();
        ui->lineEdit_secondary->hide();

        ui->label_ip->show();
        ui->label_port->show();
        ui->lineEdit_ip->show();
        ui->lineEdit_port->show();

    } else {
        ui->label_ip->hide();
        ui->label_port->hide();
        ui->lineEdit_ip->hide();
        ui->lineEdit_port->hide();

        ui->label_primary->show();
        ui->label_secondary->show();
        ui->lineEdit_primary->show();
        ui->lineEdit_secondary->show();
    }
}

void CameraEditTabSettings::onLanguageChanged()
{
    //
    ui->label_channel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->label_channelName->setText(GET_TEXT("CHANNELMANAGE/30009", "Channel Name"));
    ui->label_protocol->setText(GET_TEXT("CHANNELMANAGE/30014", "Protocol"));
    ui->label_ip->setText(GET_TEXT("COMMON/1033", "IP Address"));
    ui->label_transportProtocol->setText(GET_TEXT("CHANNELMANAGE/30015", "Transport Protocol"));
    ui->label_transferMode->setText(GET_TEXT("FISHEYE/12013", "Transfer Mode"));
    ui->label_fisheyeInstallMode->setText(GET_TEXT("FISHEYE/12010", "On-board Installation Mode"));
    ui->label_fisheyeDisplayMode->setText(GET_TEXT("FISHEYE/12011", "On-board Display Mode"));
    ui->label_channelId->setText(GET_TEXT("FISHEYE/12016", "Channel ID"));
    ui->label_userName->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->label_password->setText(GET_TEXT("COMMON/1008", "Password"));
    ui->label_timeSetting->setText(GET_TEXT("CHANNELMANAGE/30018", "Time Setting"));
    ui->checkBox_timeSetting->setText(GET_TEXT("CHANNELMANAGE/30019", "Sync Time With NVR"));
    ui->label_primary->setText(GET_TEXT("CHANNELMANAGE/30024", "primary"));
    ui->label_secondary->setText(GET_TEXT("CHANNELMANAGE/30017", "secondary"));
    ui->labelHTTPSPort->setText(GET_TEXT("SYSTEMNETWORK/71059", "HTTPS Port"));

    //
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
    ui->pushButton_applly->setText(GET_TEXT("COMMON/1003", "Apply"));
}

void CameraEditTabSettings::on_comboBox_protocol_activated(int index)
{
    const IPC_PROTOCOL &protocol = static_cast<IPC_PROTOCOL>(ui->comboBox_protocol->itemData(index).toInt());
    if (m_perProtocol == IPC_PROTOCOL_ONVIF || m_perProtocol == IPC_PROTOCOL_MILESIGHT) {
        m_ipAddress = ui->lineEdit_ip->text();
    }
    int transportProtocolIndex = ui->comboBox_transportProtocol->currentIndex();
    ui->widgetSpacer->hide();
    ui->comboBox_transportProtocol->removeItemFromData(TRANSPROTOCOL_ROH);
    switch (protocol) {
    case IPC_PROTOCOL_ONVIF:
        ui->label_ip->setText(GET_TEXT("COMMON/1033", "IP Address"));
        ui->lineEdit_ip->setText(m_ipAddress);
        lineEditIPShow(true);
        ui->comboBox_transportProtocol->removeItemFromData(TRANSPROTOCOL_ROH);
        break;
    case IPC_PROTOCOL_MILESIGHT:
        ui->label_ip->setText(GET_TEXT("COMMON/1033", "IP Address"));
        ui->lineEdit_ip->setText(m_ipAddress);
        lineEditIPShow(true);
        ui->comboBox_transportProtocol->addItem(GET_TEXT("CHANNELMANAGE/174000", "Encryption"), TRANSPROTOCOL_ROH);
        break;
    case IPC_PROTOCOL_RTSP:
        lineEditIPShow(false);
        ui->comboBox_transportProtocol->removeItemFromData(TRANSPROTOCOL_ROH);
        break;
    case IPC_PROTOCOL_MSDOMAIN:
        lineEditIPShow(true);
        ui->lineEdit_ip->setText(m_msddns);
        ui->label_ip->setText(GET_TEXT("CAMERA/143000", "Domain Address"));
        ui->label_port->hide();
        ui->lineEdit_port->hide();
        ui->widgetSpacer->show();
        ui->comboBox_transportProtocol->removeItemFromData(TRANSPROTOCOL_ROH);
        break;
    default:
        break;
    }
    if (transportProtocolIndex == TRANSPROTOCOL_ROH && ui->comboBox_transportProtocol->count() < TRANSPROTOCOL_MAX) {
        ui->comboBox_transportProtocol->setCurrentIndexFromData(TRANSPROTOCOL_AUTO);
    }
    on_comboBox_transportProtocol_activated(ui->comboBox_transportProtocol->currentIntData());
    m_perProtocol = protocol;
}

void CameraEditTabSettings::on_comboBox_channel_activated(int index)
{
    int channel = ui->comboBox_channel->itemData(index).toInt();
    ui->lineEdit_channelName->setText(qMsNvr->channelName(channel));
}

void CameraEditTabSettings::on_pushButton_ok_clicked()
{
    int ret = saveSettings();
    if (ret == 0) {
        //log
        struct camera cam;
        qMsNvr->readDatabaseCamera(&cam, m_channel_previous);
        qMsNvr->writeLog(cam, SUB_OP_CONFIG_LOCAL, SUB_PARAM_EDIT_IPC);
        //
        m_edit->accept();
    }
}

void CameraEditTabSettings::on_pushButton_cancel_clicked()
{
#if 0
    ui->comboBox_frameSize_main->clear();
    ui->comboBox_frameRate_main->clear();
    ui->comboBox_bitRate_main->clear();
    ui->comboBox_frameInterval_main->clear();
    ui->comboBox_frameSize_sub->clear();
    ui->comboBox_frameRate_sub->clear();
    ui->comboBox_bitRate_sub->clear();
    ui->comboBox_frameInterval_sub->clear();
#endif
    m_edit->reject();
}

void CameraEditTabSettings::on_pushButton_applly_clicked()
{
    int ret = saveSettings();
    if (ret == 0) {
        //log
        struct camera cam;
        qMsNvr->readDatabaseCamera(&cam, m_channel_previous);
        qMsNvr->writeLog(cam, SUB_OP_CONFIG_LOCAL, SUB_PARAM_EDIT_IPC);
    }
}

void CameraEditTabSettings::on_lineEdit_ip_textEdited(const QString &arg1)
{
    if (ui->comboBox_protocol->currentIntData() == IPC_PROTOCOL_MSDOMAIN) {
        QRegExp rx("(^ddns\\.milesight\\.com\\/)(.{0,12})");
        if (rx.indexIn(arg1) == -1) {
            ui->lineEdit_ip->setText(m_msddns);
            return;
        }
        QString strIP = rx.cap(2).toUpper();
        m_msddns = rx.cap(1) + strIP;
        ui->lineEdit_ip->setText(m_msddns);
    }
}

void CameraEditTabSettings::on_lineEdit_primary_textEdited(const QString &arg1)
{
    if (ui->comboBox_protocol->currentIntData() == IPC_PROTOCOL_RTSP) {
        QRegExp rx("(^rtsp:\\/\\/)");
        if (rx.indexIn(arg1) == -1) {
            ui->lineEdit_primary->setText(m_primary);
            return;
        }
        m_primary = arg1;
    }
}

void CameraEditTabSettings::on_lineEdit_secondary_textEdited(const QString &arg1)
{
    if (ui->comboBox_protocol->currentIntData() == IPC_PROTOCOL_RTSP) {
        QRegExp rx("(^rtsp:\\/\\/)");
        if (rx.indexIn(arg1) == -1) {
            ui->lineEdit_secondary->setText(m_secondary);
            return;
        }
        m_secondary = arg1;
    }
}

void CameraEditTabSettings::on_comboBox_transportProtocol_activated(int index)
{
    Q_UNUSED(index)
    if (ui->comboBox_transportProtocol->currentIntData() == TRANSPROTOCOL_ROH) {
        ui->lineEditHTTPSPort->show();
        ui->labelHTTPSPort->show();
        ui->label_port->setText(GET_TEXT("SYSTEMNETWORK/71057", "HTTP Port"));
    } else {
        ui->lineEditHTTPSPort->hide();
        ui->labelHTTPSPort->hide();
        ui->label_port->setText(GET_TEXT("CHANNELMANAGE/30011", "Port"));
    }       
}

