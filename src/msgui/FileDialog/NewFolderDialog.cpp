#include "NewFolderDialog.h"
#include "ui_NewFolderDialog.h"
#include <QtDebug>
#include "MessageBox.h"
#include "MsLanguage.h"
#include "MsGlobal.h"

NewFolderDialog::NewFolderDialog(QWidget *parent) :
    BaseShadowDialog(parent),
    ui(new Ui::NewFolderDialog)
{
    ui->setupUi(this);
    ui->lineEdit_name->setMaxLength(32);
    ui->lineEdit_name->setCheckMode(MyLineEdit::FolderNameCheck);

    onLanguageChanged();
}

NewFolderDialog::~NewFolderDialog()
{
    delete ui;
}

QString NewFolderDialog::name() const
{
    return ui->lineEdit_name->text();
}

void NewFolderDialog::showEvent(QShowEvent *event)
{
    ui->lineEdit_name->clear();
    BaseDialog::showEvent(event);
}

void NewFolderDialog::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("COMMONBACKUP/100014", "New Folder"));
    ui->label_name->setText(GET_TEXT("COMMONBACKUP/100016", "Folder Name"));
    ui->pushButton_create->setText(GET_TEXT("DISKMANAGE/72074", "Create"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void NewFolderDialog::on_pushButton_create_clicked()
{
    const QString &name = ui->lineEdit_name->text();
    if (!ui->lineEdit_name->checkValid()) {
        return;
    }
    emit sig_newFolder(name);
    accept();
}

void NewFolderDialog::on_pushButton_cancel_clicked()
{
    reject();
}
