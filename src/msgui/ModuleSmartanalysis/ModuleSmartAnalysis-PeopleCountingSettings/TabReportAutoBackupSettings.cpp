#include "TabReportAutoBackupSettings.h"
#include "ui_TabReportAutoBackupSettings.h"
#include "EventLoop.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsMessage.h"

TabReportAutoBackupSettings::TabReportAutoBackupSettings(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabReportAutoBackupSettings)
{
    ui->setupUi(this);

    ui->checkBoxGroupChannel->setCount(qMsNvr->maxChannel());
    ui->checkBoxGroupGroup->setCount(MAX_PEOPLECNT_GROUP, 3);

    ui->comboBoxType->beginEdit();
    ui->comboBoxType->clear();
    ui->comboBoxType->addItem(GET_TEXT("PEOPLECOUNTING_SEARCH/140000", "People Counting by Camera"), REPORT_AUTO_BACKUP_PEOPLECNT_BY_CAMERA);
    ui->comboBoxType->addItem(GET_TEXT("PEOPLECOUNTING_SEARCH/140001", "People Counting by Group"), REPORT_AUTO_BACKUP_PEOPLECNT_BY_GROUP);
    ui->comboBoxType->addItem(GET_TEXT("PEOPLECOUNTING_SEARCH/140002", "Regional People Counting"), REPORT_AUTO_BACKUP_PEOPLECNT_BY_REGION);
    ui->comboBoxType->endEdit();

    ui->comboBoxAutoBackup->beginEdit();
    ui->comboBoxAutoBackup->clear();
    ui->comboBoxAutoBackup->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBoxAutoBackup->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
    ui->comboBoxAutoBackup->endEdit();

    ui->comboBoxStay->beginEdit();
    ui->comboBoxStay->clear();
    ui->comboBoxStay->addItem(GET_TEXT("COMMON/1006", "All"), 0);
    ui->comboBoxStay->addItem(GET_TEXT("REGIONAL_PEOPLECOUNTING/103309", "More than"), 1);
    ui->comboBoxStay->addItem(GET_TEXT("REGIONAL_PEOPLECOUNTING/103310", "Less than"), 2);
    ui->comboBoxStay->endEdit();

    ui->comboBoxStayTime->beginEdit();
    ui->comboBoxStayTime->clear();
    ui->comboBoxStayTime->addItem("5s", 5);
    ui->comboBoxStayTime->addItem("10s", 10);
    ui->comboBoxStayTime->addItem("30s", 30);
    ui->comboBoxStayTime->addItem("60s", 60);
    ui->comboBoxStayTime->addItem("100s", 100);
    ui->comboBoxStayTime->addItem("300s", 300);
    ui->comboBoxStayTime->addItem("600s", 600);
    ui->comboBoxStayTime->addItem("1800s", 1800);
    ui->comboBoxStayTime->endEdit();

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

    ui->comboBoxFileFormat->addItem("CSV", 0);
    ui->comboBoxFileFormat->setEnabled(false);

    ui->widgetLineType->setCount(5);
    QStringList typeList;
    typeList << GET_TEXT("PEOPLECOUNTING_SEARCH/145022", "Total")
             << GET_TEXT("PEOPLECOUNTING_SEARCH/145003", "Line") + " 1"
             << GET_TEXT("PEOPLECOUNTING_SEARCH/145003", "Line") + " 2"
             << GET_TEXT("PEOPLECOUNTING_SEARCH/145003", "Line") + " 3"
             << GET_TEXT("PEOPLECOUNTING_SEARCH/145003", "Line") + " 4";
    ui->widgetLineType->setCheckBoxTest(typeList);

    onLanguageChanged();
}

TabReportAutoBackupSettings::~TabReportAutoBackupSettings()
{
    delete ui;
}

