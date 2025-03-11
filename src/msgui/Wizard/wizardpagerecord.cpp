#include "wizardpagerecord.h"
#include "ui_wizardpagerecord.h"
#include "LogWrite.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "msuser.h"

WizardPageRecord::WizardPageRecord(QWidget *parent)
    : AbstractWizardPage(parent)
    , ui(new Ui::WizardPageRecord)
{
    ui->setupUi(this);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

WizardPageRecord::~WizardPageRecord()
{
    delete ui;
}

void WizardPageRecord::initializeData()
{
}

void WizardPageRecord::saveSetting()
{
    int chan_no = 0;
    if (ui->checkBox_record->isChecked()) {
        struct resp_set_all_record allRecord;
        memset(&allRecord, 0, sizeof(struct resp_set_all_record));
        allRecord.enable = 1;
        for (int i(0); i < MAX_CAMERA && i < qMsNvr->maxChannel(); i++) {
            chan_no = i + 1;
            allRecord.channel[i] = 1;
        }
        sendMessage(REQUEST_FLAG_SET_ALL_RECORD, &allRecord, sizeof(struct resp_set_all_record));

        //TODO: log
        struct log_data log_data;
        memset(&log_data, 0, sizeof(struct log_data));
        log_data.log_data_info.subType = SUB_OP_CONFIG_LOCAL;
        log_data.log_data_info.parameter_type = SUB_PARAM_RECORD_SCHED;
        snprintf(log_data.log_data_info.user, sizeof(log_data.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
        log_data.log_data_info.chan_no = chan_no;
        MsWriteLog(log_data);
    }
}

void WizardPageRecord::previousPage()
{
    showWizardPage(Wizard_P2P);
}

void WizardPageRecord::nextPage()
{
    saveSetting();
    finishWizard();
}

void WizardPageRecord::skipWizard()
{
    saveSetting();
    AbstractWizardPage::skipWizard();
}

void WizardPageRecord::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void WizardPageRecord::onLanguageChanged()
{
    ui->checkBox_record->setText(GET_TEXT("WIZARD/11031", "Set all channels continuous record"));
}
