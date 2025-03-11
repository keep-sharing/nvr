#include "PosPrivacySettings.h"
#include "ui_PosPrivacySettings.h"
#include "MsLanguage.h"

PosPrivacySettings::PosPrivacySettings(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::PosPrivacySettings)
{
    ui->setupUi(this);

    ui->lineEditPrivacy1->setMaxLength(32);
    ui->lineEditPrivacy2->setMaxLength(32);
    ui->lineEditPrivacy3->setMaxLength(32);

    onLanguageChanged();
}

PosPrivacySettings::~PosPrivacySettings()
{
    delete ui;
}

void PosPrivacySettings::setConfig(Db_POS_CONFIG *config)
{
    m_config = config;
    ui->lineEditPrivacy1->setText(m_config->privacy1);
    ui->lineEditPrivacy2->setText(m_config->privacy2);
    ui->lineEditPrivacy3->setText(m_config->privacy3);
}

void PosPrivacySettings::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("POS/130023", "Privacy Settings"));
    ui->labelPrivacy1->setText(QString("%1 1").arg(GET_TEXT("POS/130033", "Privacy")));
    ui->labelPrivacy2->setText(QString("%1 2").arg(GET_TEXT("POS/130033", "Privacy")));
    ui->labelPrivacy3->setText(QString("%1 3").arg(GET_TEXT("POS/130033", "Privacy")));
    ui->pushButtonOk->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButtonCancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void PosPrivacySettings::on_pushButtonOk_clicked()
{
    snprintf(m_config->privacy1, sizeof(m_config->privacy1), "%s", ui->lineEditPrivacy1->text().toLocal8Bit().data());
    snprintf(m_config->privacy2, sizeof(m_config->privacy2), "%s", ui->lineEditPrivacy2->text().toLocal8Bit().data());
    snprintf(m_config->privacy3, sizeof(m_config->privacy3), "%s", ui->lineEditPrivacy3->text().toLocal8Bit().data());
    accept();
}

void PosPrivacySettings::on_pushButtonCancel_clicked()
{
    reject();
}
