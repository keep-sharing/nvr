#include "TabNvrAlarmOutput.h"
#include "ui_TabNvrAlarmOutput.h"
#include "EffectiveTimeNvrAlarmOutput.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsMessage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "channelcopydialog.h"

TabNvrAlarmOutput::TabNvrAlarmOutput(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabNvrAlarmOutput)
{
    ui->setupUi(this);

    int alarmOutCount = qMsNvr->maxAlarmOutput();
    if (alarmOutCount < 2) {
        ui->pushButton_copy->hide();
    }
    ui->buttonGroup_alarm->setCount(alarmOutCount);
    connect(ui->buttonGroup_alarm, SIGNAL(buttonClicked(int)), this, SLOT(onButtonGroupClicked(int)));

    ui->comboBox_alarmType->clear();
    ui->comboBox_alarmType->addItem(GET_TEXT("ALARMIN/52005", "NO"), 0);
    ui->comboBox_alarmType->addItem(GET_TEXT("ALARMIN/52006", "NC"), 1);

    ui->comboBox_dwellTime->clear();
    ui->comboBox_dwellTime->addItem("1s", 1);
    ui->comboBox_dwellTime->addItem("5s", 5);
    ui->comboBox_dwellTime->addItem("10s", 10);
    ui->comboBox_dwellTime->addItem("30s", 30);
    ui->comboBox_dwellTime->addItem("60s", 60);
    ui->comboBox_dwellTime->addItem("120s", 120);
    ui->comboBox_dwellTime->addItem("300s", 300);
    ui->comboBox_dwellTime->addItem("600s", 600);
    ui->comboBox_dwellTime->addItem(GET_TEXT("EVENTSTATUS/63011", "Manually Clear"), -1);

    m_effective = new EffectiveTimeNvrAlarmOutput(this);

    connect(ui->pushButton_copy, SIGNAL(clicked(bool)), this, SLOT(onPushButtonCopyClicked()));
    connect(ui->pushButton_apply, SIGNAL(clicked(bool)), this, SLOT(onPushButtonApplyClicked()));
    connect(ui->pushButton_back, SIGNAL(clicked(bool)), this, SLOT(onPushButtonBackClicked()));
    onLanguageChanged();
}

TabNvrAlarmOutput::~TabNvrAlarmOutput()
{
    if (m_alarmoutSchedule) {
        delete m_alarmoutSchedule;
        m_alarmoutSchedule = nullptr;
    }
    delete ui;
}

void TabNvrAlarmOutput::initializeData()
{
    ui->buttonGroup_alarm->setCurrentIndex(0);
}

void TabNvrAlarmOutput::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void TabNvrAlarmOutput::onLanguageChanged()
{
    ui->label_alarmOut->setText(GET_TEXT("ALARMOUT/53001", "Alarm Output No."));
    ui->label_alarmName->setText(GET_TEXT("ALARMIN/52003", "Alarm Name"));
    ui->label_alarmType->setText(GET_TEXT("ALARMIN/52004", "Alarm Type"));
    ui->label_dwellTime->setText(GET_TEXT("ALARMOUT/53002", "Delay Time"));
    ui->label_effectiveTime->setText(GET_TEXT("ALARMOUT/53003", "Effective Time"));
    ui->pushButton_effectiveTime->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->label_note->setText(GET_TEXT("ALARMIN/52010", "Note:Alarm Name will not be copied into other channels."));
    ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
}

