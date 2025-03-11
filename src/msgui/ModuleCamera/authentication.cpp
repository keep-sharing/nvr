#include "authentication.h"
#include "ui_authentication.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "centralmessage.h"
#include "myqt.h"
#include <QtDebug>

Authentication::Authentication(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::Authentication)
{
    ui->setupUi(this);

    ui->pushButton_ok->setFocus();
    //
    ui->lineEdit_userName->setCheckMode(MyLineEdit::UserNameCheck);
    ui->lineEdit_password->setCheckMode(MyLineEdit::EmptyCheck);
    onLanguageChanged();
}

Authentication::~Authentication()
{
    delete ui;
}

void Authentication::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("CHANNELMANAGE/154000", "Authentication Edit"));
    ui->label_Username->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->label_password->setText(GET_TEXT("COMMON/1008", "Password"));
    ui->label_note->setText(GET_TEXT("CHANNELMANAGE/30068", "Note: Edit authentication for selected cameras."));

    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void Authentication::on_pushButton_ok_clicked()
{
    if (!isInputValid()) {
        return;
    }

    int count = 0;
    camera cam_array[MAX_CAMERA];

    read_cameras(SQLITE_FILE_NAME, cam_array, &count);
    for (int i = 0; i < m_chnAuthCheckedList.size(); i++) {
        int channel = m_chnAuthCheckedList.at(i);
        snprintf(cam_array[channel].username, sizeof(cam_array[channel].username), "%s", ui->lineEdit_userName->text().toStdString().c_str());
        snprintf(cam_array[channel].password, sizeof(cam_array[channel].password), "%s", ui->lineEdit_password->text().toStdString().c_str());
        qMsNvr->writeDatabaseCamera(&cam_array[channel]);

        sendMessageOnly(REQUEST_FLAG_UPDATE_IPC, (void *)&channel, sizeof(int));
    }

    emit changed();
    close();
}

void Authentication::on_pushButton_cancel_clicked()
{
    close();
}

bool Authentication::isInputValid()
{
    bool valid = ui->lineEdit_userName->checkValid();
    valid = ui->lineEdit_password->checkValid() && valid;
    if (!valid) {
        return false;
    }
    return true;
}

void Authentication::append_edit_authentication(int channel)
{
    m_chnAuthCheckedList.append(channel);
}

void Authentication::initializeData()
{
    char value[64] = { 0 };
    m_chnAuthCheckedList.clear();

    get_param_value(SQLITE_FILE_NAME, PARAM_POE_PSD, value, sizeof(value), POE_DEFAULT_PASSWORD);
    ui->lineEdit_userName->setText("admin");
    ui->lineEdit_password->setText(trUtf8("%1").arg(value));
}
