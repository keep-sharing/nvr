#include "LogoutChannel.h"
#include "ui_LogoutChannel.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "mainwindow.h"
#include <QFile>
#include <QReadWriteLock>

QReadWriteLock logoutMutex(QReadWriteLock::Recursive);
LogoutChannel *LogoutChannel::self = nullptr;

LogoutChannel::LogoutChannel(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::LogoutChannel)
{
    ui->setupUi(this);

    QFile file(":/style/style/settingstyle.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(file.readAll());
    }
    file.close();

    ui->checkBoxGroupChannel->setCount(qMsNvr->maxChannel());

    ui->labelDisplayMode->setTranslatableText("LIVEVIEW/20085", "Display Mode");

    ui->comboBoxDisplayMode->beginEdit();
    ui->comboBoxDisplayMode->clear();
    ui->comboBoxDisplayMode->addTranslatableItem("ANPR/103001", "Regular Mode", RegularMode);
    if (qMsNvr->isSupportTargetMode() && !qMsNvr->isSlaveMode()) {
        ui->comboBoxDisplayMode->addTranslatableItem("TARGETMODE/103206", "Target Mode", TargetMode);
    }
    if (!qMsNvr->isSlaveMode()) {
        ui->comboBoxDisplayMode->addTranslatableItem("OCCUPANCY/74255", "Occupancy Mode", OccupancyMode);
    }

    ui->comboBoxDisplayMode->endEdit();

    ui->labelDisplayChannel->setTranslatableText("SYSTEMGENERAL/70069", "Display Channel");

    ui->comboBoxDisplayGroup->beginEdit();
    ui->comboBoxDisplayGroup->clear();
    for (int i = 0; i < MAX_PEOPLECNT_GROUP; ++i) {
        ui->comboBoxDisplayGroup->addItem(QString("%1").arg(i + 1), i);
    }
    ui->comboBoxDisplayGroup->endEdit();

    initializeData();

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

LogoutChannel::~LogoutChannel()
{
    self = nullptr;
    delete ui;
}

LogoutChannel *LogoutChannel::instance()
{
    if (!self) {
        QWriteLocker locker(&logoutMutex);
        if (!self) {
            MainWindow *mainWindow = MainWindow::instance();
            if (mainWindow) {
                self = new LogoutChannel(mainWindow);
            }
        }
    }
    return self;
}

void LogoutChannel::initializeData()
{
    QWriteLocker locker(&logoutMutex);
    readLogoutMode();
    readLogoutChannel();
    readLogoutGroup();
}

int LogoutChannel::logoutMode()
{
    QReadLocker locker(&logoutMutex);
    return m_mode;
}

int LogoutChannel::logoutGroup()
{
    QReadLocker locker(&logoutMutex);
    return m_logoutGroup;
}

void LogoutChannel::logout()
{
    QWriteLocker locker(&logoutMutex);
    m_isLogout = true;
}

void LogoutChannel::clearLogout()
{
    QWriteLocker locker(&logoutMutex);
    m_isLogout = false;
}

bool LogoutChannel::isLogout()
{
    QReadLocker locker(&logoutMutex);
    return m_isLogout;
}

bool LogoutChannel::isLogoutChannel(int channel)
{
    QReadLocker locker(&logoutMutex);
    bool result = false;
    if (m_logoutChannel.size() > channel) {
        result = m_logoutChannel.at(channel) == QChar('1');
    }
    return result;
}

void LogoutChannel::setTempLogin(bool login)
{
    QWriteLocker locker(&logoutMutex);
    m_isTempLogin = login;
}

bool LogoutChannel::isTempLogin()
{
    QReadLocker locker(&logoutMutex);
    return m_isTempLogin;
}

void LogoutChannel::showLogoutChannel()
{
    ui->comboBoxDisplayMode->setCurrentIndexFromData(m_mode);
    ui->comboBoxDisplayGroup->setCurrentIndexFromData(m_logoutGroup);
    ui->checkBoxGroupChannel->setCheckedFromString(m_logoutChannel);
    show();
}

void LogoutChannel::readLogoutMode()
{
    QWriteLocker locker(&logoutMutex);
    m_mode = get_param_int(SQLITE_FILE_NAME, "logout_mode", 0);
}

void LogoutChannel::writeLogoutMode()
{
    QReadLocker locker(&logoutMutex);
    set_param_int(SQLITE_FILE_NAME, "logout_mode", m_mode);
}

void LogoutChannel::readLogoutChannel()
{
    QWriteLocker locker(&logoutMutex);
    char value[65] = { 0 };
    get_param_value(SQLITE_FILE_NAME, "logout_channel", value, sizeof(value), "");
    m_logoutChannel = QString(value);
}

void LogoutChannel::writeLogoutChannel()
{
    QReadLocker locker(&logoutMutex);
    set_param_value(SQLITE_FILE_NAME, "logout_channel", m_logoutChannel.toStdString().c_str());
}

void LogoutChannel::readLogoutGroup()
{
    QWriteLocker locker(&logoutMutex);
    m_logoutGroup = get_param_int(SQLITE_FILE_NAME, "logout_group", 0);
}

void LogoutChannel::writeLogoutGroup()
{
    QReadLocker locker(&logoutMutex);
    set_param_int(SQLITE_FILE_NAME, "logout_group", m_logoutGroup);
}

void LogoutChannel::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("SYSTEMGENERAL/70068", "Display When Logout"));
    ui->labelDisplayMode->retranslate();
    ui->comboBoxDisplayMode->retranslate();
    ui->labelDisplayChannel->retranslate();
    ui->labelDisplayGroup->setText(GET_TEXT("SYSTEMGENERAL/70070", "Display Group No."));
    ui->pushButtonOk->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButtonCancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void LogoutChannel::on_comboBoxDisplayMode_indexSet(int index)
{
    int mode = ui->comboBoxDisplayMode->itemData(index).toInt();
    switch (mode) {
    case RegularMode:
    case TargetMode:
        ui->labelDisplayGroup->hide();
        ui->comboBoxDisplayGroup->hide();
        ui->labelDisplayChannel->show();
        ui->checkBoxGroupChannel->show();
        break;
    case OccupancyMode:
        ui->labelDisplayGroup->show();
        ui->comboBoxDisplayGroup->show();
        ui->labelDisplayChannel->hide();
        ui->checkBoxGroupChannel->hide();
        break;
    }
}

void LogoutChannel::on_pushButtonOk_clicked()
{
    m_mode = ui->comboBoxDisplayMode->currentData().toInt();
    writeLogoutMode();
    m_logoutGroup = ui->comboBoxDisplayGroup->currentData().toInt();
    writeLogoutGroup();
    m_logoutChannel = ui->checkBoxGroupChannel->checkedMask();
    writeLogoutChannel();
    //
    accept();
}

void LogoutChannel::on_pushButtonCancel_clicked()
{
    reject();
}