void TabReportAutoBackupSettings::initializeData()
{
    memset(&m_settings, 0, sizeof(m_settings));
    int result = read_peoplecnt_auto_backup(SQLITE_FILE_NAME, &m_settings);
    QString text = QString("\n==read_peoplecnt_auto_backup, result:%1==").arg(result);
    for (int i = 0; i < MAX_REPORT_AUTO_BACKUP_TYPE_COUNT; ++i) {
        REPORT_AUTO_BACKUP_SETTING_S &setting = m_settings.settings[i];
        text += QString("\n--type:%1").arg(i);
        text += QString("\n----enable:%1").arg(setting.enable);
        text += QString("\n----triChannels:%1").arg(setting.triChannels);
        text += QString("\n----triGroups:%1").arg(setting.triGroups);
        text += QString("\n----stayLengthType:%1").arg(setting.stayLengthType);
        text += QString("\n----stayLengthValue:%1").arg(setting.stayLengthValue);
        text += QString("\n----backupDay:%1").arg(setting.backupDay);
        text += QString("\n----backupTime:%1").arg(setting.backupTime);
        text += QString("\n----fileFormat:%1").arg(setting.fileFormat);
        text += QString("\n----timeRange:%1").arg(setting.timeRange);
        text += QString("\n----backupTo:%1").arg(setting.backupTo);
    }
    qMsDebug() << qPrintable(text);

    m_currentType = -1;
    ui->comboBoxType->setCurrentIndexFromData(REPORT_AUTO_BACKUP_PEOPLECNT_BY_CAMERA);
}

void TabReportAutoBackupSettings::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SET_REPORT_AUTO_BACKUP_SETTINGS:
        ON_RESPONSE_FLAG_SET_REPORT_AUTO_BACKUP_SETTINGS(message);
        break;
    default:
        break;
    }
}

void TabReportAutoBackupSettings::ON_RESPONSE_FLAG_SET_REPORT_AUTO_BACKUP_SETTINGS(MessageReceive *message)
{
    Q_UNUSED(message)

    gEventLoopExit(0);
}

void TabReportAutoBackupSettings::setChannelVisible(bool enable)
{
    ui->checkBoxGroupChannel->setVisible(enable);
    ui->widgetChannel->setVisible(enable);
}

void TabReportAutoBackupSettings::setGroupVisible(bool enable)
{
    ui->checkBoxGroupGroup->setVisible(enable);
    ui->widgetGroup->setVisible(enable);
}

void TabReportAutoBackupSettings::setLengthOfStayVisible(bool enable)
{
    ui->labelLengthOfStay->setVisible(enable);
    ui->widgetLengthOfStay->setVisible(enable);
}

void TabReportAutoBackupSettings::setLineVisible(bool enable)
{
    ui->widgetLineType->setVisible(enable);
    ui->widgetLine->setVisible(enable);
}

void TabReportAutoBackupSettings::saveSetting(int type)
{
    REPORT_AUTO_BACKUP_SETTING_S &setting = m_settings.settings[type];
    setting.enable = ui->comboBoxAutoBackup->currentIntData();
    setting.backupDay = ui->comboBoxDay->currentIntData();
    snprintf(setting.backupTime, sizeof(setting.backupTime), "%s", ui->timeEdit->time().toString("HH:mm:ss").toStdString().c_str());
    setting.fileFormat = 0;
    setting.timeRange = ui->comboBoxTimeRange->currentIntData();
    setting.backupTo = 0;
    if (ui->checkBoxExtemalDevice->isChecked()) {
        setting.backupTo |= REPORT_AUTO_BACKUP_TO_DEVICE;
    }
    if (ui->checkBoxEmail->isChecked()) {
        setting.backupTo |= REPORT_AUTO_BACKUP_TO_EMAIL;
    }

    switch (type) {
    case REPORT_AUTO_BACKUP_PEOPLECNT_BY_CAMERA: {
        quint64 lineMask = ui->widgetLineType->checkedFlags();
        lineMask |= (lineMask & 1) << 5;
        setting.lineMask = static_cast<int>(lineMask >> 1);
        snprintf(setting.triChannels, sizeof(setting.triChannels), "%s", ui->checkBoxGroupChannel->checkedMask().toStdString().c_str());
        break;
    }
    case REPORT_AUTO_BACKUP_PEOPLECNT_BY_GROUP: {
        snprintf(setting.triGroups, sizeof(setting.triGroups), "%s", ui->checkBoxGroupGroup->checkedMask().toStdString().c_str());
        break;
    }
    case REPORT_AUTO_BACKUP_PEOPLECNT_BY_REGION: {
        snprintf(setting.triChannels, sizeof(setting.triChannels), "%s", ui->checkBoxGroupChannel->checkedMask().toStdString().c_str());
        setting.stayLengthType = ui->comboBoxStay->currentIntData();
        setting.stayLengthValue = ui->comboBoxStayTime->currentIntData();
        break;
    }
    default:
        break;
    }
}

