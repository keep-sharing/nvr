#include "FilePassword.h"
#include "ui_FilePassword.h"
#include "MessageBox.h"
#include "MsLanguage.h"

FilePassword::FilePassword(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::FilePassword)
{
    ui->setupUi(this);
    ui->lineEditPassword->setCheckMode(MyLineEdit::EmptyCheck);

    onLanguageChanged();
}

FilePassword::~FilePassword()
{
    delete ui;
}

void FilePassword::setMode(int mode)
{
    m_mode = mode;
    switch (mode) {
    case ModeEncrption:
        ui->label_title->setText(GET_TEXT("CAMERAMAINTENANCE/152005", "File Encrption"));
        break;
    case ModeDecryption:
        ui->label_title->setText(GET_TEXT("CAMERAMAINTENANCE/152006", "File Decrption"));
        ui->labelConfirm->hide();
        ui->lineEditConfirm->hide();
        break;
    }
}

QString FilePassword::getPassword()
{
    return ui->lineEditPassword->text();
}

void FilePassword::onLanguageChanged()
{
    ui->labelPassword->setText(GET_TEXT("COMMON/1008", "Password"));
    ui->labelConfirm->setText(GET_TEXT("USER/74055", "Confirm Password"));

    ui->pushButtonOK->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButtonCancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void FilePassword::on_pushButtonOK_clicked()
{
    if (!ui->lineEditPassword->checkValid()) {
        return;
    }
    switch (m_mode) {
    case ModeEncrption: {
        if (ui->lineEditConfirm->text() != ui->lineEditPassword->text()) {
            ui->lineEditConfirm->setCustomValid(false, GET_TEXT("USER/74014", "Passwords don't match."));
            return;
        }

        break;
    }
    case ModeDecryption:
        const int &result = MessageBox::question(this, GET_TEXT("CAMERAMAINTENANCE/152004", "It will restart the device, continue?"));
        if (result == MessageBox::Cancel) {
            reject();
            return;
        }
        break;
    }
    accept();
}

void FilePassword::on_pushButtonCancel_clicked()
{
    reject();
}
