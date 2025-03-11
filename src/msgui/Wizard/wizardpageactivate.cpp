#include "wizardpageactivate.h"
#include "ui_wizardpageactivate.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsApplication.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include <QCryptographicHash>
#include <QtDebug>

WizardPageActivate::WizardPageActivate(QWidget *parent)
    : AbstractWizardPage(parent)
    , ui(new Ui::WizardPageActivate)
{
    ui->setupUi(this);

    MsLanguage::instance()->initializeComboBox(ui->comboBox_language);
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    ui->lineEdit_password->setCheckMode(MyLineEdit::PasswordCheck);

    onLanguageChanged();
}

WizardPageActivate::~WizardPageActivate()
{
    delete ui;
}

void WizardPageActivate::initializeData()
{
    ui->lineEdit_userName->setText("admin");
}

void WizardPageActivate::saveSetting()
{
    memset(&m_adminUser, 0, sizeof(db_user));
    read_user(SQLITE_FILE_NAME, &m_adminUser, 0);
    //
    const QString &adminPassword = ui->lineEdit_password->text();
    const QString &adminPasswordMD5 = QCryptographicHash::hash(adminPassword.toLatin1(), QCryptographicHash::Md5).toHex();
    const QString &confirmPassword = ui->lineEdit_confirm->text();
    //已经在web激活
    if (!QString(m_adminUser.password_ex).isEmpty()) {
        if (adminPassword.isEmpty()) {

        } else {
            ShowMessageBox(GET_TEXT("WIZARD/11078", "Set password failed, this NVR has been activated."));
        }
        setWizardMode(ModeNormal);
        showWizardPage(Wizard_User);
        return;
    }
    //
    if (ui->lineEdit_password->checkTooLong()) {
        ui->lineEdit_confirm->setTipFronSize(11);
    } else {
        ui->lineEdit_confirm->setTipFronSize(14);
    }
    bool valid = ui->lineEdit_password->checkValid();
    if (!ui->lineEdit_password->text().isEmpty() && adminPassword != confirmPassword) {
        ui->lineEdit_confirm->setCustomValid(false, GET_TEXT("MYLINETIP/112008", "Passwords do not match."));
        valid = false;
    }
    if (!valid ) {
        return;
    }
    //
    snprintf(m_adminUser.password_ex, sizeof(m_adminUser.password_ex), "%s", adminPassword.toStdString().c_str());
    snprintf(m_adminUser.password, sizeof(m_adminUser.password), "%s", adminPasswordMD5.toStdString().c_str());
    snprintf(m_adminUser.username, sizeof(m_adminUser.password), "%s", "admin");
    write_user(SQLITE_FILE_NAME, &m_adminUser);
    //
    if (qMsNvr->isPoe()) {
        ShowMessageBox(GET_TEXT("WIZARD/11079", "NVR activated! The NVR password will also be used to activate the PoE channels."));
        set_param_value(SQLITE_FILE_NAME, PARAM_POE_PSD, confirmPassword.toUtf8().data());
        sendMessage(REQUEST_FLAG_MODIFY_POE_CAMERA_PWD, nullptr, 0);
    }
    sendMessageOnly(REQUEST_FLAG_USER_UPDATE, nullptr, 0);
    showWizardPage(wizard_Pattern);
}

void WizardPageActivate::previousPage()
{
}

void WizardPageActivate::nextPage()
{
    saveSetting();
}

void WizardPageActivate::skipWizard()
{
}

void WizardPageActivate::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void WizardPageActivate::onLanguageChanged()
{
    ui->label_userName->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->label_password->setText(GET_TEXT("COMMON/1008", "Password"));
    ui->label_confirm->setText(GET_TEXT("USER/74055", "Confirm Password"));
    ui->label_language->setText(GET_TEXT("WIZARD/11011", "Language"));
}

void WizardPageActivate::on_comboBox_language_activated(int index)
{
    static QMap<int, int> tempLanguageMap;

    const int &currentLanguage = MsLanguage::instance()->currentLanguage();
    switch (currentLanguage) {
    case 16:
    case 17:
    case 19:
        tempLanguageMap.insert(currentLanguage, 0);
        break;
    }

    const int &id = ui->comboBox_language->itemData(index).toInt();
    switch (id) {
    case 16:
    case 17:
    case 19:
        tempLanguageMap.insert(id, 0);
        break;
    }
    MsLanguage::instance()->changeLanguage(id);

    if (tempLanguageMap.contains(id) && tempLanguageMap.size() > 1) {
        if (MessageBox::Yes == MessageBox::englishQuestion(this, "Language changes will take effect after rebooting. Are you sure to reboot now?")) {
            //qMsNvr->reboot();
            //qMsApp->setAboutToReboot(true);
            //m_waitting->//showWait();
        }
    }
}

