#include "wizardpageuser.h"
#include "ui_wizardpageuser.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsApplication.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "myqt.h"
#include <QCryptographicHash>
#include <QFontDatabase>

extern "C" {
#include "msdb.h"
}

WizardPageUser::WizardPageUser(QWidget *parent)
    : AbstractWizardPage(parent)
    , ui(new Ui::WizardPageUser)
{
    ui->setupUi(this);

    MsLanguage::instance()->initializeComboBox(ui->comboBox_language);
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
    ui->lineEdit_password->setCheckMode(MyLineEdit::EmptyCheck);
    ui->lineEdit_newPassword->setCheckMode(MyLineEdit::PasswordCheck);

    on_checkBox_newPassword_clicked(false);
}

WizardPageUser::~WizardPageUser()
{
    delete ui;
}

void WizardPageUser::saveSetting()
{
    const QString &adminPassword = ui->lineEdit_password->text();
    const QString &adminPasswordMD5 = QCryptographicHash::hash(adminPassword.toLatin1(), QCryptographicHash::Md5).toHex();

    db_user user;
    memset(&user, 0, sizeof(db_user));
    read_user_by_name(SQLITE_FILE_NAME, &user, "admin");

    if (adminPasswordMD5 != QString(user.password)) {
        ui->lineEdit_password->setCustomValid(false, GET_TEXT("MYLINETIP/112010", "Incorrect Password."));
        return;
    }
    if (ui->checkBox_newPassword->isChecked()) {
        const QString &newPassword = ui->lineEdit_newPassword->text();
        const QString &confirmPassword = ui->lineEdit_confirm->text();

        if (ui->lineEdit_newPassword->checkTooLong()) {
            ui->lineEdit_confirm->setTipFronSize(11);
        } else {
            ui->lineEdit_confirm->setTipFronSize(14);
        }

        bool valid = ui->lineEdit_newPassword->checkValid();

        if (newPassword != confirmPassword) {
            ui->lineEdit_confirm->setCustomValid(false, GET_TEXT("USER/74014", "Passwords don't match."));
            valid = false;
        }
        if (!valid) {
            return;
        }

        const QString &newAdminPasswordMD5 = QCryptographicHash::hash(newPassword.toLatin1(), QCryptographicHash::Md5).toHex();
        snprintf(user.password, sizeof(user.password), "%s", newAdminPasswordMD5.toStdString().c_str());
        snprintf(user.password_ex, sizeof(user.password_ex), "%s", newPassword.toStdString().c_str());
        write_user(SQLITE_FILE_NAME, &user);
        sendMessageOnly(REQUEST_FLAG_USER_UPDATE, nullptr, 0);
        //
        if (user.type == USERLEVEL_ADMIN) {
            if (qMsNvr->isPoe()) {
                const int result = MessageBox::question(this, GET_TEXT("WIZARD/11076", "Sync new admin password to current connected PoE channels?"));
                if (result == MessageBox::Yes) {
                    set_param_value(SQLITE_FILE_NAME, PARAM_POE_PSD, newPassword.toUtf8().data());
                    sendMessage(REQUEST_FLAG_MODIFY_POE_CAMERA_PWD, nullptr, 0);
                }
            }
        }
    }
    //
    showWizardPage(Wizard_Time);
}

void WizardPageUser::previousPage()
{
}

void WizardPageUser::nextPage()
{
    saveSetting();
}

void WizardPageUser::skipWizard()
{
    AbstractWizardPage::skipWizard();
}

void WizardPageUser::initializeData()
{
}

void WizardPageUser::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void WizardPageUser::onLanguageChanged()
{
    ui->label_password->setText(GET_TEXT("WIZARD/11007", "Admin Password "));
    ui->label_newPassword->setText(GET_TEXT("WIZARD/11007", "Admin Password"));
    ui->checkBox_newPassword->setText(GET_TEXT("WIZARD/11008", "New Admin Password"));
    ui->label_newPassword->setText(GET_TEXT("WIZARD/11009", "New Password"));
    ui->label_confirm->setText(GET_TEXT("USER/74055", "Confirm Password"));
    ui->label_language->setText(GET_TEXT("WIZARD/11011", "Language"));
}

void WizardPageUser::on_checkBox_newPassword_clicked(bool checked)
{
    ui->label_newPassword->setEnabled(checked);
    ui->lineEdit_newPassword->setEnabled(checked);
    ui->label_confirm->setEnabled(checked);
    ui->lineEdit_confirm->setEnabled(checked);
}

void WizardPageUser::on_comboBox_language_activated(int index)
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

    if (!tempLanguageMap.isEmpty()) {
        if (MessageBox::Yes == MessageBox::englishQuestion(this, "Language changes will take effect after rebooting. Are you sure to reboot now?")) {
            //qMsNvr->reboot();
            //qMsApp->setAboutToReboot(true);
            //m_waitting->//showWait();
        }
    }
}


