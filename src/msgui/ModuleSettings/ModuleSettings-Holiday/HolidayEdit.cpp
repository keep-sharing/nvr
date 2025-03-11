#include "HolidayEdit.h"
#include "ui_HolidayEdit.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"

extern "C" {
#include "msdb.h"
}

HolidayEdit::HolidayEdit(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::HolidayEdit)
{
    ui->setupUi(this);

    const QDate &currentDate = QDate::currentDate();
    ui->dateEdit_start->setDate(currentDate);
    ui->dateEdit_end->setDate(currentDate);
    ui->lineEdit_name->setCheckMode(MyLineEdit::EmptyCheck);

    m_cacheData = new holiday;

    onLanguageChanged();
}

HolidayEdit::~HolidayEdit()
{
    if (m_cacheData) {
        delete m_cacheData;
        m_cacheData = nullptr;
    }
    delete ui;
}

void HolidayEdit::showHolidayEdit(holiday *day)
{
    m_sourceData = day;
    memset(m_cacheData, 0, sizeof(holiday));
    memcpy(m_cacheData, m_sourceData, sizeof(struct holiday));

    ui->lineEdit_name->setText(QString(m_cacheData->name));
    ui->checkBox_enable->setChecked(m_cacheData->enable);
    ui->comboBox_style->setCurrentIndex(m_cacheData->type);
    on_comboBox_style_activated(ui->comboBox_style->currentIndex());

    switch (m_cacheData->type) {
    case 0: //day
    {
        QDate startDate(m_cacheData->start_year, m_cacheData->start_mon, m_cacheData->start_mday);
        QDate endDate(m_cacheData->end_year, m_cacheData->end_mon, m_cacheData->end_mday);
        ui->dateEdit_start->setDate(startDate);
        ui->dateEdit_end->setDate(endDate);
        break;
    }
    case 1: //month
    {
        ui->comboBox_startMonth->setCurrentIndex(m_cacheData->start_mon - 1);
        ui->comboBox_endMonth->setCurrentIndex(m_cacheData->end_mon - 1);
        ui->comboBox_startDay->setCurrentIndex(m_cacheData->start_mday - 1);
        ui->comboBox_endDay->setCurrentIndex(m_cacheData->end_mday - 1);
        break;
    }
    case 2: //week
    {
        ui->comboBox_startMonth_2->setCurrentIndex(m_cacheData->start_mon - 1);
        ui->comboBox_endMonth_2->setCurrentIndex(m_cacheData->end_mon - 1);
        ui->comboBox_startWeek->setCurrentIndex(m_cacheData->start_mweek - 1);
        ui->comboBox_endWeek->setCurrentIndex(m_cacheData->end_mweek - 1);
        ui->comboBox_startDay_2->setCurrentIndex(m_cacheData->start_wday);
        ui->comboBox_endDay_2->setCurrentIndex(m_cacheData->end_wday);
        break;
    }
    default:
        break;
    }
}

