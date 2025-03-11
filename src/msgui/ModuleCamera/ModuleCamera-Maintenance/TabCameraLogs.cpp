#include "TabCameraLogs.h"
#include "ui_TabCameraLogs.h"

#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "MyFileSystemDialog.h"
#include "centralmessage.h"

extern "C" {
#include "msg.h"
#include "recortsp.h"
}

const int CameraInfoRole = Qt::UserRole + 100;
TabCameraLogs::TabCameraLogs(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabCameraLogs)
{
    ui->setupUi(this);
    ui->comboBoxMainType->beginEdit();
    ui->comboBoxMainType->clear();
    ui->comboBoxMainType->addItem(GET_TEXT("CAMERAMAINTENANCE/152009", "All Types"), IPC_LOG_MAIN_ALL);
    ui->comboBoxMainType->addItem(GET_TEXT("CAMERAMAINTENANCE/152010", "Event"), IPC_LOG_MAIN_EVENT);
    ui->comboBoxMainType->addItem(GET_TEXT("CAMERAMAINTENANCE/152011", "Operation"), IPC_LOG_MAIN_OP);
    ui->comboBoxMainType->addItem(GET_TEXT("CAMERAMAINTENANCE/152012", "Information"), IPC_LOG_MAIN_INFO);
    ui->comboBoxMainType->addItem(GET_TEXT("CAMERAMAINTENANCE/152013", "Exception"), IPC_LOG_MAIN_EXCEPT);
    ui->comboBoxMainType->addItem(GET_TEXT("CAMERAMAINTENANCE/152014", "Smart"), IPC_LOG_MAIN_SMART);
    ui->comboBoxMainType->endEdit();

    ui->tableView->setHorizontalHeaderItem(MaintenanceTableView::ColumnProgress, QString(GET_TEXT("CAMERAMAINTENANCE/152000", "Backup Progress")));
    ui->lineEditExport->setCheckMode(MyLineEdit::EmptyCheck);
    onLanguageChanged();
}

TabCameraLogs::~TabCameraLogs()
{
    delete ui;
}

void TabCameraLogs::initializeData()
{
    ui->comboBoxMainType->setCurrentIndexFromData(0);
    ui->dateEditStart->setDate(QDate::currentDate());
    ui->timeEditStart->setTime(QTime(0, 0));
    ui->dateEditEnd->setDate(QDate::currentDate());
    ui->timeEditEnd->setTime(QTime(23, 59, 59));
    ui->lineEditExport->clear();
    ui->tableView->scrollToTop();
    ui->tableView->clearSort();

    m_stateMap.clear();
    //showWait();
    sendMessage(REQUEST_FLAG_GET_IPCLIST, nullptr, 0);
}

void TabCameraLogs::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPCLIST:
        ON_RESPONSE_FLAG_GET_IPCLIST(message);
        break;
    case RESPONSE_FLAG_GET_IPC_LOG:
        ON_RESPONSE_FLAG_GET_IPC_LOG(message);
        break;
    }
}

bool TabCameraLogs::isCloseable()
{
    if (ui->pushButtonBackup->isEnabled()) {
        return true;
    }
    int result = MessageBox::question(this, GET_TEXT("CAMERAMAINTENANCE/38010", "Exit will stop all unfinished progress, continue?"));
    if (result == MessageBox::Cancel) {
        return false;
    }

    if (ui->pushButtonBackup->isEnabled()) {
        return true;
    }
    m_isCancel = true;
    m_isAboutToClose = true;
    //showWait();
    return false;
}

bool TabCameraLogs::isChangeable()
{
    if (ui->pushButtonBackup->isEnabled()) {
        return true;
    }
    int result = MessageBox::question(this, GET_TEXT("CAMERAMAINTENANCE/38010", "Exit will stop all unfinished progress, continue?"));
    if (result == MessageBox::Cancel) {
        return false;
    } else {
        m_isCancel = true;
        m_isAboutToClose = false;
        //showWait();
        return true;
    }
}

