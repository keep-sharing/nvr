#include "PageEventStatus.h"
#include "ui_PageEventStatus.h"
#include "centralmessage.h"
#include "itemicondelegate.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include <QtDebug>

PageEventStatus::PageEventStatus(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::EventStatus)
{
    ui->setupUi(this);

    ui->widget_tab->addTab(GET_TEXT("EVENTSTATUS/63002", "Camera Event"));
    ui->widget_tab->addTab(GET_TEXT("AUDIO_ALARM/159003", "Alarm Input/Output"));
    ui->widget_tab->addTab(GET_TEXT("SMARTEVENT/55000", "VCA"));
    connect(ui->widget_tab, SIGNAL(tabClicked(int)), this, SLOT(onTabClicked(int)));

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimerUpdate()));
    m_timer->setInterval(2000);

    initializeTableAlarm();
    initializeTableSmartEvent();
    initializeTableCmaera();

    onLanguageChanged();
}

PageEventStatus::~PageEventStatus()
{
    delete ui;
}

void PageEventStatus::initializeData()
{
    ui->widget_tab->setCurrentTab(0);

    sendMessage(REQUEST_FLAG_GET_IPCLIST, nullptr, 0);
    sendMessage(REQUEST_FLAG_GET_VAC_SUPPORT_ALL, nullptr, 0);
    onTimerUpdate();
}

bool PageEventStatus::canAutoLogout()
{
    return false;
}

void PageEventStatus::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPCLIST:
        ON_RESPONSE_FLAG_GET_IPCLIST(message);
        break;
    case RESPONSE_FLAG_GET_VAC_SUPPORT_ALL:
        ON_RESPONSE_FLAG_GET_VAC_SUPPORT_ALL(message);
        break;
    case RESPONSE_FLAG_GET_EVENT_STATUS:
        ON_RESPONSE_FLAG_GET_EVENT_STATUS(message);
        break;
    default:
        break;
    }
}

void PageEventStatus::ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message)
{
    resq_get_ipcdev *ipcdev_array = (resq_get_ipcdev *)message->data;
    int count = message->header.size / sizeof(resq_get_ipcdev);
    for (int i = 0; i < count; ++i) {
        const resq_get_ipcdev &ipcdev = ipcdev_array[i];

        AlarmKey key1(ipcdev.chanid, 0);
        AlarmKey key2(ipcdev.chanid, 1);
        m_alarminMap[key1].alarmCount = ipcdev.alarmin;
        m_alarminMap[key2].alarmCount = ipcdev.alarmin;
        m_alarmoutMap[key1].alarmCount = ipcdev.alarmout;
        m_alarmoutMap[key2].alarmCount = ipcdev.alarmout;
    }
}

void PageEventStatus::ON_RESPONSE_FLAG_GET_VAC_SUPPORT_ALL(MessageReceive *message)
{
    ms_vca_support_all *vcaSupportInfo = (ms_vca_support_all *)message->data;
    if (!vcaSupportInfo) {
        qWarning() << "EventStatus::ON_RESPONSE_FLAG_GET_VAC_SUPPORT_ALL, data is null.";
        return;
    }
    memcpy(&m_vcaSupportInfo, vcaSupportInfo, sizeof(ms_vca_support_all));
}

