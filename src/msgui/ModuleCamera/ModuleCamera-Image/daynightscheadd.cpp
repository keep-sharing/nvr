#include "daynightscheadd.h"
#include "ui_daynightscheadd.h"
#include "centralmessage.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include <QDebug>
#include <QLineEdit>
#include <QValidator>

extern "C" {
#include "msg.h"
}

DayNightScheAdd::DayNightScheAdd(bool isWhite, QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::DayNightScheAdd)
{
    ui->setupUi(this);

    ui->comboBox_exposureLevelAdd->clear();
    for (int i = 0; i < 11; i++) {
        ui->comboBox_exposureLevelAdd->addItem(QString("%1").arg(i));
    }
    ui->comboBox_exposureLevelAdd->setCurrentIndex(5);

    ui->spinBox_startHour->setRange(0, 23);
    ui->spinBox_startMinute->setRange(0, 59);
    ui->spinBox_endHour->setRange(0, 24);
    ui->spinBox_endMinute->setRange(0, 59);
    ui->spinBox_startHour->setValue(0);
    ui->spinBox_startMinute->setValue(0);
    ui->spinBox_endHour->setValue(24);
    ui->spinBox_endMinute->setValue(0);

    ui->lineEdit_gainLevelAdd->setText(QString("%1").arg(100));
    ui->lineEdit_gainLevelAdd->setCheckMode(MyLineEdit::RangeCheck, 1, 100);

    ui->comboBox_latemcyIrCutAdd->clear();
    for (int i = 1; i <= 20; i++) {
        ui->comboBox_latemcyIrCutAdd->addItem(QString("%1s").arg(i));
    }
    ui->comboBox_latemcyIrCutAdd->setCurrentIndex(4);

    ui->comboBox_irCutAdd->clear();
    ui->comboBox_irCutAdd->addItem(GET_TEXT("IMAGE/37316", "Off"));
    ui->comboBox_irCutAdd->addItem(GET_TEXT("IMAGE/37317", "On"));

    ui->comboBox_irLedAdd->clear();
    if (isWhite) {
        ui->comboBox_irLedAdd->addItem(GET_TEXT("IMAGE/37366", "All LED Off"), 0);
        ui->comboBox_irLedAdd->addItem(GET_TEXT("IMAGE/37367", "IR LED On"), 1);
        ui->comboBox_irLedAdd->addItem(GET_TEXT("IMAGE/37368", "White LED On"), 2);
    } else {
        ui->comboBox_irLedAdd->addItem(GET_TEXT("IMAGE/37316", "Off"), 0);
        ui->comboBox_irLedAdd->addItem(GET_TEXT("IMAGE/37317", "On"), 1);
    }

    ui->comboBox_colorModeAdd->clear();
    ui->comboBox_colorModeAdd->addItem(GET_TEXT("IMAGE/37312", "B/W"));
    ui->comboBox_colorModeAdd->addItem(GET_TEXT("IMAGE/337313", "Color"));

    slotTranslateUi();
}

DayNightScheAdd::~DayNightScheAdd()
{
    delete ui;
}

void DayNightScheAdd::setExposureCtrl(int type)
{
    m_exposureCtrl = type;
    initShutter();
}

