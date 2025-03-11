#include "recordschedule.h"
#include "ui_recordschedule.h"
#include "LogWrite.h"
#include "MyDebug.h"
#include "bluelabel.h"
#include "centralmessage.h"
#include "channelcopydialog.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "msuser.h"
#include "EventLoop.h"

RecordSchedule::RecordSchedule(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::RecordSchedule)
{
    ui->setupUi(this);

    ui->widget_tab->addTab(GET_TEXT("LOG/64083", "Record Schedule"), TabRecoreSchedule);
    //13版本去掉Batch Settings
    //ui->widget_tab->addTab(GET_TEXT("CHANNELMANAGE/30055", "Batch Settings"), TabBatchSettings);
    ui->widget_tab->addTab(GET_TEXT("RECORDMODE/90000", "Record Settings"), TabRecordSettings);
    connect(ui->widget_tab, SIGNAL(tabClicked(int)), this, SLOT(onTabClicked(int)));

    ui->widget_button->setCount(qMsNvr->maxChannel());
    connect(ui->widget_button, SIGNAL(buttonClicked(int)), this, SLOT(onChannelButtonClicked(int)));

    //
    m_scheduleTypeGroup = new QButtonGroup(this);
    m_scheduleTypeGroup->addButton(ui->pushButton_erase);
    m_scheduleTypeGroup->addButton(ui->pushButton_continuous);
    m_scheduleTypeGroup->addButton(ui->pushButton_event);
    m_scheduleTypeGroup->addButton(ui->pushButton_motion);
    m_scheduleTypeGroup->addButton(ui->pushButton_alarm);
    m_scheduleTypeGroup->addButton(ui->pushButton_smartEvent);
    m_scheduleTypeGroup->addButton(ui->pushButton_smartAnalysis);
    m_scheduleTypeGroup->addButton(ui->pushButtonAudioAlarm);

    //
    ui->widget_checkGroup->setCount(qMsNvr->maxChannel());
    connect(ui->widget_checkGroup, SIGNAL(checkBoxClicked(int, bool)), this, SLOT(onChannelCheckBoxClicked(int, bool)));

    connect(ui->pushButton_copy, SIGNAL(clicked(bool)), this, SLOT(onPushButtonCopyClicked()));
    connect(ui->pushButton_apply, SIGNAL(clicked(bool)), this, SLOT(onPushButtonApplyClicked()));
    connect(ui->pushButton_back, SIGNAL(clicked(bool)), this, SLOT(onPushButtonBackClicked()));

    connect(ui->pushButton_startRecord, SIGNAL(clicked(bool)), this, SLOT(onPushButtonStartRecordClicked()));
    connect(ui->pushButton_stopRecord, SIGNAL(clicked(bool)), this, SLOT(onPushButtonStopRecordClicked()));
    connect(ui->pushButton_back_2, SIGNAL(clicked(bool)), this, SLOT(onPushButtonBackClicked()));

    //
    ui->widget_button_2->setCount(qMsNvr->maxChannel());
    connect(ui->widget_button_2, SIGNAL(buttonClicked(int)), this, SLOT(onChannelButton2Clicked(int)));

    ui->comboBox_preRecord->clear();
    ui->comboBox_preRecord->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    if (isSupportPreviousRecord()) {
        for (int i = 1; i <= PREVRECORDDURATION_SEC_MAX; i++) {
            ui->comboBox_preRecord->addItem(GET_TEXT("RECORDADVANCE/91007", "%1s").arg(i), i);
        }
    } else {
        ui->label_preRecord->hide();
        ui->comboBox_preRecord->hide();
    }

    ui->comboBox_postRecord->clear();
    ui->comboBox_postRecord->addItem(GET_TEXT("RECORDADVANCE/91009", "%1s").arg(10), -3);
    ui->comboBox_postRecord->addItem(GET_TEXT("RECORDADVANCE/91009", "%1s").arg(15), -2);
    ui->comboBox_postRecord->addItem(GET_TEXT("RECORDADVANCE/91009", "%1s").arg(30), -1);
    ui->comboBox_postRecord->addItem(GET_TEXT("RECORDADVANCE/91008", "%1min").arg(1), 1);
    ui->comboBox_postRecord->addItem(GET_TEXT("RECORDADVANCE/91008", "%1min").arg(2), 2);
    ui->comboBox_postRecord->addItem(GET_TEXT("RECORDADVANCE/91008", "%1min").arg(3), 3);
    ui->comboBox_postRecord->addItem(GET_TEXT("RECORDADVANCE/91008", "%1min").arg(4), 4);
    ui->comboBox_postRecord->addItem(GET_TEXT("RECORDADVANCE/91008", "%1min").arg(5), 5);
    ui->comboBox_postRecord->addItem(GET_TEXT("RECORDADVANCE/91008", "%1min").arg(10), 10);
    ui->comboBox_postRecord->addItem(GET_TEXT("RECORDADVANCE/91008", "%1min").arg(20), 20);
    ui->comboBox_postRecord->addItem(GET_TEXT("RECORDADVANCE/91008", "%1min").arg(30), 30);
    ui->comboBox_postRecord->addItem(GET_TEXT("RECORDADVANCE/91008", "%1min").arg(40), 40);
    ui->comboBox_postRecord->addItem(GET_TEXT("RECORDADVANCE/91008", "%1min").arg(50), 50);
    ui->comboBox_postRecord->addItem(GET_TEXT("RECORDADVANCE/91008", "%1min").arg(60), 60);

    ui->comboBox_audioRecord->clear();
    ui->comboBox_audioRecord->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_audioRecord->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);

    ui->comboBox_recordStream->clear();
    ui->comboBox_recordStream->addItem(GET_TEXT("IMAGE/37333", "Primary Stream"), 0);
    ui->comboBox_recordStream->addItem(GET_TEXT("CHANNELMANAGE/30057", "Secondary Stream"), 1);
    //MSHN-9188 QT-Record Settings：3536g平台，Record Stream Type设置选项中去掉Primary + Second Stream选项；【需求更改】
    if (qMsNvr->isSupportDualStreamVideoRecording()) {
        ui->comboBox_recordStream->addItem(GET_TEXT("RECORDADVANCE/91020", "Primary+Secondary Stream"), 2);
    }

    ui->comboBox_videoDueTime->clear();
    ui->comboBox_videoDueTime->addItem(GET_TEXT("RECORDADVANCE/91012", "Unlimit"), 0);
    ui->comboBox_videoDueTime->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);

    ui->comboBox_anr->clear();
    ui->comboBox_anr->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_anr->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);

    ui->lineEdit_expiredTime->setCheckMode(MyLineEdit::RangeCheck, 1, 120);
    onLanguageChanged();
}

