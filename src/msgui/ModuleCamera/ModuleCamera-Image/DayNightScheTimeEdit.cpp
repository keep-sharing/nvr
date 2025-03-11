#include "DayNightScheTimeEdit.h"
#include "ui_msscheduleedit.h"
#include "DayNightSchedule.h"
#include "MsLanguage.h"

DayNightScheTimeEdit::DayNightScheTimeEdit(QWidget *parent)
    : MsScheduleEdit(parent)
{
    setDayVisible(false);

    clearType();
    addType(GET_TEXT("IMAGE/162001", "Template") + "1", DayNightSchedule::ACTION_TEMPLATE1);
    addType(GET_TEXT("IMAGE/162001", "Template") + "2", DayNightSchedule::ACTION_TEMPLATE2);
    addType(GET_TEXT("IMAGE/162001", "Template") + "3", DayNightSchedule::ACTION_TEMPLATE3);
    addType(GET_TEXT("IMAGE/162001", "Template") + "4", DayNightSchedule::ACTION_TEMPLATE4);
    addType(GET_TEXT("IMAGE/162001", "Template") + "5", DayNightSchedule::ACTION_TEMPLATE5);

    ui->label_type->setText(GET_TEXT("IMAGE/162001", "Template"));
}