bool TabCameraLogs::canAutoLogout()
{
    if (ui->pushButtonBackup->isEnabled()) {
        return true;
    } else {
        return false;
    }
}

void TabCameraLogs::onLanguageChanged()
{
    ui->labelMainType->setText(GET_TEXT("LOG/64003", "Main Type"));
    ui->labelSubType->setText(GET_TEXT("LOG/64004", "Sub Type"));
    ui->labelStart->setText(GET_TEXT("LOG/64001", "Start Time"));
    ui->labelEnd->setText(GET_TEXT("LOG/64002", "End Time"));
    ui->labelExport->setText(GET_TEXT("SYSTEMGENERAL/70063", "Export Directory"));

    ui->pushButtonBackup->setText(GET_TEXT("PROFILE/76001", "Backup"));
    ui->pushButtonBrowse->setText(GET_TEXT("CAMERAMAINTENANCE/38004", "Browse"));

    ui->pushButtonRefresh->setText(GET_TEXT("COMMON/1035", "Refresh"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
}

void TabCameraLogs::ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message)
{
    //closeWait();

    m_allCameraList.clear();
    resq_get_ipcdev *ipcdev_array = (resq_get_ipcdev *)message->data;
    int count = message->header.size / sizeof(resq_get_ipcdev);
    for (int i = 0; i < count; ++i) {
        const resq_get_ipcdev &ipcdev = ipcdev_array[i];
        //不显示第三方
        if (!QString(ipcdev.sn).isEmpty() && QString(ipcdev.sn) != QString("TX_MS20119_X01201112250001000100010")) {
            continue;
        }

        //RTSP forbidden
        if (ipcdev.protocol == IPC_PROTOCOL_RTSP)
            continue;

        m_allCameraList.append(ipcdev);
    }
    ui->tableView->refreshTable(m_allCameraList, m_stateMap);
    if (!ui->pushButtonBackup->isEnabled()) {
        ui->tableView->setCheckListEnable(false);
    }
}

void TabCameraLogs::ON_RESPONSE_FLAG_GET_IPC_LOG(MessageReceive *message)
{
    if (!message->data) {
        m_eventLoop.exit(-1);
        return;
    }
    int result = *(static_cast<int *>(message->data));
    return m_eventLoop.exit(result);
}

void TabCameraLogs::setUpgradeEnabled(bool enable)
{
    ui->tableView->setCheckListEnable(enable);
    ui->pushButtonBrowse->setEnabled(enable);
    ui->pushButtonBackup->setEnabled(enable);
    ui->comboBoxMainType->setEnabled(enable);
    ui->comboBoxSubType->setEnabled(enable);
    ui->timeEditStart->setEnabled(enable);
    ui->dateEditStart->setEnabled(enable);
    ui->timeEditEnd->setEnabled(enable);
    ui->dateEditEnd->setEnabled(enable);
    ui->pushButtonRefresh->clearHover();
}

void TabCameraLogs::on_pushButtonBrowse_clicked()
{
    ui->pushButtonBrowse->clearUnderMouse();
    ui->pushButtonBrowse->clearFocus();

    QString filePath = MyFileSystemDialog::instance()->getOpenDirectory();
    if (filePath.isEmpty()) {
        ui->lineEditExport->clear();
        return;
    }
    ui->lineEditExport->setText(filePath);
}

void TabCameraLogs::on_pushButtonBackup_clicked()
{
    if (!ui->lineEditExport->checkValid()) {
        ui->lineEditExport->setCustomValid(false, GET_TEXT("MYLINETIP/112001", "Invalid."));
        return;
    }
    m_filePath = ui->lineEditExport->text();

    const QDateTime startDateTime(ui->dateEditStart->date(), ui->timeEditStart->time());
    const QDateTime endDateTime(ui->dateEditEnd->date(), ui->timeEditEnd->time());
    if (startDateTime > endDateTime) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100031", "End time must be later than start time."));
        return;
    }

    m_channelMap.clear();
    for (int i = 0; i < ui->tableView->rowCount(); ++i) {
        if (ui->tableView->isItemChecked(i)) {
            const resq_get_ipcdev &ipcdev = ui->tableView->itemData(i, MaintenanceTableView::ColumnCheck, CameraInfoRole).value<resq_get_ipcdev>();
            m_channelMap.insert(ipcdev.chanid, 0);
        }
    }
    if (m_channelMap.isEmpty()) {
        ShowMessageBox(GET_TEXT("CAMERAMAINTENANCE/152003", "Please select at least one device."));
        return;
    }

    setUpgradeEnabled(false);
    for (int i = 0; i < ui->tableView->rowCount(); ++i) {
        const resq_get_ipcdev &ipcdev = ui->tableView->itemData(i, MaintenanceTableView::ColumnCheck, CameraInfoRole).value<resq_get_ipcdev>();
        MaintenanceTablelItemInfo &maintenanceTablelItemInfo = m_stateMap[ipcdev.chanid];
        if (ui->tableView->isItemChecked(i)) {
            maintenanceTablelItemInfo.isChecked = true;
            MsCameraVersion cameraVersion = qMsNvr->cameraVersion(ipcdev.chanid);
            if (ipcdev.state == RTSP_CLIENT_CONNECT && cameraVersion >= MsCameraVersion(7, 76)) {
                maintenanceTablelItemInfo.state = GET_TEXT("UPGRADE/75027", "Waiting");
            } else {
                maintenanceTablelItemInfo.state = GET_TEXT("UPGRADE/75024", "Failure");
            }
            ui->tableView->setItemText(i, MaintenanceTableView::ColumnProgress, GET_TEXT("UPGRADE/75027", "Waiting"));
        } else {
            maintenanceTablelItemInfo.isChecked = false;
        }
    }

    ReqIpcLog reqIpcLog;
    memset(&reqIpcLog, 0, sizeof(ReqIpcLog));
    snprintf(reqIpcLog.exportPath, sizeof(reqIpcLog.exportPath), "%s", m_filePath.toStdString().c_str());
    snprintf(reqIpcLog.startTime, sizeof(reqIpcLog.startTime), "%s", startDateTime.toString("yyyy-MM-dd-HH:mm:ss").toStdString().c_str());
    snprintf(reqIpcLog.endTime, sizeof(reqIpcLog.endTime), "%s", endDateTime.toString("yyyy-MM-dd-HH:mm:ss").toStdString().c_str());
    reqIpcLog.mainType = static_cast<IPC_LOG_MAIN_E>(ui->comboBoxMainType->currentIntData());
    reqIpcLog.subType = static_cast<IPC_LOG_SUB_E>(ui->comboBoxSubType->currentIntData());

    for (auto iter = m_channelMap.constBegin(); iter != m_channelMap.constEnd(); ++iter) {
        if (m_isCancel) {
            break;
        }
        int channel = iter.key();

        if (m_stateMap.value(channel).state == GET_TEXT("UPGRADE/75024", "Failure")) {
            ui->tableView->setUpgradeProgressText(m_stateMap, channel, GET_TEXT("UPGRADE/75024", "Failure"));
            continue;
        }

        reqIpcLog.chnId = channel;
        STRUCT(ReqIpcLog, &reqIpcLog,
               FIELD(int, chnId);
               FIELD(int, mainType);
               FIELD(int, subType);
               FIELD(char *, startTime);
               FIELD(char *, endTime);
               FIELD(char *, exportPath));
        sendMessage(REQUEST_FLAG_GET_IPC_LOG, &reqIpcLog, sizeof(ReqIpcLog));
        ui->tableView->setUpgradeProgressText(m_stateMap, channel, GET_TEXT("CAMERAMAINTENANCE/152081", "Exporting"));
        // int result = m_eventLoop.exec();
        // switch (result) {
        // case 0:
        //     ui->tableView->setUpgradeProgressText(m_stateMap, channel, GET_TEXT("UPGRADE/75023", "Success"));
        //     break;
        // case -1:
        //     ui->tableView->setUpgradeProgressText(m_stateMap, channel, GET_TEXT("UPGRADE/75024", "Failure"));
        //     break;
        // case -2:
        //     ui->tableView->setUpgradeProgressText(m_stateMap,channel, GET_TEXT("CAMERAMAINTENANCE/152083", "No data"));
        //     break;
        // }
    }
    setUpgradeEnabled(true);

    if (m_isCancel) {
        //closeWait();
        m_isCancel = false;
        if (m_isAboutToClose) {
            back();
        } else {
            initializeData();
        }
    }
}

