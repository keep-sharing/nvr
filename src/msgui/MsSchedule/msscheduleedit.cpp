#include "msscheduleedit.h"
#include "ui_msscheduleedit.h"

#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "daycopydialog.h"

#include <QtDebug>

MsScheduleEdit::MsScheduleEdit(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::MsScheduleEdit)
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
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1031", "Holiday"), 7);

    ui->comboBox_type->clear();
    ui->comboBox_type->addItem(GET_TEXT("PLAYBACK/80053", "Continuous"), TIMING_RECORD);
    ui->comboBox_type->addItem(GET_TEXT("RECORDMODE/90013", "Event"), EVENT_RECORD);
    ui->comboBox_type->addItem(GET_TEXT("MOTION/51000", "Motion Detection"), MOTION_RECORD);
    ui->comboBox_type->addItem(GET_TEXT("AUDIO_ALARM/159000", "Audio Alarm"), AUDIO_ALARM_RECORD);
    ui->comboBox_type->addItem(GET_TEXT("ALARMIN/52001", "Alarm Input"), ALARM_RECORD);
    ui->comboBox_type->addItem(GET_TEXT("SMARTEVENT/55000", "VCA"), SMART_EVT_RECORD);
    ui->comboBox_type->addItem(GET_TEXT("ANPR/103054", "Smart Analysis"), ANPT_EVT_RECORD);

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

MsScheduleEdit::~MsScheduleEdit()
{
    for (int i = 0; i <= 7; ++i) {
        delete[] m_scheduleArray[i];
    }
    delete[] m_scheduleArray;

    delete ui;
}

void MsScheduleEdit::setScheduleArray(uint16_t **scheduleArray)
{
    for (int i = 0; i <= 7; ++i) {
        memcpy(m_scheduleArray[i], scheduleArray[i], sizeof(uint16_t) * MAX_SCHEDULE_SEC);
    }

    showTime();
}

uint16_t **MsScheduleEdit::scheduleArray() const
{
    return m_scheduleArray;
}

void MsScheduleEdit::addType(const QString &name, int type)
{
    ui->comboBox_type->addItem(name, type);
}

void MsScheduleEdit::clearType()
{
    ui->comboBox_type->clear();
}

void MsScheduleEdit::setSingleEditType(int type)
{
    clearType();
    addType("", type);
    ui->label_type->hide();
    ui->comboBox_type->hide();
}

void MsScheduleEdit::setHolidayVisible(bool visible)
{
    m_holidayVisible = visible;
    ui->comboBox_day->clear();
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1024", "Sunday"), 0);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1025", "Monday"), 1);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1026", "Tuesday"), 2);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1027", "Wednesday"), 3);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1028", "Thursday"), 4);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1029", "Friday"), 5);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1030", "Saturday"), 6);
    if (visible) {
        ui->comboBox_day->addItem(GET_TEXT("COMMON/1031", "Holiday"), 7);
    }
}

void MsScheduleEdit::setDayVisible(bool visible)
{
    ui->label_day->setVisible(visible);
    ui->comboBox_day->setVisible(visible);
    ui->pushButton_copy->setVisible(visible);
}

void MsScheduleEdit::showTime()
{
    for (int i = 0; i < m_timeEditList.size(); ++i) {
        MyTimeRangeEdit *timeEdit = m_timeEditList.at(i);
        timeEdit->clearTime();
    }

    const int day = ui->comboBox_day->currentData().toInt();
    const int type = ui->comboBox_type->currentData().toInt();
    QList<ScheduleTime> times = ScheduleTime::fromSecondsHash(m_scheduleArray[day]);
    int index = 0;
    for (int i = 0; i < times.size(); ++i) {
        const ScheduleTime &time = times.at(i);
        if (time.type() == type) {
            MyTimeRangeEdit *timeEdit = m_timeEditList.at(index);
            timeEdit->setTime(time.beginMinute(), time.endMinute());
            index++;
        }
        if (index >= 12) {
            break;
        }
    }
}

bool MsScheduleEdit::saveSetting()
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

void MsScheduleEdit::onLanguageChanged()
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

void MsScheduleEdit::on_comboBox_day_activated(int index)
{
    Q_UNUSED(index)
    showTime();
}

void MsScheduleEdit::on_comboBox_type_activated(int index)
{
    Q_UNUSED(index)
    showTime();
}

void MsScheduleEdit::onTimeEditingFinished(int beginMinutes, int endMinutes)
{
    if (beginMinutes >= endMinutes) {
        //ShowMessageBox(GET_TEXT("VIDEOLOSS/50013","Time error."));
        return;
    }

    int day = ui->comboBox_day->currentData().toInt();
    int type = ui->comboBox_type->currentData().toInt();

    int beginSec = beginMinutes * 60;
    int endSec = endMinutes * 60;

    for (int i = beginSec; i <= endSec; ++i) {
        m_scheduleArray[day][i] = static_cast<uint16_t>(type);
    }
}

void MsScheduleEdit::onTimeCleared(int beginMinutes, int endMinutes)
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

void MsScheduleEdit::on_pushButton_copy_clicked()
{
    const int day = ui->comboBox_day->currentData().toInt();
    DayCopyDialog dayCopy(this);
    dayCopy.setCurrentDay(day);
    dayCopy.setHolidayVisible(m_holidayVisible);
    const int &result = dayCopy.exec();
    if (result == DayCopyDialog::Accepted) {
        uint16_t *tempDay = new uint16_t[MAX_SCHEDULE_SEC];
        memcpy(tempDay, m_scheduleArray[day], sizeof(uint16_t) * MAX_SCHEDULE_SEC);
        QList<int> dayList = dayCopy.dayList();
        qDebug() << "MsScheduleEdit::on_pushButton_copy_clicked()," << dayList;
        for (int i = 0; i < dayList.size(); ++i) {
            const int otherDay = dayList.at(i);
            memcpy(m_scheduleArray[otherDay], tempDay, sizeof(uint16_t) * MAX_SCHEDULE_SEC);
        }
        delete[] tempDay;
    }
}

void MsScheduleEdit::on_pushButton_ok_clicked()
{
    if (!saveSetting()) {
        return;
    }
    emit editingFinished();
    accept();
}

void MsScheduleEdit::on_pushButton_cancel_clicked()
{
    reject();
}
