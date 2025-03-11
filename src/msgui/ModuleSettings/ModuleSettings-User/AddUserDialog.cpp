#include "AddUserDialog.h"
#include "ui_AddUserDialog.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "edituserlimitsdialog.h"
#include "myqt.h"
#include "setunlockpattern.h"
#include <QCryptographicHash>

AddUserDialog::AddUserDialog(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::AddUser)
{
    ui->setupUi(this);

    ui->comboBox_userLevel->clear();
    ui->comboBox_userLevel->addItem(GET_TEXT("USER/74006", "Operator"), USERLEVEL_OPERATOR);
    ui->comboBox_userLevel->addItem(GET_TEXT("USER/74007", "Viewer"), USERLEVEL_USER);

    ui->comboBox_patternEnable->clear();
    ui->comboBox_patternEnable->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_patternEnable->addItem(GET_TEXT("COMMON/1009", "Eanble"), 1);

    ui->pushButton_patternEdit->setEnabled(false);
    m_pattern_text = "0";

    ui->lineEdit_userName->setCheckMode(MyLineEdit::UserNameCheck);
    ui->lineEdit_password->setCheckMode(MyLineEdit::PasswordCheck);

    memset(&m_userOperator, 0, sizeof(db_user));
    memset(&m_userViewer, 0, sizeof(db_user));
    m_userOperator.type = USERLEVEL_OPERATOR;
    m_userViewer.type = USERLEVEL_USER;
    get_user_default_permission(&m_userOperator);
    get_user_default_permission(&m_userViewer);

    onLanguageChanged();
}

AddUserDialog::~AddUserDialog()
{
    delete ui;
}

void AddUserDialog::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("CHANNELMANAGE/154002", "User Add"));
    ui->label_adminPassword->setText(GET_TEXT("WIZARD/11007", "Admin Password "));
    ui->label_userName->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->label_password->setText(GET_TEXT("COMMON/1008", "Password"));
    ui->label_confirm->setText(GET_TEXT("USER/74055", "Confirm Password"));
    ui->label_userLevel->setText(GET_TEXT("USER/74001", "User Level"));
    ui->label_unlockPattern->setText(GET_TEXT("WIZARD/11080", "Unlock Pattern"));
    ui->label_setUnlockPattern->setText(GET_TEXT("USER/74062", "Set Unlock Pattern"));
    ui->pushButton_patternEdit->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
    ui->pushButtonPermissionsEdit->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->labelUserPermissions->setText(GET_TEXT("USER/142001", "User Permissions"));
}