void TabReportAutoBackupSettings::onLanguageChanged()
{
    ui->labelReportType->setText(GET_TEXT("OCCUPANCY/74204", "Report Type"));
    ui->labelReportAutoBackup->setText(GET_TEXT("REPORTAUTOBACKUP/114002", "Report Auto Backup"));
    ui->labelGroup->setText(GET_TEXT("OCCUPANCY/74203", "Group"));
    ui->labelChannel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->labelDay->setText(GET_TEXT("REPORTAUTOBACKUP/114010", "Day"));
    ui->labelTime->setText(GET_TEXT("REPORTAUTOBACKUP/114011", "Time"));
    ui->labelFileFormat->setText(GET_TEXT("REPORTAUTOBACKUP/114001", "File Format"));
    ui->labelBackupTimeRange->setText(GET_TEXT("REPORTAUTOBACKUP/114003", "Backup Time Range"));
    ui->labelBackupTo->setText(GET_TEXT("REPORTAUTOBACKUP/114004", "Backup to"));
    ui->labelLengthOfStay->setText(GET_TEXT("REGIONAL_PEOPLECOUNTING/103102", "Length of Stay"));

    ui->checkBoxEmail->setText(GET_TEXT("REPORTAUTOBACKUP/114012", "Email"));
    ui->checkBoxExtemalDevice->setText(GET_TEXT("REPORTAUTOBACKUP/114008", "External Device"));

    ui->pushButtonApply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->labelLine->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/145003", "Line"));
}

void TabReportAutoBackupSettings::on_comboBoxType_indexSet(int index)
{
    if (m_currentType >= 0) {
        saveSetting(m_currentType);
    }
    m_currentType = ui->comboBoxType->itemData(index).toInt();
    switch (m_currentType) {
    case REPORT_AUTO_BACKUP_PEOPLECNT_BY_CAMERA:
        setChannelVisible(true);
        setGroupVisible(false);
        setLengthOfStayVisible(false);
        setLineVisible(true);
        break;
    case REPORT_AUTO_BACKUP_PEOPLECNT_BY_GROUP:
        setChannelVisible(false);
        setGroupVisible(true);
        setLengthOfStayVisible(false);
        setLineVisible(false);
        break;
    case REPORT_AUTO_BACKUP_PEOPLECNT_BY_REGION:
        setChannelVisible(true);
        setGroupVisible(false);
        setLengthOfStayVisible(true);
        setLineVisible(false);
        break;
    default:
        break;
    }
    const REPORT_AUTO_BACKUP_SETTING_S &setting = m_settings.settings[m_currentType];
    ui->comboBoxAutoBackup->setCurrentIndexFromData(setting.enable);
    ui->checkBoxGroupChannel->setCheckedFromString(setting.triChannels);
    ui->checkBoxGroupGroup->setCheckedFromString(setting.triGroups);
    ui->comboBoxDay->setCurrentIndexFromData(setting.backupDay);
    ui->timeEdit->setTime(QTime::fromString(setting.backupTime, "HH:mm:ss"));
    ui->comboBoxTimeRange->setCurrentIndexFromData(setting.timeRange);
    ui->checkBoxExtemalDevice->setChecked(setting.backupTo & REPORT_AUTO_BACKUP_TO_DEVICE);
    ui->checkBoxEmail->setChecked(setting.backupTo & REPORT_AUTO_BACKUP_TO_EMAIL);
    ui->comboBoxStay->setCurrentIndexFromData(setting.stayLengthType);
    ui->comboBoxStayTime->setCurrentIndexFromData(setting.stayLengthValue);
    int lineMask = setting.lineMask << 1;
    lineMask |= (lineMask & (1 << 5)) >> 5;
    ui->widgetLineType->setCheckedFromInt(static_cast<quint32>(lineMask));
}

