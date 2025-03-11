#include "ptzmaskedit.h"
#include "ui_ptzmaskedit.h"
#include "MsLanguage.h"

PtzMaskEdit::PtzMaskEdit(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::PtzMaskEdit)
{
    ui->setupUi(this);

    ui->comboBox_type->clear();
    ui->comboBox_type->addItem(GET_TEXT("IMAGE/37322", "White"), 0);
    ui->comboBox_type->addItem(GET_TEXT("IMAGE/37323", "Black"), 1);
    ui->comboBox_type->addItem(GET_TEXT("IMAGE/37324", "Blue"), 2);
    ui->comboBox_type->addItem(GET_TEXT("IMAGE/37403", "Yellow"), 3);
    ui->comboBox_type->addItem(GET_TEXT("IMAGE/37404", "Green"), 4);
    ui->comboBox_type->addItem(GET_TEXT("IMAGE/37405", "Brown"), 5);
    ui->comboBox_type->addItem(GET_TEXT("IMAGE/37406", "Red"), 6);
    ui->comboBox_type->addItem(GET_TEXT("IMAGE/37407", "Purple"), 7);

    ui->lineEdit_name->setCheckMode(MyLineEdit::EmptyCheck);
    onLanguageChanged();
}

PtzMaskEdit::~PtzMaskEdit()
{
    delete ui;
}

int PtzMaskEdit::execEdit(mask_area_ex *area, int maxZoomRatio)
{
    m_area = area;
    ui->spinBox_ratio->setMaximum(maxZoomRatio);

    ui->lineEdit_name->setText(QString(area->name));
    if (area->fill_color == 8) {
        ui->label_type->hide();
        ui->comboBox_type->hide();
    } else {
        ui->comboBox_type->setCurrentIndexFromData(area->fill_color);
    }
    ui->spinBox_ratio->setValue(area->ratio);

    return exec();
}

void PtzMaskEdit::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("PTZCONFIG/36062", "PTZ Privacy Mask Edit"));
    ui->label_type->setText(GET_TEXT("IMAGE/120005", "Mask Color"));
    ui->label_name->setText(GET_TEXT("CAMERASTATUS/62002", "Name"));
    ui->label_ratio->setText(GET_TEXT("PTZCONFIG/36053", "Active Zoom Ratio"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void PtzMaskEdit::on_pushButton_ok_clicked()
{
    if (!ui->lineEdit_name->checkValid()) {
        return;
    }
    snprintf(m_area->name, sizeof(m_area->name), "%s", ui->lineEdit_name->text().toStdString().c_str());
    if (m_area->fill_color != 8) {
        m_area->fill_color = ui->comboBox_type->currentData().toInt();
    }
    m_area->ratio = ui->spinBox_ratio->value();

    accept();
}

void PtzMaskEdit::on_pushButton_cancel_clicked()
{
    reject();
}
