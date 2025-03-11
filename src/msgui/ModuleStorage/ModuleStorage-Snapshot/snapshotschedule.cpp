#include "snapshotschedule.h"
#include "ui_snapshotschedule.h"
#include "centralmessage.h"
#include "channelcopydialog.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include <QtDebug>

SnapshotSchedule::SnapshotSchedule(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::SnapshotSchedule)
{
    ui->setupUi(this);

    ui->widget_tab->addTab(GET_TEXT("SNAPSHOT/95001", "Snapshot Schedule"), TabSnapshotSchedule);
    //13版本去掉Batch Settings
    //ui->widget_tab->addTab(GET_TEXT("CHANNELMANAGE/30055", "Batch Settings"), TabBatchSettings);
    ui->widget_tab->addTab(GET_TEXT("SNAPSHOT/95002", "Snapshot Settings"), TabSnapshotSettings);

    // schedule

    connect(ui->widget_tab, SIGNAL(tabClicked(int)), this, SLOT(onTabClicked(int)));
    ui->widget_button->setCount(qMsNvr->maxChannel());
    connect(ui->widget_button, SIGNAL(buttonClicked(int)), this, SLOT(onChannelButtonClicked(int)));

    m_scheduleTypeGroup = new QButtonGroup(this);
    m_scheduleTypeGroup->addButton(ui->pushButton_erase);
    m_scheduleTypeGroup->addButton(ui->pushButton_continuous);
    m_scheduleTypeGroup->addButton(ui->pushButton_event);
    m_scheduleTypeGroup->addButton(ui->pushButton_motion);
    m_scheduleTypeGroup->addButton(ui->pushButton_alarm);
    m_scheduleTypeGroup->addButton(ui->pushButton_smartEvent);
    m_scheduleTypeGroup->addButton(ui->pushButton_smartAnalysis);
    m_scheduleTypeGroup->addButton(ui->pushButtonAudioAlarm);


    connect(ui->pushButton_copy, SIGNAL(clicked(bool)), this, SLOT(onPushButtonCopyClicked()));
    connect(ui->pushButton_apply, SIGNAL(clicked(bool)), this, SLOT(onPushButtonApplyClicked()));
    connect(ui->pushButton_back, SIGNAL(clicked(bool)), this, SLOT(onPushButtonBackClicked()));

    //batch settings
    ui->widget_checkGroup->setCount(qMsNvr->maxChannel());
    connect(ui->widget_checkGroup, SIGNAL(checkBoxClicked(int, bool)), this, SLOT(onChannelCheckBoxClicked(int, bool)));
    connect(ui->pushButton_startSnapshot, SIGNAL(clicked(bool)), this, SLOT(onPushButtonStartSnapshotClicked()));
    connect(ui->pushButton_stopSnapshot, SIGNAL(clicked(bool)), this, SLOT(onPushButtonStopSnapshotClicked()));
    connect(ui->pushButton_back_3, SIGNAL(clicked(bool)), this, SLOT(onPushButtonBackClicked()));

    // settings
    ui->widget_button2->setCount(qMsNvr->maxChannel());
    connect(ui->widget_button2, SIGNAL(buttonClicked(int)), this, SLOT(onChannelButtonClicked(int)));

    ui->comboBox_quality->clear();
    ui->comboBox_quality->addItem(GET_TEXT("COMMON/1017", "High"), 0);
    ui->comboBox_quality->addItem(GET_TEXT("SNAPSHOT/95007", "Middle"), 1);
    ui->comboBox_quality->addItem(GET_TEXT("COMMON/1016", "Low"), 2);

    ui->comboBox_quality->hide();
    ui->label_quality->hide();

    ui->comboBox_interval->clear();
    ui->comboBox_interval->addItem(GET_TEXT("SNAPSHOT/95008", "Seconds"), 0);
    ui->comboBox_interval->addItem(GET_TEXT("SNAPSHOT/95009", "Minutes"), 1);
    ui->comboBox_interval->addItem(GET_TEXT("SNAPSHOT/95010", "Hours"), 2);

    ui->comboBox_dueTime->clear();
    ui->comboBox_dueTime->addItem(GET_TEXT("RECORDADVANCE/91012", "Unlimit"), 0);
    ui->comboBox_dueTime->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);

    connect(ui->pushButton_copy_2, SIGNAL(clicked(bool)), this, SLOT(onPushButtonCopyClicked()));
    connect(ui->pushButton_apply_2, SIGNAL(clicked(bool)), this, SLOT(onPushButtonApplyClicked()));
    connect(ui->pushButton_back_2, SIGNAL(clicked(bool)), this, SLOT(onPushButtonBackClicked()));

    ui->lineEdit_expiredTime->setCheckMode(MyLineEdit::RangeCheck, 1, 120);
    onLanguageChanged();
}

