#include "TabPtzAutoTracking.h"
#include "ui_TabPtzAutoTracking.h"
#include "EffectiveTimePtzAutoTracking.h"
#include "FaceData.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "channelcopydialog.h"
#include "drawmotion.h"
#include <qmath.h>

TabPtzAutoTracking::TabPtzAutoTracking(QWidget *parent)
    : PtzBasePage(parent)
    , ui(new Ui::PtzAutoTrackingPage)
{
    ui->setupUi(this);

    ui->channelGroup->setCount(qMsNvr->maxChannel());
    connect(ui->channelGroup, SIGNAL(buttonClicked(int)), this, SLOT(onChannelGroupClicked(int)));

    ui->comboBox_zoomRatioMode->clear();
    ui->comboBox_zoomRatioMode->addItem(GET_TEXT("PTZCONFIG/36029", "Auto Mode"), 0);
    ui->comboBox_zoomRatioMode->addItem(GET_TEXT("PTZCONFIG/36030", "Customize"), 1);
    ui->lineEdit_time->setCheckMode(MyLineEdit::RangeCheck, 5, 300);

    ui->labelTrackingObject->setVisible(false);
    ui->widgetTrackingObject->setVisible(false);

    m_regionDraw = new DrawMotion(this);
    ui->video->showDrawWidget(m_regionDraw);

    onLanguageChanged();
}

TabPtzAutoTracking::~TabPtzAutoTracking()
{
    delete ui;
}

void TabPtzAutoTracking::initializeData()
{
    ui->channelGroup->setCurrentIndex(currentChannel());
}

void TabPtzAutoTracking::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_AUTO_TRACKING:
        ON_RESPONSE_FLAG_GET_AUTO_TRACKING(message);
        break;
    case RESPONSE_FLAG_SET_AUTO_TRACKING:
        ON_RESPONSE_FLAG_SET_AUTO_TRACKING(message);
        break;
    case RESPONSE_FLAG_GET_VCA_HUMANDETECTION:
        ON_RESPONSE_FLAG_GET_VCA_HUMANDETECTION(message);
        break;
    case RESPONSE_FLAG_SET_VCA_HUMANDETECTION:
        ON_RESPONSE_FLAG_SET_VCA_HUMANDETECTION(message);
        break;
    case RESPONSE_FLAG_GET_IPC_DIGITPOS_ZOOM:
        ON_RESPONSE_FLAG_GET_IPC_DIGITPOS_ZOOM(message);
        break;
    case RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY:
        ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(message);
        break;
    }
}

int TabPtzAutoTracking::waitForGetAutoTracking()
{
    int channel = currentChannel();
    Q_UNUSED(channel)

    return 1;
}

void TabPtzAutoTracking::ON_RESPONSE_FLAG_GET_AUTO_TRACKING(MessageReceive *message)
{
    m_eventLoop.exit();

    ms_auto_tracking *auto_tracking = (ms_auto_tracking *)message->data;
    memset(&m_auto_tracking, 0, sizeof(ms_auto_tracking));

    if (qMsNvr->isCameraVersionEqualOrGreaterThan(currentChannel(), MsCameraVersion(7, 75))) {
        ui->label_zoomRatioMode->show();
        ui->comboBox_zoomRatioMode->show();
        ui->label_setZoomRatio->show();
        ui->widget_setZoomRatio->show();
    } else {
        ui->label_zoomRatioMode->hide();
        ui->comboBox_zoomRatioMode->hide();
        ui->label_setZoomRatio->hide();
        ui->widget_setZoomRatio->hide();
    }

    if (auto_tracking) {
        memcpy(&m_auto_tracking, auto_tracking, sizeof(ms_auto_tracking));
        qDebug() << "--RESPONSE_FLAG_GET_AUTO_TRACKING--"
                 << "\n----channel:" << m_auto_tracking.chanid
                 << "\n----enable:" << m_auto_tracking.enable
                 << "\n----show:" << m_auto_tracking.show
                 << "\n----sensitivity:" << m_auto_tracking.sensitivity
                 << "\n----time:" << m_auto_tracking.time
                 << "\n----manual_tracking:" << m_auto_tracking.manual_tracking
                 << "\n----zoom_mode:" << m_auto_tracking.zoom_mode
                 << "\n----zoom_number:" << m_auto_tracking.zoom_number
                 << "\n----human&vehicler:" << m_auto_tracking.auto_tracking_object;

        isSupportHumanVehicle();
        ui->checkBox_enable->setChecked(auto_tracking->enable);
        ui->checkBox_show->setChecked(auto_tracking->show);
        on_checkBox_enable_clicked(auto_tracking->enable);
        ui->horizontalSlider_sensitivity->setValue(auto_tracking->sensitivity);
        ui->lineEdit_time->setText(QString("%1").arg(auto_tracking->time));
        ui->comboBox_zoomRatioMode->setCurrentIndexFromData(auto_tracking->zoom_mode);
        m_regionDraw->setRegion(auto_tracking->area);

        //低于8.0.1版本Report或非ai ptz选项不显示
        char version[50];
        //get_channel_version_name(currentChannel(), version, sizeof(version));
        MsCameraVersion cameraVersion(version);
        if (cameraVersion < MsCameraVersion(8, 1) || !m_auto_tracking.system_human_vehicle_support) {
            ui->labelReport->hide();
            ui->checkBoxReport->hide();
        } else {
            ui->labelReport->show();
            ui->checkBoxReport->show();
        }
        ui->checkBoxReport->setChecked(m_auto_tracking.reportMotion);
    }

    if (auto_tracking && auto_tracking->zoom_mode == 0) {
        m_isSetZoomClicked = false;
    } else {
        m_isSetZoomClicked = true;
    }
}

