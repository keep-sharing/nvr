#include "imagescheduleedit.h"
#include "ui_msscheduleedit.h"
#include <QtDebug>
#include "MsLanguage.h"
#include "MessageBox.h"
#include "daycopydialog.h"

ImageScheduleEdit::ImageScheduleEdit(QWidget *parent) :
    MsScheduleEdit(parent)
{
    ui->comboBox_day->clear();
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1024", "Sunday"), 0);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1025", "Monday"), 1);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1026", "Tuesday"), 2);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1027", "Wednesday"), 3);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1028", "Thursday"), 4);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1029", "Friday"), 5);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1030", "Saturday"), 6);

    setHolidayVisible(false);
}