SnapshotSchedule::~SnapshotSchedule()
{
    delete ui;
}

void SnapshotSchedule::initializeData()
{
    m_currentChannel = 0;
    ui->widget_tab->setCurrentTab(0);
    ui->pushButton_continuous->setChecked(true);
    on_pushButton_continuous_clicked();
}

void SnapshotSchedule::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SET_SNAPSHOT_SCHE:
        ON_RESPONSE_FLAG_SET_SNAPSHOT_SCHE(message);
        break;
    case RESPONSE_FLAG_SET_SNAPSHOT:
        ON_RESPONSE_FLAG_SET_SNAPSHOT(message);
        break;
    case RESPONSE_FLAG_SET_ALL_SNAPSHOT:
        ON_RESPONSE_FLAG_SET_ALL_SNAPSHOT(message);
        break;
    }
}

void SnapshotSchedule::onLanguageChanged()
{
    // schedule
    ui->pushButton_erase->setText(GET_TEXT("RECORDMODE/90012", "Erase"));
    ui->pushButton_continuous->setText(GET_TEXT("SNAPSHOT/95011", "Continuous"));
    ui->pushButton_event->setText(GET_TEXT("RECORDMODE/90013", "Event"));
    ui->groupBox_event->setTitle(GET_TEXT("RECORDMODE/90013", "Event"));
    ui->pushButton_motion->setText(GET_TEXT("MOTION/51000", "Motion Detection"));
    ui->pushButton_alarm->setText(GET_TEXT("ALARMIN/52001", "Alarm Input"));
    ui->pushButton_smartEvent->setText(GET_TEXT("SMARTEVENT/55000", "VCA"));
    ui->pushButton_smartAnalysis->setText(GET_TEXT("ANPR/103054", "Smart Analysis"));
    ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButtonAudioAlarm->setText(GET_TEXT("AUDIO_ALARM/159000", "Audio Alarm"));

    //batch
    ui->label_note->setText(GET_TEXT("SNAPSHOT/95019", "<html><head/><body><p>Start Snapshot: Selected channels will be set to continuous snapshot.</p><p>Stop Snapshot: Selected channels’ continuous schedule will be erased.</p></body></html>"));
    ui->pushButton_startSnapshot->setText(GET_TEXT("SNAPSHOT/95020", "Start Snapshot"));
    ui->pushButton_stopSnapshot->setText(GET_TEXT("SNAPSHOT/95021", "Stop Snapshot"));
    ui->pushButton_back_3->setText(GET_TEXT("COMMON/1002", "Back"));


    // settings
    ui->label_quality->setText(GET_TEXT("SNAPSHOT/95004", "Snapshot Quality"));
    ui->label_interval->setText(GET_TEXT("SNAPSHOT/95005", "Snapshot Interval"));
    ui->label_dueTime->setText(GET_TEXT("SNAPSHOT/95006", "Snapshot Due Time"));
    ui->pushButton_copy_2->setText(GET_TEXT("COMMON/1005", "Copy"));
    ui->pushButton_back_2->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButton_apply_2->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->label_expiredTime->setText(GET_TEXT("RECORDADVANCE/91015", "Expired Time"));
    ui->label_expiredTime_2->setText(GET_TEXT("RECORDADVANCE/91016", "1~120 Days"));
}

void SnapshotSchedule::onTabClicked(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
    m_currenrtTab = index;
    switch (index) {
    case TabSnapshotSchedule:
        m_copyList.clear();
        ui->widget_button->setCurrentIndex(m_currentChannel);
        break;
    case TabBatchSettings:
        ui->widget_video3->playVideo(0);
        ui->widget_checkGroup->clearCheck();
        break;
    case TabSnapshotSettings:
        m_copyList.clear();
        ui->widget_video2->playVideo(m_currentChannel);
        ui->widget_button2->setCurrentIndex(m_currentChannel);
        onChannelButtonClicked(m_currentChannel);
        break;
    default:
        break;
    }
}

