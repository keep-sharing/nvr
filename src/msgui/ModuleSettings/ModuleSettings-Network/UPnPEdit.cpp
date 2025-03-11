#include "UPnPEdit.h"
#include "ui_UPnPEdit.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include <QtDebug>

UPnPEdit::UPnPEdit(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::UPnPEdit)
{
    ui->setupUi(this);
    ui->lineEdit_externalPort->setCheckMode(MyLineEdit::RangeCheck, 1, 65535);

    onLanguageChanged();
}

UPnPEdit::~UPnPEdit()
{
    delete ui;
}

void UPnPEdit::initializeData(const QString &type, int port)
{
    ui->lineEdit_type->setText(type);
    ui->lineEdit_externalPort->setText(QString::number(port));
    ui->lineEdit_externalPort->setFocus();
    ui->lineEdit_externalPort->selectAll();
}

int UPnPEdit::externalPort()
{
    return ui->lineEdit_externalPort->text().toInt();
}

void UPnPEdit::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("SYSTEMNETWORK/71149", "UPnP Edit"));
    ui->label_portType->setText(GET_TEXT("SYSTEMNETWORK/71144", "Port Type"));
    ui->label_externalPort->setText(GET_TEXT("SYSTEMNETWORK/71145", "External Port"));

    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void UPnPEdit::on_pushButton_ok_clicked()
{

    if (!ui->lineEdit_externalPort->checkValid()) {
        return;
    }
    accept();
}

void UPnPEdit::on_pushButton_cancel_clicked()
{
    reject();
}
