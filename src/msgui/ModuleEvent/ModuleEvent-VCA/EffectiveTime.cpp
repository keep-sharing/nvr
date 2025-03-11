#include "EffectiveTime.h"
#include "ui_EffectiveTime.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"

EffectiveTime::EffectiveTime(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::EffectiveTime)
{
    ui->setupUi(this);
    //setTitleWidget(ui->widget_title);

    connect(ui->pushButton_ok, SIGNAL(clicked(bool)), this, SLOT(onPushButton_okClicked()));
    connect(ui->pushButton_cancel, SIGNAL(clicked(bool)), this, SLOT(onPushButton_cancelClicked()));

    QButtonGroup *buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(ui->pushButton_effective);
    buttonGroup->addButton(ui->pushButton_erase);
    ui->pushButton_effective->setChecked(true);

    onLanguageChanged();
}

EffectiveTime::~EffectiveTime()
{
    clearCache();
    delete ui;
}

void EffectiveTime::readSmartData(int channel, SMART_EVENT_TYPE type)
{
    if (m_eventSchedule) {
        return;
    } else {
        m_eventSchedule = new smart_event_schedule;
    }
    ui->label_effective->setColor(QString("#FCAEC8"));
    m_channel = channel;
    m_actionType = SMART_EVT_RECORD;
    m_smartType = type;
    read_smart_event_effective_schedule(SQLITE_FILE_NAME, m_eventSchedule, channel, type);
    ui->schedule->setCurrentType(SMART_EVT_RECORD);
    ui->schedule->setSchedule(m_eventSchedule->schedule_day);
    ui->schedule->setSingleEditType(SMART_EVT_RECORD);
}

void EffectiveTime::readMotionData(int channel)
{
    if (m_eventSchedule) {
        return;
    } else {
        m_eventSchedule = new smart_event_schedule;
    }
    ui->label_effective->setColor(QString("#1BC15B"));
    m_channel = channel;
    m_actionType = MOTION_ACTION;
    read_motion_effective_schedule(SQLITE_FILE_NAME, m_eventSchedule, channel);
    ui->schedule->setCurrentType(MOTION_ACTION);
    ui->schedule->setSchedule(m_eventSchedule->schedule_day);
    ui->schedule->setSingleEditType(MOTION_ACTION);
}

void EffectiveTime::readAlarminData(int channel)
{
    if (m_eventSchedule) {
        return;
    } else {
        m_eventSchedule = new smart_event_schedule;
    }
    ui->label_effective->setColor(QString("#FF3600"));
    m_channel = channel;
    m_actionType = ALARMIN_ACTION;
    read_alarmin_effective_schedule(SQLITE_FILE_NAME, m_eventSchedule, channel);
    ui->schedule->setCurrentType(ALARMIN_ACTION);
    ui->schedule->setSchedule(m_eventSchedule->schedule_day);
    ui->schedule->setSingleEditType(ALARMIN_ACTION);
}

void EffectiveTime::readAlarmoutData(int channel)
{
    if (m_alarm_out_schedule) {
        return;
    } else {
        m_alarm_out_schedule = new alarm_out_schedule;
    }
    memset(m_alarm_out_schedule, 0, sizeof(alarm_out_schedule));

    ui->label_effective->setColor(QString("#FF3600"));
    m_channel = channel;
    m_actionType = ALARMOUT_ACTION;
    read_alarm_out_schedule(SQLITE_FILE_NAME, m_alarm_out_schedule, channel);
    ui->schedule->setCurrentType(ALARMOUT_ACTION);
    ui->schedule->setSchedule(m_alarm_out_schedule->schedule_day);
    ui->schedule->setSingleEditType(ALARMOUT_ACTION);
}

void EffectiveTime::readAnprData(int channel, ANPR_MODE_TYPE type)
{
    if (m_eventSchedule) {
        return;
    } else {
        m_eventSchedule = new smart_event_schedule;
    }
    ui->label_effective->setColor(QString("#FFE793"));
    m_channel = channel;
    m_actionType = ANPT_EVT_RECORD;
    m_anprType = type;
    read_anpr_effective_schedule(SQLITE_FILE_NAME, m_eventSchedule, channel, type);
    ui->schedule->setCurrentType(ANPT_EVT_RECORD);
    ui->schedule->setSchedule(m_eventSchedule->schedule_day);
    ui->schedule->setSingleEditType(ANPT_EVT_RECORD);
}

