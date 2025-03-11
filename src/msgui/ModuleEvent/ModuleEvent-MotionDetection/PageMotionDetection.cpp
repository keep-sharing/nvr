#include "PageMotionDetection.h"
#include "ui_PageMotionDetection.h"
#include "ActionMotion.h"
#include "EffectiveTimeMotionDetection.h"
#include "EventLoop.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsCameraVersion.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "channelcopydialog.h"

PageMotionDetection::PageMotionDetection(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::PageMotionDetection)
{
    ui->setupUi(this);

    ui->channelGroup->setCount(qMsNvr->maxChannel());
    connect(ui->channelGroup, SIGNAL(buttonClicked(int)), this, SLOT(onButtonGroupClicked(int)));

    ui->slider_sensitivity->setRange(1, 10);

    m_drawGrid = new GraphicsDrawGrid();
    ui->video->addGraphicsItem(m_drawGrid);

    m_action = new ActionMotion(this);
    m_effective = new EffectiveTimeMotionDetection(this);

    onLanguageChanged();
}

PageMotionDetection::~PageMotionDetection()
{
    if (m_eventSchedule) {
        delete m_eventSchedule;
    }
    if (m_motion) {
        delete m_motion;
    }
    if (m_motionScheduleAudible) {
        delete m_motionScheduleAudible;
    }
    if (m_motionScheduleEmail) {
        delete m_motionScheduleEmail;
    }
    if (m_motionSchedulePopup) {
        delete m_motionSchedulePopup;
    }
    if (m_motionSchedulePTZ) {
        delete m_motionSchedulePTZ;
    }
    if (m_ptzActionParams) {
        delete[] m_ptzActionParams;
    }

    delete ui;
}

void PageMotionDetection::initializeData()
{
    memset(&m_motionmap, 0, sizeof(req_set_motionmap));
    ui->channelGroup->setCurrentIndex(0);

    setSettingEnable(false);
}

void PageMotionDetection::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_FISHEYE_MODE:
        ON_RESPONSE_FLAG_GET_FISHEYE_MODE(message);
        break;
    case RESPONSE_FLAG_GET_IPCMTMAP:
        ON_RESPONSE_FLAG_GET_IPCMTMAP(message);
        break;
    case RESPONSE_FLAG_SET_IPCMTMAP:
        ON_RESPONSE_FLAG_SET_IPCMTMAP(message);
        break;
    default:
        break;
    }
}

void PageMotionDetection::ON_RESPONSE_FLAG_GET_IPCMTMAP(MessageReceive *message)
{
    //closeWait();

    struct resp_get_motionmap *motionmap = (resp_get_motionmap *)message->data;
    if (!motionmap) {
        qWarning() << "MotionDetection::ON_RESPONSE_FLAG_GET_IPCMTMAP, data is null.";
        return;
    }
    qDebug() << QString("MotionDetection::ON_RESPONSE_FLAG_GET_IPCMTMAP, channel: %1, enable: %2, sensitivity: %3, result: %4, modelType: %5")
                    .arg(motionmap->chanid)
                    .arg(motionmap->enable)
                    .arg(motionmap->sensitivity)
                    .arg(motionmap->result)
                    .arg(motionmap->modelType);

    //不支持motion
    if (motionmap->result == 1) {
        if (motionmap->modelType != DICONNECT) {
            ShowMessageBox(GET_TEXT("MOTION/51007", "Getting parameters failed. Please try to configure on the camera side."));
            return;
        }
    }

    ui->widgetContainer->setEnabled(true);
    setSettingEnable(true);
    ui->checkBox_enable->setChecked(motionmap->enable);
    on_checkBox_enable_clicked(ui->checkBox_enable->isChecked());
    if (motionmap->sensitivity < 1) {
        ui->slider_sensitivity->setValue(5);
    } else {
        ui->slider_sensitivity->setValue(motionmap->sensitivity);
    }
    m_drawGrid->setRegion(motionmap->mapbuf);
}

