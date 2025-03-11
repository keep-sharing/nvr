#include "ThresholdsEdit.h"
#include "ui_ThresholdsEdit.h"
#include "MsLanguage.h"

ThresholdsEdit::ThresholdsEdit(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::ThresholdsEdit)
{
    ui->setupUi(this);
    ui->checkableLineIn->setRange(1, 9999);
    ui->checkableLineOut->setRange(1, 9999);
    ui->checkableLineCapacity->setRange(1, 9999);
    ui->checkableLineSum->setRange(1, 9999);

    onLanguageChanged();
}

ThresholdsEdit::~ThresholdsEdit()
{
    delete ui;
}

void ThresholdsEdit::initializeData(PCNT_CNT_E enableFlag, const int *alarmThresholds)
{
    ui->checkableLineIn->setValue(alarmThresholds[PCNT_CNT_BIT_IN]);
    ui->checkableLineOut->setValue(alarmThresholds[PCNT_CNT_BIT_OUT]);
    ui->checkableLineCapacity->setValue(alarmThresholds[PCNT_CNT_BIT_CAPACITY]);
    ui->checkableLineSum->setValue(alarmThresholds[PCNT_CNT_BIT_SUM]);

    ui->checkableLineIn->setChecked(enableFlag & PCNT_CNT_IN);
    ui->checkableLineOut->setChecked(enableFlag & PCNT_CNT_OUT);
    ui->checkableLineCapacity->setChecked(enableFlag & PCNT_CNT_CAPACITY);
    ui->checkableLineSum->setChecked(enableFlag & PCNT_CNT_SUM);
}

void ThresholdsEdit::getThresholdsData(PCNT_CNT_E &enableFlag, int *alarmThresholds)
{
    uint temp = 0;
    if (ui->checkableLineIn->isChecked()) {
        temp |= PCNT_CNT_IN;
    }
    if (ui->checkableLineOut->isChecked()) {
        temp |= PCNT_CNT_OUT;
    }
    if (ui->checkableLineCapacity->isChecked()) {
        temp |= PCNT_CNT_CAPACITY;
    }
    if (ui->checkableLineSum->isChecked()) {
        temp |= PCNT_CNT_SUM;
    }
    enableFlag = static_cast<PCNT_CNT_E>(temp);
    alarmThresholds[PCNT_CNT_BIT_IN] = ui->checkableLineIn->value();
    alarmThresholds[PCNT_CNT_BIT_OUT] = ui->checkableLineOut->value();
    alarmThresholds[PCNT_CNT_BIT_CAPACITY] = ui->checkableLineCapacity->value();
    alarmThresholds[PCNT_CNT_BIT_SUM] = ui->checkableLineSum->value();
}

void ThresholdsEdit::onLanguageChanged()
{
    ui->labelIn->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/145017", "In"));
    ui->labelOut->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/145018", "Out"));
    ui->labelSum->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/145019", "Sum"));
    ui->labelCapacity->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/145020", "Capacity"));
    ui->label_title->setText(GET_TEXT("REGIONAL_PEOPLECOUNTING/158001", "Thresholds"));
    ui->pushButtonOk->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButtonCancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void ThresholdsEdit::on_pushButtonOk_clicked()
{
    bool valid = true;
    if (ui->checkableLineIn->isChecked()) {
        valid &= ui->checkableLineIn->checkValid();
    }
    if (ui->checkableLineOut->isChecked()) {
        valid &= ui->checkableLineOut->checkValid();
    }
    if (ui->checkableLineCapacity->isChecked()) {
        valid &= ui->checkableLineCapacity->checkValid();
    }
    if (ui->checkableLineSum->isChecked()) {
        valid &= ui->checkableLineSum->checkValid();
    }
    if (valid) {
        accept();
    } else {
        return;
    }
}

void ThresholdsEdit::on_pushButtonCancel_clicked()
{
    reject();
}