void PageEventStatus::ON_RESPONSE_FLAG_GET_EVENT_STATUS(MessageReceive *message)
{
    static QPixmap pixmapOn(":/status/status/alarm_on.png");
    static QPixmap pixmapOff(":/status/status/alarm_off.png");

    struct resp_event_status *event_status = (struct resp_event_status *)message->data;
    if (!event_status) {
        qWarning() << "EventStatus::ON_RESPONSE_FLAG_GET_EVENT_STATUS, data is null.";
        return;
    }

    int channelCount = qMsNvr->maxChannel();
    for (int i = 0; i < channelCount; ++i) {
        const int &channel = i;
        QString cameraIP = ui->tableView_camera->itemText(i, 2);
        bool videoloss = (event_status->video_loss >> channel) & static_cast<long long>(0x01);
        updateTableCameraStatus(ui->tableView_camera, i, 3, cameraIP, videoloss);

        bool motion = (event_status->motion_detect >> channel) & static_cast<long long>(0x01);
        updateTableCameraStatus(ui->tableView_camera, i, 4, cameraIP, motion);

        bool audioAlarm = (event_status->audioAlarm >> channel) & static_cast<long long>(0x01);
        updateTableCameraStatus(ui->tableView_camera, i, 5, cameraIP, audioAlarm);
        //
        bool regionin_detect = (event_status->regionin_detect >> channel) & static_cast<long long>(0x01);
        updateTableCameraStatus(ui->tableView_smart, i, 3, cameraIP, regionin_detect);

        bool regionexit_detect = (event_status->regionexit_detect >> channel) & static_cast<long long>(0x01);
        updateTableCameraStatus(ui->tableView_smart, i, 4, cameraIP, regionexit_detect);

        bool advanced_motion_detect = (event_status->advanced_motion_detect >> channel) & static_cast<long long>(0x01);
        updateTableCameraStatus(ui->tableView_smart, i, 5, cameraIP, advanced_motion_detect);

        bool tamper_detect = (event_status->tamper_detect >> channel) & static_cast<long long>(0x01);
        updateTableCameraStatus(ui->tableView_smart, i, 6, cameraIP, tamper_detect);

        bool linecross_detect = (event_status->linecross_detect >> channel) & static_cast<long long>(0x01);
        updateTableCameraStatus(ui->tableView_smart, i, 7, cameraIP, linecross_detect);

        bool loiter_detect = (event_status->loiter_detect >> channel) & static_cast<long long>(0x01);
        updateTableCameraStatus(ui->tableView_smart, i, 8, cameraIP, loiter_detect);

        bool human_detect = (event_status->human_detect >> channel) & static_cast<long long>(0x01);
        updateTableCameraStatus(ui->tableView_smart, i, 9, cameraIP, human_detect);

        bool object_left_remove = (event_status->object_left_remove >> channel) & static_cast<long long>(0x01);
        updateTableCameraStatus(ui->tableView_smart, i, 10, cameraIP, object_left_remove);
    }
    ui->tableView_camera->reorder();
    ui->tableView_smart->reorder();
    //nvr alarm in
    for (int i = 0; i < qMsNvr->maxAlarmInput(); ++i) {
        bool alarmin = (event_status->alarm_in >> i) & static_cast<long long>(0x01);

        AlarmKey key(i);
        m_alarminMap[key].status = alarmin;
    }
    //ipc alarm in
    for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
        bool alarmin1 = (event_status->ipc_alarm_in[0] >> i) & static_cast<quint64>(0x01);
        bool alarmin2 = (event_status->ipc_alarm_in[1] >> i) & static_cast<quint64>(0x01);
        int type1 = (event_status->ipc_alarm_in_type[0] >> i) & static_cast<quint64>(0x01);
        int type2 = (event_status->ipc_alarm_in_type[1] >> i) & static_cast<quint64>(0x01);

        AlarmKey key1(i, 0);
        AlarmKey key2(i, 1);
        m_alarminMap[key1].status = alarmin1;
        m_alarminMap[key2].status = alarmin2;
        m_alarminMap[key1].alarmType = type1;
        m_alarminMap[key2].alarmType = type2;
    }
    //alarmin table
    int alarminRow = 0;
    for (auto iter = m_alarminMap.constBegin(); iter != m_alarminMap.constEnd(); ++iter) {
        const AlarmKey &key = iter.key();
        const AlarmInfo &info = iter.value();
        ui->tableView_alarmIn->setItemText(alarminRow, 0, key.numberName());
        if (key.type() == AlarmKey::CameraAlarm && (info.alarmCount == 0 || (key.alarmId() == 1 && info.alarmCount == 1))) {
            ui->tableView_alarmIn->setItemText(alarminRow, 1, "-");
            ui->tableView_alarmIn->setItemText(alarminRow, 2, "-");
            ui->tableView_alarmIn->setItemText(alarminRow, 3, "-");
        } else {
            ui->tableView_alarmIn->setItemText(alarminRow, 1, info.alarmName);
            ui->tableView_alarmIn->setItemText(alarminRow, 2, info.alarmTypeString());
            if (info.status) {
                ui->tableView_alarmIn->setItemPixmap(alarminRow, 3, pixmapOn);
            } else {
                ui->tableView_alarmIn->setItemPixmap(alarminRow, 3, pixmapOff);
            }
        }

        alarminRow++;
    }

    //nvr alarm out
    for (int i = 0; i < qMsNvr->maxAlarmOutput(); ++i) {
        bool alarmout = (event_status->alarm_out >> i) & static_cast<long long>(0x01);

        AlarmKey key(i);
        m_alarmoutMap[key].status = alarmout;
    }
    //ipc alarm out
    for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
        bool alarmout1 = (event_status->ipc_alarm_out[0] >> i) & static_cast<quint64>(0x01);
        bool alarmout2 = (event_status->ipc_alarm_out[1] >> i) & static_cast<quint64>(0x01);
        int type1 = (event_status->ipc_alarm_out_type[0] >> i) & static_cast<quint64>(0x01);
        int type2 = (event_status->ipc_alarm_out_type[1] >> i) & static_cast<quint64>(0x01);

        AlarmKey key1(i, 0);
        AlarmKey key2(i, 1);
        m_alarmoutMap[key1].status = alarmout1;
        m_alarmoutMap[key2].status = alarmout2;
        m_alarmoutMap[key1].alarmType = type1;
        m_alarmoutMap[key2].alarmType = type2;
    }
    //alarmout table
    int alarmoutRow = 0;
    for (auto iter = m_alarmoutMap.constBegin(); iter != m_alarmoutMap.constEnd(); ++iter) {
        const AlarmKey &key = iter.key();
        const AlarmInfo &info = iter.value();
        ui->tableView_alarmOut->setItemText(alarmoutRow, 0, key.numberName());
        if (key.type() == AlarmKey::CameraAlarm && (info.alarmCount == 0 || (key.alarmId() == 1 && info.alarmCount == 1))) {
            ui->tableView_alarmOut->setItemText(alarmoutRow, 1, "-");
            ui->tableView_alarmOut->setItemText(alarmoutRow, 2, "-");
            ui->tableView_alarmOut->setItemText(alarmoutRow, 3, "-");
            ui->tableView_alarmOut->setItemText(alarmoutRow, 4, "-");
        } else {
            ui->tableView_alarmOut->setItemText(alarmoutRow, 1, info.alarmName);
            ui->tableView_alarmOut->setItemText(alarmoutRow, 2, info.alarmTypeString());
            ui->tableView_alarmOut->setItemText(alarmoutRow, 3, info.delayString());
            if (info.status) {
                ui->tableView_alarmOut->setItemPixmap(alarmoutRow, 4, pixmapOn);
            } else {
                ui->tableView_alarmOut->setItemPixmap(alarmoutRow, 4, pixmapOff);
            }
        }

        alarmoutRow++;
    }
}

