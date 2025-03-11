#include "SearchCameraActivate.h"
#include "ui_SearchCameraActivate.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "myqt.h"
#include <QFile>

extern "C" {
#include "msg.h"
}

SearchCameraActivate::SearchCameraActivate(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::SearchCameraActivate)
{
    ui->setupUi(this);

    QFile file(":/style/style/settingstyle.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(file.readAll());
    }
    file.close();

    ui->lineEdit_Password->setCheckMode(MyLineEdit::PasswordCheck);
    ui->lineEdit_Username->setCheckMode(MyLineEdit::UserNameCheck);

    onLanguageChanged();
}

SearchCameraActivate::~SearchCameraActivate()
{
    delete ui;
}

void SearchCameraActivate::on_pushButton_ok_clicked()
{
    char password[MAX_PASSWORD_CHAR] = { 0 };
    char confirm[MAX_PASSWORD_CHAR] = { 0 };

    snprintf(password, sizeof(password), "%s", ui->lineEdit_Password->text().toStdString().c_str());
    snprintf(confirm, sizeof(password), "%s", ui->lineEdit_Confirm->text().toStdString().c_str());

    if (ui->lineEdit_Password->checkTooLong()) {
        ui->lineEdit_Confirm->setTipFronSize(11);
        ui->lineEdit_Username->setTipFronSize(11);
    } else {
        ui->lineEdit_Confirm->setTipFronSize(14);
        ui->lineEdit_Username->setTipFronSize(14);
    }

    bool valid = ui->lineEdit_Username->checkValid();
    valid = ui->lineEdit_Password->checkValid() && valid;
    if (!ui->lineEdit_Password->text().isEmpty() && strcmp(password, confirm) != 0) {
        ui->lineEdit_Confirm->setCustomValid(false, GET_TEXT("MYLINETIP/112008", "Passwords do not match."));
        valid = false;
    }
    if (!valid) {
        return;
    }

    emit sigCameraActive(ui->lineEdit_Password->text().trimmed());
    accept();
}

void SearchCameraActivate::on_pushButton_cancel_clicked()
{
    reject();
}

void SearchCameraActivate::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("CAMERASEARCH/32004", "Camera Activation"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));

    ui->label_Username->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->label_Password->setText(GET_TEXT("WIZARD/11069", "Password"));
    ui->label_Confirm->setText(GET_TEXT("USER/74055", "Confirm Password"));

    ui->lineEdit_Password->clear();
    ui->lineEdit_Confirm->clear();

    ui->pushButton_ok->setFocus();
}
