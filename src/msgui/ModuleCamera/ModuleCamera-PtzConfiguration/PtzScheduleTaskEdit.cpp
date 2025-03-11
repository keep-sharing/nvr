#include "PtzScheduleTaskEdit.h"
#include "ui_PtzScheduleTaskEdit.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "PtzScheduleTask.h"
#include "daycopydialog.h"

#include <QtDebug>

PtzScheduleTaskEdit::PtzScheduleTaskEdit(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::PtzScheduleTaskEdit)
{
    ui->setupUi(this);
    ui->comboBox_day->clear();
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1024", "Sunday"), 0);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1025", "Monday"), 1);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1026", "Tuesday"), 2);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1027", "Wednesday"), 3);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1028", "Thursday"), 4);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1029", "Friday"), 5);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1030", "Saturday"), 6);

    m_scheduleArray = new uint16_t *[MAX_DAY_NUM];
    for (int i = 0; i <= 7; ++i) {
        m_scheduleArray[i] = new uint16_t[MAX_SCHEDULE_SEC] { 0 };
    }

    for (int i = 0; i < 12; ++i) {
        MyTimeRangeEdit *timeEdit = findChild<MyTimeRangeEdit *>(QString("myTimeRangeEdit_%1").arg(i + 1));
        if (timeEdit) {
            connect(timeEdit, SIGNAL(timeEditingFinished(int, int)), this, SLOT(onTimeEditingFinished(int, int)));
            connect(timeEdit, SIGNAL(timeCleared(int, int)), this, SLOT(onTimeCleared(int, int)));
            m_timeEditList.append(timeEdit);
        }
    }

    onLanguageChanged();
}

PtzScheduleTaskEdit::~PtzScheduleTaskEdit()
{
    for (int i = 0; i <= 7; ++i) {
        delete[] m_scheduleArray[i];
    }
    delete[] m_scheduleArray;
    delete ui;
}

void PtzScheduleTaskEdit::setScheduleArray(uint16_t **scheduleArray, bool supportWiper, bool supportSpeedDome)
{
    for (int i = 0; i <= 7; ++i) {
        memcpy(m_scheduleArray[i], scheduleArray[i], sizeof(uint16_t) * MAX_SCHEDULE_SEC);
    }
    m_supportWiper = supportWiper;
    m_supportSpeedDome = supportSpeedDome;

    ui->comboBox_type->clear();
    ui->comboBox_type->addItem(GET_TEXT("PTZDIALOG/21005", "Close"), PtzScheduleTask::ACTION_CLOSE);
    ui->comboBox_type->addItem(GET_TEXT("PTZDIALOG/21014", "Auto Scan"), PtzScheduleTask::ACTION_AUTOSCAN);
    ui->comboBox_type->addItem(GET_TEXT("PTZDIALOG/21015", "Preset"), PtzScheduleTask::ACTION_PRESET);
    ui->comboBox_type->addItem(GET_TEXT("PTZDIALOG/21016", "Patrol"), PtzScheduleTask::ACTION_PATROL);
    ui->comboBox_type->addItem(GET_TEXT("PTZDIALOG/21017", "Pattern"), PtzScheduleTask::ACTION_PATTERN);
    ui->comboBox_type->addItem(GET_TEXT("PTZCONFIG/166012","Self Check"), PtzScheduleTask::ACTION_CHECK);
    if (m_supportSpeedDome) {
        ui->comboBox_type->addItem(GET_TEXT("PTZCONFIG/166013","Tilt Scan"), PtzScheduleTask::ACTION_TILTSCAN);
        ui->comboBox_type->addItem(GET_TEXT("PTZCONFIG/166014","Panorama Scan"), PtzScheduleTask::ACTION_PANORAMASCAN);
    }
    ui->comboBox_day->setCurrentIndexFromData(0);
    ui->comboBoxNo->setCurrentIndexFromData(0);
    ui->comboBoxNo->hide();
    ui->labelNo->hide();
    on_comboBox_day_activated(0);
}

