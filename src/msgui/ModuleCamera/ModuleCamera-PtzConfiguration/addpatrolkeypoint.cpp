#include "addpatrolkeypoint.h"
#include "ui_addpatrolkeypoint.h"
#include "MsCameraVersion.h"
#include "MsLanguage.h"

AddPatrolKeyPoint::AddPatrolKeyPoint(Mode mode, int channel, QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::AddPatrolKeyPoint)
{
    ui->setupUi(this);

    switch (mode) {
    case ModeAdd:
        ui->label_title->setText(GET_TEXT("PTZCONFIG/36018", "Key Point Add"));
        break;
    case ModeEdit:
        ui->label_title->setText(GET_TEXT("PTZCONFIG/36017", "Key Point Edit"));
        break;
    default:
        break;
    }
    //78版本及以上有300个预置点
    MsCameraVersion cameraVersion = MsCameraVersion::fromChannel(channel);
    int maxCount = cameraVersion >= MsCameraVersion(7, 78) ? 300 : 255;
    ui->spinBox_preset->setMaximum(maxCount);
    onLanguageChanged();
}

AddPatrolKeyPoint::~AddPatrolKeyPoint()
{
    delete ui;
}

void AddPatrolKeyPoint::showKeyPointAdd(int keyIndex)
{
    ui->spinBox_time->setValue(ui->spinBox_time->minimum());
    ui->lineEdit_key->setText(QString::number(keyIndex + 1));
}

void AddPatrolKeyPoint::showKeyPointEdit(int keyIndex)
{
    ui->lineEdit_key->setText(QString::number(keyIndex + 1));
}

void AddPatrolKeyPoint::setPresetPoint(int value)
{
    ui->spinBox_preset->setValue(value);
}

int AddPatrolKeyPoint::presetPoint() const
{
    return ui->spinBox_preset->value();
}

void AddPatrolKeyPoint::setScanTime(int value)
{
    ui->spinBox_time->setValue(value);
}

int AddPatrolKeyPoint::scanTime() const
{
    return ui->spinBox_time->value();
}

void AddPatrolKeyPoint::setScanSpeed(int value)
{
    ui->spinBox_speed->setValue(value);
}

int AddPatrolKeyPoint::scanSpeed() const
{
    return ui->spinBox_speed->value();
}

void AddPatrolKeyPoint::setScanTimeRange(int min, int max)
{
    ui->spinBox_time->setRange(min, max);
}

void AddPatrolKeyPoint::onLanguageChanged()
{
    ui->labelKeyPoint->setText(GET_TEXT("PTZCONFIG/36023", "Key Point"));
    ui->label_preset->setText(GET_TEXT("PTZCONFIG/36019", "Preset Point"));
    ui->label_scanSpeed->setText(GET_TEXT("PTZCONFIG/36026", "Scan Speed"));
    ui->label_scanTime->setText(GET_TEXT("PTZCONFIG/36025", "Scan Time"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
}

void AddPatrolKeyPoint::on_pushButton_ok_clicked()
{
    accept();
}

void AddPatrolKeyPoint::on_pushButton_cancel_clicked()
{
    reject();
}