void EffectiveTime::clearCache()
{
    if (m_eventSchedule) {
        delete m_eventSchedule;
        m_eventSchedule = nullptr;
    }
    if (m_alarm_out_schedule) {
        delete m_alarm_out_schedule;
        m_alarm_out_schedule = nullptr;
    }
}

void EffectiveTime::apply()
{
    REQ_UPDATE_CHN req = {0};
    switch (m_actionType) {
    case MOTION_ACTION:
        if (!m_eventSchedule) {
            return;
        }
        memset(m_eventSchedule, 0, sizeof(smart_event_schedule));
        ui->schedule->getSchedule(m_eventSchedule->schedule_day);
        qDebug()<<"[gsjt debug]write to motion effective from effective time by channel:"<<m_channel;
        write_motion_effective_schedule(SQLITE_FILE_NAME, m_eventSchedule, m_channel);
        break;
    case SMART_EVT_RECORD:
        if (!m_eventSchedule) {
            return;
        }
        memset(m_eventSchedule, 0, sizeof(smart_event_schedule));
        ui->schedule->getSchedule(m_eventSchedule->schedule_day);
        write_smart_event_effective_schedule(SQLITE_FILE_NAME, m_eventSchedule, m_channel, (SMART_EVENT_TYPE)m_smartType);
        ms_set_bit(&req.chnMask, m_channel, 1);
        sendMessageOnly(REQUEST_FLAG_SET_SMART_EVENT, (void *)&req, sizeof(req));
        break;
    case ANPT_EVT_RECORD:
        if (!m_eventSchedule) {
            return;
        }
        memset(m_eventSchedule, 0, sizeof(smart_event_schedule));
        ui->schedule->getSchedule(m_eventSchedule->schedule_day);
        write_anpr_effective_schedule(SQLITE_FILE_NAME, m_eventSchedule, m_channel, (ANPR_MODE_TYPE)m_anprType);
        break;
    case ALARMIN_ACTION:
        if (!m_eventSchedule) {
            return;
        }
        memset(m_eventSchedule, 0, sizeof(smart_event_schedule));
        ui->schedule->getSchedule(m_eventSchedule->schedule_day);
        write_alarmin_effective_schedule(SQLITE_FILE_NAME, m_eventSchedule, m_channel);
        sendMessage(REQUEST_FLAG_SET_ALARMIN, (void *)&m_channel, sizeof(int));
        break;
    case ALARMOUT_ACTION:
        if (!m_alarm_out_schedule) {
            return;
        }
        memset(m_alarm_out_schedule, 0, sizeof(alarm_out_schedule));
        ui->schedule->getSchedule(m_alarm_out_schedule->schedule_day);
        write_alarm_out_schedule(SQLITE_FILE_NAME, m_alarm_out_schedule, m_channel);
        sendMessageOnly(REQUEST_FLAG_SET_ALARMOUT, (void *)&m_channel, sizeof(int));
        break;
    default:
        break;
    }

    clearCache();
}

bool EffectiveTime::isAddToVisibleList()
{
    return true;
}

void EffectiveTime::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("SMARTEVENT/55015", "Effective Time"));

    ui->pushButton_effective->setText(GET_TEXT("SMARTEVENT/55015", "Effective Time"));
    ui->pushButton_erase->setText(GET_TEXT("RECORDMODE/90012", "Erase"));

    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void EffectiveTime::on_pushButton_effective_clicked()
{
    ui->schedule->setCurrentType(m_actionType);
}

void EffectiveTime::on_pushButton_erase_clicked()
{
    ui->schedule->setCurrentType(NONE);
}

void EffectiveTime::onPushButton_okClicked()
{
    close();
}

void EffectiveTime::onPushButton_cancelClicked()
{
    clearCache();
    close();
}