void TabReportAutoBackupSettings::on_comboBoxStay_indexSet(int index)
{
    int stay = ui->comboBoxStay->itemData(index).toInt();
    switch (stay) {
    case 0:
        ui->comboBoxStayTime->setVisible(false && ui->comboBoxStay->isVisible());
        break;
    default:
        ui->comboBoxStayTime->setVisible(true && ui->comboBoxStay->isVisible());
        break;
    }
}

void TabReportAutoBackupSettings::on_comboBoxDay_indexSet(int index)
{
    ui->comboBoxTimeRange->beginEdit();
    ui->comboBoxTimeRange->clear();
    switch (index) {
    case 0:
        ui->comboBoxTimeRange->addItem(GET_TEXT("REPORTAUTOBACKUP/114005", "Last day"), 1);
        break;
    default:
        ui->comboBoxTimeRange->addItem(GET_TEXT("REPORTAUTOBACKUP/114006", "Last week"), 2);
        break;
    }
    ui->comboBoxTimeRange->addItem(GET_TEXT("REPORTAUTOBACKUP/114007", "Export All"), 0);
    ui->comboBoxTimeRange->endEdit();
}

void TabReportAutoBackupSettings::on_pushButtonApply_clicked()
{
    //showWait();
    saveSetting(ui->comboBoxType->currentIntData());
    int result = write_peoplecnt_auto_backup(SQLITE_FILE_NAME, &m_settings);
    QString text = QString("\n==write_peoplecnt_auto_backup, result:%1==").arg(result);
    for (int i = 0; i < MAX_REPORT_AUTO_BACKUP_TYPE_COUNT; ++i) {
        REPORT_AUTO_BACKUP_SETTING_S &setting = m_settings.settings[i];
        text += QString("\n--type:%1").arg(i);
        text += QString("\n----enable:%1").arg(setting.enable);
        text += QString("\n----triChannels:%1").arg(setting.triChannels);
        text += QString("\n----triGroups:%1").arg(setting.triGroups);
        text += QString("\n----stayLengthType:%1").arg(setting.stayLengthType);
        text += QString("\n----stayLengthValue:%1").arg(setting.stayLengthValue);
        text += QString("\n----backupDay:%1").arg(setting.backupDay);
        text += QString("\n----backupTime:%1").arg(setting.backupTime);
        text += QString("\n----fileFormat:%1").arg(setting.fileFormat);
        text += QString("\n----timeRange:%1").arg(setting.timeRange);
        text += QString("\n----backupTo:%1").arg(setting.backupTo);
    }
    qMsDebug() << qPrintable(text);
    sendMessage(REQUEST_FLAG_SET_REPORT_AUTO_BACKUP_SETTINGS, nullptr, 0);
    gEventLoopExec();
    //closeWait();
}

void TabReportAutoBackupSettings::on_pushButtonBack_clicked()
{
    back();
}

void TabReportAutoBackupSettings::on_comboBoxAutoBackup_indexSet(int index)
{
    ui->checkBoxGroupChannel->setEnabled(index);
    ui->checkBoxGroupGroup->setEnabled(index);
    ui->comboBoxDay->setEnabled(index);
    ui->widgetLengthOfStay->setEnabled(index);
    ui->timeEdit->setEnabled(index);

    ui->comboBoxTimeRange->setEnabled(index);
    ui->widgetLineType->setEnabled(index);
    ui->widgetBackupto->setEnabled(index);
}