uint16_t **PtzScheduleTaskEdit::scheduleArray() const
{
    return m_scheduleArray;
}

void PtzScheduleTaskEdit::showTime()
{
    for (int i = 0; i < m_timeEditList.size(); ++i) {
        MyTimeRangeEdit *timeEdit = m_timeEditList.at(i);
        timeEdit->clearTime();
    }

    const int day = ui->comboBox_day->currentIntData();
    const int type = ui->comboBox_type->currentIntData();
    const int id = ui->comboBoxNo->currentIntData();
    int currentType;
    if (type == PtzScheduleTask::ACTION_PRESET || type == PtzScheduleTask::ACTION_PATROL || type == PtzScheduleTask::ACTION_PATTERN) {
        currentType = id;
    } else {
        currentType = type;
    }
    QList<ScheduleTime> times = ScheduleTime::fromSecondsHash(m_scheduleArray[day]);
    int index = 0;
    for (int i = 0; i < times.size(); ++i) {
        const ScheduleTime &time = times.at(i);
        if (time.type() == currentType) {
            MyTimeRangeEdit *timeEdit = m_timeEditList.at(index);
            timeEdit->setTime(time.beginMinute(), time.endMinute());
            index++;
        }
        if (index >= 12) {
            break;
        }
    }
}

bool PtzScheduleTaskEdit::saveSetting()
{
    //
    for (int iter = 0; iter <= 7; ++iter) {
        QList<ScheduleTime> times = ScheduleTime::fromSecondsHash(m_scheduleArray[iter]);
        if (times.size() > 12) {
            ShowMessageBox(GET_TEXT("VIDEOLOSS/50023", "12 time periods only in a single day!"));
            return false;
        }
    }
    return true;
}

void PtzScheduleTaskEdit::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("IMAGE/37319", "Schedule Edit"));

    ui->label_day->setText(GET_TEXT("RECORDMODE/90022", "Day"));
    ui->label_type->setText(GET_TEXT("COMMON/1052", "Type"));
    ui->label_time->setText(GET_TEXT("REPORTAUTOBACKUP/114011", "Time"));

    ui->label_time1->setText(GET_TEXT("RECORDMODE/90025", "Time%1").arg(1));
    ui->label_time2->setText(GET_TEXT("RECORDMODE/90025", "Time%1").arg(2));
    ui->label_time3->setText(GET_TEXT("RECORDMODE/90025", "Time%1").arg(3));
    ui->label_time4->setText(GET_TEXT("RECORDMODE/90025", "Time%1").arg(4));
    ui->label_time5->setText(GET_TEXT("RECORDMODE/90025", "Time%1").arg(5));
    ui->label_time6->setText(GET_TEXT("RECORDMODE/90025", "Time%1").arg(6));
    ui->label_time7->setText(GET_TEXT("RECORDMODE/90025", "Time%1").arg(7));
    ui->label_time8->setText(GET_TEXT("RECORDMODE/90025", "Time%1").arg(8));
    ui->label_time9->setText(GET_TEXT("RECORDMODE/90025", "Time%1").arg(9));
    ui->label_time10->setText(GET_TEXT("RECORDMODE/90025", "Time%1").arg(10));
    ui->label_time11->setText(GET_TEXT("RECORDMODE/90025", "Time%1").arg(11));
    ui->label_time12->setText(GET_TEXT("RECORDMODE/90025", "Time%1").arg(12));

    ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void PtzScheduleTaskEdit::on_comboBox_day_activated(int index)
{
    Q_UNUSED(index)
    showTime();
}