int TabPtzAutoTracking::saveAutoTracking(int channel)
{
    m_auto_tracking.chanid = channel;
    m_auto_tracking.enable = ui->checkBox_enable->isChecked();
    m_auto_tracking.show = ui->checkBox_show->isChecked();
    m_auto_tracking.sensitivity = ui->horizontalSlider_sensitivity->value();
    m_auto_tracking.time = ui->lineEdit_time->text().toInt();
    m_auto_tracking.reportMotion = ui->checkBoxReport->isChecked();
    int zoomMode = ui->comboBox_zoomRatioMode->currentData().toInt();
    if (zoomMode == 0) {
        m_auto_tracking.zoom_mode = 0;
    } else {
        //如果从Auto Mode切换至Customize模式，又未点击Set按钮，则保存时Tracking Zoom Ratio选项恢复成Auto模式
        if (!m_isSetZoomClicked) {
            m_auto_tracking.zoom_mode = 0;
            ui->comboBox_zoomRatioMode->setCurrentIndexFromData(0);
        }
    }
    m_regionDraw->getRegion(m_auto_tracking.area);
    qDebug() << "--REQUEST_FLAG_SET_AUTO_TRACKING--"
             << "\n----channel:" << m_auto_tracking.chanid
             << "\n----enable:" << m_auto_tracking.enable
             << "\n----show:" << m_auto_tracking.show
             << "\n----sensitivity:" << m_auto_tracking.sensitivity
             << "\n----time:" << m_auto_tracking.time
             << "\n----manual_tracking:" << m_auto_tracking.manual_tracking
             << "\n----zoom_mode:" << m_auto_tracking.zoom_mode
             << "\n----zoom_number:" << m_auto_tracking.zoom_number
             << "\n----human&vehicler:" << m_auto_tracking.auto_tracking_object;

    sendMessage(REQUEST_FLAG_SET_AUTO_TRACKING, &m_auto_tracking, sizeof(ms_auto_tracking));

    return 1;
}

void TabPtzAutoTracking::ON_RESPONSE_FLAG_SET_AUTO_TRACKING(MessageReceive *message)
{
    Q_UNUSED(message)

    m_eventLoop.exit();
}

void TabPtzAutoTracking::ON_RESPONSE_FLAG_GET_VCA_HUMANDETECTION(MessageReceive *message)
{
    ms_smart_event_info *info = (ms_smart_event_info *)message->data;
    memset(&m_ms_smart_event_info, 0, sizeof(ms_smart_event_info));
    if (info) {
        memcpy(&m_ms_smart_event_info, info, sizeof(ms_smart_event_info));
    }

    m_eventLoop.exit();
}

void TabPtzAutoTracking::ON_RESPONSE_FLAG_SET_VCA_HUMANDETECTION(MessageReceive *message)
{
    Q_UNUSED(message)

    m_eventLoop.exit();
}

void TabPtzAutoTracking::ON_RESPONSE_FLAG_GET_IPC_DIGITPOS_ZOOM(MessageReceive *message)
{
    ms_digitpos_zoom_state *zoom_state = static_cast<ms_digitpos_zoom_state *>(message->data);
    memset(&m_zoom_state, 0, sizeof(ms_digitpos_zoom_state));
    if (!zoom_state) {
        qWarning() << QString("PtzAutoTrackingPage::ON_RESPONSE_FLAG_GET_IPC_DIGITPOS_ZOOM, data is null.");
    } else {
        memcpy(&m_zoom_state, zoom_state, sizeof(ms_digitpos_zoom_state));
        qDebug() << QString("PtzAutoTrackingPage::ON_RESPONSE_FLAG_GET_IPC_DIGITPOS_ZOOM, chnid: %1, state: %2, zoomNumber: %3")
                        .arg(zoom_state->chnid)
                        .arg(zoom_state->state)
                        .arg(zoom_state->zoomNumber);
    }
    m_eventLoop.exit();
}

