#include "TabCameraStatus.h"
#include "ui_TabCameraStatus.h"
#include "LiveVideo.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "CameraStatusWidget.h"
#include "centralmessage.h"
#include "networkchart.h"

const int EnableRole = Qt::UserRole + 1;

TabCameraStatus::TabCameraStatus(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::PageCameraStatus)
{
    ui->setupUi(this);

    QStringList headerList;
    headerList << GET_TEXT("CHANNELMANAGE/30008", "Channel");
    headerList << GET_TEXT("CAMERASTATUS/62002", "Name");
    headerList << GET_TEXT("COMMON/1033", "IP Address");
    headerList << GET_TEXT("MENU/10004", "Record");
    headerList << GET_TEXT("CHANNELMANAGE/30066", "Frame Rate");
    headerList << GET_TEXT("CHANNELMANAGE/30060", "Bit Rate");
    headerList << GET_TEXT("CHANNELMANAGE/30058", "Frame Size");
    headerList << GET_TEXT("MENU/10006", "Status");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setHeaderCheckable(false);
    ui->tableView->setColumnCount(headerList.size());
    ui->tableView->setSortType(ColumnStatus, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnChannel, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnIP, SortFilterProxyModel::SortIP);

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimerout()));
    m_timer->setInterval(5000);
}

TabCameraStatus::~TabCameraStatus()
{
    delete ui;
}

void TabCameraStatus::initializeData()
{
    ui->tableView->clearSort();
    onTimerout();
}

void TabCameraStatus::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPCLIST:
        ON_RESPONSE_FLAG_GET_IPCLIST(message);
        break;
    default:
        break;
    }
}

void TabCameraStatus::resizeEvent(QResizeEvent *event)
{
    int tableWidth = ui->tableView->width();
    int columnWidth = (tableWidth - 50) / ui->tableView->columnCount();
    for (int i = 0; i < ui->tableView->columnCount(); ++i) {
        ui->tableView->setColumnWidth(i, columnWidth);
    }
    QWidget::resizeEvent(event);
}

void TabCameraStatus::showEvent(QShowEvent *event)
{
    if (!m_timer->isActive()) {
        m_timer->start();
    }

    QWidget::showEvent(event);
}

void TabCameraStatus::hideEvent(QHideEvent *event)
{
    m_timer->stop();

    QWidget::hideEvent(event);
}

void TabCameraStatus::onTimerout()
{
    sendMessage(REQUEST_FLAG_GET_IPCLIST, nullptr, 0);
}
/**
* @description 获取通道状态信息,并显示通道Disconnected原因
*  @date            2021.02.26
* @author:        Kirin
*/
void TabCameraStatus::ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message)
{
    ui->tableView->clearContent();

    resq_get_ipcdev *ipcdev = (resq_get_ipcdev *)message->data;
    int ipcdevCount = message->header.size / sizeof(resq_get_ipcdev);
    if (!ipcdev) {
        qMsWarning() << "data is null.";
        return;
    }

    QMap<int, resq_get_ipcdev> ipcdevMap;
    for (int i = 0; i < ipcdevCount; ++i) {
        const resq_get_ipcdev &ipc = ipcdev[i];
        int channel = ipc.chanid;
        ipcdevMap.insert(channel, ipc);
    }

    const int maxChannel = qMsNvr->maxChannel();

    int row = 0;
    for (int i = 0; i < maxChannel; ++i, ++row) {
        QString strName("-");
        QString strIP("-");
        QString strRecord("-");
        QString strFrameRate("-");
        QString strBitRate("-");
        QString strFrameSize("-");

        //channel
        ui->tableView->setItemIntValue(row, ColumnChannel, i + 1);

        if (ipcdevMap.contains(i)) {
            const resq_get_ipcdev &ipc = ipcdevMap.value(i);
            int channel = ipc.chanid;

            //name
            strName = qMsNvr->channelName(channel);
            ui->tableView->setItemText(row, ColumnName, strName);
            ui->tableView->setItemToolTip(i, ColumnName, ui->tableView->itemText(i, ColumnName));

            //ip
            strIP = ipc.ipaddr;

            const resp_camera_status &camera_status = LiveVideo::streamStatus(channel);
            //record
            if (camera_status.record == 1) {
                strRecord = GET_TEXT("COMMON/1012", "ON");
            } else {
                strRecord = GET_TEXT("COMMON/1013", "OFF");
            }
            strFrameRate = QString("%1 fps").arg(camera_status.status[STREAM_TYPE_MAINSTREAM].frame_rate);
            strBitRate = QString("%1").arg(NetworkChart::valueString(camera_status.status[STREAM_TYPE_MAINSTREAM].bit_rate));
            strFrameSize = QString("%1x%2").arg(camera_status.status[STREAM_TYPE_MAINSTREAM].cur_res.width).arg(camera_status.status[STREAM_TYPE_MAINSTREAM].cur_res.height);
            //status
            CameraStatusWidget *statusWidget = new CameraStatusWidget();
            statusWidget->setState(ipc.state);
            statusWidget->setStateString(ipc.connect_state);
            ui->tableView->setItemWidget(row, ColumnStatus, statusWidget);
            ui->tableView->setItemData(row, ColumnStatus, statusWidget->stateValue(), SortIntRole);

            //
            ui->tableView->setItemText(row, ColumnIP, strIP);
            ui->tableView->setItemText(row, ColumnRecord, strRecord);
            ui->tableView->setItemText(row, ColumnFrameRate, strFrameRate);
            ui->tableView->setItemText(row, ColumnBitRate, strBitRate);
            ui->tableView->setItemText(row, ColumnFrameSize, strFrameSize);
        } else {
            ui->tableView->setItemText(row, ColumnName, "-");
            ui->tableView->setItemToolTip(i, ColumnName, ui->tableView->itemText(i, ColumnName));
            ui->tableView->setItemText(row, ColumnIP, "-");
            ui->tableView->setItemText(row, ColumnRecord, "-");
            ui->tableView->setItemText(row, ColumnFrameRate, "-");
            ui->tableView->setItemText(row, ColumnBitRate, "-");
            ui->tableView->setItemText(row, ColumnFrameSize, "-");
            ui->tableView->setItemText(row, ColumnStatus, "-");
        }
    }
    ui->tableView->reorder();
}
