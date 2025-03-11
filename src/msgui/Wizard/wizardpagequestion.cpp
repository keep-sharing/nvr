#include "ui_wizardpagequestion.h"

#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "wizardpagequestion.h"

#include <QtDebug>

extern "C" {
#include "msdb.h"
}

WizardPageQuestion::WizardPageQuestion(QWidget *parent)
    : AbstractWizardPage(parent)
    , ui(new Ui::WizardPageQuestion)
{
    ui->setupUi(this);
}

WizardPageQuestion::~WizardPageQuestion()
{
    delete ui;
}

void WizardPageQuestion::initializeData()
{
    onLanguageChanged();
}

void WizardPageQuestion::saveSetting()
{
    const QString &answer1 = ui->lineEdit_answer1->text().trimmed();
    const QString &answer2 = ui->lineEdit_answer2->text().trimmed();
    const QString &answer3 = ui->lineEdit_answer3->text().trimmed();
    if (answer1.isEmpty() && answer2.isEmpty() && answer3.isEmpty()) {
        ShowMessageBox(GET_TEXT("WIZARD/11058", "You can set up Security Questions later in Settings-User."));
        showWizardPage(Wizard_Time);
        return;
    }
    if (answer1.isEmpty() || answer2.isEmpty() || answer3.isEmpty()) {
        ShowMessageBox(GET_TEXT("WIZARD/11060", "Security Question&Answers cannot be empty!"));
        return;
    }

    saveQuestion();
    ShowMessageBox(GET_TEXT("WIZARD/11059", "Security questions saved successfully!"));
    showWizardPage(Wizard_Time);
}

void WizardPageQuestion::previousPage()
{
}

void WizardPageQuestion::nextPage()
{
    saveSetting();
}

void WizardPageQuestion::skipWizard()
{
    const QString &answer1 = ui->lineEdit_answer1->text().trimmed();
    const QString &answer2 = ui->lineEdit_answer2->text().trimmed();
    const QString &answer3 = ui->lineEdit_answer3->text().trimmed();
    if (answer1.isEmpty() && answer2.isEmpty() && answer3.isEmpty()) {
        //MSHN-6593, skip时不弹出这个提示
        //ShowMessageBox(GET_TEXT("WIZARD/11058", "You can set up Security Questions later in Settings-User."));
        AbstractWizardPage::skipWizard();
        return;
    }
    if (answer1.isEmpty() || answer2.isEmpty() || answer3.isEmpty()) {
        ShowMessageBox(GET_TEXT("WIZARD/11060", "Security Question&Answers cannot be empty!"));
        return;
    }
    const int result = MessageBox::question(this, GET_TEXT("WIZARD/11044", "Save the Security Question settings?"));
    if (result == MessageBox::Yes) {
        saveQuestion();
    }
    AbstractWizardPage::skipWizard();
}

void WizardPageQuestion::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void WizardPageQuestion::saveQuestion()
{
    const QString &answer1 = ui->lineEdit_answer1->text().trimmed();
    const QString &answer2 = ui->lineEdit_answer2->text().trimmed();
    const QString &answer3 = ui->lineEdit_answer3->text().trimmed();

    struct squestion sqa;
    memset(&sqa, 0x0, sizeof(struct squestion));
    sqa.enable = 1;
    sqa.sqtype = ui->comboBox_question1->currentIndex();
    snprintf(sqa.answer, sizeof(sqa.answer), "%s", answer1.toStdString().c_str());
    if (sqa.sqtype == SQA_CUSTOMIZED_NO) {
        snprintf(sqa.squestion, sizeof(sqa.squestion), "%s", ui->comboBox_question1->currentText().trimmed().toStdString().c_str());
    }
    write_encrypted_list(SQLITE_FILE_NAME, &sqa, 1);

    memset(&sqa, 0x0, sizeof(struct squestion));
    sqa.enable = 1;
    sqa.sqtype = ui->comboBox_question2->currentIndex();
    snprintf(sqa.answer, sizeof(sqa.answer), "%s", answer2.toStdString().c_str());
    if (sqa.sqtype == SQA_CUSTOMIZED_NO) {
        snprintf(sqa.squestion, sizeof(sqa.squestion), "%s", ui->comboBox_question2->currentText().trimmed().toStdString().c_str());
    }
    write_encrypted_list(SQLITE_FILE_NAME, &sqa, 2);

    memset(&sqa, 0x0, sizeof(struct squestion));
    sqa.enable = 1;
    sqa.sqtype = ui->comboBox_question3->currentIndex();
    snprintf(sqa.answer, sizeof(sqa.answer), "%s", answer3.toStdString().c_str());
    if (sqa.sqtype == SQA_CUSTOMIZED_NO) {
        snprintf(sqa.squestion, sizeof(sqa.squestion), "%s", ui->comboBox_question3->currentText().trimmed().toStdString().c_str());
    }
    write_encrypted_list(SQLITE_FILE_NAME, &sqa, 3);
}

void WizardPageQuestion::onLanguageChanged()
{
}