void TabPtzAutoTracking::ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(MessageReceive *message)
{
    resp_image_display *data = (resp_image_display *)message->data;
    memset(&m_image_display, 0, sizeof(resp_image_display));
    if (!data) {
        qMsWarning() << "data is null.";
        m_eventLoop.exit(-1);
        return;
    }
    memcpy(&m_image_display, data, sizeof(resp_image_display));
    qMsDebug() << "zoom_limit:" << m_image_display.image.zoom_limit;

    m_eventLoop.exit();
}

void TabPtzAutoTracking::setSettingEnable(bool enable)
{
    if (!enable) {
        ui->ptzControlPanel->clearPreset();
        m_regionDraw->setEnabled(false);
        ui->labelReport->hide();
        ui->checkBoxReport->hide();
    }
    ui->ptzControlPanel->setEnabled(enable);
    ui->widget_maskContent->setEnabled(enable);
    ui->pushButton_copy->setEnabled(enable);
    ui->pushButton_apply->setEnabled(enable);
}

void TabPtzAutoTracking::clearSetting()
{
    ui->checkBox_enable->setChecked(false);
    ui->checkBox_show->setChecked(false);
    ui->checkBoxReport->setChecked(false);
}

void TabPtzAutoTracking::isSupportHumanVehicle()
{
    //AI机型支持system_human_vehicle_support
    bool isSupport = m_auto_tracking.system_human_vehicle_support;
    ui->labelTrackingObject->setVisible(isSupport);
    ui->widgetTrackingObject->setVisible(isSupport);

    if (isSupport) {
        int humanChecked = m_auto_tracking.auto_tracking_object & (1 << 0);
        ui->checkBoxHuman->setChecked(humanChecked);

        int vehicleChecked = m_auto_tracking.auto_tracking_object & (1 << 1);
        ui->checkBoxVehicle->setChecked(vehicleChecked);
    }
}

void TabPtzAutoTracking::onLanguageChanged()
{
    ui->ptzControlPanel->onLanguageChanged();

    ui->label_enable->setText(GET_TEXT("PTZCONFIG/36052", "Auto Tracking"));
    ui->checkBox_enable->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->label_enable_2->setText(GET_TEXT("PTZCONFIG/36057", "Show Tracking"));
    ui->checkBox_show->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->labelRegion->setText(GET_TEXT("ANPR/103021", "Region"));
    ui->pushButtonSetAll->setText(GET_TEXT("MOTION/51004", "Set All"));
    ui->pushButtonDeleteAll->setText(GET_TEXT("PLAYBACK/80119", "Delete All"));
    ui->label_sensitivity->setText(GET_TEXT("MOTION/51003", "Sensitivity"));
    ui->label_time->setText(GET_TEXT("PTZCONFIG/36058", "Max. Tracking Time"));
    ui->label_zoomRatioMode->setText(GET_TEXT("PTZCONFIG/36027", "Tracking Zoom Ratio"));
    ui->label_setZoomRatio->setText(GET_TEXT("PTZCONFIG/36028", "Tracking Zoom Ratio Settings"));
    ui->pushButton_setZoomRatio->setText(GET_TEXT("PTZDIALOG/21021", "Set"));
    ui->label_schedule->setText(GET_TEXT("PTZCONFIG/36060", "Auto Tracking Schedule"));
    ui->pushButton_editSchedule->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->label_note->setText(GET_TEXT("PTZCONFIG/36031", "Please set the tracking zoom ratio by adjusting the zoom button."));

    ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->labelTrackingObject->setText(GET_TEXT("IMAGE/120008", "Tracking Object"));
    ui->checkBoxHuman->setText(GET_TEXT("TARGETMODE/103201", "Human"));
    ui->checkBoxVehicle->setText(GET_TEXT("TARGETMODE/103202", "Vehicle"));

    ui->labelReport->setText(GET_TEXT("PTZCONFIG/148000", "Report to Motion Detection"));
    ui->checkBoxReport->setText(GET_TEXT("COMMON/1009", "Enable"));
}

