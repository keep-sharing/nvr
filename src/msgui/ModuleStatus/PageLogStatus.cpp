#include "PageLogStatus.h"
#include "ui_PageLogStatus.h"
#include "LogDetail.h"
#include "MessageBox.h"
#include "MessageFilter.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "MyFileSystemDialog.h"
#include "centralmessage.h"
#include "progressdialog.h"
#include <qmath.h>

extern "C" {
#include "log.h"
}

const int MaxSearchLogCount = 10000;
const int LogIndexRole = Qt::UserRole + 14;

PageLogStatus::PageLogStatus(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::LogStatus)
{
    ui->setupUi(this);

    ui->comboBox_maintype->addItem(GET_TEXT("COMMON/1006", "All"), MAIN_ALL);
    ui->comboBox_maintype->addItem(GET_TEXT("MENU/10005", "Event"), MAIN_EVENT);
    ui->comboBox_maintype->addItem(GET_TEXT("LOG/64073", "Operation"), MAIN_OP);
    ui->comboBox_maintype->addItem(GET_TEXT("LOG/64074", "Information"), MAIN_INFO);
    ui->comboBox_maintype->addItem(GET_TEXT("EXCEPTION/54000", "Exception"), MAIN_EXCEPT);
    ui->comboBox_maintype->addItem(GET_TEXT("LOG/64063", "Debug"), MAIN_DEBUG);

    QStringList tableHeaders;
    tableHeaders << "";
    tableHeaders << GET_TEXT("CAMERASEARCH/32002", "NO.");
    tableHeaders << GET_TEXT("LOG/64003", "Main Type");
    tableHeaders << GET_TEXT("LOG/64005", "Time");
    tableHeaders << GET_TEXT("LOG/64004", "Sub Type");
    tableHeaders << GET_TEXT("LOG/64006", "Parameter");
    tableHeaders << GET_TEXT("CHANNELMANAGE/30008", "Channel");
    tableHeaders << GET_TEXT("LOG/64008", "User");
    tableHeaders << GET_TEXT("LOG/64141", "Remote Host IP");
    tableHeaders << GET_TEXT("LOG/64139", "Details");
    ui->tableView->setHorizontalHeaderLabels(tableHeaders);
    ui->tableView->setColumnCount(tableHeaders.count());
    ui->tableView->hideColumn(ColumnCheck);
    ui->tableView->setHeaderCheckable(false);
    ui->tableView->setItemDelegateForColumn(ColumnDetails, new ItemButtonDelegate(QPixmap(":/status/status/detail.png"), QSize(18, 18), this));
    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onTableItemClicked(int, int)));
    connect(ui->tableView, SIGNAL(itemDoubleClicked(int, int)), this, SLOT(onTableItemDoubleClicked(int, int)));
    ui->tableView->setSortableForColumn(ColumnDetails, false);
    ui->tableView->setSortType(ColumnNumber, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnChannel, SortFilterProxyModel::SortInt);
    ui->tableView->setColumnWidth(ColumnNumber, 100);
    ui->tableView->setColumnWidth(ColumnMainType, 200);
    ui->tableView->setColumnWidth(ColumnTime, 200);
    ui->tableView->setColumnWidth(ColumnSubType, 200);
    ui->tableView->setColumnWidth(ColumnParameter, 200);
    ui->tableView->setColumnWidth(ColumnChannel, 150);
    ui->tableView->setColumnWidth(ColumnUser, 150);
    ui->tableView->setColumnWidth(ColumnRemoteHostIP, 200);

    //
    ui->comboBoxChannel->addItem(GET_TEXT("COMMON/1006", "All"), -2);
    ui->comboBoxChannel->addItem("N/A", -1);
    for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
        ui->comboBoxChannel->addItem(QString("%1").arg(i + 1), i);
    }

    //
    m_progressSearch = new ProgressDialog(this);
    connect(m_progressSearch, SIGNAL(sig_cancel()), this, SLOT(onSearchCanceled()));
    m_progressExport = new ProgressDialog(this);
    connect(m_progressExport, SIGNAL(sig_cancel()), this, SLOT(onExportCanceled()));

    //
    m_logDetail = new LogDetail(this);
    connect(m_logDetail, SIGNAL(previousLog()), this, SLOT(onPreviousLog()));
    connect(m_logDetail, SIGNAL(nextLog()), this, SLOT(onNextLog()));

    gMessageFilter.installMessageFilter(RESPONSE_FLAG_PROGRESS_LOG, this);
    gMessageFilter.installMessageFilter(RESPONSE_FLAG_PROGRESS_LOG_EXPORT, this);

    onLanguageChanged();
}

PageLogStatus::~PageLogStatus()
{
    delete ui;
}

void PageLogStatus::initializeData()
{
    const QDateTime &dateTime = QDateTime::currentDateTime();
    ui->dateEdit_startDate->setDate(dateTime.date());
    ui->dateEdit_endDate->setDate(dateTime.date());
    ui->timeEdit_startTime->setTime(QTime(0, 0, 0));
    ui->timeEdit_endTime->setTime(QTime(23, 59, 59));

    ui->comboBoxChannel->setLineEditText(GET_TEXT("COMMON/1006", "All"));
    ui->comboBoxChannel->checkedAll();

    ui->comboBox_maintype->setCurrentIndex(0);
    on_comboBox_maintype_activated(0);
}

void PageLogStatus::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_LOG_SEARCH:
        ON_RESPONSE_FLAG_LOG_SEARCH(message);
        break;
    case RESPONSE_FLAG_LOG_EXPORT:
        ON_RESPONSE_FLAG_LOG_EXPORT(message);
        break;
    case RESPONSE_FLAG_LOG_GET_DETAIL:
        ON_RESPONSE_FLAG_LOG_GET_DETAIL(message);
        break;
    }
}

void PageLogStatus::filterMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_PROGRESS_LOG:
        ON_RESPONSE_FLAG_PROGRESS_LOG(message);
        break;
    case RESPONSE_FLAG_PROGRESS_LOG_EXPORT:
        ON_RESPONSE_FLAG_PROGRESS_LOG_EXPORT(message);
        break;
    }
}

