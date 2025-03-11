#include "heatmapschedule.h"
#include "ui_EffectiveTime.h"
#include "MsLanguage.h"

HeatMapSchedule::HeatMapSchedule(QWidget *parent) :
    EffectiveTime(parent)
{
    ui->schedule->setHolidayVisible(false);
    //兼容IPC的排程有个enable概念
    ui->schedule->setCheckDayEnable(true);

    //FIXME 等中心定义，非0
    m_actionType = 20;

    ui->schedule->setTypeColor(m_actionType, QColor("#2CB8E4"));
    ui->label_effective->setColor(QString("#2CB8E4"));

    //
    ui->label_title->setText(GET_TEXT("HEATMAP/104006", "Heat Map Schedule"));
    ui->pushButton_effective->setText(GET_TEXT("HEATMAP/104000", "Heat Map"));
}

void HeatMapSchedule::showSchedule(schedule_day *day_array)
{
    m_dayArray = day_array;

    //兼容IPC没有ActionType的情况，自定义一个非0值
    for (int i = 0; i < 8; ++i)
    {
        schedule_day &day = m_dayArray[i];
        schdule_item *item_array = day.schedule_item;
        for (int j = 0; j < 48; ++j)
        {
            schdule_item &item = item_array[j];
            item.action_type = m_actionType;
        }
    }

    //
    ui->schedule->setCurrentType(m_actionType);
    ui->schedule->setSchedule(m_dayArray);
    ui->schedule->setSingleEditType(m_actionType);
}

void HeatMapSchedule::onPushButton_okClicked()
{
    ui->schedule->getSchedule(m_dayArray);
    accept();
}

void HeatMapSchedule::onPushButton_cancelClicked()
{
    reject();
}
