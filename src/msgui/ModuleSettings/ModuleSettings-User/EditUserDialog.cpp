#include "EditUserDialog.h"
#include "ui_EditUserDialog.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "centralmessage.h"
#include "edituserlimitsdialog.h"
#include "msuser.h"
#include "myqt.h"
#include "setunlockpattern.h"
#include <QCryptographicHash>
extern "C" {
#include "log.h"
}
EditUserDialog::EditUserDialog(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::EditUserPassword)
{
    ui->setupUi(this);
    ui->lineEdit_userName->setCheckMode(MyLineEdit::UserNameCheck);
    ui->lineEdit_newPassword->setCheckMode(MyLineEdit::PasswordCheck);

    memset(&m_userOperator, 0, sizeof(db_user));
    memset(&m_userViewer, 0, sizeof(db_user));

    onLanguageChanged();
}

EditUserDialog::~EditUserDialog()
{
    delete ui;
}

int EditUserDialog::execEdit(const db_user &user)
{
    memcpy(&m_userInfo, &user, sizeof(db_user));
    memcpy(&m_userOperator, &user, sizeof(db_user));
    memcpy(&m_userViewer, &user, sizeof(db_user));
    m_userOperator.type = USERLEVEL_OPERATOR;
    m_userViewer.type = USERLEVEL_USER;
    if (user.type == USERLEVEL_OPERATOR) {
        get_user_default_permission(&m_userViewer);
    } else if (user.type == USERLEVEL_USER) {
        get_user_default_permission(&m_userOperator);
    }

    ui->lineEdit_userName->setText(QString(user.username));

    ui->comboBox_changePassword->clear();
    ui->comboBox_changePassword->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_changePassword->addItem(GET_TEXT("COMMON/1009", "Eanble"), 1);
    on_comboBox_changePassword_activated(0);

    ui->comboBox_patternEnable->clear();
    ui->comboBox_patternEnable->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_patternEnable->addItem(GET_TEXT("COMMON/1009", "Eanble"), 1);

    m_pattern_text = user.pattern_psw;
    int index = m_pattern_text.size() >= 4 ? 1 : 0;
    ui->comboBox_patternEnable->setCurrentIndex(index);
    on_comboBox_patternEnable_activated(index);

    if (gMsUser.isAdmin()) {
        if (user.type == USERLEVEL_ADMIN) {
            ui->lineEdit_userName->setReadOnly(true);
            ui->comboBox_userLevel->clear();
            ui->comboBox_userLevel->addItem(GET_TEXT("USER/74005", "Admin"), USERLEVEL_ADMIN);
            ui->comboBox_userLevel->setEnabled(false);
            ui->pushButtonPermissionsEdit->setEnabled(false);
        } else {
            ui->lineEdit_userName->setReadOnly(false);
            ui->comboBox_userLevel->clear();
            ui->comboBox_userLevel->addItem(GET_TEXT("USER/74006", "Operator"), USERLEVEL_OPERATOR);
            ui->comboBox_userLevel->addItem(GET_TEXT("USER/74007", "Viewer"), USERLEVEL_USER);
            ui->comboBox_userLevel->setEnabled(true);
        }
    }
    ui->comboBox_userLevel->setCurrentIndexFromData(user.type);

    return exec();
}