RecordSchedule::~RecordSchedule()
{
    delete ui;
}

void RecordSchedule::initializeData()
{
    ui->widget_tab->setCurrentTab(0);

    ui->pushButton_continuous->setChecked(true);
    on_pushButton_continuous_clicked();
}

void RecordSchedule::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SET_ALL_RECORD:
        ON_RESPONSE_FLAG_SET_ALL_RECORD(message);
        break;
    case RESPONSE_FLAG_SET_RECPARAM:
        ON_RESPONSE_FLAG_SET_RECPARAM(message);
        break;
    case RESPONSE_FLAG_GET_ANR_SUPPORT:
        ON_RESPONSE_FLAG_GET_ANR_SUPPORT(message);
        break;
    case RESPONSE_FLAG_SET_ANR_SUPPORT:
        ON_RESPONSE_FLAG_SET_ANR_SUPPORT(message);
        break;
    case RESPONSE_FLAG_GET_IPC_CONNECT_STATE:
        ON_RESPONSE_FLAG_IPC_CONNECT_STATE(message);
        break;
    case RESPONSE_FLAG_SET_RECORDSCHED:
        ON_RESPONSE_FLAG_SET_RECORDSCHED(message);
        break;
    }
}

void RecordSchedule::ON_RESPONSE_FLAG_SET_ALL_RECORD(MessageReceive *message)
{
    Q_UNUSED(message);
    //closeWait();
}

