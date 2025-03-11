#include "TabHeatMapAutoBackup.h"
#include "ui_TabHeatMapAutoBackup.h"
#include "EventLoop.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsMessage.h"

TabHeatMapAutoBackup::TabHeatMapAutoBackup(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabHeatMapAutoBackup)
{
    ui->setupUi(this);
    ui->checkBoxGroupChannel->setCount(qMsNvr->maxChannel());

    ui->comboBoxAutoBackup->beginEdit();
    ui->comboBoxAutoBackup->clear();
    ui->comboBoxAutoBackup->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBoxAutoBackup->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
    ui->comboBoxAutoBackup->endEdit();

    ui->comboBoxDay->beginEdit();
    ui->comboBoxDay->clear();
    ui->comboBoxDay->addItem(GET_TEXT("AUTOREBOOT/78001", "Everyday"), 0);
    ui->comboBoxDay->addItem(GET_TEXT("COMMON/1024", "Sunday"), 1);
    ui->comboBoxDay->addItem(GET_TEXT("COMMON/1025", "Monday"), 2);
    ui->comboBoxDay->addItem(GET_TEXT("COMMON/1026", "Tuesday"), 3);
    ui->comboBoxDay->addItem(GET_TEXT("COMMON/1027", "Wednesday"), 4);
    ui->comboBoxDay->addItem(GET_TEXT("COMMON/1028", "Thursday"), 5);
    ui->comboBoxDay->addItem(GET_TEXT("COMMON/1029", "Friday"), 6);
    ui->comboBoxDay->addItem(GET_TEXT("COMMON/1030", "Saturday"), 7);
    ui->comboBoxDay->endEdit();
    onLanguageChanged();
}

TabHeatMapAutoBackup::~TabHeatMapAutoBackup()
{
    delete ui;
}

void TabHeatMapAutoBackup::initializeData()
{
    memset(&m_settings, 0, sizeof(m_settings));
    int result = read_peoplecnt_auto_backup(SQLITE_FILE_NAME, &m_settings);
    QString text = QString("\n==read_peoplecnt_auto_backup, result:%1==").arg(result);
    const REPORT_AUTO_BACKUP_SETTING_S &setting = m_settings.settings[REPORT_AUTO_BACKUP_HEATMAP];
    ui->comboBoxAutoBackup->setCurrentIndexFromData(setting.enable);
    ui->checkBoxGroupChannel->setCheckedFromString(setting.triChannels);
    m_currentRangeType = setting.timeRange;
    ui->checkBoxSpaceHeatMap->setChecked(setting.fileFormat & REPORT_AUTO_BACKUP_FORMAT_HEATMAP_SPACE);
    ui->checkBoxTimeHeatMap->setChecked(setting.fileFormat & REPORT_AUTO_BACKUP_FORMAT_HEATMAP_TIME);
    ui->comboBoxDay->setCurrentIndexFromData(setting.backupDay);
    ui->timeEdit->setTime(QTime::fromString(setting.backupTime, "HH:mm:ss"));
    ui->comboBoxTimeRange->setCurrentIndexFromData(setting.timeRange);
    ui->checkBoxExtemalDevice->setChecked(setting.backupTo & REPORT_AUTO_BACKUP_TO_DEVICE);
    ui->checkBoxEmail->setChecked(setting.backupTo & REPORT_AUTO_BACKUP_TO_EMAIL);
}

void TabHeatMapAutoBackup::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SET_REPORT_AUTO_BACKUP_SETTINGS:
        ON_RESPONSE_FLAG_SET_REPORT_AUTO_BACKUP_SETTINGS(message);
        break;
    default:
        break;
    }
}

void TabHeatMapAutoBackup::ON_RESPONSE_FLAG_SET_REPORT_AUTO_BACKUP_SETTINGS(MessageReceive *message)
{
    Q_UNUSED(message)

    gEventLoopExit(0);
}

void TabHeatMapAutoBackup::onLanguageChanged()
{
    ui->labelReportType->setText(GET_TEXT("OCCUPANCY/74204", "Report Type"));
    ui->labelReportAutoBackup->setText(GET_TEXT("REPORTAUTOBACKUP/114002", "Report Auto Backup"));
    ui->labelChannel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->labelDay->setText(GET_TEXT("REPORTAUTOBACKUP/114010", "Day"));
    ui->labelTime->setText(GET_TEXT("REPORTAUTOBACKUP/114011", "Time"));
    ui->labelBackupTimeRange->setText(GET_TEXT("REPORTAUTOBACKUP/114003", "Backup Time Range"));
    ui->labelBackupTo->setText(GET_TEXT("REPORTAUTOBACKUP/114004", "Backup to"));

    ui->checkBoxEmail->setText(GET_TEXT("REPORTAUTOBACKUP/114012", "Email"));
    ui->checkBoxExtemalDevice->setText(GET_TEXT("REPORTAUTOBACKUP/114008", "External Device"));
    ui->checkBoxSpaceHeatMap->setText(GET_TEXT("HEATMAP/104008", "Space Heat Map"));
    ui->checkBoxTimeHeatMap->setText(GET_TEXT("HEATMAP/104009", "Time Heat Map"));

    ui->pushButtonApply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
}