void TabPtzAutoTracking::onChannelGroupClicked(int channel)
{
    setCurrentChannel(channel);
    ui->video->playVideo(channel);

    ui->ptzControlPanel->setCurrentChannel(channel);
    if (!LiveView::instance()->isChannelConnected(channel)) {
        clearSetting();
        setSettingEnable(false);
        ui->labelReport->show();
        ui->checkBoxReport->show();
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
        return;
    }

    struct ipc_system_info system_info;
    memset(&system_info, 0, sizeof(system_info));
    //get_ipc_system_info(channel, &system_info);

    //showWait();
    if (system_info.system_auto_tracking_support == -1) {
        if (!qMsNvr->isMsCamera(channel)) {
            //closeWait();
            clearSetting();
            setSettingEnable(false);
            ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
            return;
        }

        gPtzDataManager->beginGetData(channel);
        gPtzDataManager->waitForGetCameraModelType();
        if (qMsNvr->isFisheye(channel)) {
            //closeWait();
            clearSetting();
            setSettingEnable(false);
            ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
            return;
        }
        gPtzDataManager->waitForGetCameraModelInfo();
        gPtzDataManager->waitForGetPtzSupport();
        if (!gPtzDataManager->isPtzSupport()) {
            //closeWait();
            clearSetting();
            setSettingEnable(false);
            ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
            return;
        }
        if (!gPtzDataManager->isPresetEnable()) {
            //closeWait();
            clearSetting();
            setSettingEnable(false);
            ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
            return;
        }
    } else if (system_info.system_auto_tracking_support == 0) {
        //closeWait();
        clearSetting();
        setSettingEnable(false);
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        return;
    } else {
        gPtzDataManager->beginGetData(channel);
        gPtzDataManager->waitForGetCameraModelInfo();
    }

    ui->widgetMessage->hide();
    setSettingEnable(true);

    gPtzDataManager->waitForGetPtzOvfInfo();
    resp_ptz_ovf_info *ptz_ovf_info = gPtzDataManager->ptzOvfInfo();
    ui->ptzControlPanel->showPresetData(ptz_ovf_info->preset, gPtzDataManager->maxPresetCount(channel));

    gPtzDataManager->waitForGetPtzSpeed();
    ui->ptzControlPanel->editSpeedValue(gPtzDataManager->ptzSpeed());

    gPtzDataManager->waitForGetAutoScan();
    ui->ptzControlPanel->setAutoScanChecked(gPtzDataManager->autoScan());

    waitForGetAutoTracking();

    sendMessage(REQUEST_FLAG_GET_IPCIMAGE_DISPLAY, &channel, sizeof(int));
    //m_eventLoop.exec();

    //closeWait();
}

void TabPtzAutoTracking::on_pushButton_copy_clicked()
{
    if (ui->checkBox_enable->isChecked() && !ui->lineEdit_time->checkValid()) {
        return;
    }
    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(currentChannel());
    int result = copy.exec();
    if (result == QDialog::Accepted) {
        QList<int> channelList = copy.checkedList();
        //
        //showWait();
        saveAutoTracking(currentChannel());
        for (int i = 0; i < channelList.size(); ++i) {
            int channel = channelList.at(i);
            if (channel != currentChannel()) {
                saveAutoTracking(channel);
            }
        }
        //closeWait();
    }
}

void TabPtzAutoTracking::on_pushButton_apply_clicked()
{
    if (ui->checkBox_enable->isChecked() && !ui->lineEdit_time->checkValid()) {
        return;
    }
    int channel = currentChannel();
    if (ui->checkBox_enable->isChecked() && ui->checkBoxVehicle->isVisible() && m_auto_tracking.auto_tracking_object == 0) {
        const int result = MessageBox::question(this, GET_TEXT("IMAGE/120009", "Auto Tracking will not work if Tracking Object is not selected, continue?"));
        if (result == MessageBox::Cancel) {
            return;
        }
    }

    //showWait();
    //和vca互斥
    if (ui->checkBox_enable->isChecked()) {
        sendMessage(REQUEST_FLAG_GET_VCA_HUMANDETECTION, &channel, sizeof(int));
        //m_eventLoop.exec();
        if (m_ms_smart_event_info.enable) {
            //closeWait();
            MessageBox::Result result = MessageBox::question(this, GET_TEXT("SMARTEVENT/55064", "VCA will be disabled, confirm to continue?"));
            if (result == MessageBox::Yes) {
                //showWait();
                m_ms_smart_event_info.enable = false;
                sendMessage(REQUEST_FLAG_SET_VCA_HUMANDETECTION, &m_ms_smart_event_info, sizeof(ms_smart_event_info));
                //m_eventLoop.exec();
            } else {
                return;
            }
        }
        //和人脸互斥
        gFaceData.getFaceConfig(currentChannel());
        if (gFaceData.isFaceConflict()) {
            const int &result = MessageBox::question(this, GET_TEXT("FACE/141053", "Face Detection will be disabled, continue?"));
            if (result == MessageBox::Yes) {
                gFaceData.setFaceDisable();
            } else {
                //closeWait();
                return;
            }
        }
    }

    saveAutoTracking(channel);
    //closeWait();
}

