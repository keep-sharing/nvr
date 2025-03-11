#include "EffectiveTimePeopleCounting.h"
#include "MsLanguage.h"
#include "ui_EffectiveTimeAbstract.h"


EffectiveTimePeopleCounting::EffectiveTimePeopleCounting(QWidget *parent)
    : EffectiveTimeAbstract(parent)
{
    //兼容IPC的排程有个enable概念
    ui->schedule->setCheckDayEnable(true);
}

EffectiveTimePeopleCounting::EffectiveTimePeopleCounting(int mode, QWidget *parent)
    : EffectiveTimeAbstract(parent)
{
    m_mode = mode;
}


EffectiveTimePeopleCounting::~EffectiveTimePeopleCounting()
{
}

void EffectiveTimePeopleCounting::setSchedule(schedule_day *scheduleDay)
{
    auto &data = m_dataMap[0];
    memcpy(&data.schedule, scheduleDay, sizeof(data.schedule));
    for (int i = 0; i < MAX_DAY_NUM; i++) {
        for (int j = 0; j < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; j++)
            data.schedule.schedule_day[i].schedule_item[j].action_type = scheduleType();
    }
}

void EffectiveTimePeopleCounting::getSchedule(schedule_day *scheduleDay)
{
    auto &data = m_dataMap[dataIndex()];
    memcpy(scheduleDay, &data.schedule, sizeof(data.schedule));
    QString strZeroTime = QString("00:00");

    for (int i = 0; i < MAX_DAY_NUM; i++) {
        for (int j = 0; j < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; j++) {
            if (scheduleDay[i].schedule_item[j].start_time[0] == '\0' || scheduleDay[i].schedule_item[j].end_time[0] == '\0') {
                snprintf(scheduleDay[i].schedule_item[j].start_time, sizeof(scheduleDay[i].schedule_item[j].start_time), "%s", strZeroTime.toStdString().c_str());
                snprintf(scheduleDay[i].schedule_item[j].end_time, sizeof(scheduleDay[i].schedule_item[j].end_time), "%s", strZeroTime.toStdString().c_str());
            }
        }
        scheduleDay[i].wholeday_enable = 1;
    }
}

QString EffectiveTimePeopleCounting::titleText() const
{
    QString str;
    switch (m_mode) {
    case PeopleCountingSettingsMode:
        str = GET_TEXT("REGIONAL_PEOPLECOUNTING/158002", "People Counting Schedule");
        break;
    case RegionalPeopleCountingSettingsMode:
        str = GET_TEXT("REGIONAL_PEOPLECOUNTING/158005", "Regional People Counting Schedule");
        break;
    case HeatMapMode:
        str = GET_TEXT("REGIONAL_PEOPLECOUNTING/158006", "Heatmap Schedule");
        break;
    default:
        str = GET_TEXT("SMARTEVENT/55015", "Effective Time");
    }
    return str;
}

QString EffectiveTimePeopleCounting::pushButtonEffectiveText() const
{
    return titleText();
}

QColor EffectiveTimePeopleCounting::scheduleColor() const
{
    return QColor("#ffe793");
}

int EffectiveTimePeopleCounting::scheduleType() const
{
    return ANPT_EVT_RECORD;
}

schedule_day *EffectiveTimePeopleCounting::readSchedule()
{
    if (m_dataMap.contains(dataIndex())) {
        auto &data = m_dataMap[dataIndex()];
        return data.schedule.schedule_day;
    } else {
        auto &data = m_dataMap[dataIndex()];
        memset(&data.schedule, 0, sizeof(data.schedule));
        return data.schedule.schedule_day;
    }
}

void EffectiveTimePeopleCounting::saveSchedule()
{
}

void EffectiveTimePeopleCounting::saveSchedule(int dataIndex)
{
    Q_UNUSED(dataIndex)
}
