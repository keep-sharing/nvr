#include "userlogin.h"
#include "ui_userlogin.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "mslogin.h"
#include "msuser.h"
#include "networkcommond.h"
#include "resetpassword.h"
#include "settingcontent.h"
#include <QCryptographicHash>
#include "MyDebug.h"

UserLogin::UserLogin(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::UserLogin)
{
    ui->setupUi(this);

    //
    const db_user &user = gMsUser.currentUserInfo();
    m_mapUser.insert(user.username, user);
    ui->comboBox_user->addItem(user.username);
    //ui->comboBox_user->setEnabled(false);
    ui->label_info->hide();
    //
    ui->lineEdit_password->setCheckMode(MyLineEdit::EmptyCheck);

    connect(ui->gustureLock, SIGNAL(drawStart()), this, SLOT(onDrawStart()));
    connect(ui->gustureLock, SIGNAL(drawFinished(QString)), this, SLOT(onDrawFinished(QString)));

    NetworkCommond::instance()->setPageMode(MOD_SHUTDOWN);

    ui->comboBox_user->installEventFilter(this);

    onLanguageChanged();
}

UserLogin::~UserLogin()
{
    NetworkCommond::instance()->setPageMode(MOD_LIVEVIEW);
    delete ui;
}

int UserLogin::execLogin()
{
    bool isGustureAvailable = gMsUser.isGustureAvailable();
    setGustureLockVisible(isGustureAvailable);
    ui->toolButtonLoginWay->setVisible(isGustureAvailable);
    return BaseShadowDialog::exec();
}

bool UserLogin::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == ui->comboBox_user) {
        switch (e->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
        case QEvent::Wheel:
        case QEvent::HoverEnter:
        case QEvent::HoverLeave:
        case QEvent::HoverMove:
        case QEvent::FocusIn:
        case QEvent::FocusOut:
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::Enter:
        case QEvent::Leave:
            return true;
        default:
            break;
        }
    }
    return BaseShadowDialog::eventFilter(obj, e);
}

void UserLogin::setGustureLockVisible(bool visible)
{
    QRect rc;
    if (visible) {
        ui->label_password->hide();
        ui->lineEdit_password->hide();
        ui->pushButton_login->hide();
        ui->widgetGustureLock->show();
        ui->toolButtonLoginWay->setText(GET_TEXT("USER/74067", "Login with password?"));
        rc.setSize(QSize(587, 600));
    } else {
        ui->label_password->show();
        ui->lineEdit_password->show();
        ui->pushButton_login->show();
        ui->widgetGustureLock->hide();
        ui->toolButtonLoginWay->setText(GET_TEXT("USER/74066", "Login with unlock pattern?"));
        rc.setSize(QSize(587, 400));
    }
    rc.moveCenter(geometry().center());
    setGeometry(rc);
}

void UserLogin::login(LOGIN_MODE mode, const QString &name, const QString &password)
{
    int seconds = 0;
    MsLogin::instance()->menuLogin(name, seconds);
    if (seconds > 0) {
        ui->label_info->setText(GET_TEXT("WIZARD/11084", "This user has been locked, please retry after %1 minutes %2 seconds.").arg(seconds / 60).arg(seconds % 60));
        ui->label_info->show();
        return;
    }

    const db_user &user = m_mapUser.value(name);
    bool success = false;
    switch (mode) {
    case LOGIN_NORMAL: {
        const QString &adminPasswordMD5 = QCryptographicHash::hash(password.toLatin1(), QCryptographicHash::Md5).toHex();
        if (adminPasswordMD5 == QString(user.password)) {
            success = true;
        }
        break;
    }
    case LOGIN_GUSTURE: {
        if (password == QString(user.pattern_psw)) {
            success = true;
        }
        break;
    }
    default:
        break;
    }

    if (success) {
        MsLogin::instance()->menuLoginSuccess(name);

        reboot_conf conf;
        read_autoreboot_conf(SQLITE_FILE_NAME, &conf);
        conf.login = 1;
        strcpy(conf.username, user.username);
        write_autoreboot_conf(SQLITE_FILE_NAME, &conf);
        sendMessage(REQUEST_FLAG_SET_AUTO_REBOOT, NULL, 0);

        accept();
    } else {
        int attempt = 0;
        MsLogin::instance()->menuLoginFailed(name, attempt);
        if (attempt == 0) {
            ui->label_info->setText(GET_TEXT("WIZARD/11083", "This user has been locked, please retry after %1 minites.").arg(5));
        } else {
            ui->label_info->setText(GET_TEXT("WIZARD/11032", "Login failed! %1 attempts left!").arg(attempt));
        }
        ui->label_info->show();
    }
}

void UserLogin::onLanguageChanged()
{
    ui->label_mainTitle->setText(GET_TEXT("WIZARD/11033", "User Login"));
    ui->label_subTitle->setText(GET_TEXT("WIZARD/11004", "Network Video Recorder"));
    ui->label_name->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->label_password->setText(GET_TEXT("COMMON/1008", "Password"));
    ui->toolButtonForgetPassword->setText(GET_TEXT("WIZARD/11068", "Forget Password?"));
    ui->pushButton_login->setText(GET_TEXT("WIZARD/11037", "Login"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void UserLogin::on_comboBox_user_activated(int index)
{
    Q_UNUSED(index)

    ui->label_info->hide();
}

void UserLogin::on_lineEdit_password_textChanged(const QString &text)
{
    Q_UNUSED(text)

    ui->label_info->hide();
}

void UserLogin::on_pushButton_login_clicked()
{
    const QString &userName = ui->comboBox_user->currentText();
    const QString &adminPassword = ui->lineEdit_password->text();
    if (!ui->lineEdit_password->checkValid()) {
        return;
    }

    login(LOGIN_NORMAL, userName, adminPassword);
}

void UserLogin::on_pushButton_cancel_clicked()
{
    reject();
}

void UserLogin::on_toolButtonForgetPassword_clicked()
{
    ResetPassword reset(SettingContent::instance());
    if (!reset.initializeData()) {
        ShowMessageBox(GET_TEXT("WIZARD/11071", "Security Question is not set，cannot reset the password."));
        return;
    }
    reset.exec();
}

void UserLogin::on_toolButtonLoginWay_clicked()
{
    bool visible = ui->widgetGustureLock->isVisible();
    setGustureLockVisible(!visible);
}

void UserLogin::onDrawStart()
{
    ui->label_info->setText(GET_TEXT("USER/74060", "Release mouse when finished."));
    ui->label_info->show();
}

void UserLogin::onDrawFinished(QString text)
{
    login(LOGIN_GUSTURE, ui->comboBox_user->currentText(), text);
}