void PageEventStatus::resizeEvent(QResizeEvent *event)
{
    int w = width() - 100;

    //
    ui->tableView_camera->setColumnWidth(0, 100);
    ui->tableView_camera->setColumnWidth(1, 200);
    ui->tableView_camera->setColumnWidth(2, 300);
    ui->tableView_camera->setColumnWidth(3, (w - 600) / 3);
    ui->tableView_camera->setColumnWidth(4, (w - 600) / 3);
    ui->tableView_camera->setColumnWidth(5, (w - 600) / 3);

    //
    ui->tableView_alarmIn->setColumnWidth(0, 100);
    ui->tableView_alarmIn->setColumnWidth(1, 300);
    ui->tableView_alarmIn->setColumnWidth(2, 300);

    //
    ui->tableView_alarmOut->setColumnWidth(0, 100);
    ui->tableView_alarmOut->setColumnWidth(1, 300);
    ui->tableView_alarmOut->setColumnWidth(2, 200);
    ui->tableView_alarmOut->setColumnWidth(3, 200);

    //

    ui->tableView_smart->setColumnWidth(0, 90);
    ui->tableView_smart->setColumnWidth(1, 90);
    ui->tableView_smart->setColumnWidth(2, 130);

    ui->tableView_smart->setColumnWidth(3, 140);
    ui->tableView_smart->setColumnWidth(4, 130);
    ui->tableView_smart->setColumnWidth(5, 210);
    ui->tableView_smart->setColumnWidth(6, 150);
    ui->tableView_smart->setColumnWidth(7, 100);
    ui->tableView_smart->setColumnWidth(8, 90);
    ui->tableView_smart->setColumnWidth(9, 150);

    QWidget::resizeEvent(event);
}

void PageEventStatus::hideEvent(QHideEvent *)
{
    m_timer->stop();
}

