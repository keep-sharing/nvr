#include "StorageGeneralSettings.h"
#include "ui_StorageGeneralSettings.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"

StorageGeneralSettings::StorageGeneralSettings(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::StorageGeneralSettings)
{
    ui->setupUi(this);

    ui->comboBox_recycleMode->clear();
    ui->comboBox_recycleMode->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_recycleMode->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);

    ui->comboBox_eSATAFunction->clear();
    ui->comboBox_eSATAFunction->addItem(QString(GET_TEXT("RECORDADVANCE/91019", "Storage")), 0);
    ui->comboBox_eSATAFunction->addItem(QString(GET_TEXT("PROFILE/76001", "Backup")), 1);
    //只有MS-N8032-UH和MS-N8064-UH有esata功能，其他机型没有。
    if (!qMsNvr->isSupportESataFunction()) {
        ui->label_eSATAFunction->hide();
        ui->comboBox_eSATAFunction->hide();
    }

    onLanguageChanged();
}

StorageGeneralSettings::~StorageGeneralSettings()
{
    delete ui;
}

void StorageGeneralSettings::initializeData()
{
    initializeAdvancedPage();
}

void StorageGeneralSettings::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_REC_ADVANCED:
        ON_RESPONSE_FLAG_GET_REC_ADVANCED(message);
        break;
    case RESPONSE_FLAG_SET_REC_RECYCLEMODE:
        ON_RESPONSE_FLAG_SET_REC_RECYCLEMODE(message);
        break;
    }
}

void StorageGeneralSettings::ON_RESPONSE_FLAG_GET_REC_ADVANCED(MessageReceive *message)
{
    RECORD_ADVANCED *record = (RECORD_ADVANCED *)message->data;
    if (!record) {
        qWarning() << "RecordSetting::ON_RESPONSE_FLAG_GET_REC_ADVANCED, data is null.";
        return;
    }
    m_recordAdvanced.recycle_mode = record->recycle_mode;
    m_recordAdvanced.esata_type = record->esata_type;
    ui->comboBox_recycleMode->setCurrentIndexFromData(record->recycle_mode);
    ui->comboBox_eSATAFunction->setCurrentIndexFromData(record->esata_type);
}

void StorageGeneralSettings::ON_RESPONSE_FLAG_SET_REC_RECYCLEMODE(MessageReceive *message)
{
    Q_UNUSED(message)

    //closeWait();
}

void StorageGeneralSettings::initializeAdvancedPage()
{
    sendMessage(REQUEST_FLAG_GET_REC_ADVANCED, nullptr, 0);
}

void StorageGeneralSettings::onLanguageChanged()
{
    ui->label_recycleMode->setText(GET_TEXT("DISKMANAGE/72006", "Recycle Mode"));
    ui->label_eSATAFunction->setText(GET_TEXT("RECORDADVANCE/91018", "eSATA Function"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void StorageGeneralSettings::on_pushButton_apply_clicked()
{
    const int &recycle_mode = ui->comboBox_recycleMode->currentData().toInt();
    const int &esata_type = ui->comboBox_eSATAFunction->currentData().toInt();

    struct req_record_conf record_conf;
    memset(&record_conf, 0, sizeof(struct req_record_conf));
    if (recycle_mode != m_recordAdvanced.recycle_mode || esata_type != m_recordAdvanced.esata_type) {
        if (!recycle_mode && m_recordAdvanced.recycle_mode) {
            int result = MessageBox::question(this, GET_TEXT("RECORDADVANCE/91022", "Disabling Recycle Mode will keep the record files but stop recording when disk is full."));
            if (result != MessageBox::Yes) {
                return;
            }
        }
        record_conf.mode = recycle_mode;
        record_conf.esata = esata_type;
        m_recordAdvanced.recycle_mode = recycle_mode;
        m_recordAdvanced.esata_type = esata_type;
        sendMessage(REQUEST_FLAG_SET_REC_RECYCLEMODE, (void *)&record_conf, sizeof(struct req_record_conf));
        //等待RESPONSE_FLAG_SET_REC_RECYCLEMODE
        //showWait();
    }
}

void StorageGeneralSettings::on_pushButton_back_clicked()
{
    emit sig_back();
}