void TabHeatMapAutoBackup::on_comboBoxAutoBackup_indexSet(int index)
{
    ui->checkBoxGroupChannel->setEnabled(index);
    ui->comboBoxDay->setEnabled(index);
    ui->timeEdit->setEnabled(index);
    ui->widgetReportType->setEnabled(index);
    ui->comboBoxTimeRange->setEnabled(index);
    ui->widgetBackupTo->setEnabled(index);
    ui->comboBoxTimeRange->setCurrentIndexFromData(m_currentRangeType);
}

void TabHeatMapAutoBackup::on_comboBoxDay_indexSet(int index)
{
    //todo
    ui->comboBoxTimeRange->beginEdit();
    ui->comboBoxTimeRange->clear();
    switch (index) {
    case 0:
        if (!ui->checkBoxTimeHeatMap->isChecked()) {
            ui->comboBoxTimeRange->addItem(GET_TEXT("HEATMAP/147000", "Last Hour"), 3);
            ui->comboBoxTimeRange->addItem(GET_TEXT("HEATMAP/147001","Interval 1 hour"), 4);
        }
        ui->comboBoxTimeRange->addItem(GET_TEXT("REPORTAUTOBACKUP/114005", "Last day"), 1);
        m_currentRangeType = 1;
        break;
    default:
        ui->comboBoxTimeRange->addItem(GET_TEXT("REPORTAUTOBACKUP/114006", "Last week"), 2);
        m_currentRangeType = 2;
        ui->checkBoxTimeHeatMap->setEnabled(true);
        break;
    }
    ui->comboBoxTimeRange->endEdit();
    ui->comboBoxTimeRange->setCurrentIndexFromData(m_currentRangeType);
}

void TabHeatMapAutoBackup::on_comboBoxTimeRange_indexSet(int index)
{
    Q_UNUSED(index);
    if (ui->comboBoxTimeRange->currentIntData() == 3 || ui->comboBoxTimeRange->currentIntData() == 4) {
        ui->checkBoxTimeHeatMap->setEnabled(false);
    } else {
        ui->checkBoxTimeHeatMap->setEnabled(true);
    }
    m_currentRangeType = ui->comboBoxTimeRange->currentIntData();
}

void TabHeatMapAutoBackup::on_pushButtonApply_clicked()
{
    //showWait();
    REPORT_AUTO_BACKUP_SETTING_S &setting = m_settings.settings[REPORT_AUTO_BACKUP_HEATMAP];
    setting.enable = ui->comboBoxAutoBackup->currentIntData();
    snprintf(setting.triChannels, sizeof(setting.triChannels), "%s", ui->checkBoxGroupChannel->checkedMask().toStdString().c_str());
    setting.backupDay = ui->comboBoxDay->currentIntData();
    snprintf(setting.backupTime, sizeof(setting.backupTime), "%s", ui->timeEdit->time().toString("HH:mm:ss").toStdString().c_str());
    //fileformat
    setting.fileFormat = 0;
    if (ui->checkBoxSpaceHeatMap->isChecked()) {
        setting.fileFormat |= REPORT_AUTO_BACKUP_FORMAT_HEATMAP_SPACE;
    }
    if (ui->checkBoxTimeHeatMap->isChecked()) {
        setting.fileFormat |= REPORT_AUTO_BACKUP_FORMAT_HEATMAP_TIME;
    }

    setting.timeRange = ui->comboBoxTimeRange->currentIntData();

    setting.backupTo = 0;
    if (ui->checkBoxExtemalDevice->isChecked()) {
        setting.backupTo |= REPORT_AUTO_BACKUP_TO_DEVICE;
    }
    if (ui->checkBoxEmail->isChecked()) {
        setting.backupTo |= REPORT_AUTO_BACKUP_TO_EMAIL;
    }

    write_peoplecnt_auto_backup(SQLITE_FILE_NAME, &m_settings);
    sendMessage(REQUEST_FLAG_SET_REPORT_AUTO_BACKUP_SETTINGS, nullptr, 0);
    gEventLoopExec();
    //closeWait();
}

void TabHeatMapAutoBackup::on_pushButtonBack_clicked()
{
    back();
}

void TabHeatMapAutoBackup::on_checkBoxTimeHeatMap_clicked(bool checked)
{
    if (ui->comboBoxDay->currentIntData() == 0) {
        ui->comboBoxTimeRange->beginEdit();
        ui->comboBoxTimeRange->clear();
        if (!checked) {
            ui->comboBoxTimeRange->addItem(GET_TEXT("HEATMAP/147000", "Last Hour"), 3);
            ui->comboBoxTimeRange->addItem(GET_TEXT("HEATMAP/147001","Interval 1 hour"), 4);
        }
        ui->comboBoxTimeRange->addItem(GET_TEXT("REPORTAUTOBACKUP/114005", "Last day"), 1);
        ui->comboBoxTimeRange->endEdit();
        ui->comboBoxTimeRange->setCurrentIndexFromData(m_currentRangeType);
    }
}
