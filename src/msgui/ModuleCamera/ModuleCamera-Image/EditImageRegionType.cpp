#include "EditImageRegionType.h"
#include "ui_EditImageRegionType.h"
#include "MsLanguage.h"

EditImageRegionType::EditImageRegionType(QWidget *parent) :
    BaseShadowDialog(parent),
    ui(new Ui::EditImageRegionType)
{
    ui->setupUi(this);
    ui->comboBoxColor->clear();
    ui->comboBoxColor->beginEdit();
    ui->comboBoxColor->addItem(GET_TEXT("IMAGE/37322", "White"), 0);
    ui->comboBoxColor->addItem(GET_TEXT("IMAGE/37323", "Black"), 1);
    ui->comboBoxColor->addItem(GET_TEXT("IMAGE/37324", "Blue"), 2);
    ui->comboBoxColor->addItem(GET_TEXT("IMAGE/37403", "Yellow"), 3);
    ui->comboBoxColor->addItem(GET_TEXT("IMAGE/37404", "Green"), 4);
    ui->comboBoxColor->addItem(GET_TEXT("IMAGE/37405", "Brown"), 5);
    ui->comboBoxColor->addItem(GET_TEXT("IMAGE/37406", "Red"), 6);
    ui->comboBoxColor->addItem(GET_TEXT("IMAGE/37407", "Purple"), 7);
    ui->comboBoxColor->endEdit();

    onLanguageChanged();
}

EditImageRegionType::~EditImageRegionType()
{
    delete ui;
}

void EditImageRegionType::setColor(int color)
{
    ui->comboBoxColor->setCurrentIndexFromData(color);
}

int EditImageRegionType::getColor()
{
    return ui->comboBoxColor->currentIntData();
}

void EditImageRegionType::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("IMAGE/120006", "Privacy Mask Edit"));
    ui->labelColor->setText(GET_TEXT("IMAGE/120005", "Mask Color"));
    ui->pushButtonOK->setText(GET_TEXT("COMMON/1001","OK"));
    ui->pushButtonCancel->setText(GET_TEXT("COMMON/1004","Cancel"));
}

void EditImageRegionType::on_pushButtonOK_clicked()
{
    accept();
}

void EditImageRegionType::on_pushButtonCancel_clicked()
{
    reject();
}