void PageLogStatus::ON_RESPONSE_FLAG_LOG_SEARCH(MessageReceive *message)
{
    struct req_log_search *log_search = (struct req_log_search *)message->data;
    if (!log_search) {
        qCritical() << QString("LogStatus::ON_RESPONSE_FLAG_LOG_SEARCH, data is null");
        return;
    }
    m_allCount = log_search->total;

    m_listLog.clear();
    if (m_allCount == 0) {
        ShowMessageBox(GET_TEXT("LOG/64010", "No matching logs."));
    } else {
        for (int i = 0; i < log_search->num; ++i) {
            m_listLog.append(log_search->data[i]);
        }
    }

    if (m_pageCount == 0) {
        m_pageCount = qCeil(m_allCount / 100.0);
        ui->lineEdit_page->setMaxPage(m_pageCount);
        ui->label_count->setText(GET_TEXT("LOG/64100", "Total %1 Items").arg(m_allCount));
        if (qMsNvr->is3536c()) {
            if (m_allCount >= 2000) {
                ShowMessageBox(GET_TEXT("LOG/64016", "Over %1 log results, and the top %2 will be shown.").arg(2000).arg(2000));
            }
        } else {
            if (m_allCount >= MaxSearchLogCount) {
                ShowMessageBox(GET_TEXT("LOG/64012", "Over 10000 log results, and the top 10000 will be shown."));
            }
        }
    }

    updateTable();

    m_progressSearch->setProgressValue(100);
    m_progressSearch->hideProgress();
}

/**
 * @brief LogStatus::ON_RESPONSE_FLAG_PROGRESS_LOG
 * search 进度
 * @param message
 */
void PageLogStatus::ON_RESPONSE_FLAG_PROGRESS_LOG(MessageReceive *message)
{
    if (!isVisible()) {
        return;
    }

    PROGRESS_BAR_T *progress = (PROGRESS_BAR_T *)message->data;
    if (!progress) {
        return;
    }
    m_progressSearch->setProgressValue(progress->percent);
    if (progress->percent >= 100) {
        m_progressSearch->setProgressValue(100);
        m_progressSearch->hideProgress();
    }
}

void PageLogStatus::ON_RESPONSE_FLAG_PROGRESS_LOG_EXPORT(MessageReceive *message)
{
    if (!isVisible()) {
        return;
    }

    PROGRESS_BAR_T *progress = (PROGRESS_BAR_T *)message->data;
    if (!progress) {
        return;
    }
    m_progressExport->setProgressValue(progress->percent);
    if (progress->percent >= 100) {
        m_progressExport->setProgressValue(100);
        m_progressExport->hideProgress();
    }
}

void PageLogStatus::ON_RESPONSE_FLAG_LOG_EXPORT(MessageReceive *message)
{
    m_progressExport->hide();
    if (!message->data) {
        qWarning() << "LogStatus::ON_RESPONSE_FLAG_LOG_EXPORT, data is null.";
        return;
    }
    int result = *((int *)message->data);
    qDebug() << QString("LogStatus::ON_RESPONSE_FLAG_LOG_EXPORT, result = %1").arg(result);
    switch (result) {
    case -1:
        ShowMessageBox("Logs export failed!");
        break;
    default:
        break;
    }
}

void PageLogStatus::ON_RESPONSE_FLAG_LOG_GET_DETAIL(MessageReceive *message)
{
    char *detailstr = (char *)message->data;
    if (!detailstr) {
        return;
    }
    m_logDetail->setDetail(QString(detailstr));
}

QString PageLogStatus::mainTypeString(int type)
{
    QString value;
    switch (type) {
    case MAIN_ALL:
        value = GET_TEXT("COMMON/1006", "All");
        break;
    case MAIN_EVENT:
        value = GET_TEXT("MENU/10005", "Event");
        break;
    case MAIN_OP:
        value = GET_TEXT("LOG/64073", "Operation");
        break;
    case MAIN_INFO:
        value = GET_TEXT("LOG/64074", "Information");
        break;
    case MAIN_EXCEPT:
        value = GET_TEXT("EXCEPTION/54000", "Exception");
        break;
    case MAIN_DEBUG:
        value = GET_TEXT("LOG/64063", "Debug");
        break;
    default:
        value = GET_TEXT("LOG/64076", "N/A");
        break;
    }
    return value;
}

