#include "PosConnectionModeSettings.h"
#include "ui_PosConnectionModeSettings.h"
#include "MsLanguage.h"
#include "MsDevice.h"
#include "MessageBox.h"

PosConnectionModeSettings::PosConnectionModeSettings(QWidget *parent) :
    BaseShadowDialog(parent),
    ui(new Ui::PosConnectionModeSettings)
{
    ui->setupUi(this);

    ui->lineEditPosIPAddress->setCheckMode(MyLineEdit::IPv4Check);
    ui->lineEditPosPort->setCheckMode(MyLineEdit::RangeCanEmptyCheck, 1, 65535);
    ui->lineEditNvrPort->setCheckMode(MyLineEdit::RangeCheck, 1024, 65535);

    onLanguageChanged();
}

PosConnectionModeSettings::~PosConnectionModeSettings()
{
    delete ui;
}

void PosConnectionModeSettings::setConfig(int index, Db_POS_CONFIG *currentConfigs, Db_POS_CONFIG *lastConfigs)
{
    m_index = index;
    m_lastConfigs = lastConfigs;
    m_currentConfigs = currentConfigs;

    const Db_POS_CONFIG &config = m_currentConfigs[m_index];

    ui->lineEditPosIPAddress->setText(config.ip);
    if (config.port == 0) {
        ui->lineEditPosPort->clear();
    } else {
        ui->lineEditPosPort->setText(QString::number(config.port));
    }
    if (config.nvrPort == 0) {
        ui->lineEditNvrPort->clear();
    } else {
        ui->lineEditNvrPort->setText(QString::number(config.nvrPort));
    }
}

void PosConnectionModeSettings::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("POS/130007", "Connection Mode Settings"));
    ui->labelPosIPAddress->setText(GET_TEXT("POS/130032", "POS IP Address"));
    ui->pushButtonOk->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButtonCancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void PosConnectionModeSettings::on_pushButtonOk_clicked()
{
    bool check1 = ui->lineEditPosIPAddress->checkValid();
    bool check2 = ui->lineEditPosPort->checkValid();
    bool check3 = ui->lineEditNvrPort->checkValid();
    if (!check1 || !check2 || !check3) {
        return;
    }

    int nvrPort = ui->lineEditNvrPort->text().toInt();
    for (int i = 0; i < MAX_POS_CLIENT; ++i) {
        const Db_POS_CONFIG &config = m_currentConfigs[i];
        if (config.id == m_index) {
            continue;
        }
        if (!config.enable) {
            continue;
        }
        if (config.nvrPort == nvrPort) {
            ui->lineEditNvrPort->setCustomValid(false, GET_TEXT("MYLINETIP/112011", "Already existed."));
            return;
        }
    }

    const Db_POS_CONFIG &lastConfig = m_lastConfigs[m_index];
    network_more netMore;
    read_port(SQLITE_FILE_NAME, &netMore);
    if (nvrPort != lastConfig.nvrPort) {
        if (qMsNvr->isPortUsed(nvrPort) || nvrPort == netMore.https_port) {
            ui->lineEditNvrPort->setCustomValid(false, GET_TEXT("MYLINETIP/112011", "Already existed."));
            return;
        }
    }

    Db_POS_CONFIG &config = m_currentConfigs[m_index];
    snprintf(config.ip, sizeof(config.ip), "%s", ui->lineEditPosIPAddress->text().toLocal8Bit().data());
    config.port = ui->lineEditPosPort->text().toInt();
    config.nvrPort = ui->lineEditNvrPort->text().toInt();
    accept();
}

void PosConnectionModeSettings::on_pushButtonCancel_clicked()
{
    reject();
}

