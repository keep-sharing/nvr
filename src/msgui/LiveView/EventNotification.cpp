#include "EventNotification.h"
#include "ui_EventNotification.h"
#include "MsLanguage.h"
#include "centralmessage.h"
#include <QDesktopWidget>
#include <QTimer>
#include <QtDebug>
#include "MsDevice.h"

EventNotification::EventNotification(QWidget *parent)
    : BasePopup(parent)
    , ui(new Ui::EventNotification)
{
    ui->setupUi(this);

    QStringList treeInfoHeader;
    treeInfoHeader << "Empty";
    treeInfoHeader << "Event/Exception";
    treeInfoHeader << "Information";
    ui->treeView_info->setHorizontalHeaderLabels(treeInfoHeader);
    ui->treeView_info->hideColumn(0);
    ui->treeView_info->setColumnWidth(1, 200);

    QStringList treeSettingHeader;
    treeSettingHeader << "";
    treeSettingHeader << GET_TEXT("SYSTEMGENERAL/164000", "All");
    ui->treeView_setting->setHorizontalHeaderLabels(treeSettingHeader);
    ui->treeView_setting->setColumnWidth(0, 30);
    ui->treeView_setting->setHorizontalHeaderCheckable(true);
    ui->treeView_setting->setItemDelegateForColumn(0, new ItemCheckedButtonDelegate(ItemCheckedButtonDelegate::ButtonCheckBox, this));
    ui->treeView_setting->clearContent();
    if (qMsNvr->is3536a()) {
        ui->treeView_setting->setRowCount(12);
    } else {
        ui->treeView_setting->setRowCount(8);
    }
    ui->treeView_setting->setItemText(0, 0, "");
    ui->treeView_setting->setItemText(0, 1, "Network Disconnected");
    ui->treeView_setting->setItemText(1, 0, "");
    ui->treeView_setting->setItemText(1, 1, "IP Address Conflict");
    ui->treeView_setting->setItemText(2, 0, "");
    ui->treeView_setting->setItemText(2, 1, "Disk Full");
    ui->treeView_setting->setItemText(3, 0, "");
    ui->treeView_setting->setItemText(3, 1, "Record Failed");
    ui->treeView_setting->setItemText(4, 0, "");
    ui->treeView_setting->setItemText(4, 1, "Disk Error");
    ui->treeView_setting->setItemText(5, 0, "");
    ui->treeView_setting->setItemText(5, 1, "Disk Uninitialized");
    ui->treeView_setting->setItemText(6, 0, "");
    ui->treeView_setting->setItemText(6, 1, "No Disk");
    ui->treeView_setting->setItemText(7, 0, "");
    ui->treeView_setting->setItemText(7, 1, "Disk Offine");
    if (qMsNvr->is3536a()) {
       ui->treeView_setting->setItemText(8, 0, "");
        ui->treeView_setting->setItemText(8, 1, "Disk Heat");
        ui->treeView_setting->setItemText(9, 0, "");
        ui->treeView_setting->setItemText(9, 1, "Disk Microtherm");
        ui->treeView_setting->setItemText(10, 0, "");
        ui->treeView_setting->setItemText(10, 1, "Disk Connection Exception");
        ui->treeView_setting->setItemText(11, 0, "");
        ui->treeView_setting->setItemText(11, 1, "Disk Strike");
    }

    memset(&m_alarms, 0, sizeof(trigger_alarms));

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(getException()));
    m_timer->setInterval(10000);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

EventNotification::~EventNotification()
{
    delete ui;
}

void EventNotification::setPos(const QPoint &p)
{
    m_pos = p;
}

QPoint EventNotification::calculatePos()
{
    return m_pos;
}

void EventNotification::closePopup(BasePopup::CloseType type)
{
    Q_UNUSED(type)
}

void EventNotification::setExceptionMap(const QMap<QString, QString> &exceptionMap)
{
    ui->treeView_info->clearContent();
    ui->treeView_info->setRowCount(exceptionMap.size());
    QMap<QString, QString>::const_iterator iter = exceptionMap.constBegin();
    int row = 0;
    for (; iter != exceptionMap.constEnd(); ++iter) {
        ui->treeView_info->setItemText(row, 1, iter.key());
        ui->treeView_info->setItemText(row, 2, iter.value());
        row++;
    }
}

void EventNotification::getException()
{
    sendMessage(REQUEST_FLAG_GET_EXCEPTION, (void *)NULL, 0);
}

void EventNotification::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_EXCEPTION:
        ON_RESPONSE_FLAG_GET_EXCEPTION(message);
        break;
    }
}

