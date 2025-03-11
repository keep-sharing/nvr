#include "PageUserSettingOperator.h"
#include "ui_PageUserSettingOperator.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "mainwindow.h"
#include "msuser.h"
#include "setunlockpattern.h"
#include <QCryptographicHash>
#include <QtDebug>
extern "C" {
#include "log.h"
}
PageUserSettingOperator::PageUserSettingOperator(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::UserSettingOperator)
{
    ui->setupUi(this);

    ui->widget_limit->setReadOnly(true);

    ui->lineEdit_oldPassword->setCheckMode(MyLineEdit::EmptyCheck);
    ui->lineEdit_newPassword->setCheckMode(MyLineEdit::PasswordCheck);

    onLanguageChanged();
}

PageUserSettingOperator::~PageUserSettingOperator()
{
    delete ui;
}

void PageUserSettingOperator::initializeData()
{
    gMsUser.updateData();
    m_user = gMsUser.currentUserInfo();

    ui->lineEdit_userName->setText(QString(m_user.username));

    ui->widget_limit->initializeUserLimit(m_user);

    ui->comboBoxChangePw->clear();
    ui->comboBoxChangePw->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBoxChangePw->addItem(GET_TEXT("COMMON/1009", "Eanble"), 1);
    on_comboBoxChangePw_activated(0);

    ui->comboBoxUnlockPattern->clear();
    ui->comboBoxUnlockPattern->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBoxUnlockPattern->addItem(GET_TEXT("COMMON/1009", "Eanble"), 1);

    m_pattern_text = m_user.pattern_psw;
    int index = m_pattern_text.size() >= 4 ? 1 : 0;
    ui->comboBoxUnlockPattern->setCurrentIndex(index);
    on_comboBoxUnlockPattern_activated(index);
}

void PageUserSettingOperator::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void PageUserSettingOperator::onLanguageChanged()
{
    ui->label_userName->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->label_oldPassword->setText(GET_TEXT("COMMON/1008", "Password"));
    ui->label_newPassword->setText(GET_TEXT("WIZARD/11009", "New Password"));
    ui->label_confirmPassword->setText(GET_TEXT("USER/74055", "Confirm Password"));
    ui->labelChangePw->setText(GET_TEXT("USER/74075", "Change Password"));
    ui->labelUnlockPattern->setText(GET_TEXT("WIZARD/11080", "Unlock Pattern"));
    ui->labelSetUnlock->setText(GET_TEXT("USER/74062", "Set Unlock Pattern"));
    ui->pushButtonEdit->setText(GET_TEXT("COMMON/1019", "Edit"));

    ui->widget_limit->onLanguageChanged();

    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void PageUserSettingOperator::on_pushButton_apply_clicked()
{
    const QString &oldPassword = ui->lineEdit_oldPassword->text();
    const QString &oldPasswordMD5 = QCryptographicHash::hash(oldPassword.toLatin1(), QCryptographicHash::Md5).toHex();
    const QString &newPassword = ui->lineEdit_newPassword->text();
    const QString &confirmPassword = ui->lineEdit_confirmPassword->text();

    bool valid = ui->lineEdit_oldPassword->checkValid();
    if (valid && oldPasswordMD5 != QString(m_user.password)) {
        ui->lineEdit_oldPassword->setCustomValid(false, GET_TEXT("MYLINETIP/112010", "Incorrect Password."));
        valid = false;
    }
    if (!valid) {
        return;
    }
    if (ui->comboBoxChangePw->currentIntData()) {
        valid &= ui->lineEdit_newPassword->checkValid();
        if (!newPassword.isEmpty() && newPassword != confirmPassword) {
            ui->lineEdit_confirmPassword->setCustomValid(false, GET_TEXT("MYLINETIP/112008", "Passwords do not match. "));
            valid = false;
        }
        if (!valid) {
            return;
        }

        const QString &newPasswordMD5 = QCryptographicHash::hash(newPassword.toLatin1(), QCryptographicHash::Md5).toHex();
        snprintf(m_user.password, sizeof(m_user.password), "%s", newPasswordMD5.toStdString().c_str());
        snprintf(m_user.password_ex, sizeof(m_user.password_ex), "%s", newPassword.toStdString().c_str());
    }

    //unlock pattern
    if (ui->comboBoxUnlockPattern->currentIndex()) {
        if (m_pattern_text.size() < 4) {
            m_pattern_text = "0";
            int result = MessageBox::question(this, GET_TEXT("USER/74061", "Unlock pattern is not set, it will be disabled if you exit, continue?"));
            if (result == MessageBox::Cancel) {
                return;
            }
            ui->comboBoxUnlockPattern->setCurrentIndex(1);
            on_comboBoxUnlockPattern_activated(1);
        }
    } else {
        m_pattern_text = "0";
    }
    snprintf(m_user.pattern_psw, sizeof(m_user.pattern_psw), "%s", m_pattern_text.toStdString().c_str());
    write_user(SQLITE_FILE_NAME, &m_user);

    ui->lineEdit_oldPassword->clear();
    ui->lineEdit_newPassword->clear();
    ui->lineEdit_confirmPassword->clear();
    //
    sendMessageOnly(REQUEST_FLAG_USER_UPDATE, nullptr, 0);

    ExecMessageBox(GET_TEXT("CAMERAEDIT/31010", "Save Successfully."));
    bool hasChangePsw = ui->comboBoxUnlockPattern->currentIntData() && m_changePsw;
    if (ui->comboBoxChangePw->currentIntData() || hasChangePsw) {
        qMsNvr->closeTalkback();
        qMsNvr->writeLog(SUB_OP_LOGOUT_LOCAL);
        emit sig_back();
        QTimer::singleShot(100, MainWindow::instance(), SLOT(logout()));
    }
}

void PageUserSettingOperator::on_pushButton_back_clicked()
{
    emit sig_back();
}

void PageUserSettingOperator::on_pushButtonEdit_clicked()
{
    ui->pushButtonEdit->setAttribute(Qt::WA_UnderMouse, false);
    SetUnlockPattern SetPattern(this);
    int result = SetPattern.exec();
    if (result == BaseShadowDialog::Accepted) {
        m_changePsw = true;
        m_pattern_text = SetUnlockPattern::instance()->getText();
    }
    qDebug() << QString("on_pushButton_patternEdit_clicked  result:[%1] m_pattern_text:[%2]").arg(result).arg(m_pattern_text);
}

void PageUserSettingOperator::on_comboBoxChangePw_activated(int index)
{
    bool enable = ui->comboBoxChangePw->itemData(index).toBool();
    ui->lineEdit_newPassword->setEnabled(enable);
    ui->lineEdit_confirmPassword->setEnabled(enable);
}

void PageUserSettingOperator::on_comboBoxUnlockPattern_activated(int index)
{
    bool enable = ui->comboBoxUnlockPattern->itemData(index).toBool();
    ui->pushButtonEdit->setEnabled(enable);
}
