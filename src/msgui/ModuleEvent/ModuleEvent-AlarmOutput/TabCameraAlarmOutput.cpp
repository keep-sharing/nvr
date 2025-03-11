#include "TabCameraAlarmOutput.h"
#include "ui_TabCameraAlarmOutput.h"
#include "EffectiveTimeCameraAlarmOutput.h"
#include "EventLoop.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsMessage.h"
#include "MsWaitting.h"
#include "MyButtonGroup.h"
#include "channelcopydialog.h"

TabCameraAlarmOutput::TabCameraAlarmOutput(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabCameraAlarmOutput)
{
    ui->setupUi(this);

    ui->channelGroup->setCount(qMsNvr->maxChannel());
    connect(ui->channelGroup, SIGNAL(buttonClicked(int)), this, SLOT(onChannelGroupClicked(int)));

    m_alarmButtonGroup = new MyButtonGroup(this);
    m_alarmButtonGroup->addButton(ui->pushButton_alarm1, Alarmoutput1);
    m_alarmButtonGroup->addButton(ui->pushButton_alarm2, Alarmoutput2);
    connect(m_alarmButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onAlarmOutputGroupClicked(int)));

    ui->comboBox_type->clear();
    ui->comboBox_type->addItem(GET_TEXT("ALARMIN/52005", "NO"), 0);
    ui->comboBox_type->addItem(GET_TEXT("ALARMIN/52006", "NC"), 1);

    ui->comboBox_delay->clear();
    ui->comboBox_delay->addItem("1s", 1);
    ui->comboBox_delay->addItem("5s", 5);
    ui->comboBox_delay->addItem("10s", 10);
    ui->comboBox_delay->addItem("30s", 30);
    ui->comboBox_delay->addItem("60s", 60);
    ui->comboBox_delay->addItem("120s", 120);
    ui->comboBox_delay->addItem("300s", 300);
    ui->comboBox_delay->addItem("600s", 600);
    ui->comboBox_delay->addItem(GET_TEXT("EVENTSTATUS/63011", "Manually Clear"), -1);

    m_effective = new EffectiveTimeCameraAlarmOutput(this);

    setSettingEnable(false);
    onLanguageChanged();
}

TabCameraAlarmOutput::~TabCameraAlarmOutput()
{
    delete ui;
}

void TabCameraAlarmOutput::initializeData()
{
    ui->channelGroup->setCurrentIndex(0);
}

void TabCameraAlarmOutput::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPC_ALARM_OUTPUT:
        ON_RESPONSE_FLAG_GET_IPC_ALARM_OUTPUT(message);
        break;
    case RESPONSE_FLAG_SET_IPC_ALARM_OUTPUT:
        ON_RESPONSE_FLAG_SET_IPC_ALARM_OUTPUT(message);
        break;
    }
}

void TabCameraAlarmOutput::ON_RESPONSE_FLAG_GET_IPC_ALARM_OUTPUT(MessageReceive *message)
{
    //closeWait();

    memset(&m_ipc_alarm_out, 0, sizeof(ms_ipc_alarm_out));
    ms_ipc_alarm_out *ipc_alarm_out = (ms_ipc_alarm_out *)message->data;
    if (!ipc_alarm_out) {
        return;
    }
    memcpy(&m_ipc_alarm_out, ipc_alarm_out, sizeof(ms_ipc_alarm_out));

    if (ipc_alarm_out->alarmCnt > 0) {
        ui->pushButton_alarm2->setVisible(ipc_alarm_out->alarmCnt > 1);
        ui->widgetMessage->hide();
        readAlarmoutName();
        setSettingEnable(true);
        showAlarmoutput(Alarmoutput1);
        ui->pushButton_alarm1->setChecked(true);
    } else {
        setSettingEnable(false);
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
    }
}

void TabCameraAlarmOutput::ON_RESPONSE_FLAG_SET_IPC_ALARM_OUTPUT(MessageReceive *message)
{
    Q_UNUSED(message)

    gEventLoopExit(0);
}

void TabCameraAlarmOutput::setSettingEnable(bool enable)
{
    ui->widget_settings->setEnabled(enable);
    ui->pushButton_copy->setEnabled(enable);
    ui->pushButton_apply->setEnabled(enable);
}

void TabCameraAlarmOutput::showAlarmoutput(TabCameraAlarmOutput::AlarmIndex index)
{
    alarm_chn_out_name &alarm_name = m_alarm_name_array[index];
    ui->lineEdit_name->setText(QString(alarm_name.name));
    ui->comboBox_delay->setCurrentIndexFromData(alarm_name.delay_time);

    ui->comboBox_type->setCurrentIndexFromData(m_ipc_alarm_out.alarmType[index]);
}

