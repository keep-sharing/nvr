#include "mslogin.h"
#include "ui_mslogin.h"
#include "LogWrite.h"
#include "centralmessage.h"
#include "LogoutChannel.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "msuser.h"
#include "networkcommond.h"
#include "resetpassword.h"
#include "settingcontent.h"
#include "splashdialog.h"
#include <QCryptographicHash>
#include <QDesktopWidget>
#include <QFile>
#include <QMouseEvent>
#include <QPainter>
#include <QtDebug>

MsLogin *MsLogin::s_msLogin = nullptr;

MsLogin::MsLogin(QWidget *parent)
    : BaseWidget(parent)
    , ui(new Ui::MsLogin)
{
    ui->setupUi(this);

    s_msLogin = this;

    //
    QFile file(":/style/style/wizardstyle.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(file.readAll());
    } else {
        qWarning() << QString("MsLogin load style failed.");
    }
    file.close();

    connect(ui->widget_pattern, SIGNAL(drawStart()), this, SLOT(onDrawStart()));
    connect(ui->widget_pattern, SIGNAL(drawFinished(QString)), this, SLOT(onDrawFinished(QString)));
    //
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();

    //
    ui->label_info->setText("");
    hide();
    //
    ui->lineEdit_password->setCheckMode(MyLineEdit::EmptyCheck);
}

MsLogin::~MsLogin()
{
    s_msLogin = nullptr;
    delete ui;
    qDebug() << "MsLogin::~MsLogin()";
}

MsLogin *MsLogin::instance()
{
    return s_msLogin;
}

void MsLogin::getUsers(int fill)
{
    m_mapUser.clear();
    struct login_left showLeft;
    showLeft.attempt = 4;
    showLeft.lockTime = 0;

    db_user user_array[MAX_USER];
    int userCount = 0;
    read_users(SQLITE_FILE_NAME, user_array, &userCount);
    for (int i = 0; i < userCount; ++i) {
        const db_user &user = user_array[i];

        if (!QString(user.username).isEmpty()) {
            m_mapUser.insert(user.username, user);
            if (fill) {
                ui->comboBox_user->addItem(user.username);
                if (!m_mapFailed.contains(user.username))
                    m_mapFailed.insert(user.username, showLeft);
            }
        }
    }
}

void MsLogin::login_success(const db_user &user)
{
    reboot_conf conf;
    read_autoreboot_conf(SQLITE_FILE_NAME, &conf);
    conf.login = 1;
    strcpy(conf.username, user.username);
    write_autoreboot_conf(SQLITE_FILE_NAME, &conf);
    sendMessage(REQUEST_FLAG_SET_AUTO_REBOOT, NULL, 0);

    gMsUser.setCurrentUserInfo(user);
    emit loginFinished();
    hide();

    if (m_mapFailed.contains(user.username)) {
        struct login_left showLeft = m_mapFailed.value(user.username);
        showLeft.attempt = 4;
        m_mapFailed.insert(user.username, showLeft);
    }
    //
    qMsNvr->writeLog(SUB_OP_LOGIN_LOCAL);
}

void MsLogin::login_failed(QString userName)
{
    ui->lineEdit_password->clear();
    struct login_left showLeft = m_mapFailed.value(userName);
    if (showLeft.attempt > 1) {
        showLeft.attempt--;
        ui->label_info->setText(GET_TEXT("WIZARD/11032", "Login failed! %1 attempts left!").arg(showLeft.attempt));
    } else {
        if (showLeft.attempt == 1) {
            ui->label_info->setText(GET_TEXT("WIZARD/11083", "This user has been locked, please retry after %1 minites.").arg(5));
            showLeft.lockTime = time(0);
        }
        showLeft.attempt = 0;
    }
    m_mapFailed.insert(userName, showLeft);
}