void AddUserDialog::on_pushButton_ok_clicked()
{
    int userCount = 0;
    read_user_count(SQLITE_FILE_NAME, &userCount);
    if (userCount >= MAX_USER) {
        ShowMessageBox(GET_TEXT("USER/74009", "Reached the maximum number of users."));
        return;
    }

    //
    db_user adminUser;
    memset(&adminUser, 0, sizeof(db_user));
    read_user(SQLITE_FILE_NAME, &adminUser, 0);
    const QString &strAdminPasswordMD5 = QCryptographicHash::hash(ui->lineEdit_adminPassword->text().toLatin1(), QCryptographicHash::Md5).toHex();
    if (strAdminPasswordMD5 != QString(adminUser.password)) {
        ui->lineEdit_adminPassword->setCustomValid(false, GET_TEXT("MYLINETIP/112010", "Incorrect Password."));
        return;
    }

    //
    QString strUserName = ui->lineEdit_userName->text();
    bool valid = ui->lineEdit_userName->checkValid();

    db_user user;
    memset(&user, 0, sizeof(db_user));
    if (valid) {
        read_user_by_name(SQLITE_FILE_NAME, &user, const_cast<char *>(strUserName.toStdString().c_str()));
        if (!QString(user.username).isEmpty()) {
            ui->lineEdit_userName->setCustomValid(false, GET_TEXT("MYLINETIP/112011", "Already existed."));
            valid = false;
        }
    }

    QString strPassword = ui->lineEdit_password->text();
    QString strConfirm = ui->lineEdit_confirm->text();
    valid &= ui->lineEdit_password->checkValid();

    if (!strPassword.isEmpty()) {
        if (strPassword != strConfirm) {
            ui->lineEdit_confirm->setCustomValid(false, GET_TEXT("USER/74014", "Passwords don't match."));
            valid = false;
        }
    }
    if (!valid) {
        return;
    }

    if (ui->comboBox_patternEnable->currentIndex()) {
        if (m_pattern_text.size() < 4) {
            m_pattern_text = "0";
            int result = MessageBox::question(this, GET_TEXT("USER/74061", "Unlock pattern is not set, it will be disabled if you exit, continue?"));
            if (result == MessageBox::Cancel) {
                return;
            }
            ui->comboBox_patternEnable->setCurrentIndex(1);
            on_comboBox_patternEnable_activated(1);
        }
    } else {
        m_pattern_text = "0";
    }
    //
    if (ui->comboBox_userLevel->currentIntData() == USERLEVEL_OPERATOR) {
        memcpy(&user, &m_userOperator, sizeof(db_user));
    } else {
        memcpy(&user, &m_userViewer, sizeof(db_user));
    }
    user.enable = 1;
    snprintf(user.pattern_psw, sizeof(user.pattern_psw), "%s", m_pattern_text.toStdString().c_str());
    const QString &strPasswordMD5 = QCryptographicHash::hash(strPassword.toLatin1(), QCryptographicHash::Md5).toHex();
    snprintf(user.username, sizeof(user.username), "%s", strUserName.toStdString().c_str());
    snprintf(user.password, sizeof(user.password), "%s", strPasswordMD5.toStdString().c_str());
    snprintf(user.password_ex, sizeof(user.password_ex), "%s", strPassword.toStdString().c_str());
    user.type = ui->comboBox_userLevel->currentData().toInt();
    switch (user.type) {
    case USERLEVEL_OPERATOR:
        user.permission = (ACCESS_STATE_LOG | ACCESS_PLAYBACK | ACCESS_PLAYBACKEXPORT | ACCESS_PTZ_CTRL | ACCESS_PTZ_SETTINGS | ACCESS_EMERGENCY_REC | ACCESS_SNAPSHOT);
        user.remote_permission = (ACCESS_STATE_LOG | ACCESS_PLAYBACK | ACCESS_PLAYBACKEXPORT | ACCESS_PTZ_CTRL | ACCESS_PTZ_SETTINGS | ACCESS_EMERGENCY_REC | ACCESS_SNAPSHOT);
        break;
    case USERLEVEL_USER:
        user.permission = (ACCESS_STATE_LOG | ACCESS_PLAYBACK);
        user.remote_permission = (ACCESS_STATE_LOG | ACCESS_PLAYBACK);
        break;
    }
    qDebug()<<"add user livePermission:"<<user.local_live_view
             <<"playbackPermission"<<user.local_playback
             <<"remote livePermission:"<<user.remote_live_view
             <<"remote playbackPermission"<<user.remote_playback
             <<"old local permission:"<<user.permission
             <<"old remote permission"<<user.remote_permission;

    add_user(SQLITE_FILE_NAME, &user);
    sendMessageOnly(REQUEST_FLAG_USER_UPDATE, nullptr, 0);

    accept();
}

void AddUserDialog::on_pushButton_cancel_clicked()
{
    reject();
}

void AddUserDialog::on_comboBox_patternEnable_activated(int index)
{
    bool enable = ui->comboBox_patternEnable->itemData(index).toBool();
    ui->pushButton_patternEdit->setEnabled(enable);
}

void AddUserDialog::on_pushButton_patternEdit_clicked()
{
    SetUnlockPattern SetPattern(this);
    int result = SetPattern.exec();
    if (result == BaseShadowDialog::Accepted) {
        m_pattern_text = SetUnlockPattern::instance()->getText();
    }
    qMsDebug() << QString("on_pushButton_patternEdit_clicked  result:[%1] m_pattern_text:[%2]").arg(result).arg(m_pattern_text);
}

void AddUserDialog::on_pushButtonPermissionsEdit_clicked()
{
    EditUserLimitsDialog editUser(this);
    if (ui->comboBox_userLevel->currentIntData() == USERLEVEL_OPERATOR) {
        editUser.setEditUserLimitInit(m_userOperator);
    } else {
        editUser.setEditUserLimitInit(m_userViewer);
    }
    const int &result = editUser.exec();
    if (result == EditUserLimitsDialog::Accepted) {
        if (ui->comboBox_userLevel->currentIntData() == USERLEVEL_OPERATOR) {
            editUser.getEditUserLimitInit(m_userOperator);
        } else {
            editUser.getEditUserLimitInit(m_userViewer);
        }
    }
    ui->pushButtonPermissionsEdit->clearFocus();
}