void SnapshotSchedule::onChannelButtonClicked(int index)
{
    ui->lineEdit_expiredTime->setValid(true);
    ui->lineEdit_interval->setValid(true);
    m_currentChannel = index;
    if (m_currenrtTab == TabSnapshotSchedule) {
        ui->widget_video->playVideo(index);
        memset(&m_dbSchedule, 0, sizeof(record_schedule));
        read_snapshot_schedule(SQLITE_FILE_NAME, &m_dbSchedule, m_currentChannel);
        showRecordSchedule();
    } else if (m_currenrtTab == TabSnapshotSettings) {
        ui->widget_video2->playVideo(index);
        memset(&m_dbSnapshot, 0, sizeof(m_dbSnapshot));
        read_snapshot(SQLITE_FILE_NAME, &m_dbSnapshot, m_currentChannel);
        //
        ui->comboBox_quality->setCurrentIndexFromData(m_dbSnapshot.quality);
        ui->comboBox_interval->setCurrentIndexFromData(m_dbSnapshot.interval_unit);
        on_comboBox_interval_activated(m_dbSnapshot.interval_unit);
        ui->lineEdit_interval->setText(QString("%1").arg(m_dbSnapshot.interval));
        if (m_dbSnapshot.expiration_date == 0) {
            ui->comboBox_dueTime->setCurrentIndexFromData(0);
            on_comboBox_dueTime_activated(0);
            ui->lineEdit_expiredTime->setText(QString("%1").arg(1));
        } else {
            ui->comboBox_dueTime->setCurrentIndexFromData(1);
            on_comboBox_dueTime_activated(1);
            ui->lineEdit_expiredTime->setText(QString("%1").arg(m_dbSnapshot.expiration_date));
        }
    }
}

void SnapshotSchedule::on_comboBox_dueTime_activated(int index)
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

void SnapshotSchedule::on_pushButton_erase_clicked()
{
    ui->widget_schedule->setCurrentType(NONE);
}

void SnapshotSchedule::on_pushButton_continuous_clicked()
{
    ui->widget_schedule->setCurrentType(TIMING_RECORD);
}

void SnapshotSchedule::on_pushButton_event_clicked()
{
    ui->widget_schedule->setCurrentType(EVENT_RECORD);
}

void SnapshotSchedule::on_pushButton_motion_clicked()
{
    ui->widget_schedule->setCurrentType(MOTION_RECORD);
}

void SnapshotSchedule::on_pushButton_alarm_clicked()
{
    ui->widget_schedule->setCurrentType(ALARM_RECORD);
}

void SnapshotSchedule::on_pushButton_smartEvent_clicked()
{
    ui->widget_schedule->setCurrentType(SMART_EVT_RECORD);
}

void SnapshotSchedule::on_pushButtonAudioAlarm_clicked()
{
    ui->widget_schedule->setCurrentType(AUDIO_ALARM_RECORD);
}

void SnapshotSchedule::on_pushButton_smartAnalysis_clicked()
{
    ui->widget_schedule->setCurrentType(ANPT_EVT_RECORD);
}

void SnapshotSchedule::showRecordSchedule()
{
    ui->widget_schedule->setSchedule(m_dbSchedule.schedule_day);
}

void SnapshotSchedule::onPushButtonCopyClicked()
{
    if (!ui->lineEdit_expiredTime->checkValid()) {
        return;
    }
    m_copyList.clear();
    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(m_currentChannel);
    int result = copy.exec();
    if (result == QDialog::Accepted) {
        m_copyList = copy.checkedList();
        QTimer::singleShot(0, this, SLOT(onPushButtonApplyClicked()));
    }
}

void SnapshotSchedule::onPushButtonApplyClicked()
{
    bool valid = ui->lineEdit_interval->checkValid();
    valid &= ui->lineEdit_expiredTime->checkValid();
    if (!valid) {
        return;
    }
    if (m_copyList.isEmpty()) {
        m_copyList.append(m_currentChannel);
    }

    if (m_currenrtTab == TabSnapshotSchedule) {
        ui->widget_schedule->getSchedule(m_dbSchedule.schedule_day);
    } else if (m_currenrtTab == TabSnapshotSettings) {

        m_dbSnapshot.quality = ui->comboBox_quality->currentData().toInt();
        m_dbSnapshot.interval = ui->lineEdit_interval->text().toInt();
        m_dbSnapshot.interval_unit = ui->comboBox_interval->currentData().toInt();

        if (ui->comboBox_dueTime->currentIndex() == 1) {
            m_dbSnapshot.expiration_date = ui->lineEdit_expiredTime->text().toInt();
        } else {
            m_dbSnapshot.expiration_date = 0;
        }
    }

    //showWait();
    QTimer::singleShot(0, this, SLOT(onCopyData()));
}

void SnapshotSchedule::onPushButtonBackClicked()
{
    emit sig_back();
}

