#include "daynightscheedit.h"
#include "ui_daynightscheedit.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "centralmessage.h"
#include <QDebug>
#include <QLineEdit>
#include <QValidator>

extern "C" {
#include "msg.h"
}

DayNightScheEdit::DayNightScheEdit(bool isWhite, QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::DayNightScheEdit)
{
    ui->setupUi(this);

    ui->comboBox_exposureLevel->clear();
    for (int i = 0; i < 11; i++) {
        ui->comboBox_exposureLevel->addItem(QString("%1").arg(i));
    }
    ui->comboBox_exposureLevel->setCurrentIndex(5);

    ui->lineEdit_gainLevel->setCheckMode(MyLineEdit::RangeCheck, 1, 100);

    ui->comboBox_latemcyIrCut->clear();
    for (int i = 1; i <= 20; i++) {
        ui->comboBox_latemcyIrCut->addItem(QString("%1s").arg(i));
    }

    ui->comboBox_irCut->clear();
    ui->comboBox_irCut->addItem(GET_TEXT("IMAGE/37316", "Off"));
    ui->comboBox_irCut->addItem(GET_TEXT("IMAGE/37317", "On"));

    ui->comboBox_irLed->clear();
    if (isWhite) {
        ui->comboBox_irLed->addItem(GET_TEXT("IMAGE/37366", "All LED Off"), 0);
        ui->comboBox_irLed->addItem(GET_TEXT("IMAGE/37367", "IR LED On"), 1);
        ui->comboBox_irLed->addItem(GET_TEXT("IMAGE/37368", "White LED On"), 2);
    } else {
        ui->comboBox_irLed->addItem(GET_TEXT("IMAGE/37316", "Off"), 0);
        ui->comboBox_irLed->addItem(GET_TEXT("IMAGE/37317", "On"), 1);
    }

    ui->comboBox_colorMode->clear();
    ui->comboBox_colorMode->addItem(GET_TEXT("IMAGE/37312", "B/W"));
    ui->comboBox_colorMode->addItem(GET_TEXT("IMAGE/337313", "Color"));

    slotTranslateUi();
}

DayNightScheEdit::~DayNightScheEdit()
{
    delete ui;
}

