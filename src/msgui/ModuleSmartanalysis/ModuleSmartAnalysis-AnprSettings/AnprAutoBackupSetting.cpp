#include "AnprAutoBackupSetting.h"
#include "ui_AnprAutoBackupSetting.h"
#include "MessageBox.h"
#include "MessageFilter.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "MyFileModel.h"
#include "centralmessage.h"
#include <QElapsedTimer>

AnprAutoBackupSetting::AnprAutoBackupSetting(QWidget *parent)
    : AnprBasePage(parent)
    , ui(new Ui::AnprAutoBackupSetting)
{
    ui->setupUi(this);
    ui->comboBoxAutoBackup->beginEdit();
    ui->comboBoxAutoBackup->clear();
    ui->comboBoxAutoBackup->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBoxAutoBackup->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
    ui->comboBoxAutoBackup->endEdit();

    ui->comboBoxDay->beginEdit();
    ui->comboBoxDay->addItem(GET_TEXT("AUTOREBOOT/78001", "Everyday"));
    ui->comboBoxDay->endEdit();

    ui->timeEditBackupTime->setEnabled(false);
    ui->checkboxGroup->setCountFromChannelName(qMsNvr->maxChannel());
    ui->lineEditBackupStatus->setEnabled(false);
    ui->lineEditLastBacupStatus->setEnabled(false);

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimerout()));
    m_timer->setInterval(5000);

    m_esata_usb = new resp_usb_info;
    m_backup_status = new resp_esata_backup_status;
    m_esata_backup = new esata_auto_backup;

    onLanguageChanged();
}

AnprAutoBackupSetting::~AnprAutoBackupSetting()
{
    if (m_esata_backup) {
        delete m_esata_backup;
        m_esata_backup = nullptr;
    }
    if (m_backup_status) {
        delete m_backup_status;
        m_backup_status = nullptr;
    }
    if (m_esata_backup) {
        delete m_esata_backup;
        m_esata_backup = nullptr;
    }
    delete ui;
}

void AnprAutoBackupSetting::initializeData(int channel)
{
    Q_UNUSED(channel);
    read_anpr_auto_backup(SQLITE_FILE_NAME, m_esata_backup);
    ui->timeEditBackupTime->setDateTime(QDateTime::fromString(m_esata_backup->backup_time, "hh:mm:ss"));
    ui->checkboxGroup->setCheckedFromString(m_esata_backup->tri_channels);
    ui->lineEditBackupStatus->clear();
    ui->lineEditLastBacupStatus->clear();

    ui->checkBoxPicture->setChecked(m_esata_backup->file_type & (1 << 2));
    ui->checkBoxPlateList->setChecked(m_esata_backup->file_type & (1 << 3));

    //showWait();

    sendMessage(REQUEST_FLAG_GET_EXPORT_DISK, nullptr, 0);
    //m_eventLoop.exec();
    sendMessage(REQUEST_FLAG_GET_ESATA_BACKUP_STATUS, nullptr, 0);
    //m_eventLoop.exec();

    ui->comboBoxAutoBackup->setCurrentIndex(m_esata_backup->enable);

    //closeWait();
}

void AnprAutoBackupSetting::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_EXPORT_DISK:
        ON_RESPONSE_FLAG_GET_EXPORT_DISK(message);
        break;
    case RESPONSE_FLAG_FORMAT_ESATA_DISK:
        ON_RESPONSE_FLAG_FORMAT_ESATA_DISK(message);
        break;
    case RESPONSE_FLAG_GET_ESATA_BACKUP_STATUS:
        ON_RESPONSE_FLAG_GET_ESATA_BACKUP_STATUS(message);
        break;
    case RESPONSE_FLAG_SET_ESATA_AUTO_BACKUP:
        ON_RESPONSE_FLAG_SET_ESATA_AUTO_BACKUP(message);
        break;
    case RESPONSE_FLAG_FORMAT_EXPORT_DISK:
        ON_RESPONSE_FLAG_FORMAT_EXPORT_DISK(message);
        break;
    case RESPONSE_FLAG_FORMAT_MSFS_DISK:
        ON_RESPONSE_FLAG_FORMAT_MSFS_DISK(message);
        break;
    default:
        break;
    }
}