QString PageLogStatus::subTypeString(int type)
{
    QString value;
    switch (type) {
    case SUB_ALL:
        value = GET_TEXT("COMMON/1006", "All");
        break;
        //Event Begin
    case SUB_ALARM_IN:
        value = GET_TEXT("ALARMIN/52001", "Alarm Input");
        break;
    case SUB_ALARM_OUT:
        value = GET_TEXT("ALARMOUT/53000", "Alarm Output");
        break;
    case SUB_MOTION_START:
        value = GET_TEXT("LOG/64018", "Start Motion Detection");
        break;
    case SUB_MOTION_END:
        value = GET_TEXT("LOG/64019", "Stop Motion Detection");
        break;
    case SUB_CAMERAIO:
        value = GET_TEXT("LOG/64020", "Camera I/O");
        break;
    case SUB_AUDIODETECT:
        value = GET_TEXT("LOG/64021", "Volume Detection");
        break;
    case SUB_REGION_ENTRANCE_ALARM:
        value = GET_TEXT("LOG/64148", "Start Region Entrance Alarm");
        break;
    case SUB_STOP_REGION_ENTRANCE_ALARM:
        value = GET_TEXT("LOG/64149", "Stop Region Entrance Alarm");
        break;
    case SUB_REGION_EXITING_ALARM:
        value = GET_TEXT("LOG/64150", "Start Region Exiting Alarm");
        break;
    case SUB_STOP_REGION_EXITING_ALARM:
        value = GET_TEXT("LOG/64151", "Stop Region Exiting Alarm");
        break;
    case SUB_START_ADVANCED_MOTION:
        value = GET_TEXT("LOG/64152", "Start Advanced Motion Detection");
        break;
    case SUB_STOP_ADVANCED_MOTION:
        value = GET_TEXT("LOG/64153", "Stop Advanced Motion Detection");
        break;
    case SUB_TAMPERING_ALARM:
        value = GET_TEXT("LOG/64154", "Start Tamper Detection");
        break;
    case SUB_STOP_TAMPERING_ALARM:
        value = GET_TEXT("LOG/64155", "Stop Tamper Detection");
        break;
    case SUB_LINE_CROSSING_ALARM:
        value = GET_TEXT("LOG/64156", "Start Line Crossing Alarm");
        break;
    case SUB_STOP_LINE_CROSSING_ALARM:
        value = GET_TEXT("LOG/64157", "Stop Line Crossing Alarm");
        break;
    case SUB_LOITERING_ALARM:
        value = GET_TEXT("LOG/64158", "Start Loitering Alarm");
        break;
    case SUB_STOP_LOITERING_ALARM:
        value = GET_TEXT("LOG/64159", "Stop Loitering Alarm");
        break;
    case SUB_HUMAN_DETECTION_ALARM:
        value = GET_TEXT("LOG/64160", "Start Human Detection Alarm");
        break;
    case SUB_STOP_HUMAN_DETECTION_ALARM:
        value = GET_TEXT("LOG/64161", "Stop Human Detection Alarm");
        break;
    case SUB_PEOPLE_COUNTING_IN:
        value = GET_TEXT("LOG/64162", "People Counting In");
        break;
    case SUB_PEOPLE_COUNTING_OUT:
        value = GET_TEXT("LOG/64163", "People Counting Out");
        break;
    case SUB_OBJECT_LEFT_START:
        value = GET_TEXT("LOG/64184", "Start Object Left Alarm");
        break;
    case SUB_OBJECT_LEFT_END:
        value = GET_TEXT("LOG/64185", "Stop Object Left Alarm");
        break;
    case SUB_OBJECT_REMOVE_START:
        value = GET_TEXT("LOG/64179", "Start Object Removed Alarm");
        break;
    case SUB_OBJECT_REMOVE_END:
        value = GET_TEXT("LOG/64180", "Stop Object Removed Alarm");
        break;
    case SUB_IPC_PEOPLE_COUNT_START:
        value = GET_TEXT("LOG/64186", "Start People Counting Alarm");
        break;
    case SUB_IPC_PEOPLE_COUNT_END:
        value = GET_TEXT("LOG/64187", "Stop People Counting Alarm");
        break;
    case SUB_ANPR:
        value = "ANPR";
        break;
    case SUB_GROUP_PEOPLE_COUNT_START:
        value = GET_TEXT("LOG/64188", "Start Occupancy Group Alarm");
        break;
    case SUB_GROUP_PEOPLE_COUNT_END:
        value = GET_TEXT("LOG/64189", "Stop Occupancy Group Alarm");
        break;
    case SUB_FACE:
        value = GET_TEXT("FACE/141000", "Face Detection");
        break;
    case SUB_POS_CONNECT:
        value = GET_TEXT("POS/130040", "POS Connected");
        break;
    case SUB_POS_DISCONNECT:
        value = GET_TEXT("POS/130041", "POS Disconnected");
        break;
        //Event End

    case SUP_OP_POWER_ON:
        value = GET_TEXT("LOG/64022", "Power On");
        break;
    case SUP_OP_ABNORMAL_SHUTDOWN:
        value = GET_TEXT("LOG/64101", "Abnormal Shutdown");
        break;
    case SUB_OP_SHUTDOWN_LOCAL:
        value = GET_TEXT("LOG/64023", "Local:Shut Down");
        break;
    case SUB_OP_REBOOT_LOCAL:
        value = GET_TEXT("LOG/64024", "Local:Reboot");
        break;
    case SUB_OP_LOGIN_LOCAL:
        value = GET_TEXT("LOG/64025", "Local:Login");
        break;
    case SUB_OP_LOGOUT_LOCAL:
        value = GET_TEXT("LOG/64026", "Local:Logout");
        break;
    case SUB_OP_CONFIG_LOCAL:
        value = GET_TEXT("LOG/64027", "Local:Configure Parameters");
        break;
    //case SUB_OP_EMERGENCY_REC_START_LOCAL: value=GET_TEXT("LOG/64028","Local:Start Record");break;
    //case SUB_OP_EMERGENCY_REC_STOP_LOCAL: value=GET_TEXT("LOG/64029","Local:Stop Record");break;
    case SUB_OP_SNAPSHOT_LOCAL:
        value = GET_TEXT("LOG/64030", "Local:Capture");
        break;
    case SUB_OP_PLAYBACK_SNAPSHOT_LOCAL:
        value = GET_TEXT("LOG/64031", "Local:Playback Capture");
        break;
    case SUB_OP_SET_VIDEO_PARAM_LOCAL:
        value = GET_TEXT("LOG/64032", "Local:Image Configuration");
        break;
    case SUB_OP_PTZ_CONTROL_LOCAL:
        value = GET_TEXT("LOG/64033", "Local:PTZ Control");
        break;
    case SUP_OP_LOCK_FILE:
        value = GET_TEXT("LOG/64102", "Local:Lock File");
        break;
    case SUP_OP_UNLOCK_FILE:
        value = GET_TEXT("LOG/64103", "Local:Unlock File");
        break;
    case SUB_OP_DISK_INIT_LOCAL:
        value = GET_TEXT("LOG/64034", "Local:Initialize Disk");
        break;
    case SUB_OP_PLAYBACK_LOCAL:
        value = GET_TEXT("LOG/64035", "Local:Playback");
        break;
    case SUP_OP_ADD_IP_CHANNEL_LOCK:
        value = GET_TEXT("LOG/64104", "Local:Add IP Channel");
        break;
    case SUP_OP_DELETE_IP_CHANNEL_LOCK:
        value = GET_TEXT("LOG/64105", "Local:Delete IP Channel");
        break;
    case SUP_OP_EDIT_IP_CHANNEL_LOCK:
        value = GET_TEXT("LOG/64106", "Local:Edit IP Channel");
        break;
    case SUB_OP_PROFILE_EXPORT_LOCAL:
        value = GET_TEXT("LOG/64036", "Local:Export Config File");
        break;
    case SUB_OP_PROFILE_IMPORT_LOCAL:
        value = GET_TEXT("LOG/64037", "Local:Import Config File");
        break;
    case SUB_OP_PROFILE_RESET_LOCAL:
        value = GET_TEXT("LOG/64038", "Local:Restore to Default");
        break;
    case SUP_OP_UPGRADE_IP_CHANNEL_LOCK:
        value = GET_TEXT("LOG/64107", "Local:Upgrade IP Channel");
        break;
    case SUB_OP_UPGRADE_LOCAL:
        value = GET_TEXT("LOG/64039", "Local:Upgrade");
        break;
    case SUP_OP_PLAYBACK_BY_FILE_LOCK:
        value = GET_TEXT("LOG/64108", "Local:Playback by File");
        break;
    case SUP_OP_PLAYBACK_BY_TIME_LOCK:
        value = GET_TEXT("LOG/64109", "Local:Playback by Time");
        break;
    case SUB_OP_VIDEO_EXPORT_LOCAL:
        value = GET_TEXT("LOG/64040", "Local:Export Record File");
        break;
    case SUB_OP_PICTURE_EXPORT_LOCAL:
        value = GET_TEXT("LOG/64041", "Local:Export Image File");
        break;
    case SUP_OP_ADD_NETWORK_DISK_LOCK:
        value = GET_TEXT("LOG/64110", "Local:Add Network Disk");
        break;
    case SUP_OP_DELETE_NETWORK_DISK_LOCK:
        value = GET_TEXT("LOG/64111", "Local:Delete Network Disk");
        break;
    case SUP_OP_CONFIG_NETWORK_DISK_LOCK:
        value = GET_TEXT("LOG/64112", "Local:Config Network Disk");
        break;
    case SUP_OP_TAG_OPERATION_LOCK:
        value = GET_TEXT("LOG/64113", "Local:Tag Operation");
        break;
    case SUP_OP_MAIN_SUP_MONITOR_SWITCH_LOCK:
        value = GET_TEXT("LOG/64114", "Local:Main/Sub Monitor Switch");
        break;
    case SUP_OP_PHYSICAL_DISK_SELF_CHECK_LOCK:
        value = GET_TEXT("LOG/64115", "Local:Physical Disk Self Check");
        break;
    case SUP_OP_MOUNT_DISK_LOCK:
        value = GET_TEXT("LOG/64116", "Local:Mount Disk");
        break;
    case SUP_OP_UNMOUNT_DISK_LOCK:
        value = GET_TEXT("LOG/64117", "Local:Unmount Disk");
        break;
    case SUP_OP_DELETE_ABNORMAL_LOCK:
        value = GET_TEXT("LOG/64118", "Local:Delete Abnormal Disk");
        break;
    case SUP_OP_START_EMERGENCY_RECORD_LOCK:
        value = GET_TEXT("LOG/64119", "Local:Start Emergency Record");
        break;
    case SUP_OP_STOP_EMERGENCY_RECORD_LOCK:
        value = GET_TEXT("LOG/64120", "Local:Stop Emergency Record");
        break;
    case SUP_OP_SYSTEM_TIME_SYNC_LOCAL:
        value = GET_TEXT("LOG/64147", "Local:System Time Sync");
        break;

    case SUB_OP_REBOOT_REMOTE:
        value = GET_TEXT("LOG/64042", "Remote:Reboot");
        break;
    case SUB_OP_LOGIN_REMOTE:
        value = GET_TEXT("LOG/64043", "Remote:Login");
        break;
    case SUB_OP_LOGOUT_REMOTE:
        value = GET_TEXT("LOG/64044", "Remote:Logout");
        break;
    case SUB_OP_UPGRADE_REMOTE:
        value = GET_TEXT("LOG/64045", "Remote:Upgrade");
        break;
    case SUB_OP_CONFIG_REMOTE:
        value = GET_TEXT("LOG/64046", "Remote:Configure Parameters");
        break;
    //case SUB_OP_RECORD_START_REMOTE: value=GET_TEXT("LOG/64047","Remote:Start Record");break;
    //case SUB_OP_RECORD_STOP_REMOTE: value=GET_TEXT("LOG/64048","Remote:Stop Record");break;
    case SUB_OP_SNAPSHOT_REMOTE:
        value = GET_TEXT("LOG/64049", "Remote:Capture");
        break;
    case SUB_OP_PTZ_CONTROL_REMOTE:
        value = GET_TEXT("LOG/64050", "Remote:PTZ Control");
        break;
    case SUB_OP_DISK_INIT_REMOTE:
        value = GET_TEXT("LOG/64051", "Remote:Initialize Disk");
        break;
    case SUB_OP_PREVIEW_REMOTE:
        value = GET_TEXT("LOG/64052", "Remote:Preview");
        break;
    case SUB_OP_PLAYBACK_REMOTE:
        value = GET_TEXT("LOG/64053", "Remote:Playback");
        break;
    case SUP_OP_ADD_IP_CHANNEL_REMOTE:
        value = GET_TEXT("LOG/64121", "Remote:Add IP Channel");
        break;
    case SUP_OP_DELETE_IP_CHANNEL_REMOTE:
        value = GET_TEXT("LOG/64122", "Remote:Delete IP Channel");
        break;
    case SUP_OP_EDIT_IP_CHANNEL_REMOTE:
        value = GET_TEXT("LOG/64123", "Remote:Edit IP Channel");
        break;
    case SUB_OP_PROFILE_EXPORT_REMOTE:
        value = GET_TEXT("LOG/64054", "Remote:Export Config File");
        break;
    case SUB_OP_PROFILE_IMPORT_REMOTE:
        value = GET_TEXT("LOG/64055", "Remote:Import Config File");
        break;
    case SUB_OP_ADMIN_RESET_REMOTE:
        value = GET_TEXT("LOG/64057", "Remote:Restore to Default");
        break;
    case SUP_OP_UPGRADE_IP_CHANNEL_REMOTE:
        value = GET_TEXT("LOG/64124", "Remote:Upgrade IP Channel");
        break;
    case SUB_OP_SET_VIDEO_PARAM:
        value = GET_TEXT("LOG/64058", "Remote:Image Configuration");
        break;
    case SUB_OP_PLAYBACK_SNAPSHOT_REMOTE:
        value = GET_TEXT("LOG/64059", "Remote:Playback Capture");
        break;
    case SUP_OP_ADD_NETWORK_DISK_REMOTE:
        value = GET_TEXT("LOG/64125", "Remote:Add Network Disk");
        break;
    case SUP_OP_DELETE_NETWORK_DISK_REMOTE:
        value = GET_TEXT("LOG/64126", "Remote:Delete Network Disk");
        break;
    case SUP_OP_CONFIG_NETWORK_DISK_REMOTE:
        value = GET_TEXT("LOG/64127", "Remote:Config Network Disk");
        break;
    case SUP_OP_MOUNT_DISK_REMOTE:
        value = GET_TEXT("LOG/64128", "Remote:Mount Disk");
        break;
    case SUP_OP_UNMOUNT_DISK_REMOTE:
        value = GET_TEXT("LOG/64129", "Remote:Unmount Disk");
        break;
    case SUP_OP_DELETE_ABNORMAL_DISK_REMOTE:
        value = GET_TEXT("LOG/64130", "Remote:Delete Abnormal Disk");
        break;
    case SUP_OP_SYSTEM_TIME_SYNC_REMOTE:
        value = GET_TEXT("LOG/64138", "Remote:System Time Sync");
        break;
    case SUB_OP_UPGRADE_OEM:
        value = GET_TEXT("LOG/64060", "Remote:Upgrade OEM");
        break;

    case SUP_OP_FAILOVER_SLAVE_ADD_MASTER:
        value = GET_TEXT("LOG/64164", "Local:Add Master");
        break;
    case SUP_OP_FAILOVER_SLAVE_DEL_MASTER:
        value = GET_TEXT("LOG/64165", "Local:Delete Master");
        break;
    case SUP_OP_FAILOVER_SLAVE_ADD_IPC:
        value = GET_TEXT("LOG/64181", "Local: Add slave IP Channel");
        break;
    case SUP_OP_FAILOVER_SLAVE_DEL_IPC:
        value = GET_TEXT("LOG/64182", "Local: Delete slave IP Channel");
        break;

    case SUB_EXCEPT_VIDEO_LOSS:
        value = GET_TEXT("VIDEOLOSS/50001", "Video Loss");
        break;
    case SUB_EXCEPT_DISK_FULL:
        value = GET_TEXT("LIVEVIEW/20043", "Disk Full");
        break;
    case SUB_EXCEPT_DISK_FAILURE:
        value = GET_TEXT("LIVEVIEW/20045", "Disk Error");
        break;
    case SUB_EXCEPT_NETWORK_DISCONNECT:
        value = GET_TEXT("LIVEVIEW/20042", "Network Disconnect");
        break;
    case SUB_EXCEPT_RECORD_FAIL:
        value = GET_TEXT("LOG/64065", "Record Fail");
        break;
    case SUB_EXCEPT_SNAPSHOT_FAIL:
        value = GET_TEXT("LOG/64066", "Capture Fail");
        break;
    case SUB_EXCEPT_IPC_CONFLICT:
        value = GET_TEXT("LOG/64072", "NVR IP conflict");
        break;

    case SUB_EXCEPT_DISK_UNINITIALIZED:
        value = GET_TEXT("LOG/64075", "Disk Uninitialized");
        break;
    case SUB_EXCEPT_PLAYBACK_SNAPSHOT_FAIL:
        value = GET_TEXT("LOG/64080", "Playback Snapshot Failed");
        break;

    case SUB_EXCEPT_FAILOVER_MASTER_ANR_ERROR:
        value = GET_TEXT("LOG/64168", "Master Hot Spare Returning Error");
        break;
    case SUB_EXCEPT_FAILOVER_SLAVE_WORK_ERROR:
        value = GET_TEXT("LOG/64169", "Slave Hot Spare Error");
        break;
    case SUB_EXCEPT_DATE_ERROR:
        value = GET_TEXT("LOG/64183", "Date Error");
        break;
    case SUB_EXCEPT_NVR_IP_CONFLICT:
        value = GET_TEXT("LIVEVIEW/168005", "IP Address Conflict");
        break;
    case SUB_EXCEPT_DISK_OFFLINE:
        value = GET_TEXT("EXCEPTION/176000", "Disk Offine");
        break;
    case SUB_EXCEPT_DISK_HEAT:
        value = GET_TEXT("EXCEPTION/176001", "Disk Heat");
        break;
    case SUB_EXCEPT_DISK_MICROTHERM:
        value = GET_TEXT("EXCEPTION/176002", "Disk Microtherm");
        break;
    case SUB_EXCEPT_DISK_CONNECTION_EXCEPTION:
        value = GET_TEXT("EXCEPTION/176003", "Disk Connection Exception");
        break;
    case SUB_EXCEPT_DISK_DISK_STRIKE:
        value = GET_TEXT("EXCEPTION/176004", "Disk Strike");
        break;
    case SUB_INFO_DISK_INFO:
        value = GET_TEXT("LOG/64068", "Disk Information");
        break;
    case SUP_INFO_NETWORK_DISK_INFORMATION:
        value = GET_TEXT("LOG/64131", "Network Disk Information");
        break;
    case SUB_INFO_DISK_SMART:
        value = GET_TEXT("LOG/64069", "S.M.A.R.T Information");
        break;
    case SUB_INFO_RECORD_START:
        value = GET_TEXT("LOG/64070", "Start Record");
        break;
    case SUB_INFO_RECORD_STOP:
        value = GET_TEXT("LOG/64071", "Stop Record");
        break;
    case SUP_INFO_DELETE_EXPRIRED_VIDEO:
        value = GET_TEXT("LOG/64132", "Delete Expired Video");
        break;
    case SUP_INFO_SYSTEM_OPERATION_STATUS:
        value = GET_TEXT("LOG/64133", "System Operation Status");
        break;
    case SUP_INFO_START_ANR_RECORD:
        value = GET_TEXT("LOG/64134", "Start Hot Spare Record");
        break;
    case SUP_INFO_STOP_ANR_RECORD:
        value = GET_TEXT("LOG/64135", "Stop Hot Spare Record");
        break;
    case SUP_INFO_ADD_IP_CHANNEL_ANR_TIME:
        value = GET_TEXT("LOG/64136", "Add IP Channel ANR Time");
        break;
    case SUP_INFO_DELETE_IP_CHANNEL_ANR_TIME:
        value = GET_TEXT("LOG/64137", "Delete IP Channel ANR Time");
        break;

    case SUP_INFO_FAILOVER_MASTER_INFO:
        value = GET_TEXT("LOG/64170", "Master Information");
        break;
    case SUP_INFO_FAILOVER_MASTER_ANR_START:
        value = GET_TEXT("LOG/64171", "Master Start Hot Spare Returning");
        break;
    case SUP_INFO_FAILOVER_MASTER_ANR_END:
        value = GET_TEXT("LOG/64172", "Master Hot Spare Return Finished");
        break;
    case SUP_INFO_FAILOVER_SLAVE_START_WORK:
        value = GET_TEXT("LOG/64173", "Slave Hot Spare Start");
        break;
    case SUP_INFO_FAILOVER_SLAVE_STOP_WORK:
        value = GET_TEXT("LOG/64174", "Slave Hot Spare Stop");
        break;
    case SUP_INFO_FAILOVER_SLAVE_IPC_START:
        value = GET_TEXT("LOG/64175", "Slave Start Recording");
        break;
    case SUP_INFO_FAILOVER_SLAVE_IPC_STOP:
        value = GET_TEXT("LOG/64176", "Slave Stop Recording");
        break;
    case SUP_INFO_IP_CHANNEL_ANR_START:
        value = GET_TEXT("LOG/64177", "IP Channel Start ANR Returning");
        break;
    case SUP_INFO_IP_CHANNEL_ANR_FINISH:
        value = GET_TEXT("LOG/64178", "IP Channel ANR Returning Finished");
        break;

    case SUB_PARAM_NONE:
        value = GET_TEXT("LOG/64076", "N/A");
        break;
    case SUB_PARAM_ADD_IPC:
        value = GET_TEXT("LOG/64077", "Add IPC");
        break;
    case SUB_PARAM_DEL_IPC:
        value = GET_TEXT("LOG/64078", "Delete IPC");
        break;
    case SUB_PARAM_EDIT_IPC:
        value = GET_TEXT("LOG/64079", "Edit IPC");
        break;
    case SUB_PARAM_PTZ_CONFIG:
        value = GET_TEXT("CHANNELMANAGE/30004", "PTZ Configuration");
        break;
    case SUB_PARAM_RECORD_MODE:
        value = GET_TEXT("LOG/64081", "Record Mode");
        break;
    case SUB_PARAM_RECORD_CFG:
        value = GET_TEXT("LOG/64082", "Record Configuation");
        break;
    case SUB_PARAM_RECORD_SCHED:
        value = GET_TEXT("LOG/64083", "Record Schedule");
        break;
    case SUB_PARAM_VIDEOLOSS:
        value = GET_TEXT("LOG/64084", "Video Loss Configuration");
        break;
    case SUB_PARAM_MOTION:
        value = GET_TEXT("LOG/64085", "Motion Detection Configuration");
        break;
    case SUB_PARAM_ALARMINPUT:
        value = GET_TEXT("LOG/64086", "Alarm Input Configuration");
        break;
    case SUB_PARAM_ALARMOUTPUT:
        value = GET_TEXT("LOG/64087", "Alarm Output Configuration");
        break;
    case SUB_PARAM_LAYOUTCONFIG:
        value = GET_TEXT("LAYOUT/40000", "Layout Configuration");
        break;
    case SUB_PARAM_SYSTEM_GENERAL:
        value = GET_TEXT("LOG/64089", "System General Configuration");
        break;
    case SUB_PARAM_NETWORK:
        value = GET_TEXT("LOG/64090", "Network Configuration");
        break;
    case SUB_PARAM_NETWORK_PPPOE:
        value = GET_TEXT("LOG/64091", "PPPOE Configuration");
        break;
    case SUB_PARAM_NETWORK_DDNS:
        value = GET_TEXT("LOG/64092", "DNS Configuration");
        break;
    case SUB_PARAM_NETWORK_MAIL:
        value = GET_TEXT("LOG/64093", "Email Configuration");
        break;
    case SUB_PARAM_NETWORK_MORE:
        value = GET_TEXT("LOG/64094", "Network Other");
        break;
    case SUB_PARAM_DISK_RECYCLE_MODE:
        value = GET_TEXT("LOG/64095", "Disk Recycle Mode");
        break;
    case SUB_PARAM_HOLIDAY:
        value = GET_TEXT("LOG/64096", "Holiday Configuration");
        break;
    case SUB_PARAM_ADD_USER:
        value = GET_TEXT("LOG/64097", "Add User");
        break;
    case SUB_PARAM_DEL_USER:
        value = GET_TEXT("LOG/64098", "Delete User");
        break;
    case SUB_PARAM_AUDIO_FILE:
        value = GET_TEXT("LOG/64190", "System Audio Configuration");
        break;

    case SUB_DEBUG_WAR:
        value = GET_TEXT("LOG/64061", "Warning");
        break;
    case SUB_DEBUG_INF:
        value = GET_TEXT("LOG/64062", "Infomation");
        break;
    case SUB_DEBUG_DBG:
        value = GET_TEXT("LOG/64063", "Debug");
        break;
    case SUB_DEBUG_ERR:
        value = GET_TEXT("LOG/64064", "Error");
        break;

    default:
        value = GET_TEXT("LOG/64076", "N/A");
        break;
    }
    return value;
}