void RecordSchedule::showRecordSchedule()
{
    ui->schedule->setSchedule(m_record.schedule_day);
}

void RecordSchedule::onLanguageChanged()
{
    ui->pushButton_erase->setText(GET_TEXT("RECORDMODE/90012", "Erase"));
    ui->pushButton_continuous->setText(GET_TEXT("RECORDMODE/90011", "Continuous"));
    ui->pushButton_event->setText(GET_TEXT("RECORDMODE/90013", "Event"));
    ui->groupBox->setTitle(GET_TEXT("RECORDMODE/90013", "Event"));
    ui->pushButton_motion->setText(GET_TEXT("MOTION/51000", "Motion Detection"));
    ui->pushButton_alarm->setText(GET_TEXT("ALARMIN/52001", "Alarm Input"));
    ui->pushButton_smartEvent->setText(GET_TEXT("SMARTEVENT/55000", "VCA"));
    ui->pushButton_smartAnalysis->setText(GET_TEXT("ANPR/103054", "Smart Analysis"));
    ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));

    //
    ui->label_note1->setText(GET_TEXT("RECORDMODE/90018", "<html><head/><body><p>Start Record: Selected channels will be set to continuous record.</p><p>Stop Record: Selected channels’ continuous schedule will be erased.</p></body></html>"));
    ui->pushButton_startRecord->setText(GET_TEXT("LOG/64070", "Start Record"));
    ui->pushButton_stopRecord->setText(GET_TEXT("LOG/64071", "Stop Record"));
    ui->pushButton_back_2->setText(GET_TEXT("COMMON/1002", "Back"));

    //
    ui->label_preRecord->setText(GET_TEXT("RECORDADVANCE/91001", "Pre Record"));
    ui->label_postRecord->setText(GET_TEXT("RECORDADVANCE/91003", "Post Record"));
    ui->label_audioRecord->setText(GET_TEXT("RECORDADVANCE/91004", "Audio Record"));
    ui->label_recordStream->setText(GET_TEXT("RECORDADVANCE/91005", "Record Stream Type"));
    ui->label_videoDueTime->setText(GET_TEXT("RECORDADVANCE/91006", "Video Due Time"));
    ui->label_expiredTime->setText(GET_TEXT("RECORDADVANCE/91015", "Expired Time"));
    ui->label_expiredTime_2->setText(GET_TEXT("RECORDADVANCE/91016", "1~120 Days"));
    ui->label_anr->setText(GET_TEXT("RECORDADVANCE/91024", "ANR"));
    ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_copy_2->setText(GET_TEXT("COMMON/1005", "Copy"));
    ui->pushButton_back_3->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButton_apply_2->setText(GET_TEXT("COMMON/1003", "Apply"));

    ui->pushButtonAudioAlarm->setText(GET_TEXT("AUDIO_ALARM/159000", "Audio Alarm"));
}

void RecordSchedule::onTabClicked(int index)
{
    ui->stackedWidget->setCurrentIndex(index);

    switch (index) {
    case TabRecoreSchedule:
        ui->widget_button->setCurrentIndex(m_currentChannel);
        break;
    case TabBatchSettings:
        ui->video2->playVideo(0);
        ui->widget_checkGroup->clearCheck();
        break;
    case TabRecordSettings:
        ui->widget_video_2->playVideo(m_currentChannel);
        ui->widget_button_2->setCurrentIndex(m_currentChannel);
        onChannelButton2Clicked(m_currentChannel);
        break;
    default:
        break;
    }
}

void RecordSchedule::onChannelButtonClicked(int index)
{
    m_currentChannel = index;
    ui->widget_video->playVideo(index);

    memset(&m_record, 0, sizeof(record_schedule));
    read_record_schedule(SQLITE_FILE_NAME, &m_record, m_currentChannel);
    showRecordSchedule();
}

void RecordSchedule::onChannelCheckBoxClicked(int channel, bool checked)
{
    if (checked) {
        ui->video2->playVideo(channel);
    }
}

