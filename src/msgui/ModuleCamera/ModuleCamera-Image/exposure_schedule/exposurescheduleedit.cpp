#include "exposurescheduleedit.h"
#include "ui_exposurescheduleedit.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "daycopydialog.h"

ExposureScheduleEdit::ExposureScheduleEdit(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::ExposureScheduleEdit)
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

    ui->comboBox_type->clear();
    ui->comboBox_type->addItem(GET_TEXT("IMAGE/37203", "Manual Mode"), ExposureActionManual);

    m_scheduleArray = new ExposureManualValue *[MAX_DAY_NUM_IPC];
    for (int i = 0; i <= 6; ++i) {
        m_scheduleArray[i] = new ExposureManualValue[MAX_SCHEDULE_SEC] {};
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

ExposureScheduleEdit::~ExposureScheduleEdit()
{
    for (int i = 0; i <= 6; ++i) {
        delete[] m_scheduleArray[i];
    }
    delete[] m_scheduleArray;
    delete ui;
}

void ExposureScheduleEdit::setScheduleArray(ExposureManualValue **scheduleArray)
{
    for (int i = 0; i <= 6; ++i) {
        memcpy(m_scheduleArray[i], scheduleArray[i], sizeof(ExposureManualValue) * MAX_SCHEDULE_SEC);
    }

    ui->comboBox_type->setCurrentIndex(0);
    on_comboBox_type_activated(0);
}

ExposureManualValue **ExposureScheduleEdit::scheduleArray() const
{
    return m_scheduleArray;
}

void ExposureScheduleEdit::addType(const QString &name, int type)
{
    ui->comboBox_type->addItem(name, type);
}

void ExposureScheduleEdit::clearType()
{
    ui->comboBox_type->clear();
}

void ExposureScheduleEdit::setManualModeMap(const QMap<int, ExposureManualValue> &map)
{
    ui->comboBox_manual->clear();
    for (auto iter = map.constBegin(); iter != map.constEnd(); ++iter) {
        const int key = iter.key();
        const ExposureManualValue &value = iter.value();
        ui->comboBox_manual->addItem(QString::number(key + 1), QVariant::fromValue(value));
    }
}

void ExposureScheduleEdit::showTime()
{
    for (int i = 0; i < m_timeEditList.size(); ++i) {
        MyTimeRangeEdit *timeEdit = m_timeEditList.at(i);
        timeEdit->clearTime();
    }

    const int day = ui->comboBox_day->currentData().toInt();
    const int actionType = ui->comboBox_type->currentData().toInt();
    ExposureManualValue type = ui->comboBox_manual->currentData().value<ExposureManualValue>();
    type.actionType = actionType;
    QList<ExposureScheduleTime> times = ExposureScheduleTime::fromSecondsHash(m_scheduleArray[day]);
    int index = 0;
    for (int i = 0; i < times.size(); ++i) {
        const ExposureScheduleTime &time = times.at(i);
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

bool ExposureScheduleEdit::saveSetting()
{
    //
    for (int i = 0; i <= 6; i++) {
        QList<ExposureScheduleTime> times = ExposureScheduleTime::fromSecondsHash(m_scheduleArray[i]);
        if (times.size() > 24) {
            ShowMessageBox(GET_TEXT("VIDEOLOSS/50023", "12 time periods only in a single day!"));
            return false;
        }
    }
    return true;
}

void ExposureScheduleEdit::onLanguageChanged()
{
    ui->label_day->setText(GET_TEXT("RECORDMODE/90022", "Day"));
    ui->label_type->setText(GET_TEXT("COMMON/1052", "Type"));

    ui->label_time->setText(GET_TEXT("LOG/64005", "Time"));
    ui->label_manual->setText(GET_TEXT("IMAGE/37213", "Manual Mode No."));

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

void ExposureScheduleEdit::on_comboBox_day_activated(int index)
{
    Q_UNUSED(index)

    showTime();
}

void ExposureScheduleEdit::on_comboBox_type_activated(int index)
{
    const int mode = ui->comboBox_type->itemData(index).toInt();
    switch (mode) {
    case ExposureActionAuto: {
        ui->label_manual->hide();
        ui->comboBox_manual->hide();
        break;
    }
    case ExposureActionManual: {
        ui->label_manual->show();
        ui->comboBox_manual->show();
        break;
    }
    default:
        break;
    }

    showTime();
}

void ExposureScheduleEdit::on_comboBox_manual_activated(int index)
{
    Q_UNUSED(index)

    showTime();
}

void ExposureScheduleEdit::onTimeEditingFinished(int beginMinutes, int endMinutes)
{
    if (beginMinutes >= endMinutes) {
        return;
    }

    int day = ui->comboBox_day->currentData().toInt();
    int actionType = ui->comboBox_type->currentData().toInt();
    ExposureManualValue type = ui->comboBox_manual->currentData().value<ExposureManualValue>();
    type.actionType = actionType;

    int beginSec = beginMinutes * 60;
    int endSec = endMinutes * 60;

    for (int i = beginSec; i <= endSec; ++i) {
        m_scheduleArray[day][i] = type;
    }
}

void ExposureScheduleEdit::onTimeCleared(int beginMinutes, int endMinutes)
{
    if (beginMinutes >= endMinutes) {
        return;
    }

    int day = ui->comboBox_day->currentData().toInt();
    ExposureManualValue type;

    int beginSec = beginMinutes * 60;
    int endSec = endMinutes * 60;

    for (int i = beginSec; i <= endSec; ++i) {
        m_scheduleArray[day][i] = type;
    }
}

void ExposureScheduleEdit::on_pushButton_copy_clicked()
{
    const int day = ui->comboBox_day->currentData().toInt();
    DayCopyDialog dayCopy(this);
    dayCopy.setCurrentDay(day);
    dayCopy.setHolidayVisible(false);
    const int &result = dayCopy.exec();
    if (result == DayCopyDialog::Accepted) {
        ExposureManualValue *tempDay = new ExposureManualValue[MAX_SCHEDULE_SEC];
        memcpy(tempDay, m_scheduleArray[day], sizeof(ExposureManualValue) * MAX_SCHEDULE_SEC);
        QList<int> dayList = dayCopy.dayList();
        for (int i = 0; i < dayList.size(); ++i) {
            const int otherDay = dayList.at(i);
            if (otherDay >= MAX_DAY_NUM_IPC) {
                continue;
            }
            memcpy(m_scheduleArray[otherDay], tempDay, sizeof(ExposureManualValue) * MAX_SCHEDULE_SEC);
        }
        delete[] tempDay;
    }
}

void ExposureScheduleEdit::on_pushButton_ok_clicked()
{
    if (!saveSetting()) {
        return;
    }
    emit editingFinished();
    accept();
}

void ExposureScheduleEdit::on_pushButton_cancel_clicked()
{
    reject();
}