void PageLogStatus::searchLogPage(int page)
{
    m_criteria.page = page;

    QDateTime startDateTime = QDateTime::fromTime_t(m_criteria.start_time);
    QDateTime endDateTime = QDateTime::fromTime_t(m_criteria.end_time);
    qMsDebug() << qPrintable(QString("\n time: %1 - %2").arg(startDateTime.toString("yyyy-MM-dd HH:mm:ss")).arg(endDateTime.toString("yyyy-MM-dd HH:mm:ss")))
               << qPrintable(QString("\n chnMask(64<->1): %1").arg(m_criteria.chnMask, 64, 2, QLatin1Char('0')))
               << qPrintable(QString("\n bSeachNoChn: %1").arg(m_criteria.bSeachNoChn))
               << qPrintable(QString("\n page: %1/%2").arg(page).arg(m_pageCount));
    sendMessage(REQUEST_FLAG_LOG_SEARCH, &m_criteria, sizeof(struct search_criteria));

    m_progressSearch->setTitle(GET_TEXT("COMMONBACKUP/100038", "Searching..."));
    m_progressSearch->showProgress();
}

void PageLogStatus::updateTable()
{
    ui->tableView->clearContent();
    ui->tableView->setRowCount(m_listLog.size());
    for (int i = 0; i < m_listLog.size(); ++i) {
        const log_data &log = m_listLog.at(i);
        //记录数组下标，排序改变后可以取到正确的数据
        ui->tableView->setItemData(i, ColumnNumber, i, LogIndexRole);
        ui->tableView->setItemIntValue(i, ColumnNumber, m_pageIndex * 100 + i + 1);
        ui->tableView->setItemText(i, ColumnMainType, mainTypeString(log.log_data_info.mainType));
        ui->tableView->setItemText(i, ColumnTime, QDateTime::fromTime_t(log.log_data_info.pts).toString("yyyy-MM-dd HH:mm:ss"));
        ui->tableView->setItemText(i, ColumnSubType, subTypeString(log.log_data_info.subType));
        ui->tableView->setItemText(i, ColumnParameter, subTypeString(log.log_data_info.parameter_type));

        if (LogDetail::isDiskType(log.log_data_info.mainType, log.log_data_info.subType)) {
            ui->tableView->setItemText(i, ColumnChannel, GET_TEXT("LOG/64076", "N/A"));
            //设置一个大的值，排序在后面
            ui->tableView->setItemData(i, ColumnChannel, 10000, SortIntRole);
        } else {
            if (log.log_data_info.chan_no < 1) {
                ui->tableView->setItemText(i, ColumnChannel, GET_TEXT("LOG/64076", "N/A"));
                //设置一个大的值，排序在后面
                ui->tableView->setItemData(i, ColumnChannel, 10000, SortIntRole);
            } else {
                ui->tableView->setItemIntValue(i, ColumnChannel, log.log_data_info.chan_no);
            }
        }

        QString strUser = QString(log.log_data_info.user);
        if (strUser.isEmpty()) {
            strUser = GET_TEXT("LOG/64076", "N/A");
        }
        ui->tableView->setItemText(i, ColumnUser, strUser);
        //remote
        if (log.log_data_info.remote) {
            QString strIP;
            char ip[24] = { 0 };
            snprintf(ip, sizeof(ip), "%s", "0.0.0.0");
            if (strlen(log.log_data_info.ip) == 0) {
                strIP = QString("%1").arg(GET_TEXT("LOG/64076", "N/A"));
            } else if (strncmp(log.log_data_info.ip, "::ffff:", 7) == 0 && strchr(log.log_data_info.ip, '.')) //hrz.milesight
            {
                strIP = QString("%1").arg(log.log_data_info.ip + 7);
            } else if (QString(log.log_data_info.ip).contains("%")) {
                QStringList strList = QString(log.log_data_info.ip).split("%");
                strIP = QString("%1").arg(strList.first());
            } else if (!memcmp(log.log_data_info.ip, ip, sizeof(ip))) {
                strIP = QString("%1").arg(GET_TEXT("LOG/64011", "Localhost"));
            } else {
                strIP = QString("%1").arg(log.log_data_info.ip);
            }
            ui->tableView->setItemText(i, ColumnRemoteHostIP, strIP);
        } else {
            ui->tableView->setItemText(i, ColumnRemoteHostIP, GET_TEXT("LOG/64076", "N/A"));
        }
    }
    ui->tableView->reorder();

    ui->label_page->setText(GET_TEXT("LAYOUT/40004", "Page: %1~%2").arg(m_pageIndex + 1).arg(m_pageCount));
    ui->lineEdit_page->setText(QString::number(m_pageIndex + 1));
}