void RecordSchedule::copyData()
{
    if (m_copyList.isEmpty()) {
        return;
    } else {
        int channel = m_copyList.takeFirst();
        write_record_schedule(SQLITE_FILE_NAME, &m_record, channel);
        sendMessage(REQUEST_FLAG_SET_RECORDSCHED, &channel, sizeof(int));
        //
        struct log_data log_data;
        memset(&log_data, 0, sizeof(struct log_data));
        log_data.log_data_info.subType = SUB_OP_CONFIG_LOCAL;
        log_data.log_data_info.parameter_type = SUB_PARAM_RECORD_SCHED;
        snprintf(log_data.log_data_info.user, sizeof(log_data.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
        log_data.log_data_info.chan_no = channel + 1;
        MsWriteLog(log_data);
    }
}

void RecordSchedule::on_pushButton_erase_clicked()
{
    ui->schedule->setCurrentType(NONE);
}

void RecordSchedule::on_pushButton_continuous_clicked()
{
    ui->schedule->setCurrentType(TIMING_RECORD);
}

void RecordSchedule::on_pushButton_event_clicked()
{
    ui->schedule->setCurrentType(EVENT_RECORD);
}

void RecordSchedule::on_pushButton_motion_clicked()
{
    ui->schedule->setCurrentType(MOTION_RECORD);
}

void RecordSchedule::on_pushButton_alarm_clicked()
{
    ui->schedule->setCurrentType(ALARM_RECORD);
}

void RecordSchedule::on_pushButton_smartEvent_clicked()
{
    ui->schedule->setCurrentType(SMART_EVT_RECORD);
}

void RecordSchedule::on_pushButton_smartAnalysis_clicked()
{
    ui->schedule->setCurrentType(ANPT_EVT_RECORD);
}

void RecordSchedule::on_pushButtonAudioAlarm_clicked()
{
    ui->schedule->setCurrentType(AUDIO_ALARM_RECORD);
}

void RecordSchedule::onPushButtonCopyClicked()
{
    m_copyList.clear();

    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(m_currentChannel);
    int result = copy.exec();
    if (result == ChannelCopyDialog::Accepted) {
        m_copyList = copy.checkedList();
        QTimer::singleShot(0, this, SLOT(onPushButtonApplyClicked()));
    }
}

void RecordSchedule::onPushButtonApplyClicked()
{
    if (m_copyList.isEmpty()) {
        m_copyList.append(m_currentChannel);
    }
    ui->schedule->getSchedule(m_record.schedule_day);

    //
    //showWait();
    while (!m_copyList.isEmpty()) {
        copyData();
        gEventLoopExec();
    }
    //closeWait();
}

void RecordSchedule::onPushButtonBackClicked()
{
    emit sig_back();
}

void RecordSchedule::onPushButtonStartRecordClicked()
{
    //showWait();

    int chan_no = 0;
    struct resp_set_all_record allRecord;
    memset(&allRecord, 0, sizeof(struct resp_set_all_record));
    allRecord.enable = 1;
    QList<bool> checkStateList = ui->widget_checkGroup->checkStateList();
    for (int i = 0; i < checkStateList.size(); ++i) {
        if (checkStateList.at(i)) {
            allRecord.channel[i] = 1;
            chan_no = i + 1;
        }
    }
    sendMessage(REQUEST_FLAG_SET_ALL_RECORD, &allRecord, sizeof(struct resp_set_all_record));
    struct log_data log_data;
    memset(&log_data, 0, sizeof(struct log_data));
    log_data.log_data_info.subType = SUB_OP_CONFIG_LOCAL;
    log_data.log_data_info.parameter_type = SUB_PARAM_RECORD_SCHED;
    snprintf(log_data.log_data_info.user, sizeof(log_data.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
    log_data.log_data_info.chan_no = chan_no;
    MsWriteLog(log_data);
}

void RecordSchedule::onPushButtonStopRecordClicked()
{
    //showWait();

    int chan_no = 0;
    struct resp_set_all_record allRecord;
    memset(&allRecord, 0, sizeof(struct resp_set_all_record));
    allRecord.enable = 0;
    QList<bool> checkStateList = ui->widget_checkGroup->checkStateList();
    for (int i = 0; i < checkStateList.size(); ++i) {
        if (checkStateList.at(i)) {
            allRecord.channel[i] = 1;
            chan_no = i + 1;
        }
    }
    sendMessage(REQUEST_FLAG_SET_ALL_RECORD, &allRecord, sizeof(struct resp_set_all_record));
    struct log_data log_data;
    memset(&log_data, 0, sizeof(struct log_data));
    log_data.log_data_info.subType = SUB_OP_CONFIG_LOCAL;
    log_data.log_data_info.parameter_type = SUB_PARAM_RECORD_SCHED;
    snprintf(log_data.log_data_info.user, sizeof(log_data.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
    log_data.log_data_info.chan_no = chan_no;
    MsWriteLog(log_data);
}

bool RecordSchedule::isSupportPreviousRecord()
{
#if 0
    const QString &strPrefix = qMsNvr->deviceInfo().prefix;
    if (strPrefix == "8" || strPrefix == "7" || strPrefix == "5")
    {
        return true;
    }
    else
    {
        return false;
    }
#else
    return true;
#endif
}

void RecordSchedule::on_pushButton_copy_2_clicked()
{
    if (!ui->lineEdit_expiredTime->checkValid()) {
        return;
    }
    m_copyList.clear();

    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(m_currentChannel);
    int result = copy.exec();
    if (result == ChannelCopyDialog::Accepted) {
        m_copyList = copy.checkedList();
        QTimer::singleShot(0, this, SLOT(on_pushButton_apply_clicked()));
    }
    on_pushButton_apply_2_clicked();
}

void RecordSchedule::on_pushButton_apply_2_clicked()
{
    if (!ui->lineEdit_expiredTime->checkValid()) {
        return;
    }
    m_dbRecord.prev_record_duration = ui->comboBox_preRecord->currentData().toInt();
    m_dbRecord.post_record_duration = ui->comboBox_postRecord->currentData().toInt();

    if (!m_dbRecord.prev_record_duration)
        m_dbRecord.prev_record_on = 0;
    else
        m_dbRecord.prev_record_on = 1;

    if (!m_dbRecord.post_record_duration)
        m_dbRecord.post_record_on = 0;
    else
        m_dbRecord.post_record_on = 1;
    m_dbRecord.audio_enable = ui->comboBox_audioRecord->currentData().toInt();
    if (ui->comboBox_videoDueTime->currentIndex() == 1) {
        m_dbRecord.record_expiration_date = ui->lineEdit_expiredTime->text().toInt();
    } else {
        m_dbRecord.record_expiration_date = 0;
    }

    m_dbCamera.record_stream = ui->comboBox_recordStream->currentData().toInt();
    m_dbCamera.anr = ui->comboBox_anr->currentData().toInt();
    m_setAnrList.clear();
    if (m_copyList.isEmpty()) {
        m_copyList.append(m_currentChannel);
    }

    //showWait();
    if (m_dbCamera.record_stream != 0) {
        sendMessage(REQUEST_FLAG_GET_IPC_CONNECT_STATE, (void *)NULL, 0);
    } else {
        QTimer::singleShot(0, this, SLOT(onCopySettingsData()));
    }
}

void RecordSchedule::on_pushButton_back_3_clicked()
{
    emit sig_back();
}

void RecordSchedule::on_comboBox_videoDueTime_activated(int index)
{
    if (index == 0) {
        ui->label_expiredTime->hide();
        ui->lineEdit_expiredTime->hide();
        ui->label_expiredTime_2->hide();
    } else {
        ui->label_expiredTime->show();
        ui->lineEdit_expiredTime->show();
        ui->label_expiredTime_2->show();
    }
}

void RecordSchedule::onCopySettingsData()
{
    int change = 0;
    if (m_copyList.isEmpty()) {
        struct req_set_anr_enable_batch reqSetAnr;
        memset(&reqSetAnr, 0x0, sizeof(struct req_set_anr_enable_batch));
        reqSetAnr.enable = m_dbCamera.anr;

        while (!m_setAnrList.isEmpty()) {
            reqSetAnr.info.chanid[reqSetAnr.info.size] = m_setAnrList.takeFirst();
            reqSetAnr.info.size++;
        }



    } else {
        const int &channel = m_copyList.takeFirst();
        struct record db_record;
        memset(&db_record, 0, sizeof(struct record));
        read_record(SQLITE_FILE_NAME, &db_record, channel);
        db_record.prev_record_duration = m_dbRecord.prev_record_duration;
        db_record.post_record_duration = m_dbRecord.post_record_duration;
        db_record.audio_enable = m_dbRecord.audio_enable;
        db_record.record_expiration_date = m_dbRecord.record_expiration_date;
        db_record.prev_record_on = m_dbRecord.prev_record_on;
        db_record.post_record_on = m_dbRecord.post_record_on;
        write_record(SQLITE_FILE_NAME, &db_record);

        struct camera db_camera;
        memset(&db_camera, 0, sizeof(camera));
        read_camera(SQLITE_FILE_NAME, &db_camera, channel);
        change = 0;
        if (db_camera.anr != m_dbCamera.anr) {
            if (!m_dbCamera.anr) {
                //1-->0
                change = 1;
                db_camera.anr = m_dbCamera.anr;
                m_setAnrList.append(channel);
            } else if (db_camera.enable && !db_camera.poe_channel && db_camera.camera_protocol != IPC_PROTOCOL_RTSP) {
                //0-->1
                change = 1;
                db_camera.anr = m_dbCamera.anr;
                m_setAnrList.append(channel);
            }
        }

        if (change || db_camera.record_stream != m_dbCamera.record_stream) {
            db_camera.record_stream = m_dbCamera.record_stream;
            qMsNvr->writeDatabaseCamera(&db_camera);
        }

        sendMessage(REQUEST_FLAG_SET_RECORDMODE, (void *)&channel, sizeof(int));
        sendMessage(REQUEST_FLAG_SET_RECPARAM, (void *)&channel, sizeof(int));
    }
}

void RecordSchedule::onChannelButton2Clicked(int index)
{
    ui->lineEdit_expiredTime->setValid(true);
    m_currentChannel = index;
    ui->widget_video_2->playVideo(index);

    memset(&m_dbRecord, 0, sizeof(m_dbRecord));
    read_record(SQLITE_FILE_NAME, &m_dbRecord, m_currentChannel);
    memset(&m_dbCamera, 0, sizeof(m_dbCamera));
    read_camera(SQLITE_FILE_NAME, &m_dbCamera, m_currentChannel);

    //
    if (m_dbRecord.prev_record_on)
        ui->comboBox_preRecord->setCurrentIndexFromData(m_dbRecord.prev_record_duration);
    else
        ui->comboBox_preRecord->setCurrentIndexFromData(0);
    if (m_dbRecord.post_record_on)
        ui->comboBox_postRecord->setCurrentIndexFromData(m_dbRecord.post_record_duration);
    else
        ui->comboBox_postRecord->setCurrentIndex(0);
    if (ui->comboBox_postRecord->currentIndex() < 0) {
        ui->comboBox_postRecord->setCurrentIndex(0);
    }
    ui->comboBox_audioRecord->setCurrentIndexFromData(m_dbRecord.audio_enable);
    ui->comboBox_recordStream->setCurrentIndexFromData(m_dbCamera.record_stream);
    if (m_dbRecord.record_expiration_date == 0) {
        ui->comboBox_videoDueTime->setCurrentIndexFromData(0);
        on_comboBox_videoDueTime_activated(0);
        ui->lineEdit_expiredTime->setText("1");
    } else {
        ui->comboBox_videoDueTime->setCurrentIndexFromData(1);
        on_comboBox_videoDueTime_activated(1);
        ui->lineEdit_expiredTime->setText(QString("%1").arg(m_dbRecord.record_expiration_date));
    }

    //anr
    if (!m_dbCamera.enable || m_dbCamera.poe_channel) {
        ui->label_anr->hide();
        ui->comboBox_anr->hide();
    } else {
        ui->label_anr->show();
        ui->comboBox_anr->show();
}
    if (m_dbCamera.camera_protocol != IPC_PROTOCOL_MILESIGHT && m_dbCamera.camera_protocol != IPC_PROTOCOL_ONVIF && m_dbCamera.camera_protocol != IPC_PROTOCOL_MSDOMAIN) {
        ui->comboBox_anr->setDisabled(true);
        ui->comboBox_anr->setCurrentIndexFromData(0);
    } else {
        ui->comboBox_anr->setDisabled(false);
        ui->comboBox_anr->setCurrentIndexFromData(m_dbCamera.anr);
        //showWait();
        sendMessage(REQUEST_FLAG_GET_ANR_SUPPORT, &m_currentChannel, sizeof(int));
    }
}

void RecordSchedule::ON_RESPONSE_FLAG_SET_RECPARAM(MessageReceive *message)
{
    Q_UNUSED(message);

    onCopySettingsData();
}

void RecordSchedule::ON_RESPONSE_FLAG_GET_ANR_SUPPORT(MessageReceive *message)
{
    struct resp_anr_support *resp = (struct resp_anr_support *)message->data;

    if (resp) {
        switch (resp->anr_support) {
        case ANR_SUPPORT_DISCONNECT:
            ui->comboBox_anr->setDisabled(true);
            break;

        case ANR_SUPPORT_NOT_EXITS:
            ui->comboBox_anr->setDisabled(true);
            break;

        case ANR_SUPPORT_NOT_MSSP:
            ui->comboBox_anr->setDisabled(true);
            break;

        case ANR_SUPPORT_ERROR:
            ui->comboBox_anr->setDisabled(true);
            break;

        case ANR_SUPPORT_NOT_SUPPORT:
            ui->comboBox_anr->setDisabled(true);
            break;

        case ANR_SUPPORT_SUPPORT:
            ui->comboBox_anr->setDisabled(false);
            break;
        }
    }

    //closeWait();
}

void RecordSchedule::ON_RESPONSE_FLAG_SET_ANR_SUPPORT(MessageReceive *message)
{
    struct resp_set_anr_enable_batch *resp = (struct resp_set_anr_enable_batch *)message->data;

    if (resp) {
        // only set anr enable batch and some channel not support ,notify message
        if (resp->enable && resp->info.size > 1) {
            for (int i = 0; i < resp->info.size; i++) {
                if (resp->info.res[i] != 0) {
                    ShowMessageBox(GET_TEXT("RECORDADVANCE/91025", "The ANR function may not be copied to some channels, probably because the channel do not support the ANR function."));
                    break;
                }
            }
        }
    }
    //closeWait();
    return;
}

void RecordSchedule::ON_RESPONSE_FLAG_IPC_CONNECT_STATE(MessageReceive *message)
{
    struct stream_state_batch *resp = (struct stream_state_batch *)message->data;
    QString channelNum = "";
    bool flag = false;
    int chnid = 0;
    int size = m_copyList.size();

    if (resp) {
        if (size == 1 && !resp->secondary[m_currentChannel]) {
            ShowMessageBox(GET_TEXT("RECORDADVANCE/91028", "Secondary stream unavailable!"));
        } else if (size > 1) {
            for (int i = 0; i < size; i++) {
                chnid = m_copyList.at(i);
                if (resp->secondary[chnid] == 0) {
                    if (!flag)
                        flag = true;
                    else
                        channelNum += "/";
                    channelNum += QString::number(chnid + 1);
                }
            }

            if (flag) {
                ShowMessageBox(GET_TEXT("RECORDADVANCE/91027", "Secondary stream of CH %1 unavailable!").arg(channelNum));
            }
        }
    }

    onCopySettingsData();
    return;
}

void RecordSchedule::ON_RESPONSE_FLAG_SET_RECORDSCHED(MessageReceive *message)
{
    Q_UNUSED(message)
    gEventLoopExit(0);
}

