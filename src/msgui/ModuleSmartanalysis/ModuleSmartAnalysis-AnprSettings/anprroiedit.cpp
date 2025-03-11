#include "anprroiedit.h"
#include "ui_anprroiedit.h"
#include "MsLanguage.h"
AnprRoiEdit::AnprRoiEdit(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::AnprRoiEdit)
{
    ui->setupUi(this);
    onLanguageChanged();
}

AnprRoiEdit::~AnprRoiEdit()
{
    delete ui;
}

void AnprRoiEdit::showEdit(const QString &name, bool isTrapeziform)
{
    if (isTrapeziform) {
        ui->lineEdit_name->setMaxLength(16);
    } else {
        ui->lineEdit_name->setMaxLength(32);
    }
    ui->lineEdit_name->setText(name);
}

QString AnprRoiEdit::name() const
{
    return ui->lineEdit_name->text();
}

void AnprRoiEdit::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("ANPR/103033", "ROI Edit"));
    ui->label_name->setText(GET_TEXT("ANPR/103034", "ROI Name"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void AnprRoiEdit::on_pushButton_ok_clicked()
{
    accept();
}

void AnprRoiEdit::on_pushButton_cancel_clicked()
{
    reject();
}