void PageLogStatus::onLanguageChanged()
{
    ui->label_count->setText(GET_TEXT("LOG/64100", "Total %1 Items").arg(m_allCount));
    ui->label_page->setText(GET_TEXT("LAYOUT/40004", "Page: %1~%2").arg(m_pageCount > 0 ? m_pageIndex + 1 : 0).arg(m_pageCount));

    ui->label_startTime->setText(GET_TEXT("LOG/64001", "Start Time"));
    ui->label_endTime->setText(GET_TEXT("LOG/64002", "End Time"));
    ui->label_mainType->setText(GET_TEXT("LOG/64003", "Main Type"));
    ui->label_subType->setText(GET_TEXT("LOG/64004", "Sub Type"));
    ui->labelChannel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->pushButton_go->setText(GET_TEXT("PLAYBACK/80065", "Go"));
    ui->pushButton_export->setText(GET_TEXT("COMMONBACKUP/100015", "Export"));
    ui->pushButton_search->setText(GET_TEXT("CAMERASEARCH/32001", "Search"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->toolButton_firstPage->setToolTip(GET_TEXT("PLAYBACK/80061", "First Page"));
    ui->toolButton_previousPage->setToolTip(GET_TEXT("PLAYBACK/80062", "Previous Page"));
    ui->toolButton_nextPage->setToolTip(GET_TEXT("PLAYBACK/80063", "Next Page"));
    ui->toolButton_lastPage->setToolTip(GET_TEXT("PLAYBACK/80064", "Last Page"));
}

void PageLogStatus::onTableItemClicked(int row, int column)
{
    switch (column) {
    case ColumnDetails: {
        int index = ui->tableView->itemData(row, ColumnNumber, LogIndexRole).toInt();
        qDebug() << QString("LogStatus::onTableItemClicked, row: %1, logIndex: %2").arg(row).arg(index);
        const log_data &log = m_listLog.at(index);
        m_logDetail->showLog(log);
        sendMessage(REQUEST_FLAG_LOG_GET_DETAIL, (void *)&log, sizeof(struct log_data));
        break;
    }
    default:
        break;
    }
}

void PageLogStatus::onTableItemDoubleClicked(int row, int column)
{
    Q_UNUSED(column)

    onTableItemClicked(row, ColumnDetails);
}

void PageLogStatus::onSearchCanceled()
{
    sendMessage(REQUEST_FLAG_LOG_SEARCH_CANCEL, nullptr, 0);
    m_progressSearch->hide();
}

void PageLogStatus::onExportCanceled()
{
    sendMessage(REQUEST_FLAG_LOG_EXPORT_CANCEL, nullptr, 0);
    m_progressExport->hide();
}

void PageLogStatus::onPreviousLog()
{
    int row = ui->tableView->currentIndex().row();
    if (row < 1) {
        return;
    }
    ui->tableView->selectRow(row - 1);
    onTableItemClicked(row - 1, ColumnDetails);
}

void PageLogStatus::onNextLog()
{
    int row = ui->tableView->currentIndex().row();
    if (row >= ui->tableView->rowCount() - 1) {
        return;
    }
    ui->tableView->selectRow(row + 1);
    onTableItemClicked(row + 1, ColumnDetails);
}

void PageLogStatus::on_comboBox_maintype_activated(int index)
{
    ui->comboBox_subType->clear();

    op_main mainType = static_cast<op_main>(ui->comboBox_maintype->itemData(index).toInt());
    switch (mainType) {
    case MAIN_ALL: {
        ui->comboBox_subType->addItem(GET_TEXT("COMMON/1006", "All"), SUB_ALL);
        break;
    }
    case MAIN_EVENT: {
        ui->comboBox_subType->addItem(GET_TEXT("COMMON/1006", "All"), SUB_ALL);
        QList<int> events;
        events << SUB_ALARM_IN << SUB_ALARM_OUT;
        events << SUB_MOTION_START << SUB_MOTION_END;
        if (qMsNvr->OEMType() == OEM_TYPE_CANON) {
            events << SUB_CAMERAIO << SUB_AUDIODETECT;
        }
        events << SUB_REGION_ENTRANCE_ALARM << SUB_STOP_REGION_ENTRANCE_ALARM;
        events << SUB_REGION_EXITING_ALARM << SUB_STOP_REGION_EXITING_ALARM;
        events << SUB_START_ADVANCED_MOTION << SUB_STOP_ADVANCED_MOTION;
        events << SUB_TAMPERING_ALARM << SUB_STOP_TAMPERING_ALARM;
        events << SUB_LINE_CROSSING_ALARM << SUB_STOP_LINE_CROSSING_ALARM;
        events << SUB_LOITERING_ALARM << SUB_STOP_LOITERING_ALARM;
        events << SUB_HUMAN_DETECTION_ALARM << SUB_STOP_HUMAN_DETECTION_ALARM;
        events << SUB_OBJECT_LEFT_START << SUB_OBJECT_LEFT_END;
        events << SUB_OBJECT_REMOVE_START << SUB_OBJECT_REMOVE_END;
        events << SUB_IPC_PEOPLE_COUNT_START << SUB_IPC_PEOPLE_COUNT_END;
        events << SUB_GROUP_PEOPLE_COUNT_START << SUB_GROUP_PEOPLE_COUNT_END;
        if (qMsNvr->isSupportTargetMode()) {
            events << SUB_ANPR;
        }
        if (qMsNvr->isSupportFaceDetection()) {
            events << SUB_FACE;
        }
        events << SUB_POS_CONNECT << SUB_POS_DISCONNECT;
        for (int i = 0; i < events.size(); ++i) {
            int event = events.at(i);
            ui->comboBox_subType->addItem(subTypeString(event), event);
        }
        break;
    }
    case MAIN_OP: {
        ui->comboBox_subType->addItem(GET_TEXT("COMMON/1006", "All"), SUB_ALL);
        for (int i = SUB_OP_LOCAL_MIN + 1; i <= SUB_OP_LOCAL_MAX; i++) {
            ui->comboBox_subType->addItem(subTypeString(i), i);
        }
        for (int i = SUB_OP_REMOTE_MIN; i <= SUB_OP_REMOTE_MAX; i++) {
            ui->comboBox_subType->addItem(subTypeString(i), i);
        }
        break;
    }
    case MAIN_INFO: {
        ui->comboBox_subType->addItem(GET_TEXT("COMMON/1006", "All"), SUB_ALL);
        for (int i = SUB_INFO_MIN; i <= SUB_INFO_MAX; i++) {
            if (SUP_INFO_ADD_IP_CHANNEL_ANR_TIME == i)
                continue;
            if (SUP_INFO_DELETE_IP_CHANNEL_ANR_TIME == i)
                continue;
            if (SUP_INFO_START_ANR_RECORD == i)
                continue;
            if (SUP_INFO_STOP_ANR_RECORD == i)
                continue;
            ui->comboBox_subType->addItem(subTypeString(i), i);
        }
        break;
    }
    case MAIN_EXCEPT: {
        ui->comboBox_subType->addItem(GET_TEXT("COMMON/1006", "All"), SUB_ALL);
        for (int i = SUB_EXCEPT_MIN; i <= SUB_EXCEPT_MAX; i++) {
            if (SUB_EXCEPT_PLAYBACK_SNAPSHOT_FAIL == i)
                continue;
            ui->comboBox_subType->addItem(subTypeString(i), i);
        }
        break;
    }
    case MAIN_DEBUG: {
        ui->comboBox_subType->addItem(GET_TEXT("COMMON/1006", "All"), SUB_ALL);
        for (int i = SUB_DEBUG_MIN; i <= SUB_DEBUG_MAX; i++) {
            ui->comboBox_subType->addItem(subTypeString(i), i);
        }
        break;
    }
    default:
        break;
    }
    ui->comboBox_subType->setLineEditText(GET_TEXT("COMMON/1006", "All"));
}

void PageLogStatus::on_toolButton_firstPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex = 0;
    searchLogPage(m_pageIndex + 1);
}

void PageLogStatus::on_toolButton_previousPage_clicked()
{
    if (m_pageIndex < 1) {
        return;
    }
    m_pageIndex--;
    searchLogPage(m_pageIndex + 1);
}

void PageLogStatus::on_toolButton_nextPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex++;
    searchLogPage(m_pageIndex + 1);
}