void AnprAutoBackupSetting::ON_RESPONSE_FLAG_GET_EXPORT_DISK(MessageReceive *message)
{
    struct resp_usb_info *usb_info_list = (struct resp_usb_info *)message->data;
    int count = message->header.size / sizeof(struct resp_usb_info);

    memset(m_esata_usb, 0, sizeof(resp_usb_info));
    m_usbInfoMap.clear();
    m_nasInfoMap.clear();
    m_diskMap.clear();
    ui->comboBoxStarageDevice->beginEdit();
    ui->comboBoxStarageDevice->clear();
    ui->comboBoxStarageDevice->addItem(GET_TEXT("PLAYBACK/80049", "Select Device"), 0);
    for (int i = 0; i < count; ++i) {
        const resp_usb_info &usb_info = usb_info_list[i];
        switch (usb_info.type) {
        case DISK_TYPE_ESATA: {
            if (m_esata_usb->type == DISK_TYPE_UNKNOWN) {
                memcpy(m_esata_usb, &usb_info, sizeof(resp_usb_info));
                ui->comboBoxStarageDevice->addItem("eSATA", m_esata_usb->port);
                m_diskMap.insert(usb_info.port, usb_info);
            }
            break;
        }
        case DISK_TYPE_NAS: {
            m_nasInfoMap.insert(usb_info.port, usb_info);
            m_diskMap.insert(usb_info.port, usb_info);
            break;
        }
        case DISK_TYPE_USB: {
            m_usbInfoMap.insert(usb_info.port, usb_info);
            m_diskMap.insert(usb_info.port, usb_info);
            break;
        }
        default:
            break;
        }
    }
    for (auto iter = m_usbInfoMap.begin(); iter != m_usbInfoMap.end(); ++iter) {
        QString itemName = QString(iter.value().dev_name).trimmed() + QString(" (USB Port%1)").arg(iter.value().port);
        ui->comboBoxStarageDevice->addItem(itemName, iter.key());
    }
    for (auto iter = m_nasInfoMap.begin(); iter != m_nasInfoMap.end(); ++iter) {
        ui->comboBoxStarageDevice->addItem(QString(iter.value().dev_name).trimmed(), iter.key());
    }
    ui->comboBoxStarageDevice->endEdit();
    ui->comboBoxStarageDevice->setCurrentIndexFromData(m_esata_backup->port);

    m_eventLoop.exit();
}

void AnprAutoBackupSetting::ON_RESPONSE_FLAG_SET_ESATA_AUTO_BACKUP(MessageReceive *message)
{
    int result = *((int *)message->data);
    if (result < 0) {
        qMsWarning() << QString("result: %1").arg(result);
    }
    m_eventLoop.exit(result);
}

void AnprAutoBackupSetting::esataUpdate()
{
    if (!m_esata_backup->enable) {
        return;
    }
    switch (m_backup_status->status) {
    case ESATA_BACKUP_INIT:
        ui->lineEditBackupStatus->setText("");
        break;
    case ESATA_BACKUP_NO_DEVICE:
        ui->lineEditBackupStatus->setText(GET_TEXT("AUTOBACKUP/149000", "No Selected Storage Device"));
        break;
    case ESATA_BACKUP_NO_FORMAT:
        ui->lineEditBackupStatus->setText(GET_TEXT("AUTOBACKUP/95211", "Unsupported Storage Device Format"));
        break;
    case ESATA_BACKUP_STANDBY:
        ui->lineEditBackupStatus->setText(GET_TEXT("AUTOBACKUP/95208", "Standby"));
        break;
    case ESATA_BACKUP_WORKING:
        ui->lineEditBackupStatus->setText(GET_TEXT("AUTOBACKUP/95209", "Working"));
    default:
        break;
    }
    ui->lineEditLastBacupStatus->setText(QString(m_backup_status->last_time));
}

void AnprAutoBackupSetting::ON_RESPONSE_FLAG_FORMAT_ESATA_DISK(MessageReceive *message)
{
    Q_UNUSED(message)

    int result = *((int *)message->data);
    qMsDebug() << "result:" << result;
    m_eventLoop.exit(result);
}

void AnprAutoBackupSetting::ON_RESPONSE_FLAG_FORMAT_MSFS_DISK(MessageReceive *message)
{
    if (!message->data) {
        qWarning() << "ON_RESPONSE_FLAG_FORMAT_MSFS_DISK, data is null.";
        return;
    }
    const int result = (*(int *)(message->data));
    switch (result) {
    case -1:
        ShowMessageBox(GET_TEXT("AUTOBACKUP/149002", "Failed to format."));
        break;
    case MF_NAS_LOW_CAP:
        ShowMessageBox(GET_TEXT("AUTOBACKUP/149002", "Failed to format."));
        break;
    }
    m_eventLoop.exit();
}

void AnprAutoBackupSetting::ON_RESPONSE_FLAG_FORMAT_EXPORT_DISK(MessageReceive *message)
{
    int result = -1;
    if (message->data) {
        result = (*(int *)(message->data));
    }
    if (result == -1) {
        ShowMessageBox(GET_TEXT("AUTOBACKUP/149002", "Failed to format."));
    }
    m_eventLoop.exit();
}