bool HolidayEdit::dealInput()
{
    QString strName = ui->lineEdit_name->text().trimmed();
    if (!ui->lineEdit_name->checkValid()) {
        return false;
    }
    if (strName.contains("'")) {
        ui->lineEdit_name->setCustomValid(false, GET_TEXT("MYLINETIP/112013", "Invalid characters: '."));
        return false;
    }
    snprintf(m_cacheData->name, sizeof(m_cacheData->name), "%s", strName.toStdString().c_str());
    int type = ui->comboBox_style->currentIndex();
    switch (type) {
    case 0: {
        QDate startDate = ui->dateEdit_start->date();
        QDate endDate = ui->dateEdit_end->date();
        if (startDate > endDate) {
            ShowMessageBox(GET_TEXT("VIDEOLOSS/50013", "Time error."));
            return false;
        }
        m_cacheData->start_year = startDate.year();
        m_cacheData->start_mon = startDate.month();
        m_cacheData->start_mday = startDate.day();
        m_cacheData->end_year = endDate.year();
        m_cacheData->end_mon = endDate.month();
        m_cacheData->end_mday = endDate.day();
        break;
    }
    case 1: {
        int startMonth = ui->comboBox_startMonth->currentIndex() + 1;
        int startDay = ui->comboBox_startDay->currentIndex() + 1;
        int endMonth = ui->comboBox_endMonth->currentIndex() + 1;
        int endDay = ui->comboBox_endDay->currentIndex() + 1;
        if (startMonth > endMonth || (startMonth == endMonth && startDay > endDay)) {
            ShowMessageBox(GET_TEXT("VIDEOLOSS/50013", "Time error."));
            return false;
        }
        m_cacheData->start_mon = startMonth;
        m_cacheData->start_mday = startDay;
        m_cacheData->end_mon = endMonth;
        m_cacheData->end_mday = endDay;
        break;
    }
    case 2: {
        //week
        int startMonth = ui->comboBox_startMonth_2->currentIndex() + 1;
        int startWeek = ui->comboBox_startWeek->currentIndex() + 1;
        int startDay = ui->comboBox_startDay_2->currentIndex();
        int endMonth = ui->comboBox_endMonth_2->currentIndex() + 1;
        int endWeek = ui->comboBox_endWeek->currentIndex() + 1;
        int endDay = ui->comboBox_endDay_2->currentIndex();
        if (startMonth > endMonth || (startMonth == endMonth && startWeek > endWeek) || (startMonth == endMonth && startWeek == endWeek && startDay > endDay)) {
            ShowMessageBox(GET_TEXT("VIDEOLOSS/50013", "Time error."));
            return false;
        }
        m_cacheData->start_mon = startMonth;
        m_cacheData->start_mweek = startWeek;
        m_cacheData->start_wday = startDay;
        m_cacheData->end_mon = endMonth;
        m_cacheData->end_mweek = endWeek;
        m_cacheData->end_wday = endDay;
        break;
    }
    default:
        break;
    }

    m_cacheData->enable = ui->checkBox_enable->isChecked();
    m_cacheData->type = type;

    return true;
}

