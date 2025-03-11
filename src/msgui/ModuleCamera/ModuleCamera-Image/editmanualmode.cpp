#include "editmanualmode.h"
#include "ui_editmanualmode.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "manualmodesettings.h"

EditManualMode::EditManualMode(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::EditManualMode)
{
    ui->setupUi(this);

    m_manualMode = static_cast<ManualModeSettings *>(parent);

    ui->slider_gainLevel->setRange(1, 100);
    ui->slider_gainLevel->setValue(50);

    slotTranslateUi();
}

EditManualMode::~EditManualMode()
{
    delete ui;
}

int EditManualMode::exposureTime() const
{
    int value = 0;
    int index = ui->comboBox_exposureTime->currentData().toInt();
    if (index == 0)
        value = 13;
    else
        value = index - 1;
    return value;
}

int EditManualMode::gainLevel() const
{
    return ui->slider_gainLevel->value();
}

void EditManualMode::setExposureIndex(int index)
{
    int value;
    if (index == 13)
        value = 0;
    else
        value = index + 1;
    ui->comboBox_exposureTime->setCurrentIndexFromData(value);
}

void EditManualMode::setGainLevelValue(int value)
{
    ui->slider_gainLevel->setValue(value);
}

void EditManualMode::setEditIndex(int index)
{
    m_editIndex = index;
}

void EditManualMode::slotTranslateUi()
{
    ui->label_exposureTime->setText(GET_TEXT("IMAGE/37210", "Exposure Time"));
    ui->label_gainLevel->setText(GET_TEXT("IMAGE/37211", "Gain Level"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void EditManualMode::setExposureCtrl(int type)
{
    int i = 0, j = 0;
    m_exposureCtrl = type;
    ui->comboBox_exposureTime->clear();
    ui->comboBox_exposureTime->addItem(exposureTimeString(13), 0);
    for (i = 0, j = 1; i < 13; ++i, ++j) {
        ui->comboBox_exposureTime->addItem(exposureTimeString(i), j);
    }
    ui->comboBox_exposureTime->setCurrentIndexFromData(3);
}

void EditManualMode::setMode(EditManualMode::ModeType type)
{
    switch (type) {
    case ModeAdd:
        ui->label_title->setText(GET_TEXT("IMAGE/37353", "Manual Mode Add"));
        break;
    case ModeEdit:
        ui->label_title->setText(GET_TEXT("IMAGE/37352", "Manual Mode Edit"));
        break;
    }
}

QString EditManualMode::exposureTimeString(int index)
{
    QString text;
    if (!m_exposureCtrl) {
        switch (index) {
        case 0:
            text = "1/5";
            break;
        case 1:
            text = "1/15";
            break;
        case 2:
            text = "1/30";
            break;
        case 3:
            text = "1/60";
            break;
        case 4:
            text = "1/120";
            break;
        case 5:
            text = "1/250";
            break;
        case 6:
            text = "1/500";
            break;
        case 7:
            text = "1/750";
            break;
        case 8:
            text = "1/1000";
            break;
        case 9:
            text = "1/2000";
            break;
        case 10:
            text = "1/4000";
            break;
        case 11:
            text = "1/10000";
            break;
        case 12:
            text = "1/100000";
            break;
        case 13:
            text = "1";
            break;
        }
    } else {
        switch (index) {
        case 0:
            text = "1/5";
            break;
        case 1:
            text = "1/10";
            break;
        case 2:
            text = "1/25";
            break;
        case 3:
            text = "1/50";
            break;
        case 4:
            text = "1/100";
            break;
        case 5:
            text = "1/250";
            break;
        case 6:
            text = "1/500";
            break;
        case 7:
            text = "1/750";
            break;
        case 8:
            text = "1/1000";
            break;
        case 9:
            text = "1/2000";
            break;
        case 10:
            text = "1/4000";
            break;
        case 11:
            text = "1/10000";
            break;
        case 12:
            text = "1/100000";
            break;
        case 13:
            text = "1";
            break;
        }
    }
    return text;
}

void EditManualMode::on_pushButton_ok_clicked()
{
    if (m_manualMode->isModeExist(exposureTime(), gainLevel(), m_editIndex)) {
        ShowMessageBox(GET_TEXT("IMAGE/37232", "Manual mode settings already exist!"));
        ui->pushButton_ok->clearUnderMouse();
        return;
    }
    accept();
}

void EditManualMode::on_pushButton_cancel_clicked()
{
    reject();
}