void PageLogStatus::on_toolButton_lastPage_clicked()
{
    if (m_pageIndex >= m_pageCount - 1) {
        return;
    }
    m_pageIndex = m_pageCount - 1;
    searchLogPage(m_pageIndex + 1);
}

void PageLogStatus::on_pushButton_go_clicked()
{
    int page = ui->lineEdit_page->text().toInt();
    if (m_pageIndex == page - 1) {
        return;
    }
    if (page < 1 || page > m_pageCount) {
        return;
    }
    m_pageIndex = page - 1;
    searchLogPage(m_pageIndex + 1);
}

void PageLogStatus::on_pushButton_export_clicked()
{
    if (m_listLog.isEmpty()) {
        return;
    }

    //
    const QString &directory = MyFileSystemDialog::instance()->exportFile();
    if (directory.isEmpty()) {
        return;
    }
    qDebug() << QString("LogStatus::on_pushButton_export_clicke, export log to %1").arg(directory);
    //const QString &deviceName = MyFileSystemDialog::instance()->currentDeviceName();
    const qint64 &deviceFreeBytes = MyFileSystemDialog::instance()->currentDeviceFreeBytes();
    if ((deviceFreeBytes >> 20) < MIN_EXPORT_DISK_FREE) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100032", "Available capacity of device is not enough."));
        return;
    }

    //
    QDateTime startDateTime = QDateTime::fromTime_t(m_criteria.start_time);
    QDateTime endDateTime = QDateTime::fromTime_t(m_criteria.end_time);
    struct req_log_export log_export;
    memset(&log_export, 0, sizeof(struct req_log_export));
    snprintf(log_export.filename, sizeof(log_export.filename), "%s", directory.toStdString().c_str());
    log_export.lang = MsLanguage::instance()->currentLanguage();
    memcpy(&log_export.search, &m_criteria, sizeof(log_export.search));
    qMsDebug() << qPrintable(QString("\n time: %1 - %2").arg(startDateTime.toString("yyyy-MM-dd HH:mm:ss")).arg(endDateTime.toString("yyyy-MM-dd HH:mm:ss")))
               << qPrintable(QString("\n filename: %1").arg(log_export.filename))
               << qPrintable(QString("\n lang: %1").arg(log_export.lang));
    sendMessage(REQUEST_FLAG_LOG_EXPORT, &log_export, sizeof(struct req_log_export));

    m_progressExport->setTitle(GET_TEXT("LOG/64145", "Exporting..."));
    m_progressExport->showProgress();
}

