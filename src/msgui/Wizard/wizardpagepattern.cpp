#include "ui_wizardpagepattern.h"
#include "wizardpagepattern.h"

#include "gusturelock.h"
#include "MessageBox.h"
#include "MsLanguage.h"
#include "MyDebug.h"

WizardPagePattern::WizardPagePattern(QWidget *parent)
    : AbstractWizardPage(parent)
    , ui(new Ui::WizardPagePattern)
{
    ui->setupUi(this);

    connect(ui->widget_pattern, SIGNAL(drawStart()), this, SLOT(onDrawStart()));
    connect(ui->widget_pattern, SIGNAL(drawFinished(QString)), this, SLOT(onDrawFinished(QString)));

    ui->comboBox_patternEnable->clear();
    ui->comboBox_patternEnable->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_patternEnable->addItem(GET_TEXT("COMMON/1009", "Eanble"), 1);

    m_pattern_text = "0";
    ui->label_unlockPattern->setText(GET_TEXT("WIZARD/11080", "Unlock Pattern"));
}

WizardPagePattern::~WizardPagePattern()
{
    delete ui;
}

void WizardPagePattern::initializeData()
{
    memset(&m_adminUser, 0, sizeof(m_adminUser));
    read_user(SQLITE_FILE_NAME, &m_adminUser, 0);

    m_pattern_text = m_adminUser.pattern_psw;
    int index = m_pattern_text.size() < 4 ? 0 : 1;
    ui->comboBox_patternEnable->setCurrentIndex(index);
    on_comboBox_patternEnable_activated(index);

    ui->label_patternTips->show();
    ui->widget_pattern->setStyle(WhiteStyle);
}

void WizardPagePattern::saveSetting()
{
    qMsDebug() << QString("WizardPagePattern::saveSetting()   [%1]").arg(m_pattern_text);
    if (ui->comboBox_patternEnable->currentIndex()) {
        if (m_pattern_text.size() < 4) {
            m_pattern_text = "0";
            int result = MessageBox::question(this, GET_TEXT("WIZARD/11081", "Set unlock pattern unsuccessfully, it will be disabled if you skip to next, continue?"));
            if (result == MessageBox::Cancel) {
                return;
            }
        }
    } else {
        m_pattern_text = "0";
    }
    snprintf(m_adminUser.pattern_psw, sizeof(m_adminUser.pattern_psw), "%s", m_pattern_text.toStdString().c_str());
    write_user(SQLITE_FILE_NAME, &m_adminUser);
    showWizardPage(Wizard_Question);
}

void WizardPagePattern::previousPage()
{
}

void WizardPagePattern::nextPage()
{
    saveSetting();
}

void WizardPagePattern::skipWizard()
{
    if (ui->comboBox_patternEnable->currentIndex()) {
        if (m_pattern_text.size() < 4) {
            m_pattern_text = "0";
            int result = MessageBox::question(this, GET_TEXT("WIZARD/11082", "Set unlock pattern unsuccessfully, it will be disabled if you exit the Wizard, continue?"));
            if (result == MessageBox::Cancel) {
                return;
            }
        }
    } else {
        m_pattern_text = "0";
    }
    snprintf(m_adminUser.pattern_psw, sizeof(m_adminUser.pattern_psw), "%s", m_pattern_text.toStdString().c_str());
    write_user(SQLITE_FILE_NAME, &m_adminUser);
    AbstractWizardPage::skipWizard();
}

void WizardPagePattern::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void WizardPagePattern::onDrawStart()
{
    m_pattern_text = "0";
    ui->label_patternTips->setText(GET_TEXT("USER/74060", "Release mouse when finished."));
}

void WizardPagePattern::onDrawFinished(QString text)
{
    qMsDebug() << QString("onDrawFinished!!!  m_pattern_text:[%1] : [%2]").arg(m_pattern_text).arg(text);
    static QString pattern = "0";
    if (step == 1) {
        if (text.size() < 4) {
            ui->label_patternTips->setText(GET_TEXT("USER/74056", "Connect at least 4 dots, please try again."));
            return;
        }

        step++;
        pattern = text;
        ui->label_patternTips->setText(GET_TEXT("USER/74057", "Please draw again to confirm."));
        return;
    } else {
        step--;
        if (pattern == text) {
            m_pattern_text = pattern;
            ui->label_patternTips->setText(GET_TEXT("USER/74058", "Set unlock pattern successfully."));
        } else {
            ui->label_patternTips->setText(GET_TEXT("USER/74059", "Unmatched pattern, please try again."));
        }
        pattern = "0";
    }
}

void WizardPagePattern::on_comboBox_patternEnable_activated(int index)
{
    step = 1;
    bool enable = ui->comboBox_patternEnable->itemData(index).toBool();
    ui->widget_pattern->setEnabled(enable);
    if (enable) {
        ui->label_patternTips->setText(GET_TEXT("USER/74063", "Please connect at least 4 dots."));
    } else {
        ui->label_patternTips->setText("");
    }
}