void TabPtzAutoTracking::on_pushButton_back_clicked()
{
    back();
}

void TabPtzAutoTracking::on_checkBox_enable_clicked(bool checked)
{
    if (checked == false) {
        ui->checkBox_show->setChecked(false);
    }

    ui->checkBox_show->setEnabled(checked);

    m_regionDraw->setEnabled(checked);
    ui->pushButtonSetAll->setEnabled(checked);
    ui->pushButtonDeleteAll->setEnabled(checked);

    ui->horizontalSlider_sensitivity->setEnabled(checked);
    ui->lineEdit_time->setEnabled(checked);
    ui->comboBox_zoomRatioMode->setEnabled(checked);
    ui->pushButton_editSchedule->setEnabled(checked);
    ui->pushButton_setZoomRatio->setEnabled(checked);
    ui->checkBoxHuman->setEnabled(checked);
    ui->checkBoxVehicle->setEnabled(checked);
    ui->checkBoxReport->setEnabled(checked);

    MsCameraVersion cameraVersion = MsCameraVersion::fromChannel(currentChannel());
    if (cameraVersion < MsCameraVersion(7, 78)) {
        m_regionDraw->setEnabled(false);
        ui->pushButtonSetAll->setEnabled(false);
        ui->pushButtonDeleteAll->setEnabled(false);
    }
}

void TabPtzAutoTracking::on_comboBox_zoomRatioMode_indexSet(int index)
{
    int value = ui->comboBox_zoomRatioMode->itemData(index).toInt();
    if (value == 0) {
        ui->label_setZoomRatio->hide();
        ui->widget_setZoomRatio->hide();
        ui->label_note->hide();
    } else {
        ui->label_setZoomRatio->show();
        ui->widget_setZoomRatio->show();
        ui->label_note->show();
    }
}

void TabPtzAutoTracking::on_comboBox_zoomRatioMode_activated(int index)
{
    on_comboBox_zoomRatioMode_indexSet(index);
}

void TabPtzAutoTracking::on_pushButton_setZoomRatio_clicked()
{
    int channel = currentChannel();
    Q_UNUSED(channel)
    if (m_zoom_state.state) {
        ui->pushButton_setZoomRatio->clearUnderMouse();
        ShowMessageBox(GET_TEXT("PTZCONFIG/36032", "Exceeding the maximum Tracking Zoom Ratio."));
        return;
    }

    m_auto_tracking.zoom_mode = ui->comboBox_zoomRatioMode->currentData().toInt();
    m_auto_tracking.zoom_number = m_zoom_state.zoomNumber;
    m_isSetZoomClicked = true;

    qDebug() << "PtzAutoTrackingPage::on_pushButton_setZoomRatio_clicked"
             << ", zoom_mode:" << m_auto_tracking.zoom_mode
             << ", zoom_number:" << m_auto_tracking.zoom_number;
}

void TabPtzAutoTracking::on_pushButton_editSchedule_clicked()
{
    EffectiveTimePtzAutoTracking effectiveTime(this);
    effectiveTime.showEffectiveTime(currentChannel(), m_auto_tracking.schedule_day);
    effectiveTime.exec();

    ui->pushButton_editSchedule->clearUnderMouse();
}

void TabPtzAutoTracking::on_pushButtonSetAll_clicked()
{
    m_regionDraw->selectAll();
}

void TabPtzAutoTracking::on_pushButtonDeleteAll_clicked()
{
    m_regionDraw->clearAll();
}

void TabPtzAutoTracking::on_checkBoxHuman_clicked(bool checked)
{
    if (checked) {
        m_auto_tracking.auto_tracking_object |= (1 << 0);
    } else {
        m_auto_tracking.auto_tracking_object &= ~(1 << 0);
    }
}

void TabPtzAutoTracking::on_checkBoxVehicle_clicked(bool checked)
{
    if (checked) {
        m_auto_tracking.auto_tracking_object |= (1 << 1);
    } else {
        m_auto_tracking.auto_tracking_object &= ~(1 << 1);
    }
}