void MsLogin::showLogin()
{
    NetworkCommond::instance()->setPageMode(MOD_LOGIN);
    //
    ui->comboBox_user->clear();
    ui->lineEdit_password->clear();
    ui->widget_pattern->setEnabled(true);
    ui->widget_pattern->setStyle(WhiteStyle);

    getUsers(1);

    int index = ui->comboBox_user->findText(gMsUser.userName());
    if (index < 0)
        index = 0;
    ui->comboBox_user->setCurrentIndex(index);

    const QString &userName = ui->comboBox_user->currentText();
    const db_user &user = m_mapUser.value(userName);
    QString pattern_text = user.pattern_psw;
    bool visible = (pattern_text.size() >= 4);
    setPatternShow(visible);
    ui->toolButton_loginWay->setVisible(visible);

    //
    raise();
    show();
}

void MsLogin::menuLogin(const QString &name, int &second)
{
    struct login_left showLeft = m_mapFailed.value(name);
    if (showLeft.attempt == 0) {
        second = 5 * 60 + showLeft.lockTime - time(0);
        if (second > 0) {
            return;
        }
        showLeft.attempt = 4;
        m_mapFailed.insert(name, showLeft);
    }
}

void MsLogin::menuLoginSuccess(const QString &name)
{
    if (m_mapFailed.contains(name)) {
        struct login_left showLeft = m_mapFailed.value(name);
        showLeft.attempt = 4;
        m_mapFailed.insert(name, showLeft);
    }
}

void MsLogin::menuLoginFailed(const QString &name, int &attempt)
{
    struct login_left showLeft = m_mapFailed.value(name);
    if (showLeft.attempt > 1) {
        showLeft.attempt--;
    } else {
        if (showLeft.attempt == 1) {
            showLeft.lockTime = time(0);
        }
        showLeft.attempt = 0;
    }
    attempt = showLeft.attempt;
    m_mapFailed.insert(name, showLeft);
}

void MsLogin::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    adjustGeometry();
}

void MsLogin::resizeEvent(QResizeEvent *event)
{
    BaseWidget::resizeEvent(event);

    adjustGeometry();
}

void MsLogin::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(rect(), *MsDevice::s_backgroundPixmap);
}

void MsLogin::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        if (LogoutChannel::instance()->isTempLogin()) {
            LogoutChannel::instance()->setTempLogin(false);
            //显示辅屏liveview
            if (g_splashSub) {
                g_splashSub->hide();
                delete g_splashSub;
                g_splashSub = nullptr;
            }
            //显示主屏liveview
            hide();
        }
    }
    BaseWidget::mousePressEvent(event);
}

void MsLogin::returnPressed()
{
    QAbstractButton *button = qobject_cast<QAbstractButton *>(focusWidget());
    qDebug() << "MsLogin::returnPressed," << button;
    if (button) {
        button->click();
    }
}

void MsLogin::onLanguageChanged()
{
    ui->label_mainTitle->setText(GET_TEXT("WIZARD/11033", "User Login"));
    ui->label_subTitle->setText(GET_TEXT("WIZARD/11004", "Network Video Recorder"));
    ui->label_name->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->label_password->setText(GET_TEXT("COMMON/1008", "Password"));
    ui->pushButton_login->setText(GET_TEXT("WIZARD/11037", "Login"));
}

void MsLogin::on_pushButton_login_clicked()
{
    const QString &adminPassword = ui->lineEdit_password->text();
    if (!ui->lineEdit_password->checkValid()) {
        return;
    }
    //每次都重新读取数据库
    getUsers();
    //
    const QString &userName = ui->comboBox_user->currentText();
    struct login_left showLeft = m_mapFailed.value(userName);
    if (showLeft.attempt == 0) {
        time_t Seconds = 5 * 60 + showLeft.lockTime - time(0);
        if (Seconds > 0) {
            ui->lineEdit_password->clear();
            ui->label_info->setText(GET_TEXT("WIZARD/11084", "This user has been locked, please retry after %1 minutes %2 seconds.").arg(Seconds / 60).arg(Seconds % 60));
            return;
        }
        showLeft.attempt = 4;
        m_mapFailed.insert(userName, showLeft);
    }

    const QString &adminPasswordMD5 = QCryptographicHash::hash(adminPassword.toLatin1(), QCryptographicHash::Md5).toHex();
    const db_user &user = m_mapUser.value(userName);
    if (adminPasswordMD5 == QString(user.password)) {
        login_success(user);
    } else {
        login_failed(userName);
    }
}