void PtzScheduleTaskEdit::on_comboBox_type_activated(int index)
{
    Q_UNUSED(index)
    ui->comboBoxNo->clear();
    switch (ui->comboBox_type->currentIntData()) {
    case PtzScheduleTask::ACTION_PRESET:
        ui->comboBoxNo->show();
        ui->labelNo->show();
        for (int i = 1; i <= 300; i++) {
            int limit = m_supportWiper ? 53 : 52;
            if (i >= 33 && i <= limit) {
                continue;
            }
            ui->comboBoxNo->addItem(QString("%1").arg(i), i + 100);
        }
        break;
    case PtzScheduleTask::ACTION_PATROL:
        ui->comboBoxNo->show();
        ui->labelNo->show();
        for (int i = 1; i <= 8; i++) {
            ui->comboBoxNo->addItem(QString("%1").arg(i), i + 400);
        }
        break;
    case PtzScheduleTask::ACTION_PATTERN:
        ui->comboBoxNo->show();
        ui->labelNo->show();
        for (int i = 1; i <= 4; i++) {
            ui->comboBoxNo->addItem(QString("%1").arg(i), i + 500);
        }
        break;
    default:
        ui->comboBoxNo->hide();
        ui->labelNo->hide();
        break;
    }

    showTime();
}

void PtzScheduleTaskEdit::on_comboBoxNo_activated(int index)
{
    Q_UNUSED(index)
    showTime();
}

void PtzScheduleTaskEdit::onTimeEditingFinished(int beginMinutes, int endMinutes)
{
    if (beginMinutes >= endMinutes) {
        //ShowMessageBox(GET_TEXT("VIDEOLOSS/50013","Time error."));
        return;
    }

    int day = ui->comboBox_day->currentData().toInt();
    int type = ui->comboBox_type->currentData().toInt();
    const int id = ui->comboBoxNo->currentIntData();
    int currentType;
    if (type == PtzScheduleTask::ACTION_PRESET || type == PtzScheduleTask::ACTION_PATROL || type == PtzScheduleTask::ACTION_PATTERN) {
        currentType = id;
    } else {
        currentType = type;
    }

    int beginSec = beginMinutes * 60;
    int endSec = endMinutes * 60;

    for (int i = beginSec; i <= endSec; ++i) {
        m_scheduleArray[day][i] = static_cast<uint16_t>(currentType);
    }
}

void PtzScheduleTaskEdit::onTimeCleared(int beginMinutes, int endMinutes)
{
    if (beginMinutes >= endMinutes) {
        return;
    }

    int day = ui->comboBox_day->currentData().toInt();
    int type = 0;

    int beginSec = beginMinutes * 60;
    int endSec = endMinutes * 60;

    for (int i = beginSec; i <= endSec; ++i) {
        m_scheduleArray[day][i] = static_cast<uint16_t>(type);
    }
}

void PtzScheduleTaskEdit::on_pushButton_copy_clicked()
{
    const int day = ui->comboBox_day->currentData().toInt();
    DayCopyDialog dayCopy(this);
    dayCopy.setCurrentDay(day);
    dayCopy.setHolidayVisible(false);
    const int &result = dayCopy.exec();
    if (result == DayCopyDialog::Accepted) {
        uint16_t *tempDay = new uint16_t[MAX_SCHEDULE_SEC];
        memcpy(tempDay, m_scheduleArray[day], sizeof(uint16_t) * MAX_SCHEDULE_SEC);
        QList<int> dayList = dayCopy.dayList();
        qDebug() << "PtzScheduleTaskEdit::on_pushButton_copy_clicked()," << dayList;
        for (int i = 0; i < dayList.size(); ++i) {
            const int otherDay = dayList.at(i);
            memcpy(m_scheduleArray[otherDay], tempDay, sizeof(uint16_t) * MAX_SCHEDULE_SEC);
        }
        delete[] tempDay;
    }
}

void PtzScheduleTaskEdit::on_pushButton_ok_clicked()
{
    if (!saveSetting()) {
        return;
    }
    emit editingFinished();
    accept();
}

void PtzScheduleTaskEdit::on_pushButton_cancel_clicked()
{
    reject();
}