void EventNotification::ON_RESPONSE_FLAG_GET_EXCEPTION(MessageReceive *message)
{
    int count = message->header.size / sizeof(struct resp_get_exception);
    resp_get_exception *exceptionList = (resp_get_exception *)message->data;

    ui->treeView_info->clearContent();
    ui->treeView_info->setRowCount(count);
    for (int i = 0; i < count; ++i) {
        const resp_get_exception &exception = exceptionList[i];
        ui->treeView_info->setItemText(i, 1, exception.pEvent);
        ui->treeView_info->setItemText(i, 2, exception.pInformation);
    }
}

void EventNotification::mousePressEvent(QMouseEvent *event)
{
    BasePopup::mousePressEvent(event);
}

void EventNotification::showEvent(QShowEvent *event)
{
    ui->stackedWidget->setCurrentIndex(0);
    if (!m_timer->isActive()) {
        m_timer->start();
        getException();
    }

    BasePopup::showEvent(event);
}

void EventNotification::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    BasePopup::hideEvent(event);
}

bool EventNotification::isNotificationChecked(int value)
{
    if ((value >> 3) & 0x01) {
        return false;
    } else {
        return true;
    }
}

void EventNotification::makeNotificationValue(int &value, bool checked)
{
    if (checked) {
        value &= ~(0x01 << 3);
    } else {
        value |= (0x01 << 3);
    }
}

void EventNotification::onLanguageChanged()
{
    QStringList treeSettingHeader;
    treeSettingHeader << "";
    treeSettingHeader << GET_TEXT("SYSTEMGENERAL/164000", "All");
    ui->treeView_setting->setHorizontalHeaderLabels(treeSettingHeader);
}

void EventNotification::on_pushButton_setting_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);

    read_trigger_alarms(SQLITE_FILE_NAME, &m_alarms);
    ui->treeView_setting->setItemChecked(0, isNotificationChecked(m_alarms.network_disconn));
    ui->treeView_setting->setItemChecked(1, isNotificationChecked(m_alarms.ipConflict));
    ui->treeView_setting->setItemChecked(2, isNotificationChecked(m_alarms.disk_full));
    ui->treeView_setting->setItemChecked(3, isNotificationChecked(m_alarms.record_fail));
    ui->treeView_setting->setItemChecked(4, isNotificationChecked(m_alarms.disk_fail));
    ui->treeView_setting->setItemChecked(5, isNotificationChecked(m_alarms.disk_unformat));
    ui->treeView_setting->setItemChecked(6, isNotificationChecked(m_alarms.no_disk));
    ui->treeView_setting->setItemChecked(7, isNotificationChecked(m_alarms.diskOffline));
    if (qMsNvr->is3536a()) {
        ui->treeView_setting->setItemChecked(8, isNotificationChecked(m_alarms.diskHeat));
        ui->treeView_setting->setItemChecked(9, isNotificationChecked(m_alarms.diskMicrotherm));
        ui->treeView_setting->setItemChecked(10, isNotificationChecked(m_alarms.diskConnectionException));
        ui->treeView_setting->setItemChecked(11, isNotificationChecked(m_alarms.diskStrike));
    }
}

void EventNotification::on_pushButton_ok_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);

    makeNotificationValue(m_alarms.network_disconn, ui->treeView_setting->isItemChecked(0));
    makeNotificationValue(m_alarms.ipConflict, ui->treeView_setting->isItemChecked(1));
    makeNotificationValue(m_alarms.disk_full, ui->treeView_setting->isItemChecked(2));
    makeNotificationValue(m_alarms.record_fail, ui->treeView_setting->isItemChecked(3));
    makeNotificationValue(m_alarms.disk_fail, ui->treeView_setting->isItemChecked(4));
    makeNotificationValue(m_alarms.disk_unformat, ui->treeView_setting->isItemChecked(5));
    makeNotificationValue(m_alarms.no_disk, ui->treeView_setting->isItemChecked(6));
    makeNotificationValue(m_alarms.diskOffline, ui->treeView_setting->isItemChecked(7));
    if (qMsNvr->is3536a()) {
        makeNotificationValue(m_alarms.diskHeat, ui->treeView_setting->isItemChecked(8));
        makeNotificationValue(m_alarms.diskMicrotherm, ui->treeView_setting->isItemChecked(9));
        makeNotificationValue(m_alarms.diskConnectionException, ui->treeView_setting->isItemChecked(10));
        makeNotificationValue(m_alarms.diskStrike, ui->treeView_setting->isItemChecked(11));
    }
    write_trigger_alarms(SQLITE_FILE_NAME, &m_alarms);

    sendMessageOnly(REQUEST_FLAG_SET_TRIGGER_ALARMS, (void *)NULL, 0);
}

void EventNotification::on_treeView_setting_clicked(const QModelIndex &index)
{
    switch (index.column()) {
    case 0: {
        bool checked = ui->treeView_setting->isItemChecked(index.row());
        ui->treeView_setting->setItemChecked(index.row(), !checked);
        break;
    }
    default:
        break;
    }
}