void DayNightScheAdd::initShutter()
{
    if (!m_exposureCtrl) {
        ui->comboBox_minShutterAdd->clear();
        ui->comboBox_minShutterAdd->addItem("1", 13);
        ui->comboBox_minShutterAdd->addItem("1/5", 0);
        ui->comboBox_minShutterAdd->addItem("1/15", 1);
        ui->comboBox_minShutterAdd->addItem("1/30", 2);
        ui->comboBox_minShutterAdd->addItem("1/60", 3);
        ui->comboBox_minShutterAdd->addItem("1/120", 4);
        ui->comboBox_minShutterAdd->addItem("1/250", 5);
        ui->comboBox_minShutterAdd->addItem("1/500", 6);
        ui->comboBox_minShutterAdd->addItem("1/750", 7);
        ui->comboBox_minShutterAdd->addItem("1/1000", 8);
        ui->comboBox_minShutterAdd->addItem("1/2000", 9);
        ui->comboBox_minShutterAdd->addItem("1/4000", 10);
        ui->comboBox_minShutterAdd->addItem("1/10000", 11);
        ui->comboBox_minShutterAdd->addItem("1/100000", 12);
        ui->comboBox_minShutterAdd->setCurrentIndexFromData(2);

        ui->comboBox_maxShutterAdd->clear();
        ui->comboBox_maxShutterAdd->addItem("1", 13);
        ui->comboBox_maxShutterAdd->addItem("1/5", 0);
        ui->comboBox_maxShutterAdd->addItem("1/15", 1);
        ui->comboBox_maxShutterAdd->addItem("1/30", 2);
        ui->comboBox_maxShutterAdd->addItem("1/60", 3);
        ui->comboBox_maxShutterAdd->addItem("1/120", 4);
        ui->comboBox_maxShutterAdd->addItem("1/250", 5);
        ui->comboBox_maxShutterAdd->addItem("1/500", 6);
        ui->comboBox_maxShutterAdd->addItem("1/750", 7);
        ui->comboBox_maxShutterAdd->addItem("1/1000", 8);
        ui->comboBox_maxShutterAdd->addItem("1/2000", 9);
        ui->comboBox_maxShutterAdd->addItem("1/4000", 10);
        ui->comboBox_maxShutterAdd->addItem("1/10000", 11);
        ui->comboBox_maxShutterAdd->addItem("1/100000", 12);
        ui->comboBox_maxShutterAdd->setCurrentIndexFromData(12);
    } else {
        ui->comboBox_minShutterAdd->clear();
        ui->comboBox_minShutterAdd->addItem("1", 13);
        ui->comboBox_minShutterAdd->addItem("1/5", 0);
        ui->comboBox_minShutterAdd->addItem("1/10", 1);
        ui->comboBox_minShutterAdd->addItem("1/25", 2);
        ui->comboBox_minShutterAdd->addItem("1/50", 3);
        ui->comboBox_minShutterAdd->addItem("1/100", 4);
        ui->comboBox_minShutterAdd->addItem("1/250", 5);
        ui->comboBox_minShutterAdd->addItem("1/500", 6);
        ui->comboBox_minShutterAdd->addItem("1/750", 7);
        ui->comboBox_minShutterAdd->addItem("1/1000", 8);
        ui->comboBox_minShutterAdd->addItem("1/2000", 9);
        ui->comboBox_minShutterAdd->addItem("1/4000", 10);
        ui->comboBox_minShutterAdd->addItem("1/10000", 11);
        ui->comboBox_minShutterAdd->addItem("1/100000", 12);
        ui->comboBox_minShutterAdd->setCurrentIndexFromData(2);

        ui->comboBox_maxShutterAdd->clear();
        ui->comboBox_maxShutterAdd->addItem("1", 13);
        ui->comboBox_maxShutterAdd->addItem("1/5", 0);
        ui->comboBox_maxShutterAdd->addItem("1/10", 1);
        ui->comboBox_maxShutterAdd->addItem("1/25", 2);
        ui->comboBox_maxShutterAdd->addItem("1/50", 3);
        ui->comboBox_maxShutterAdd->addItem("1/100", 4);
        ui->comboBox_maxShutterAdd->addItem("1/250", 5);
        ui->comboBox_maxShutterAdd->addItem("1/500", 6);
        ui->comboBox_maxShutterAdd->addItem("1/750", 7);
        ui->comboBox_maxShutterAdd->addItem("1/1000", 8);
        ui->comboBox_maxShutterAdd->addItem("1/2000", 9);
        ui->comboBox_maxShutterAdd->addItem("1/4000", 10);
        ui->comboBox_maxShutterAdd->addItem("1/10000", 11);
        ui->comboBox_maxShutterAdd->addItem("1/100000", 12);
        ui->comboBox_maxShutterAdd->setCurrentIndexFromData(12);
    }
}