void PageEventStatus::onLanguageChanged()
{
    ui->label_alarmInputList->setText(GET_TEXT("EVENTSTATUS/63003", "Alarm Input List"));
    ui->label_alarmOutputList->setText(GET_TEXT("EVENTSTATUS/63004", "Alarm Output List"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void PageEventStatus::onTimerUpdate()
{
    int index = ui->widget_tab->currentTab();
    switch (index) {
    case 0: //camera event
    case 1: //alarm
    case 2: //smart event
    case 3: //people counting
        sendMessage(REQUEST_FLAG_GET_EVENT_STATUS, nullptr, 0);
        break;
    }
}

void PageEventStatus::initializeTableCmaera()
{
    QStringList headerList;
    headerList << GET_TEXT("CHANNELMANAGE/30008", "Channel");
    headerList << GET_TEXT("COMMON/1051", "Name");
    headerList << GET_TEXT("COMMON/1033", "IP Address");
    headerList << GET_TEXT("VIDEOLOSS/50001", "Video Loss");
    headerList << GET_TEXT("VIDEOLOSS/50020", "Motion");
    headerList << GET_TEXT("AUDIO_ALARM/159000", "Audio Alarm");
    ui->tableView_camera->setHorizontalHeaderLabels(headerList);
    ui->tableView_camera->setHeaderCheckable(false);
    ui->tableView_camera->setColumnCount(headerList.size());
    ui->tableView_camera->setSortType(0, SortFilterProxyModel::SortInt);
    ui->tableView_camera->setItemDelegate(new ItemIconDelegate(this));
    ui->tableView_camera->setSortingEnabled(false);

    QMap<int, camera> cameraMap = qMsNvr->cameraMap();
    QMap<int, osd> osdMap = qMsNvr->osdMap();
    ui->tableView_camera->setRowCount(cameraMap.count());

    int row = 0;
    for (auto iter = cameraMap.constBegin(); iter != cameraMap.constEnd(); ++iter) {
        const camera &cam = iter.value();
        int channel = cam.id;
        ui->tableView_camera->setItemIntValue(row, 0, channel + 1);
        ui->tableView_smart->setItemIntValue(row, 0, channel + 1);
        if (osdMap.contains(channel)) {
            const osd &osd_info = osdMap.value(channel);
            ui->tableView_camera->setItemText(row, 1, osd_info.name);
            ui->tableView_camera->setItemToolTip(row, 1, osd_info.name);
            ui->tableView_smart->setItemText(row, 1, osd_info.name);
            ui->tableView_smart->setItemToolTip(row, 1, osd_info.name);
        }
        if (QString(cam.ip_addr).isEmpty() || !cam.enable) {
            ui->tableView_camera->setItemText(row, 2, "-");
            ui->tableView_smart->setItemText(row, 2, "-");
        } else {
            ui->tableView_camera->setItemText(row, 2, cam.ip_addr);
            ui->tableView_smart->setItemText(row, 2, cam.ip_addr);
        }
        row++;
    }
}

void PageEventStatus::initializeTableAlarm()
{
    QStringList headerList;
    headerList << GET_TEXT("CAMERASEARCH/32002", "NO.");
    headerList << GET_TEXT("ALARMIN/52003", "Alarm Name");
    headerList << GET_TEXT("ALARMIN/52004", "Alarm Type");
    headerList << GET_TEXT("MENU/10006", "Status");
    ui->tableView_alarmIn->setHorizontalHeaderLabels(headerList);
    ui->tableView_alarmIn->setHeaderCheckable(false);
    ui->tableView_alarmIn->setColumnCount(headerList.size());
    ui->tableView_alarmIn->setSortType(0, SortFilterProxyModel::SortInt);
    ui->tableView_alarmIn->setItemDelegate(new ItemIconDelegate(this));
    ui->tableView_alarmIn->setSortingEnabled(false);

    alarm_in alarmin_array[MAX_ALARM_IN];
    memset(alarmin_array, 0, sizeof(alarm_in) * MAX_ALARM_IN);
    int alarminCount = 0;
    read_alarm_ins(SQLITE_FILE_NAME, alarmin_array, &alarminCount);
    int maxAlarmInput = qMsNvr->maxAlarmInput();
    for (int i = 0; i < maxAlarmInput; ++i) {
        const alarm_in &alarm = alarmin_array[i];
        AlarmKey key(alarm.id);
        AlarmInfo info;
        info.alarmName = alarm.name;
        info.alarmType = alarm.type;
        m_alarminMap.insert(key, info);
    }
    QMap<AlarmKey, QString> channelAlarminMap = qMsNvr->allChannelAlarminNames();
    for (auto iter = channelAlarminMap.constBegin(); iter != channelAlarminMap.constEnd(); ++iter) {
        const AlarmKey &key = iter.key();
        AlarmInfo info;
        info.alarmName = key.name();
        m_alarminMap.insert(key, info);
    }
    ui->tableView_alarmIn->setRowCount(m_alarminMap.size());

    //
    headerList.clear();
    headerList << GET_TEXT("CAMERASEARCH/32002", "NO.");
    headerList << GET_TEXT("ALARMIN/52003", "Alarm Name");
    headerList << GET_TEXT("ALARMIN/52004", "Alarm Type");
    headerList << GET_TEXT("ALARMOUT/53002", "Delay Time");
    headerList << GET_TEXT("MENU/10006", "Status");
    ui->tableView_alarmOut->setHorizontalHeaderLabels(headerList);
    ui->tableView_alarmOut->setHeaderCheckable(false);
    ui->tableView_alarmOut->setColumnCount(headerList.size());
    ui->tableView_alarmOut->setSortType(0, SortFilterProxyModel::SortInt);
    ui->tableView_alarmOut->setItemDelegate(new ItemIconDelegate(this));
    ui->tableView_alarmOut->setSortingEnabled(false);

    alarm_out alarmout_array[MAX_ALARM_OUT];
    memset(alarmout_array, 0, sizeof(alarm_out) * MAX_ALARM_OUT);
    int alarmoutCount = 0;
    read_alarm_outs(SQLITE_FILE_NAME, alarmout_array, &alarmoutCount);
    int maxAlarmOutput = qMsNvr->maxAlarmOutput();
    for (int i = 0; i < maxAlarmOutput; ++i) {
        const alarm_out &alarm = alarmout_array[i];
        AlarmKey key(alarm.id);
        AlarmInfo info;
        info.alarmName = alarm.name;
        info.alarmType = alarm.type;
        info.delay = alarm.duration_time;
        m_alarmoutMap.insert(key, info);
    }
    QMap<AlarmKey, QString> channelAlarmoutMap = qMsNvr->allChannelAlarmoutNames();
    for (auto iter = channelAlarmoutMap.constBegin(); iter != channelAlarmoutMap.constEnd(); ++iter) {
        const AlarmKey &key = iter.key();
        AlarmInfo info;
        info.alarmName = key.name();
        info.delay = key.delay();
        m_alarmoutMap.insert(key, info);
    }
    ui->tableView_alarmOut->setRowCount(m_alarmoutMap.size());
}

void PageEventStatus::initializeTableSmartEvent()
{
    QStringList headerList;
    headerList << GET_TEXT("CHANNELMANAGE/30008", "Channel");
    headerList << GET_TEXT("CAMERASTATUS/62002", "Name");
    headerList << GET_TEXT("COMMON/1033", "IP Address");
    headerList << GET_TEXT("SMARTEVENT/55001", "Region Entrance");
    headerList << GET_TEXT("SMARTEVENT/55002", "Region Exiting");
    headerList << GET_TEXT("SMARTEVENT/55003", "Advanced Motion Detection");
    headerList << GET_TEXT("SMARTEVENT/55004", "Tamper Detection");
    headerList << GET_TEXT("SMARTEVENT/55005", "Line Crossing");
    headerList << GET_TEXT("SMARTEVENT/55006", "Loitering");
    headerList << GET_TEXT("SMARTEVENT/55007", "Human Detection");
    headerList << GET_TEXT("SMARTEVENT/55055", "Object Left/Removed");
    ui->tableView_smart->setHorizontalHeaderLabels(headerList);
    ui->tableView_smart->setHeaderCheckable(false);
    ui->tableView_smart->setColumnCount(headerList.size());
    ui->tableView_smart->setSortType(0, SortFilterProxyModel::SortInt);
    ui->tableView_smart->setItemDelegate(new ItemIconDelegate(this));
    ui->tableView_smart->setSortingEnabled(false);
}

void PageEventStatus::updateTableCameraStatus(TableView *table, int row, int column, QString ip, bool status)
{
    static QPixmap pixmapOn(":/status/status/alarm_on.png");
    static QPixmap pixmapOff(":/status/status/alarm_off.png");
    TableView *tmp = table;
    if (ip == "-" || ip.isEmpty()) {
        tmp->setItemText(row, column, "-");
    } else if (status) {
        tmp->setItemPixmap(row, column, pixmapOn);
    } else {
        tmp->setItemPixmap(row, column, pixmapOff);
    }
}

void PageEventStatus::onTabClicked(int index)
{
    ui->stackedWidget->setCurrentIndex(index);

    switch (index) {
    case 0: //camera event
        ui->tableView_camera->clearSort();
        break;
    case 1: //alarm
        break;
    case 2: //smart event
        ui->tableView_smart->clearSort();
        break;
    case 3: //people counting
        break;
    }
    m_timer->start();
}

void PageEventStatus::on_pushButton_back_clicked()
{
    emit sig_back();
}