void SnapshotSchedule::onCopyData()
{
    struct channel_for_batch req;
    int channel = 0;

    req.size = 0;
    while (!m_copyList.isEmpty()) {
        channel = m_copyList.takeFirst();
        if (m_currenrtTab == TabSnapshotSchedule) {
            write_snapshot_schedule(SQLITE_FILE_NAME, &m_dbSchedule, channel);
        } else if (m_currenrtTab == TabSnapshotSettings) {
            struct snapshot db_snapshot;
            memcpy(&db_snapshot, &m_dbSnapshot, sizeof(struct snapshot));
            db_snapshot.id = channel;
            write_snapshot(SQLITE_FILE_NAME, &db_snapshot);
        }

        req.chanid[req.size] = channel;
        req.size++;
    }

    if (req.size) {


    }
}

void SnapshotSchedule::ON_RESPONSE_FLAG_SET_SNAPSHOT_SCHE(MessageReceive *message)
{
    Q_UNUSED(message);

    //closeWait();
}

void SnapshotSchedule::ON_RESPONSE_FLAG_SET_SNAPSHOT(MessageReceive *message)
{
    Q_UNUSED(message);

    //closeWait();
}

void SnapshotSchedule::ON_RESPONSE_FLAG_SET_ALL_SNAPSHOT(MessageReceive *message)
{
    Q_UNUSED(message);

    //closeWait();
}

void SnapshotSchedule::on_comboBox_interval_activated(int index)
{
    if (index == 0) {
        ui->label_interval_2->setText(GET_TEXT("SNAPSHOT/95016", "10~86400 Seconds"));
        ui->lineEdit_interval->setCheckMode(MyLineEdit::RangeCheck, 10, 86400);
    } else if (index == 1) {
        ui->label_interval_2->setText(GET_TEXT("SNAPSHOT/95017", "1~1440 Minutes"));
        ui->lineEdit_interval->setCheckMode(MyLineEdit::RangeCheck, 1, 1440);
    } else if (index == 2) {
        ui->label_interval_2->setText(GET_TEXT("SNAPSHOT/95018", "1~24 Hours"));
        ui->lineEdit_interval->setCheckMode(MyLineEdit::RangeCheck, 1, 24);
    }
}

void SnapshotSchedule::onChannelCheckBoxClicked(int channel, bool checked)
{
    if (checked) {
        ui->widget_video3->playVideo(channel);
    }
}

void SnapshotSchedule::onPushButtonStartSnapshotClicked()
{
    //showWait();

    //int chan_no = 0;
    struct resp_set_all_record allRecord;
    memset(&allRecord, 0, sizeof(struct resp_set_all_record));
    allRecord.enable = 1;
    QList<bool> checkStateList = ui->widget_checkGroup->checkStateList();
    for (int i = 0; i < checkStateList.size(); ++i) {
        if (checkStateList.at(i)) {
            allRecord.channel[i] = 1;
            //chan_no = i + 1;
        }
    }
    sendMessage(REQUEST_FLAG_SET_ALL_SNAPSHOT, &allRecord, sizeof(struct resp_set_all_record));
#if 0
    struct log_data log_data;
    memset(&log_data, 0, sizeof(struct log_data));
    log_data.log_data_info.subType = SUB_OP_CONFIG_LOCAL;
    log_data.log_data_info.parameter_type = SUB_PARAM_RECORD_SCHED;
    snprintf(log_data.log_data_info.user, sizeof(log_data.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
    log_data.log_data_info.chan_no = chan_no;
    MsWriteLog(log_data);
#endif
}

void SnapshotSchedule::onPushButtonStopSnapshotClicked()
{
    //showWait();

    //int chan_no = 0;
    struct resp_set_all_record allRecord;
    memset(&allRecord, 0, sizeof(struct resp_set_all_record));
    allRecord.enable = 0;
    QList<bool> checkStateList = ui->widget_checkGroup->checkStateList();
    for (int i = 0; i < checkStateList.size(); ++i) {
        if (checkStateList.at(i)) {
            allRecord.channel[i] = 1;
            //chan_no = i + 1;
        }
    }
    sendMessage(REQUEST_FLAG_SET_ALL_SNAPSHOT, &allRecord, sizeof(struct resp_set_all_record));
#if 0
    struct log_data log_data;
    memset(&log_data, 0, sizeof(struct log_data));
    log_data.log_data_info.subType = SUB_OP_CONFIG_LOCAL;
    log_data.log_data_info.parameter_type = SUB_PARAM_RECORD_SCHED;
    snprintf(log_data.log_data_info.user, sizeof(log_data.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
    log_data.log_data_info.chan_no = chan_no;
    MsWriteLog(log_data);
#endif
}