void HolidayEdit::onLanguageChanged()
{
    ui->comboBox_style->clear();
    ui->comboBox_style->addItem(GET_TEXT("HOLIDAY/73009", "By Day"));
    ui->comboBox_style->addItem(GET_TEXT("HOLIDAY/73010", "By Month"));
    ui->comboBox_style->addItem(GET_TEXT("HOLIDAY/73011", "By Week"));

    QStringList monthList;
    monthList << GET_TEXT("COMMON/1038", "January");
    monthList << GET_TEXT("COMMON/1039", "February");
    monthList << GET_TEXT("COMMON/1040", "March");
    monthList << GET_TEXT("COMMON/1041", "April");
    monthList << GET_TEXT("COMMON/1042", "May");
    monthList << GET_TEXT("COMMON/1043", "June");
    monthList << GET_TEXT("COMMON/1044", "July");
    monthList << GET_TEXT("COMMON/1045", "August");
    monthList << GET_TEXT("COMMON/1046", "September");
    monthList << GET_TEXT("COMMON/1047", "October");
    monthList << GET_TEXT("COMMON/1048", "November");
    monthList << GET_TEXT("COMMON/1049", "December");
    ui->comboBox_startMonth->clear();
    ui->comboBox_startMonth->addItems(monthList);
    ui->comboBox_endMonth->clear();
    ui->comboBox_endMonth->addItems(monthList);
    ui->comboBox_startMonth_2->clear();
    ui->comboBox_startMonth_2->addItems(monthList);
    ui->comboBox_endMonth_2->clear();
    ui->comboBox_endMonth_2->addItems(monthList);

    QStringList weekList;
    weekList << GET_TEXT("HOLIDAY/73018", "First");
    weekList << GET_TEXT("HOLIDAY/73019", "Second");
    weekList << GET_TEXT("HOLIDAY/73020", "Third");
    weekList << GET_TEXT("HOLIDAY/73021", "Fourth");
    weekList << GET_TEXT("HOLIDAY/73022", "Last");
    ui->comboBox_startWeek->clear();
    ui->comboBox_startWeek->addItems(weekList);
    ui->comboBox_endWeek->clear();
    ui->comboBox_endWeek->addItems(weekList);

    QStringList dayList;
    dayList << GET_TEXT("COMMON/1024", "Sunday");
    dayList << GET_TEXT("COMMON/1025", "Monday");
    dayList << GET_TEXT("COMMON/1026", "Tuesday");
    dayList << GET_TEXT("COMMON/1027", "Wednesday");
    dayList << GET_TEXT("COMMON/1028", "Thursday");
    dayList << GET_TEXT("COMMON/1029", "Friday");
    dayList << GET_TEXT("COMMON/1030", "Saturday");
    ui->comboBox_startDay_2->clear();
    ui->comboBox_startDay_2->addItems(dayList);
    ui->comboBox_endDay_2->clear();
    ui->comboBox_endDay_2->addItems(dayList);

    ui->label_title->setText(GET_TEXT("HOLIDAY/73007", "Holiday Edit"));
    ui->label_name->setText(GET_TEXT("HOLIDAY/73001", "Holiday Name"));
    ui->label_holiday->setText(GET_TEXT("COMMON/1031", "Holiday"));
    ui->checkBox_enable->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->label_style->setText(GET_TEXT("HOLIDAY/73008", "Style"));
    ui->label_startDate->setText(GET_TEXT("HOLIDAY/73003", "Start Date"));
    ui->label_endDate->setText(GET_TEXT("HOLIDAY/73004", "End Date"));
    ui->label_note->setText(GET_TEXT("HOLIDAY/73012", "* Holiday schedule takes precedence over other schedules."));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void HolidayEdit::on_comboBox_style_activated(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
}

void HolidayEdit::on_comboBox_startMonth_currentIndexChanged(int index)
{
    int month = index + 1;
    QDate date = QDate::currentDate();
    date.setDate(date.year(), month, date.month());
    int days = date.daysInMonth();
    ui->comboBox_startDay->clear();
    for (int i = 0; i < days; ++i) {
        ui->comboBox_startDay->addItem(QString::number(i + 1));
    }
}

void HolidayEdit::on_comboBox_endMonth_currentIndexChanged(int index)
{
    int month = index + 1;
    QDate date = QDate::currentDate();
    date.setDate(date.year(), month, date.month());
    int days = date.daysInMonth();
    ui->comboBox_endDay->clear();
    for (int i = 0; i < days; ++i) {
        ui->comboBox_endDay->addItem(QString::number(i + 1));
    }
}

void HolidayEdit::on_pushButton_ok_clicked()
{
    if (dealInput()) {
        memset(m_sourceData, 0, sizeof(holiday));
        memcpy(m_sourceData, m_cacheData, sizeof(holiday));
        editFinish();
        accept();
    }
}

void HolidayEdit::on_pushButton_cancel_clicked()
{
    editFinish();
    reject();
}

void HolidayEdit::editFinish()
{
    const QDate &currentDate = QDate::currentDate();
    ui->dateEdit_start->setDate(currentDate);
    ui->dateEdit_end->setDate(currentDate);

    ui->comboBox_startMonth->setCurrentIndex(0);
    ui->comboBox_endMonth->setCurrentIndex(0);
    ui->comboBox_startDay->setCurrentIndex(0);
    ui->comboBox_endDay->setCurrentIndex(0);

    ui->comboBox_startMonth_2->setCurrentIndex(0);
    ui->comboBox_endMonth_2->setCurrentIndex(0);
    ui->comboBox_startWeek->setCurrentIndex(0);
    ui->comboBox_endWeek->setCurrentIndex(0);
    ui->comboBox_startDay_2->setCurrentIndex(0);
    ui->comboBox_endDay_2->setCurrentIndex(0);
}