void AnprAutoBackupSetting::ON_RESPONSE_FLAG_GET_ESATA_BACKUP_STATUS(MessageReceive *message)
{
    if (m_isFormatting) {
        return;
    }
    m_eventLoop.exit();

    if (message->isNull()) {
        qMsWarning() << message;
        return;
    }
    resp_esata_backup_status *info = static_cast<resp_esata_backup_status *>(message->data);

    QString text;
    text.append(QString("\n--status: %1").arg(info->status));
    text.append(QString("\n--last_time: %1").arg(info->last_time));
    text.append(QString("\n--percent: %1").arg(info->percent));
    text.append(QString("\n--capacity: %1").arg(info->capacity));
    qMsCDebug("qt_autobackup_status") << qPrintable(text);

    memcpy(m_backup_status, info, sizeof(resp_esata_backup_status));
    esataUpdate();
}

void AnprAutoBackupSetting::setEnabled(bool enabled)
{
    ui->timeEditBackupTime->setEnabled(enabled);
    ui->checkboxGroup->setEnabled(enabled);
    ui->comboBoxDay->setEnabled(enabled);
    ui->checkBoxPicture->setEnabled(enabled);
    ui->checkBoxPlateList->setEnabled(enabled);

    bool starageDeviceEnable = ui->comboBoxStarageDevice->count() > 1 || ui->comboBoxStarageDevice->currentIntData() != 0;
    ui->comboBoxStarageDevice->setEnabled(starageDeviceEnable && enabled);
    ui->pushButtonFormat->setEnabled(ui->comboBoxStarageDevice->isEnabled());
}

void AnprAutoBackupSetting::onLanguageChanged()
{
    ui->labelAutoBackup->setText(GET_TEXT("AUTOBACKUP/95200", "Auto Backup"));
    ui->labelStarageDevice->setText(GET_TEXT("AUTOBACKUP/95201", "Storage Device"));
    ui->labelBackupTime->setText(GET_TEXT("OCCUPANCY/74227", "Time"));
    ui->labelDay->setText(GET_TEXT("OCCUPANCY/74226", "Day"));
    ui->labelBackupChannel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->labelBackupFileType->setText(GET_TEXT("AUTOBACKUP/95216", "Backup File Type"));
    ui->label_10->setText(GET_TEXT("AUTOBACKUP/172004", "Note: The latest 24 hour's pictures and plate lists will be backed up."));
    ui->pushButtonApply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButtonFormat->setText(GET_TEXT("DISKMANAGE/72051", "Format"));
    ui->checkBoxPlateList->setText(GET_TEXT("ANPR/103059", "Plate List"));
    ui->checkBoxPicture->setText(GET_TEXT("ANPR/103061", "Picture"));
}

void AnprAutoBackupSetting::on_pushButtonBack_clicked()
{
    back();
}

void AnprAutoBackupSetting::onTimerout()
{
    if (m_isFormatting) {
        return;
    }
    sendMessage(REQUEST_FLAG_GET_ESATA_BACKUP_STATUS, nullptr, 0);
}