void EditUserDialog::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("USER/74017", "User Edit"));

    ui->label_adminPassword->setText(GET_TEXT("USER/74021", "Admin Password"));
    ui->label_userName->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->label_userLevel->setText(GET_TEXT("USER/74001", "User Level"));
    ui->label_changePassword->setText(GET_TEXT("USER/74075", "Change Password"));
    ui->label_newPassword->setText(GET_TEXT("WIZARD/11009", "New Password"));
    ui->label_confirm->setText(GET_TEXT("USER/74055", "Confirm Password"));
    ui->label_setUnlockPattern->setText(GET_TEXT("USER/74062", "Set Unlock Pattern"));
    ui->label_unlockPattern->setText(GET_TEXT("WIZARD/11080", "Unlock Pattern"));
    ui->pushButton_patternEdit->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->labelUserPermissions->setText(GET_TEXT("USER/142001", "User Permissions"));
    ui->pushButtonPermissionsEdit->setText(GET_TEXT("COMMON/1019", "Edit"));

    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void EditUserDialog::on_pushButton_ok_clicked()
{
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
    QString strNewPassword = ui->lineEdit_newPassword->text();
    QString strConfirm = ui->lineEdit_confirm->text();

    db_user user;
    if (ui->comboBox_userLevel->currentIntData() == USERLEVEL_ADMIN) {
        memcpy(&user, &m_userInfo, sizeof(db_user));
    } else if (ui->comboBox_userLevel->currentIntData() == USERLEVEL_OPERATOR) {
        memcpy(&user, &m_userOperator, sizeof(db_user));
    } else if (ui->comboBox_userLevel->currentIntData() == USERLEVEL_USER) {
        memcpy(&user, &m_userViewer, sizeof(db_user));
    }
    snprintf(user.username, sizeof(user.username), "%s", strUserName.toStdString().c_str());
    bool valid = ui->lineEdit_userName->checkValid();
    if (valid) {
        db_user userList[MAX_USER];
        int count = 0;
        read_users(SQLITE_FILE_NAME, userList, &count);
        for (int i = 0; i < count; ++i) {
            const db_user &tempUser = userList[i];
            QString userName = QString(tempUser.username);
            if (!userName.isEmpty() && userName == strUserName && tempUser.id != user.id) {
                ui->lineEdit_userName->setCustomValid(false, GET_TEXT("MYLINETIP/112011", "Already existed."));
                valid = false;
                break;
            }
        }
    }

    if (valid && ui->lineEdit_newPassword->isEnabled()) {
        valid &= ui->lineEdit_newPassword->checkValid();
        if (!ui->lineEdit_newPassword->text().isEmpty() && strNewPassword != strConfirm) {
            ui->lineEdit_confirm->setCustomValid(false, GET_TEXT("USER/74014", "Passwords don't match."));
            valid = false;
        }

        const QString &strNewPasswordMD5 = QCryptographicHash::hash(strNewPassword.toLatin1(), QCryptographicHash::Md5).toHex();

        snprintf(user.password, sizeof(user.password), "%s", strNewPasswordMD5.toStdString().c_str());
        snprintf(user.password_ex, sizeof(user.password_ex), "%s", strNewPassword.toStdString().c_str());
    }
    if (!valid) {
        return;
    }
    user.type = ui->comboBox_userLevel->currentData().toInt();

    //unlock pattern
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
    snprintf(user.pattern_psw, sizeof(user.pattern_psw), "%s", m_pattern_text.toStdString().c_str());
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

    write_user(SQLITE_FILE_NAME, &user);
    //
    if (user.type == USERLEVEL_ADMIN && ui->lineEdit_newPassword->isEnabled()) {
        if (qMsNvr->isPoe()) {
            const int result = MessageBox::question(this, GET_TEXT("WIZARD/11076", "Sync new admin password to current connected PoE channels?"));
            if (result == MessageBox::Yes) {
                set_param_value(SQLITE_FILE_NAME, PARAM_POE_PSD, strNewPassword.toUtf8().data());
                sendMessage(REQUEST_FLAG_MODIFY_POE_CAMERA_PWD, nullptr, 0);
            }
        }
    }
    //
    sendMessageOnly(REQUEST_FLAG_USER_UPDATE, nullptr, 0);
    //
    bool hasChangePsw = ui->comboBox_patternEnable->currentIntData() && m_changePsw;
    if (ui->comboBox_userLevel->currentIntData() == USERLEVEL_ADMIN && (ui->comboBox_changePassword->currentIntData() || hasChangePsw)) {
        done(TypeLogout);
        qMsNvr->closeTalkback();
        qMsNvr->writeLog(SUB_OP_LOGOUT_LOCAL);
    }
    accept();
}

void EditUserDialog::on_pushButton_cancel_clicked()
{
    reject();
}

void EditUserDialog::on_comboBox_changePassword_activated(int index)
{
    bool enable = ui->comboBox_changePassword->itemData(index).toBool();
    ui->lineEdit_newPassword->setEnabled(enable);
    ui->lineEdit_confirm->setEnabled(enable);
}

void EditUserDialog::on_comboBox_patternEnable_activated(int index)
{
    bool enable = ui->comboBox_patternEnable->itemData(index).toBool();
    ui->pushButton_patternEdit->setEnabled(enable);
}

void EditUserDialog::on_pushButton_patternEdit_clicked()
{
    SetUnlockPattern SetPattern(this);
    int result = SetPattern.exec();
    if (result == BaseShadowDialog::Accepted) {
        m_changePsw = true;
        m_pattern_text = SetUnlockPattern::instance()->getText();
    }
    qDebug() << QString("on_pushButton_patternEdit_clicked  result:[%1] m_pattern_text:[%2]").arg(result).arg(m_pattern_text);
}

void EditUserDialog::on_pushButtonPermissionsEdit_clicked()
{
    EditUserLimitsDialog editUser(this);
    if (ui->comboBox_userLevel->currentIntData() == USERLEVEL_OPERATOR) {
        editUser.setEditUserLimitInit(m_userOperator);
    } else if (ui->comboBox_userLevel->currentIntData() == USERLEVEL_USER) {
        editUser.setEditUserLimitInit(m_userViewer);
    }
    const int &result = editUser.exec();
    if (result == EditUserLimitsDialog::Accepted) {
        if (ui->comboBox_userLevel->currentIntData() == USERLEVEL_OPERATOR) {
            editUser.getEditUserLimitInit(m_userOperator);
        } else if (ui->comboBox_userLevel->currentIntData() == USERLEVEL_USER) {
            editUser.getEditUserLimitInit(m_userViewer);
        }
    }
    ui->pushButtonPermissionsEdit->clearFocus();
}
