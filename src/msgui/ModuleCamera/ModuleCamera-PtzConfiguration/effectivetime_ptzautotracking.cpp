#include "effectivetime_ptzautotracking.h"
#include "ui_EffectiveTime.h"
#include "MsLanguage.h"

EffectiveTime_PtzAutoTracking::EffectiveTime_PtzAutoTracking(QWidget *parent) :
    EffectiveTime(parent)
{
    ui->schedule->setHolidayVisible(false);

    m_actionType = 13;

    ui->label_title->setText(GET_TEXT("PTZCONFIG/36060", "Auto Tracking Schedule"));
    ui->pushButton_effective->setText(GET_TEXT("PTZCONFIG/36052", "Auto Tracking"));
}

void EffectiveTime_PtzAutoTracking::showEffectiveTime(schedule_day *day_array)
{
    m_dayArray = day_array;

    ui->label_effective->setColor(QString("#2CB8E4"));

    //
    ui->schedule->setTypeColor(m_actionType, QColor("#2CB8E4"));
    ui->schedule->setCurrentType(m_actionType);
    ui->schedule->setSchedule(m_dayArray);
    ui->schedule->setSingleEditType(m_actionType);
}

void EffectiveTime_PtzAutoTracking::onPushButton_okClicked()
{
    ui->schedule->getSchedule(m_dayArray);
    accept();
}

void EffectiveTime_PtzAutoTracking::onPushButton_cancelClicked()
{
    reject();
}