void AnprAutoBackupSetting::on_pushButtonApply_clicked()
{
    ui->pushButtonApply->clearUnderMouse();

    if (ui->comboBoxAutoBackup->currentIndex()) {
        if (!ui->checkboxGroup->hasChannelSelected() && ui->comboBoxStarageDevice->currentIntData() > 0) {
            ShowMessageBox(GET_TEXT("PLAYBACK/80028", "Please select at least one channel."));
            return;
        }
    }
    //showWait();
    if (ui->comboBoxAutoBackup->currentIndex()) {
        esata_auto_backup estainfo;
        memset(&estainfo, 0, sizeof(esata_auto_backup));
        read_esata_auto_backup(SQLITE_FILE_NAME, &estainfo);
        if (estainfo.enable) {
            MessageBox::Result result = MessageBox::question(this, GET_TEXT("AUTOBACKUP/172001", "Auto Backup will be disabled,continue?"));
            if (result == MessageBox::Cancel) {
                //closeWait();
                return;
            }
        } else {
            sendMessage(REQUEST_FLAG_GET_ESATA_BACKUP_STATUS, nullptr, 0);
            //m_eventLoop.exec();
            if (m_backup_status->status == ESATA_BACKUP_WORKING) {
                MessageBox::Result result = MessageBox::question(this, GET_TEXT("AUTOBACKUP/95213", "Current backup will be stopped if settings is changed, continue?"));
                if (result == MessageBox::Cancel) {
                    //closeWait();
                    return;
                }
            }
        }
    }
    m_esata_backup->enable = ui->comboBoxAutoBackup->currentIndex();

    int port = ui->comboBoxStarageDevice->currentIntData();
    if (m_diskMap.find(port) != m_diskMap.end()) {
        switch (m_diskMap.value(port).type) {
        case DISK_TYPE_ESATA: {
            m_esata_backup->device = AUTO_BACKUP_ESATA;
            break;
        }
        case DISK_TYPE_NAS: {
            m_esata_backup->device = AUTO_BACKUP_NAS;
            break;
        }
        case DISK_TYPE_USB: {
            m_esata_backup->device = AUTO_BACKUP_USB;
            break;
        }
        default:
            break;
        }
        m_esata_backup->port = port;
    } else if (port == 0) {
        m_esata_backup->port = port;
        m_esata_backup->device = AUTO_BACKUP_SELECT;
    }
    m_esata_backup->backup_day = 0;
    snprintf(m_esata_backup->backup_time, sizeof(m_esata_backup->backup_time), "%s", ui->timeEditBackupTime->text().toStdString().c_str());
    m_esata_backup->file_type = 0;
    if (ui->checkBoxPlateList->isChecked()) {
        m_esata_backup->file_type |= 1 << 3;
    }
    if (ui->checkBoxPicture->isChecked()) {
        m_esata_backup->file_type |= 1 << 2;
    }
    m_esata_backup->recycle = 1;
    snprintf(m_esata_backup->tri_channels, sizeof(m_esata_backup->tri_channels), ui->checkboxGroup->checkedMask().toStdString().c_str());

    QString text;
    text.append(QString("\n--enable: %1").arg(m_esata_backup->enable));
    text.append(QString("\n--backup_day: %1").arg(m_esata_backup->backup_day));
    text.append(QString("\n--backup_time: %1").arg(m_esata_backup->backup_time));
    text.append(QString("\n--stream_type: %1").arg(m_esata_backup->stream_type));
    text.append(QString("\n--file_type: %1").arg(m_esata_backup->file_type));
    text.append(QString("\n--recycle: %1").arg(m_esata_backup->recycle));
    text.append(QString("\n--tri_channels(64<->1): %1").arg(m_esata_backup->tri_channels));
    qMsDebug() << qPrintable(text);

    write_anpr_auto_backup(SQLITE_FILE_NAME, m_esata_backup);

}

void AnprAutoBackupSetting::on_pushButtonFormat_clicked()
{
    ui->pushButtonFormat->clearUnderMouse();

    int port = ui->comboBoxStarageDevice->currentIntData();
    if (m_diskMap.find(port) == m_diskMap.end()) {
        ShowMessageBox(GET_TEXT("AUTOBACKUP/149001", "Failed to format because no selected storage device."));
        return;
    }
    MessageBox::Result result = MessageBox::question(this, GET_TEXT("AUTOBACKUP/95214", "Data will be erased after formatting, continue?"));
    if (result == MessageBox::Cancel) {
        return;
    }

    //showWait();

    m_isFormatting = true;

    const resp_usb_info &usb_info = m_diskMap[port];
    switch (usb_info.type) {
    case DISK_TYPE_ESATA: {
        QMS_ASSERT(25 == usb_info.port);
        sendMessage(REQUEST_FLAG_FORMAT_ESATA_DISK, &usb_info.port, sizeof(int));
        //m_eventLoop.exec();
        break;
    }
    case DISK_TYPE_NAS: {
        sendMessage(REQUEST_FLAG_FORMAT_MSFS_DISK, &usb_info.port, sizeof(int));
        //m_eventLoop.exec();
        break;
    }
    case DISK_TYPE_USB: {
        struct req_usb_info req_usb_info;
        memset(&req_usb_info, 0, sizeof(req_usb_info));
        snprintf(req_usb_info.dev_path, sizeof(usb_info.dev_path), "%s", usb_info.dev_path);
        snprintf(req_usb_info.dev_name, sizeof(usb_info.dev_name), "%s", usb_info.dev_name);
        req_usb_info.formatType = FORMAT_FAT32_TYPE;
        sendMessage(REQUEST_FLAG_FORMAT_EXPORT_DISK, &req_usb_info, sizeof(struct req_usb_info));
        //m_eventLoop.exec();
        break;
    }
    default:
        break;
    }
    m_isFormatting = false;

    sendMessage(REQUEST_FLAG_GET_EXPORT_DISK, nullptr, 0);
    //m_eventLoop.exec();

    sendMessage(REQUEST_FLAG_GET_ESATA_BACKUP_STATUS, nullptr, 0);
    //m_eventLoop.exec();

    ui->comboBoxStarageDevice->setCurrentIndexFromData(port);

    //closeWait();
}
void AnprAutoBackupSetting::on_comboBoxAutoBackup_indexSet(int index)
{
    if (index) {
        setEnabled(true);
    } else {
        setEnabled(false);
    }
}

void AnprAutoBackupSetting::showEvent(QShowEvent *event)
{
    if (!m_timer->isActive()) {
        m_timer->start();
    }
    AnprBasePage::showEvent(event);
}

void AnprAutoBackupSetting::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    AnprBasePage::hideEvent(event);
}