void PageMotionDetection::ON_RESPONSE_FLAG_SET_IPCMTMAP(MessageReceive *message)
{
    Q_UNUSED(message)

    gEventLoopExit(0);
}

void PageMotionDetection::ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message)
{
    Q_UNUSED(message)
}

void PageMotionDetection::saveChannelSetting(int channel)
{
    m_drawGrid->getRegion(m_motionmap.mapbuf);
    m_motionmap.chanid = channel;
    m_motionmap.enable = ui->checkBox_enable->isChecked();
    m_motionmap.sensitivity = ui->slider_sensitivity->value();
    qDebug() << QString("REQUEST_FLAG_SET_IPCMTMAP, channel: %1, enable: %2, sensitivity: %3").arg(m_motionmap.chanid).arg(m_motionmap.enable).arg(m_motionmap.sensitivity);
    sendMessage(REQUEST_FLAG_SET_IPCMTMAP, (void *)&m_motionmap, sizeof(req_set_motionmap));
    gEventLoopExec();

    //
    if (ui->checkBox_enable->isChecked()) {
        struct req_set_motionsce motion_scene;
        memset(&motion_scene, 0, sizeof(req_set_motionsce));
        motion_scene.chanid = channel;
        for (int i = 0; i < 7; i++) {
            motion_scene.scedule[i].day = i;
            motion_scene.scedule[i].onoff = 1;
            for (int j = 0; j < 4; j++) {
                motion_scene.scedule[i].tmdetail[j].end_hour = 24;
                motion_scene.scedule[i].tmdetail[j].end_minute = 0;
            }
        }
        sendMessage(REQUEST_FLAG_SET_IPCMTSCE, (void *)&motion_scene, sizeof(req_set_motionsce));
    }
}