void DayNightScheAdd::slotTranslateUi()
{
    ui->label_title->setText(GET_TEXT("IMAGE/37350", "Schedule Add"));
    ui->label_startTimeAdd->setText(GET_TEXT("IMAGE/37320", "Start Time"));
    ui->label_endTimeAdd->setText(GET_TEXT("IMAGE/37321", "End Time"));
    ui->label_exposureLevelAdd->setText(GET_TEXT("IMAGE/37301", "Exposure Level"));
    ui->label_minShutterAdd->setText(GET_TEXT("IMAGE/37302", "Minimum Shutter"));
    ui->label_maxShutterAdd->setText(GET_TEXT("IMAGE/37303", "Maximum Shutter"));
    ui->label_gainLevelAdd->setText(GET_TEXT("IMAGE/37304", "Limit Gain Level"));
    ui->label_latencyIrCutAdd->setText(GET_TEXT("IMAGE/37305", "IR-CUT Latency"));
    ui->label_irCutAdd->setText(GET_TEXT("IMAGE/37306", "IR-CUT"));
    ui->label_irLedAdd->setText(GET_TEXT("IMAGE/37307", "IR LED"));
    ui->label_colorModeAdd->setText(GET_TEXT("IMAGE/37311", "Color Mode"));

    ui->pushButton_ok_add->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel_add->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void DayNightScheAdd::initAddDayNight(image_day_night *info)
{
    m_info = info;
}

void DayNightScheAdd::on_pushButton_ok_add_clicked()
{
    if (!ui->lineEdit_gainLevelAdd->checkValid()) {
        return;
    }
//    struct set_image_day_night_str info;
//    memset(&info, 0x0, sizeof(struct set_image_day_night_str));
//    info.chanid = m_channel;

    int startHour = ui->spinBox_startHour->value();
    int startMinute = ui->spinBox_startMinute->value();
    int endHour = ui->spinBox_endHour->value();
    int endMinute = ui->spinBox_endMinute->value();
    int minShutter = ui->comboBox_minShutterAdd->currentData().toInt();
    int maxShutter = ui->comboBox_maxShutterAdd->currentData().toInt();
    if (startHour * 60 + startMinute >= endHour * 60 + endMinute) {
        ShowMessageBox(GET_TEXT("IMAGE/37364", "Start time should be earlier than end time."));
        return;
    }
    if ((maxShutter == 13 && minShutter != 13) || (minShutter != 13 && minShutter > maxShutter)) {
        ShowMessageBox(GET_TEXT("IMAGE/37365", "Minimum Shutter should be larger than Maximum Shutter."));
        return;
    }
    m_info->startHour = startHour;
    m_info->startMinute = startMinute;
    m_info->endHour = endHour;
    m_info->endMinute = endMinute;
    m_info->irCutLatency = ui->comboBox_latemcyIrCutAdd->currentIndex();
    m_info->exposureLevel = ui->comboBox_exposureLevelAdd->currentIndex();
    m_info->minShutter = minShutter;
    m_info->maxShutter = maxShutter;
    m_info->gainLevel = ui->lineEdit_gainLevelAdd->text().toInt();
    m_info->irCutState = ui->comboBox_irCutAdd->currentIndex();
    m_info->irLedState = ui->comboBox_irLedAdd->currentData().toInt();
    m_info->colorMode = ui->comboBox_colorModeAdd->currentIndex();
    m_info->valid = 1;
    m_info->enable = 1;

    this->close();
    emit this->sigAddSche();
}

void DayNightScheAdd::on_pushButton_cancel_add_clicked()
{
    this->close();
}