void TabNvrAlarmOutput::copyData()
{
    int channel;
    if (m_copyList.isEmpty()) {
        return;
    } else {
        channel = m_copyList.takeFirst();
        if (channel == m_currentIndex) {
            write_alarm_out(SQLITE_FILE_NAME, &m_alarmout);

            sendMessageOnly(REQUEST_FLAG_SET_ALARMOUT, &m_currentIndex, sizeof(int));
        } else {
            //copy action
            m_alarmout.id = channel;
            //alarm name不拷贝
            alarm_out tempAlarm;
            memset(&tempAlarm, 0, sizeof(alarm_out));
            read_alarm_out(SQLITE_FILE_NAME, &tempAlarm, channel);
            tempAlarm.enable = MODE_BY_SCHED;
            memcpy(m_alarmout.name, tempAlarm.name, sizeof(m_alarmout.name));
            write_alarm_out(SQLITE_FILE_NAME, &m_alarmout);

            write_alarm_out_schedule(SQLITE_FILE_NAME, m_alarmoutSchedule, channel);

            sendMessageOnly(REQUEST_FLAG_SET_ALARMOUT, &channel, sizeof(int));
        }
    }
}

void TabNvrAlarmOutput::onButtonGroupClicked(int index)
{
    if (m_effective) {
        m_effective->clearCache();
    }
    ui->lineEdit_alarmName->setValid(true);
    m_currentIndex = index;

    memset(&m_alarmout, 0, sizeof(alarm_out));
    read_alarm_out(SQLITE_FILE_NAME, &m_alarmout, m_currentIndex);

    //
    ui->lineEdit_alarmName->setText(QString(m_alarmout.name));
    ui->comboBox_alarmType->setCurrentIndexFromData(m_alarmout.type);
    ui->comboBox_dwellTime->setCurrentIndexFromData(m_alarmout.duration_time);
}

void TabNvrAlarmOutput::on_comboBox_dwellTime_currentIndexChanged(int index)
{
    int value = ui->comboBox_dwellTime->itemData(index).toInt();
    if (value == -1) {
        ui->pushButton_clearAlarm->show();
    } else {
        ui->pushButton_clearAlarm->hide();
    }
}

void TabNvrAlarmOutput::on_pushButton_effectiveTime_clicked()
{
    ui->pushButton_effectiveTime->clearUnderMouse();

    m_effective->showEffectiveTime(-1, m_currentIndex);
}

void TabNvrAlarmOutput::on_pushButton_clearAlarm_clicked()
{

}

void TabNvrAlarmOutput::onPushButtonCopyClicked()
{
    m_copyList.clear();

    ChannelCopyDialog copy(this);
    copy.setTitle(GET_TEXT("ALARMIN/52008", "Alarm Output Copy"));
    copy.setCount(qMsNvr->maxAlarmOutput());
    copy.setCurrentChannel(m_currentIndex);
    int result = copy.exec();
    if (result == ChannelCopyDialog::Accepted) {
        m_effective->saveEffectiveTime();
        if (!m_alarmoutSchedule) {
            m_alarmoutSchedule = new alarm_out_schedule;
        }
        memset(m_alarmoutSchedule, 0, sizeof(alarm_out_schedule));
        read_alarm_out_schedule(SQLITE_FILE_NAME, m_alarmoutSchedule, m_currentIndex);

        m_copyList = copy.checkedList();
        QTimer::singleShot(0, this, SLOT(onPushButtonApplyClicked()));
    }
}

void TabNvrAlarmOutput::onPushButtonApplyClicked()
{
    if (!ui->lineEdit_alarmName->checkValid()) {
        return;
    }
    m_effective->saveEffectiveTime();

    m_alarmout.id = m_currentIndex;
    snprintf(m_alarmout.name, sizeof(m_alarmout.name), "%s", ui->lineEdit_alarmName->text().toStdString().c_str());
    m_alarmout.type = ui->comboBox_alarmType->currentData().toInt();
    m_alarmout.enable = MODE_BY_SCHED;
    m_alarmout.duration_time = ui->comboBox_dwellTime->currentData().toInt();

    if (m_copyList.isEmpty()) {
        m_copyList.append(m_currentIndex);
    }

    //这个消息没有返回，所以这样做
    //showWait();
    while (!m_copyList.isEmpty()) {
        copyData();
        qApp->processEvents();
    }
    //closeWait();
}

void TabNvrAlarmOutput::onPushButtonBackClicked()
{
    emit sig_back();
}