void MsLogin::on_comboBox_user_activated(int index)
{
    Q_UNUSED(index)

    ui->lineEdit_password->clear();
    ui->label_info->setText("");
    const QString &userName = ui->comboBox_user->currentText();
    const db_user &user = m_mapUser.value(userName);
    QString pattern_text = user.pattern_psw;

    if (pattern_text.size() < 4) {
        setPatternShow(false);
        ui->toolButton_loginWay->hide();
    } else {
        ui->toolButton_loginWay->show();
    }
}

void MsLogin::on_lineEdit_password_textChanged(const QString &text)
{
    Q_UNUSED(text)

    ui->label_info->setText("");
}

void MsLogin::onDrawStart()
{
    ui->label_info->setText(GET_TEXT("USER/74060", "Release mouse when finished."));
    ui->label_info->show();
}

void MsLogin::onDrawFinished(QString text)
{
    getUsers();
    const QString &userName = ui->comboBox_user->currentText();
    struct login_left showLeft = m_mapFailed.value(userName);
    if (showLeft.attempt == 0) {
        time_t Seconds = 5 * 60 + showLeft.lockTime - time(0);
        if (Seconds > 0) {
            ui->label_info->setText(GET_TEXT("WIZARD/11084", "This user has been locked, please retry after %1 minutes %2 seconds.").arg(Seconds / 60).arg(Seconds % 60));
            ui->label_info->show();
            return;
        }
        showLeft.attempt = 4;
        m_mapFailed.insert(userName, showLeft);
    }

    const db_user &user = m_mapUser.value(userName);
    QString pattern_text = user.pattern_psw;

    if (pattern_text == text) {
        login_success(user);
    } else {
        login_failed(userName);
    }
}

void MsLogin::on_toolButton_forgot_clicked()
{
    ResetPassword reset(SettingContent::instance());

    if (ui->widget_pattern->isVisible()) {
        const QString &userName = ui->comboBox_user->currentText();
        reset.initializeData(userName);
    } else {
        if (!reset.initializeData()) {
            ShowMessageBox(GET_TEXT("WIZARD/11071", "Security Question is not set，cannot reset the password."));
            return;
        }
    }
    reset.exec();
}

void MsLogin::setPatternShow(bool visible)
{
    ui->widgetPatternContent->setVisible(visible);
    ui->label_password->setVisible(!visible);
    ui->lineEdit_password->setVisible(!visible);
    ui->pushButton_login->setVisible(!visible);
    ui->widgetSpacer->setVisible(!visible);
    ui->label_info->setText("");

    if (visible) {
        ui->toolButton_forgot->setText(GET_TEXT("USER/74064", "Forgot unlock pattern?"));
        ui->toolButton_loginWay->setText(GET_TEXT("USER/74067", "Login with password?"));
    } else {
        ui->toolButton_forgot->setText(GET_TEXT("USER/74065", "Forgot password?"));
        ui->toolButton_loginWay->setText(GET_TEXT("USER/74066", "Login with unlock pattern?"));
        ui->lineEdit_password->clear();
    }

    adjustGeometry();
}

void MsLogin::on_toolButton_loginWay_clicked()
{
    bool visible = ui->widget_pattern->isVisible();
    setPatternShow(!visible);
}

void MsLogin::adjustGeometry()
{
    QRect rc;
    if (ui->widgetPatternContent->isVisible()) {
        rc.setWidth(width() / 4);
        rc.setHeight(height() / 2);
    } else {
        rc.setWidth(width() / 4);
        rc.setHeight(height() / 3);
    }
    rc.moveCenter(rect().center());
    ui->wizard_content->setGeometry(rc);
}