void DayNightScheEdit::slotTranslateUi()
{
    ui->label_title->setText(GET_TEXT("IMAGE/37351", "Schedule Edit"));
    ui->label_exposureLevel->setText(GET_TEXT("IMAGE/37301", "Exposure Level"));
    ui->label_minShutter->setText(GET_TEXT("IMAGE/37302", "Minimum Shutter"));
    ui->label_maxShutter->setText(GET_TEXT("IMAGE/37303", "Maximum Shutter"));
    ui->label_gainLevel->setText(GET_TEXT("IMAGE/37304", "Limit Gain Level"));
    ui->label_latencyIrCut->setText(GET_TEXT("IMAGE/37305", "IR-CUT Latency"));
    ui->label_irCut->setText(GET_TEXT("IMAGE/37306", "IR-CUT"));
    ui->label_irLed->setText(GET_TEXT("IMAGE/37307", "IR LED"));
    ui->label_colorMode->setText(GET_TEXT("IMAGE/37311", "Color Mode"));

    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void DayNightScheEdit::initDayNightEditInfo(image_day_night *data, int id, int fullcolorSupport)
{
    m_info = data;
    m_scheId = id;
    m_type = IPC_IMAGE_TYPE_SINGLE;
    ui->comboBox_exposureLevel->show();
    ui->label_exposureLevel->show();
    if (fullcolorSupport) {
        ui->label_irCut->hide();
        ui->comboBox_irCut->hide();
        ui->label_irLed->hide();
        ui->comboBox_irLed->hide();
        ui->label_latencyIrCut->hide();
        ui->comboBox_latemcyIrCut->hide();
    }
    if (m_scheId < OTHER) {
        ui->comboBox_latemcyIrCut->setCurrentIndex(data->irCutLatency - 1);
    } else {
        ui->comboBox_latemcyIrCut->setCurrentIndex(data->irCutLatency);
    }
    ui->comboBox_exposureLevel->setCurrentIndex(data->exposureLevel);
    ui->comboBox_minShutter->setCurrentIndexFromData(data->minShutter);
    ui->comboBox_maxShutter->setCurrentIndexFromData(data->maxShutter);
    ui->lineEdit_gainLevel->setText(QString("%1").arg(data->gainLevel));
    ui->comboBox_irCut->setCurrentIndex(data->irCutState);
    qWarning() << data->irLedState;
    ui->comboBox_irLed->setCurrentIndexFromData(data->irLedState);
    ui->comboBox_colorMode->setCurrentIndex(data->colorMode);
}

void DayNightScheEdit::initDayNightEditInfoMulti(ImageDaynightScene *data, int id, int type)
{
    m_infoMulti = data;
    m_scheId = id;
    m_type = type;
    if (type == IPC_IMAGE_TYPE_MULTI) {
        ui->comboBox_exposureLevel->hide();
        ui->label_exposureLevel->hide();
    }
    ui->comboBox_latemcyIrCut->setCurrentIndex(data->irCutInterval[id] - 1);
    ui->comboBox_minShutter->setCurrentIndexFromData(data->minShutter[id]);
    ui->comboBox_maxShutter->setCurrentIndexFromData(data->maxShutter[id]);
    ui->lineEdit_gainLevel->setText(QString("%1").arg(data->limitGain[id]));
    ui->comboBox_irCut->setCurrentIndex(data->irCutStatus[id]);
    qWarning() << data->irCutStatus[id];
    ui->comboBox_irLed->setCurrentIndexFromData(data->irLedStatus[id]);
    ui->comboBox_colorMode->setCurrentIndex(data->colorMode[id]);
}

void DayNightScheEdit::on_pushButton_ok_clicked()
{
    if (!ui->lineEdit_gainLevel->checkValid()) {
        return;
    }
    int minShutter = ui->comboBox_minShutter->currentData().toInt();
    int maxShutter = ui->comboBox_maxShutter->currentData().toInt();
    if ((maxShutter == 13 && minShutter != 13) || (minShutter != 13 && minShutter > maxShutter)) {
        ShowMessageBox(GET_TEXT("IMAGE/37365", "Minimum Shutter should be larger than Maximum Shutter."));
        return;
    }
    if (m_type == IPC_IMAGE_TYPE_MULTI) {
        m_infoMulti->irCutInterval[m_scheId] = ui->comboBox_latemcyIrCut->currentIndex() + 1;
        m_infoMulti->minShutter[m_scheId]    = minShutter;
        m_infoMulti->maxShutter[m_scheId]    = maxShutter;
        m_infoMulti->limitGain[m_scheId]     = ui->lineEdit_gainLevel->text().toInt();
        m_infoMulti->irCutStatus[m_scheId]   = ui->comboBox_irCut->currentIndex();
        m_infoMulti->irLedStatus[m_scheId]   = ui->comboBox_irLed->currentData().toInt();
        m_infoMulti->colorMode[m_scheId]     = ui->comboBox_colorMode->currentIndex();
        m_infoMulti->valid[m_scheId]         = 1;
    } else {
        if (m_scheId < OTHER) {
            m_info->irCutLatency = ui->comboBox_latemcyIrCut->currentIndex() + 1;
        } else {
            m_info->irCutLatency = ui->comboBox_latemcyIrCut->currentIndex();
        }

        m_info->exposureLevel = ui->comboBox_exposureLevel->currentIndex();
        m_info->minShutter    = minShutter;
        m_info->maxShutter    = maxShutter;
        m_info->gainLevel     = ui->lineEdit_gainLevel->text().toInt();
        m_info->irCutState    = ui->comboBox_irCut->currentIndex();
        m_info->irLedState    = ui->comboBox_irLed->currentData().toInt();
        m_info->colorMode     = ui->comboBox_colorMode->currentIndex();
        m_info->valid         = 1;
    }

    this->close();
    emit this->sigEditSche();
}

void DayNightScheEdit::on_pushButton_cancel_clicked()
{
    this->close();
}

void DayNightScheEdit::setExposureCtrl(int type)
{
    m_exposureCtrl = type;
    initShutter();
}

void DayNightScheEdit::initShutter()
{
    if (!m_exposureCtrl) {
        ui->comboBox_minShutter->clear();
        ui->comboBox_minShutter->addItem("1", 13);
        ui->comboBox_minShutter->addItem("1/5", 0);
        ui->comboBox_minShutter->addItem("1/15", 1);
        ui->comboBox_minShutter->addItem("1/30", 2);
        ui->comboBox_minShutter->addItem("1/60", 3);
        ui->comboBox_minShutter->addItem("1/120", 4);
        ui->comboBox_minShutter->addItem("1/250", 5);
        ui->comboBox_minShutter->addItem("1/500", 6);
        ui->comboBox_minShutter->addItem("1/750", 7);
        ui->comboBox_minShutter->addItem("1/1000", 8);
        ui->comboBox_minShutter->addItem("1/2000", 9);
        ui->comboBox_minShutter->addItem("1/4000", 10);
        ui->comboBox_minShutter->addItem("1/10000", 11);
        ui->comboBox_minShutter->addItem("1/100000", 12);
        ui->comboBox_minShutter->setCurrentIndexFromData(2);

        ui->comboBox_maxShutter->clear();
        ui->comboBox_maxShutter->addItem("1", 13);
        ui->comboBox_maxShutter->addItem("1/5", 0);
        ui->comboBox_maxShutter->addItem("1/15", 1);
        ui->comboBox_maxShutter->addItem("1/30", 2);
        ui->comboBox_maxShutter->addItem("1/60", 3);
        ui->comboBox_maxShutter->addItem("1/120", 4);
        ui->comboBox_maxShutter->addItem("1/250", 5);
        ui->comboBox_maxShutter->addItem("1/500", 6);
        ui->comboBox_maxShutter->addItem("1/750", 7);
        ui->comboBox_maxShutter->addItem("1/1000", 8);
        ui->comboBox_maxShutter->addItem("1/2000", 9);
        ui->comboBox_maxShutter->addItem("1/4000", 10);
        ui->comboBox_maxShutter->addItem("1/10000", 11);
        ui->comboBox_maxShutter->addItem("1/100000", 12);
        ui->comboBox_maxShutter->setCurrentIndexFromData(12);
    } else {
        ui->comboBox_minShutter->clear();
        ui->comboBox_minShutter->addItem("1", 13);
        ui->comboBox_minShutter->addItem("1/5", 0);
        ui->comboBox_minShutter->addItem("1/10", 1);
        ui->comboBox_minShutter->addItem("1/25", 2);
        ui->comboBox_minShutter->addItem("1/50", 3);
        ui->comboBox_minShutter->addItem("1/100", 4);
        ui->comboBox_minShutter->addItem("1/250", 5);
        ui->comboBox_minShutter->addItem("1/500", 6);
        ui->comboBox_minShutter->addItem("1/750", 7);
        ui->comboBox_minShutter->addItem("1/1000", 8);
        ui->comboBox_minShutter->addItem("1/2000", 9);
        ui->comboBox_minShutter->addItem("1/4000", 10);
        ui->comboBox_minShutter->addItem("1/10000", 11);
        ui->comboBox_minShutter->addItem("1/100000", 12);
        ui->comboBox_minShutter->setCurrentIndexFromData(2);

        ui->comboBox_maxShutter->clear();
        ui->comboBox_maxShutter->addItem("1", 13);
        ui->comboBox_maxShutter->addItem("1/5", 0);
        ui->comboBox_maxShutter->addItem("1/10", 1);
        ui->comboBox_maxShutter->addItem("1/25", 2);
        ui->comboBox_maxShutter->addItem("1/50", 3);
        ui->comboBox_maxShutter->addItem("1/100", 4);
        ui->comboBox_maxShutter->addItem("1/250", 5);
        ui->comboBox_maxShutter->addItem("1/500", 6);
        ui->comboBox_maxShutter->addItem("1/750", 7);
        ui->comboBox_maxShutter->addItem("1/1000", 8);
        ui->comboBox_maxShutter->addItem("1/2000", 9);
        ui->comboBox_maxShutter->addItem("1/4000", 10);
        ui->comboBox_maxShutter->addItem("1/10000", 11);
        ui->comboBox_maxShutter->addItem("1/100000", 12);
        ui->comboBox_maxShutter->setCurrentIndexFromData(12);
    }
}