void TabCameraAlarmOutput::saveAlarmoutput(int channel, TabCameraAlarmOutput::AlarmIndex index)
{
    writeAlarmoutName(channel);

    ms_ipc_alarm_out_cfg ipc_alarm_out_cfg;
    memset(&ipc_alarm_out_cfg, 0, sizeof(ms_ipc_alarm_out_cfg));
    ipc_alarm_out_cfg.chanid = channel;
    ipc_alarm_out_cfg.alarmid = index;
    ipc_alarm_out_cfg.alarmType = m_ipc_alarm_out.alarmType[index];
    qDebug() << QString("CameraAlarmOutput::saveAlarmoutput, channel: %1, alarm index: %2, type: %3")
                    .arg(ipc_alarm_out_cfg.chanid)
                    .arg(ipc_alarm_out_cfg.alarmid)
                    .arg(ipc_alarm_out_cfg.alarmType);
    sendMessage(REQUEST_FLAG_SET_IPC_ALARM_OUTPUT, &ipc_alarm_out_cfg, sizeof(ms_ipc_alarm_out_cfg));
}

void TabCameraAlarmOutput::readAlarmoutName()
{
    for (int i = 0; i < m_ipc_alarm_out.alarmCnt; ++i) {
        alarm_chn_out_name &alarm_name = m_alarm_name_array[i];
        memset(&alarm_name, 0, sizeof(alarm_chn_out_name));

        alarm_chn alarm_channel;
        alarm_channel.chnid = m_currentChannel;
        alarm_channel.alarmid = i;
        read_alarm_chnOut_event_name(SQLITE_FILE_NAME, &alarm_name, &alarm_channel);
    }
}

void TabCameraAlarmOutput::writeAlarmoutName(int channel)
{
    for (int i = 0; i < m_ipc_alarm_out.alarmCnt; ++i) {
        alarm_chn_out_name &alarm_name = m_alarm_name_array[i];

        //Alarm Name不可Copy
        if (channel != m_currentChannel) {
            alarm_chn_out_name temp_alarm_name;

            alarm_chn alarm_channel;
            alarm_channel.chnid = channel;
            alarm_channel.alarmid = i;
            read_alarm_chnOut_event_name(SQLITE_FILE_NAME, &temp_alarm_name, &alarm_channel);

            temp_alarm_name.delay_time = alarm_name.delay_time;
            write_alarm_chnOut_event_name(SQLITE_FILE_NAME, &temp_alarm_name);
        } else {
            write_alarm_chnOut_event_name(SQLITE_FILE_NAME, &alarm_name);
        }
    }
    sendMessageOnly(REQUEST_FLAG_SET_IPC_ALARMOUT_EVENT, (void *)&channel, sizeof(int));
}