void PageMotionDetection::onLanguageChanged()
{
    ui->label_motionDetection->setText(GET_TEXT("MOTION/51000", "Motion Detection"));
    ui->labelRegion->setText(GET_TEXT("ANPR/103021", "Region"));
    ui->label_sensitivity->setText(GET_TEXT("MOTION/51003", "Sensitivity"));
    ui->label_effectiveTime->setText(GET_TEXT("SMARTEVENT/55015", "Effective Time"));
    ui->label_action->setText(GET_TEXT("VIDEOLOSS/50002", "Action"));
    ui->checkBox_enable->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->pushButton_setAll->setText(GET_TEXT("MOTION/51004", "Set All"));
    ui->pushButtonDeleteAll->setText(GET_TEXT("PLAYBACK/80119", "Delete All"));
    ui->pushButton_effectiveTime->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->pushButton_action->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void PageMotionDetection::setSettingEnable(bool enable)
{
    ui->checkBox_enable->setEnabled(enable);
    if (enable && ui->checkBox_enable->isChecked()) {
        ui->pushButton_setAll->setEnabled(enable);
        ui->pushButtonDeleteAll->setEnabled(enable);
        m_drawGrid->setEnabled(enable);
    } else {
        ui->pushButton_setAll->setEnabled(false);
        ui->pushButtonDeleteAll->setEnabled(false);
        m_drawGrid->setEnabled(false);
    }
    ui->slider_sensitivity->setEnabled(enable);
    //effective和action永远都可以配置
    //ui->pushButton_effectiveTime->setEnabled(enable);
    //ui->pushButton_action->setEnabled(enable);
    ui->pushButton_copy->setEnabled(enable);
    ui->pushButton_apply->setEnabled(enable);
}

void PageMotionDetection::clearSetting()
{
    m_drawGrid->clearAll();
    ui->checkBox_enable->setChecked(false);
}

void PageMotionDetection::onButtonGroupClicked(int index)
{
    if (m_effective) {
        m_effective->clearCache();
    }
    if (m_action) {
        m_action->clearCache();
    }

    setSettingEnable(false);
    ui->widgetMessage->hideMessage();
    m_currentChannel = index;
    ui->video->playVideo(index);

    clearSetting();
    ui->widgetContainer->setEnabled(false);
    if (!LiveView::instance()->isChannelConnected(m_currentChannel)) {
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
        return;
    }

    if (!qMsNvr->isMsCamera(m_currentChannel)) {
        ui->widgetMessage->showMessage(GET_TEXT("MOTION/51007", "Getting parameters failed. Please try to configure on the camera side."));
        return;
    }

    //showWait();
    if (qMsNvr->isFisheye(m_currentChannel)) {
        qMsDebug() << QString("REQUEST_FLAG_GET_FISHEYE_MODE, channel: %1").arg(m_currentChannel);
        sendMessage(REQUEST_FLAG_GET_FISHEYE_MODE, (void *)&m_currentChannel, sizeof(int));
        int result = gEventLoopExec();
        if (result == -1) {
            //closeWait();
            ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
            return;
        } else if (result == -2) {
            //closeWait();
            ui->widgetMessage->showMessage(GET_TEXT("MOTION/51006", "Not supported in this fisheye mode."));
            return;
        }
    }

    sendMessage(REQUEST_FLAG_GET_IPCMTMAP, &m_currentChannel, sizeof(int));
}

void PageMotionDetection::on_checkBox_enable_clicked(bool checked)
{
    ui->pushButton_setAll->setEnabled(checked);
    ui->pushButtonDeleteAll->setEnabled(checked);
    ui->slider_sensitivity->setEnabled(checked);
    m_drawGrid->setEnabled(checked);
}

void PageMotionDetection::on_pushButton_setAll_clicked()
{
    m_drawGrid->selectAll();
}

void PageMotionDetection::on_pushButtonDeleteAll_clicked()
{
    m_drawGrid->clearAll();
}

void PageMotionDetection::on_pushButton_effectiveTime_clicked()
{
    m_effective->showEffectiveTime(m_currentChannel);
}

void PageMotionDetection::on_pushButton_action_clicked()
{
    m_action->showAction(m_currentChannel);
}

void PageMotionDetection::on_pushButton_copy_clicked()
{
    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(m_currentChannel);
    int result = copy.exec();
    if (result == ChannelCopyDialog::Accepted) {
        QList<int> copyList = copy.checkedList(false);
        quint64 copyFlags = copy.checkedFlags(false);

        //showWait();
        //先保存当前通道
        m_effective->saveEffectiveTime();
        m_action->saveAction();
        sendMessageOnly(REQUEST_FLAG_SET_MOTION, &m_currentChannel, sizeof(int));
        //
        saveChannelSetting(m_currentChannel);

        //copy effective time
        smart_event_schedule *currentEffectiveTimeSchedule = new smart_event_schedule;
        memset(currentEffectiveTimeSchedule, 0, sizeof(smart_event_schedule));
        read_motion_effective_schedule(SQLITE_FILE_NAME, currentEffectiveTimeSchedule, m_currentChannel);
        copy_motion_effective_schedules(SQLITE_FILE_NAME, currentEffectiveTimeSchedule, copyFlags);
        delete currentEffectiveTimeSchedule;
        //
        motion *currentMotion = new motion;
        memset(currentMotion, 0, sizeof(motion));
        read_motion(SQLITE_FILE_NAME, currentMotion, m_currentChannel);
        copy_motions(SQLITE_FILE_NAME, currentMotion, copyFlags);
        delete currentMotion;
        //
        motion_schedule *currentActionAudible = new motion_schedule;
        memset(currentActionAudible, 0, sizeof(motion_schedule));
        read_motion_audible_schedule(SQLITE_FILE_NAME, currentActionAudible, m_currentChannel);
        copy_motion_audible_schedules(SQLITE_FILE_NAME, currentActionAudible, copyFlags);
        delete currentActionAudible;
        //
        motion_schedule *currentActionEmail = new motion_schedule;
        memset(currentActionEmail, 0, sizeof(motion_schedule));
        read_motion_email_schedule(SQLITE_FILE_NAME, currentActionEmail, m_currentChannel);
        copy_motion_email_schedules(SQLITE_FILE_NAME, currentActionEmail, copyFlags);
        delete currentActionEmail;
        //event popup
        motion_schedule *currentActionPopup = new motion_schedule;
        memset(currentActionPopup, 0, sizeof(motion_schedule));
        read_motion_popup_schedule(SQLITE_FILE_NAME, currentActionPopup, m_currentChannel);
        copy_motion_popup_schedules(SQLITE_FILE_NAME, currentActionPopup, copyFlags);
        delete currentActionPopup;
        //ptz action schedule
        motion_schedule *currentActionPtz = new motion_schedule;
        memset(currentActionPtz, 0, sizeof(motion_schedule));
        read_motion_ptz_schedule(SQLITE_FILE_NAME, currentActionPtz, m_currentChannel);
        copy_motion_ptz_schedules(SQLITE_FILE_NAME, currentActionPtz, copyFlags);
        delete currentActionPtz;
        //ptz action
        ptz_action_params *ptzActionArray = new ptz_action_params[MAX_CAMERA];
        int ptzItemCount = 0;
        memset(ptzActionArray, 0x0, sizeof(ptz_action_params) * MAX_CAMERA);
        read_ptz_params(SQLITE_FILE_NAME, ptzActionArray, MOTION, m_currentChannel, &ptzItemCount);
        copy_ptz_params_all(SQLITE_FILE_NAME, ptzActionArray, MOTION, copyFlags);
        delete[] ptzActionArray;
        //led schedule
        SMART_SCHEDULE *scheduleLed = new SMART_SCHEDULE;
        memset(scheduleLed, 0, sizeof(SMART_SCHEDULE));
        WLED_INFO led_info_schedule;
        led_info_schedule.chnid = m_currentChannel;
        snprintf(led_info_schedule.pDbTable, sizeof(led_info_schedule.pDbTable), "%s", MOT_WLED_ESCHE);
        read_whiteled_effective_schedule(SQLITE_FILE_NAME, scheduleLed, &led_info_schedule);
        copy_whiteled_effective_schedules(SQLITE_FILE_NAME, scheduleLed, &led_info_schedule, copyFlags);
        delete scheduleLed;
        //led params
        WHITE_LED_PARAMS *ledParamsArray = new WHITE_LED_PARAMS[MAX_CAMERA];
        memset(ledParamsArray, 0, sizeof(WHITE_LED_PARAMS) * MAX_CAMERA);
        WLED_INFO led_info_params;
        led_info_params.chnid = m_currentChannel;
        snprintf(led_info_params.pDbTable, sizeof(led_info_params.pDbTable), "%s", MOT_WLED_PARAMS);
        int led_params_count = 0;
        read_whiteled_params(SQLITE_FILE_NAME, ledParamsArray, &led_info_params, &led_params_count);
        copy_whiteled_params(SQLITE_FILE_NAME, MOT_WLED_PARAMS, ledParamsArray, copyFlags);
        delete[] ledParamsArray;
        //http
        copy_http_notification_action(MOTION, m_currentChannel, copyFlags);
        //
        for (int i = 0; i < copyList.size(); ++i) {
            int channel = copyList.at(i);
            if (channel != m_currentChannel) {
                //
                sendMessageOnly(REQUEST_FLAG_SET_MOTION, &channel, sizeof(int));
                //
                saveChannelSetting(channel);
            }
        }
        //closeWait();
    }
}

void PageMotionDetection::on_pushButton_apply_clicked()
{
    //showWait();
    //
    m_effective->saveEffectiveTime();
    m_action->saveAction();
    sendMessageOnly(REQUEST_FLAG_SET_MOTION, &m_currentChannel, sizeof(int));
    //
    saveChannelSetting(m_currentChannel);
    //
    //closeWait();
}

void PageMotionDetection::on_pushButton_back_clicked()
{
    emit sig_back();
}