void TabCameraLogs::on_pushButtonBack_clicked()
{
    if (isCloseable()) {
        back();
    }
}

void TabCameraLogs::on_comboBoxMainType_indexSet(int index)
{
    ui->comboBoxSubType->beginEdit();
    ui->comboBoxSubType->clear();
    ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152015", "All"), IPC_LOG_SUB_ALL);
    switch (index) {
    case IPC_LOG_MAIN_EVENT:
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152016", "Motion Detection Start"), IPC_LOG_SUB_EVENT_MOTION_START);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152017", "Motion Detection Stop"), IPC_LOG_SUB_EVENT_MOTION_STOP);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152018", "External Input Alarm"), IPC_LOG_SUB_EVENT_ALARM_IN);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152019", "External Output Alarm"), IPC_LOG_SUB_EVENT_ALARM_OUT);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152020", "Audio Input Alarm"), IPC_LOG_SUB_EVENT_AUDIO_IN);
        break;
    case IPC_LOG_MAIN_OP:
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152021", "Reboot Remotely"), IPC_LOG_SUB_OP_REBOOT_REMOTE);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152022", "Login Remotely"), IPC_LOG_SUB_OP_LOGIN_REMOTE);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152023", "Logout Remotely"), IPC_LOG_SUB_OP_LOGOUT_REMOTE);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152024", "Upgrade Remotely"), IPC_LOG_SUB_OP_UPGRADE_REMOTE);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152025", "Config Remotely"), IPC_LOG_SUB_OP_CONFIG_REMOTE);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152026", "PTZ Control Remotely"), IPC_LOG_SUB_OP_PTZ_CONTROL_REMOTE);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152027", "Playback Remotely"), IPC_LOG_SUB_OP_PLAYBACK_REMOTE);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152028", "Export Profile Remotely"), IPC_LOG_SUB_OP_PROFILE_EXPORT_REMOTE);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152029", "Import Profile Remotely"), IPC_LOG_SUB_OP_PROFILE_IMPORT_REMOTE);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152030", "Reset Remotely"), IPC_LOG_SUB_OP_ADMIN_RESET_REMOTE);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152031", "RTSP Session Start"), IPC_LOG_SUB_OP_RTSP_START);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152032", "RTSP Session Stop"), IPC_LOG_SUB_OP_RTSP_STOP);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152033", "Video Param Set Remotely"), IPC_LOG_SUB_OP_SET_VIDEO_PARAM);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152034", "Image Param Se Remotely"), IPC_LOG_SUB_OP_SET_IMAGE_PARAM);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152035", "Vehicle Counting Reset Remotely"), IPC_LOG_SUB_OP_RESET_VEHICLE_COUNT);
        break;
    case IPC_LOG_MAIN_INFO:
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152036", "Record Start"), IPC_LOG_SUB_INFO_RECORD_START);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152037", "Record Stop"), IPC_LOG_SUB_INFO_RECORD_STOP);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152038", "Reset Locally"), IPC_LOG_SUB_INFO_LOCAL_RESET);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152039", "System Restart"), IPC_LOG_SUB_INFO_SYSTEM_RESTART);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152040", "RTSP Over Maximum"), IPC_LOG_SUB_INFO_RTSP_OVER);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152041", "RTSP IP Limit"), IPC_LOG_SUB_INFO_RTSP_IP_LIMIT);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152042", "IR-CUT Off"), IPC_LOG_SUB_INFO_IR_CUT_OFF);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152043", "IR-CUT On"), IPC_LOG_SUB_INFO_IR_CUT_ON);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152044", "IR LED Off"), IPC_LOG_SUB_INFO_IR_CUT_LED_OFF);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152045", "IR LED On"), IPC_LOG_SUB_INFO_IR_CUT_LED_ON);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152046", "White LED Off"), IPC_LOG_SUB_INFO_WHITE_LED_OFF);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152047", "White LED On"), IPC_LOG_SUB_INFO_WHITE_LED_ON);
        break;
    case IPC_LOG_MAIN_EXCEPT:
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152048", "Storage Full"), IPC_LOG_SUB_EXCEPT_DISK_FULL);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152049", "Network Disconnected"), IPC_LOG_SUB_EXCEPT_NETWORK_DISCONNECT);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152050", "IP Address Conflict"), IPC_LOG_SUB_EVENT_IP_CONFLICT);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152051", "Record Failed"), IPC_LOG_SUB_EXCEPT_RECORD_FAIL);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152052", "Snapshot Failed"), IPC_LOG_SUB_EXCEPT_SNAPSHOT_FAIL);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152053", "AV Server Died"), IPC_LOG_SUB_EXCEPT_AVSE_DIE);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152054", "RTSP Server Died"), IPC_LOG_SUB_EXCEPT_RTSP_DIE);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152055", "Web Server Died"), IPC_LOG_SUB_EXCEPT_WEBS_DIE);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152056", "SD Card Uninitialized"), IPC_LOG_SUB_EXCEPT_SD_UNINIT);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152057", "SD Card Error"), IPC_LOG_SUB_EXCEPT_SD_ERR);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152058", "No SD Card"), IPC_LOG_SUB_EXCEPT_NO_SD);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152059", "Upload to FTP Failed"), IPC_LOG_SUB_EXCEPT_FTP_FILE);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152060", "Send Email Failed"), IPC_LOG_SUB_EXCEPT_SMTP_FILE);
        break;
    case IPC_LOG_MAIN_SMART:
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152061", "Region Entrance"), IPC_LOG_SUB_SMART_VCA_INTRUSION_ENTER);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152062", "Region Exit"), IPC_LOG_SUB_SMART_VCA_INTRUSION_EXIT);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152063", "Loitering"), IPC_LOG_SUB_SMART_VCA_LOITERING);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152064", "Advanced Motion Detection"), IPC_LOG_SUB_SMART_VCA_ADVANCED_MOTION);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152065", "Object Left"), IPC_LOG_SUB_SMART_VCA_OBJECT_LEFT);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152066", "Object Removed"), IPC_LOG_SUB_SMART_VCA_OBJECT_REMOVE);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152079", "Line Crossing"), IPC_LOG_SUB_SMART_VCA_LINE_CROSSING);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152075", "Tamper Detection"), IPC_LOG_SUB_SMART_VCA_CTD);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152076", "Human Detection"), IPC_LOG_SUB_SMART_VCA_HUMAN);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152077", "People Counting"), IPC_LOG_SUB_SMART_VCA_COUNTER);
        ui->comboBoxSubType->addItem(GET_TEXT("CAMERAMAINTENANCE/152078", "Regional People Counting"), IPC_LOG_SUB_SMART_REGIONAL_PEOPLE);
        break;
    default:
        break;
    }
    ui->comboBoxSubType->endEdit();
}

void TabCameraLogs::on_pushButtonRefresh_clicked()
{
    //showWait();
    ui->tableView->updateChecked(m_stateMap);
    sendMessage(REQUEST_FLAG_GET_IPCLIST, nullptr, 0);
}