void TabCameraAlarmOutput::onLanguageChanged()
{
    ui->label_output_number->setText(GET_TEXT("ALARMOUT/53001", "Alarm Output No."));
    ui->label_name->setText(GET_TEXT("ALARMIN/52003", "Alarm Name"));
    ui->label_type->setText(GET_TEXT("ALARMIN/52004", "Alarm Type"));
    ui->label_delay->setText(GET_TEXT("ALARMOUT/53002", "Delay"));
    ui->label_effectiveTime->setText(GET_TEXT("SMARTEVENT/55015", "Effective Time"));
    ui->pushButton_effectiveTime->setText(GET_TEXT("COMMON/1019", "Edit"));

    ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void TabCameraAlarmOutput::onChannelGroupClicked(int index)
{
    if (m_effective) {
        m_effective->clearCache();
    }
    m_currentChannel = index;
    ui->lineEdit_name->clear();
    ui->comboBox_type->setCurrentIndex(0);
    ui->comboBox_delay->setCurrentIndex(0);

    ui->video->playVideo(index);
    if (!LiveView::instance()->isChannelConnected(m_currentChannel)) {
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
        setSettingEnable(false);
        return;
    } else {
        ui->widgetMessage->hide();
    }
    sendMessage(REQUEST_FLAG_GET_IPC_ALARM_OUTPUT, &m_currentChannel, sizeof(int));
    //showWait();
}

void TabCameraAlarmOutput::onAlarmOutputGroupClicked(int index)
{
    showAlarmoutput(AlarmIndex(index));
}

void TabCameraAlarmOutput::on_lineEdit_name_textEdited(const QString &text)
{
    int alarmIndex = m_alarmButtonGroup->checkedId();
    alarm_chn_out_name &alarm_name = m_alarm_name_array[alarmIndex];
    snprintf(alarm_name.name, sizeof(alarm_name.name), "%s", text.toStdString().c_str());
}

void TabCameraAlarmOutput::on_comboBox_type_activated(int index)
{
    int alarmIndex = m_alarmButtonGroup->checkedId();
    m_ipc_alarm_out.alarmType[alarmIndex] = ui->comboBox_type->itemData(index).toInt();
}

void TabCameraAlarmOutput::on_comboBox_delay_activated(int index)
{
    int alarmIndex = m_alarmButtonGroup->checkedId();
    alarm_chn_out_name &alarm_name = m_alarm_name_array[alarmIndex];
    alarm_name.delay_time = ui->comboBox_delay->itemData(index).toInt();
}

void TabCameraAlarmOutput::on_comboBox_delay_currentIndexChanged(int index)
{
    int value = ui->comboBox_delay->itemData(index).toInt();
    if (value == -1) {
        ui->pushButton_clearAlarm->show();
    } else {
        ui->pushButton_clearAlarm->hide();
    }
}

void TabCameraAlarmOutput::on_pushButton_effectiveTime_clicked()
{
    int alarmIndex = m_alarmButtonGroup->checkedId();

    m_effective->showEffectiveTime(m_currentChannel, alarmIndex);
}

void TabCameraAlarmOutput::on_pushButton_clearAlarm_clicked()
{

}

void TabCameraAlarmOutput::on_pushButton_copy_clicked()
{
    ChannelCopyDialog channelCopy(this);
    channelCopy.setCurrentChannel(m_currentChannel);
    int result = channelCopy.exec();
    if (result == QDialog::Accepted) {
        QList<int> copyList = channelCopy.checkedList(false);
        quint64 copyFlags = channelCopy.checkedFlags(false);

        m_effective->saveEffectiveTime();

        //copy effective time and action
        alarm_chn alarmChannel1;
        alarmChannel1.chnid = m_currentChannel;
        alarmChannel1.alarmid = Alarmoutput1;
        alarm_chn alarmChannel2;
        alarmChannel2.chnid = m_currentChannel;
        alarmChannel2.alarmid = Alarmoutput2;
        //effective time
        smart_event_schedule *currentEffectiveTimeSchedule1 = new smart_event_schedule;
        memset(currentEffectiveTimeSchedule1, 0, sizeof(smart_event_schedule));
        read_alarm_chnOut_effective_schedule(SQLITE_FILE_NAME, currentEffectiveTimeSchedule1, &alarmChannel1);
        copy_alarm_chnOut_effective_schedules(SQLITE_FILE_NAME, currentEffectiveTimeSchedule1, &alarmChannel1, copyFlags);
        delete currentEffectiveTimeSchedule1;
        smart_event_schedule *currentEffectiveTimeSchedule2 = new smart_event_schedule;
        memset(currentEffectiveTimeSchedule2, 0, sizeof(smart_event_schedule));
        read_alarm_chnOut_effective_schedule(SQLITE_FILE_NAME, currentEffectiveTimeSchedule2, &alarmChannel2);
        copy_alarm_chnOut_effective_schedules(SQLITE_FILE_NAME, currentEffectiveTimeSchedule2, &alarmChannel2, copyFlags);
        delete currentEffectiveTimeSchedule2;
        sendMessageOnly(REQUEST_FLAG_SET_IPC_ALARMOUT_EVENT, (void *)&copyFlags, sizeof(copyFlags));

        //
        //showWait();
        for (int i = 0; i < copyList.size(); ++i) {
            int channel = copyList.at(i);
            saveAlarmoutput(channel, Alarmoutput1);
            gEventLoopExec();
            saveAlarmoutput(channel, Alarmoutput2);
            gEventLoopExec();
        }
        //closeWait();
    }

    ui->pushButton_copy->clearUnderMouse();
}

void TabCameraAlarmOutput::on_pushButton_apply_clicked()
{
    if (!ui->lineEdit_name->checkValid()) {
        return;
    }
    //showWait();

    quint64 flag = 0;
    ms_set_bit(&flag, m_currentChannel, 1);
    m_effective->saveEffectiveTime();
    sendMessageOnly(REQUEST_FLAG_SET_IPC_ALARMOUT_EVENT, &flag, sizeof(flag));

    saveAlarmoutput(m_currentChannel, Alarmoutput1);
    gEventLoopExec();
    saveAlarmoutput(m_currentChannel, Alarmoutput2);
    gEventLoopExec();

    //closeWait();
}

void TabCameraAlarmOutput::on_pushButton_back_clicked()
{
    emit sig_back();
}