void PageLogStatus::on_pushButton_search_clicked()
{
    QDateTime startDateTime(ui->dateEdit_startDate->date(), ui->timeEdit_startTime->time());
    QDateTime endDateTime(ui->dateEdit_endDate->date(), ui->timeEdit_endTime->time());
    if (startDateTime >= endDateTime) {
        ShowMessageBox(GET_TEXT("COMMONBACKUP/100031", "End time must be later than start time."));
        return;
    }

    //
    memset(&m_criteria, 0, sizeof(search_criteria));

    m_criteria.start_time = startDateTime.toTime_t();
    m_criteria.end_time = endDateTime.toTime_t();

    const QList<int> &channels = ui->comboBoxChannel->itemCheckedList();
    for (int i = 0; i < channels.size(); ++i) {
        const int &channel = channels.at(i);
        if (channel == -2) {

        } else if (channel == -1) {
            m_criteria.bSeachNoChn = MF_TRUE;
        } else {
            m_criteria.chnMask |= ((quint64)1 << channel);
        }
    }
    //超出最大通道数的都置1，避免搜不出来，比如port是存在channel字段的
    for (int i = qMsNvr->maxChannel(); i < 64; ++i) {
        m_criteria.chnMask |= ((quint64)1 << i);
    }
    if (channels.isEmpty()) {
        m_criteria.chnMask = quint64(0xffffffffffffffff);
        m_criteria.bSeachNoChn = MF_TRUE;
    }

    m_criteria.mainType = ui->comboBox_maintype->currentData().toInt();

    m_criteria.count = 0;
    QList<int> checkedList = ui->comboBox_subType->itemCheckedList();
    if (checkedList.size() == 0) {
        m_criteria.subType[0] = SUB_ALL;
        m_criteria.count = 1;
    } else {
        int i = 0;
        if (checkedList.size() > 1 && checkedList.at(0) == SUB_ALL) {
            i = 1;
        }
        for (int j = 0; i != checkedList.size(); ++i) {
            int tmp = checkedList.at(i);
            m_criteria.subType[j++] = tmp;
            m_criteria.count++;
        }
    }

    m_pageIndex = 0;
    m_pageCount = 0;

    searchLogPage(m_pageIndex + 1);
}

void PageLogStatus::on_pushButton_back_clicked()
{
    emit sig_back();
}
