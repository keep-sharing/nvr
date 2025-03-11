#include "MaintenanceTableView.h"
#include "CameraStatusWidget.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"

extern "C" {
#include "msg.h"
}

const int CameraInfoRole = Qt::UserRole + 100;
MaintenanceTableView::MaintenanceTableView(QWidget *parent)
    : TableView(parent)
{
    QStringList headerList;

    headerList << "";
    headerList << GET_TEXT("CHANNELMANAGE/30008", "Channel");
    headerList << GET_TEXT("CHANNELMANAGE/30009", "Channel Name");
    headerList << GET_TEXT("MENU/10006", "Status");
    headerList << GET_TEXT("COMMON/1033", "IP Address");
    headerList << GET_TEXT("FISHEYE/12016", "Channel ID");
    headerList << GET_TEXT("CHANNELMANAGE/30014", "Protocol");
    headerList << GET_TEXT("CHANNELMANAGE/30027", "MAC");
    headerList << GET_TEXT("CHANNELMANAGE/30054", "Firmware Version");
    headerList << GET_TEXT("CHANNELMANAGE/30029", "Model");
    headerList << GET_TEXT("CAMERAMAINTENANCE/38005", "Upgrade Progress");
    setHorizontalHeaderLabels(headerList);
    setColumnCount(headerList.size());

    //sort
    setSortableForColumn(ColumnCheck, false);
    setSortType(ColumnChannel, SortFilterProxyModel::SortInt);
    setSortType(ColumnIP, SortFilterProxyModel::SortIP);
    setSortType(ColumnStatus, SortFilterProxyModel::SortInt);
    //column width
    setColumnWidth(ColumnCheck, 50);
    setColumnWidth(ColumnChannel, 90);
    setColumnWidth(ColumnName, 150);
    setColumnWidth(ColumnStatus, 150);
    setColumnWidth(ColumnIP, 150);
    setColumnWidth(ColumnChannelId, 100);
    setColumnWidth(ColumnMAC, 140);
    setColumnWidth(ColumnFirmware, 180);
    setColumnWidth(ColumnModel, 180);
}

void MaintenanceTableView::refreshTable(QList<resq_get_ipcdev> cameraList, QMap<int, MaintenanceTablelItemInfo> stateMap)
{
    clearContent();
    setRowCount(cameraList.size());
    int row = 0;
    for (int i = 0; i < cameraList.size(); ++i) {
        const resq_get_ipcdev &ipcdev = cameraList.at(i);
        setItemData(row, ColumnCheck, QVariant::fromValue(ipcdev), CameraInfoRole);
        if (stateMap.contains(ipcdev.chanid)) {
            setItemChecked(i,  stateMap.value(ipcdev.chanid).isChecked);
        } else {
            setItemChecked(i,  false);
        }
        //channel
        setItemIntValue(row, ColumnChannel, ipcdev.chanid + 1);
        //channel name
        setItemText(row, ColumnName, qMsNvr->channelName(ipcdev.chanid));
        setItemToolTip(row, ColumnName,itemText(row, ColumnName));
        //status
        CameraStatusWidget *statusWidget = new CameraStatusWidget();
        statusWidget->setState(ipcdev.state);
        statusWidget->setStateString(ipcdev.connect_state);
        setItemWidget(i, ColumnStatus, statusWidget);
        setItemData(i, ColumnStatus, statusWidget->stateValue(), SortIntRole);
        //ip
        setItemText(row, ColumnIP, ipcdev.ipaddr);
        //channel id
        setItemText(i, ColumnChannelId, "-");
        //protocol
        if (ipcdev.protocol == IPC_PROTOCOL_RTSP) {
            setItemText(row, ColumnProtocol, "RTSP");
        } else {
            setItemText(row, ColumnProtocol, qMsNvr->protocolName(ipcdev.protocol));
        }
        //mac
        setItemText(row, ColumnMAC, ipcdev.mac);
        //firmware
        setItemText(row, ColumnFirmware, ipcdev.fwversion);
        //model
        setItemText(row, ColumnModel, ipcdev.model);
        //upgrade progress
        if (stateMap.contains(ipcdev.chanid) && !stateMap.value(ipcdev.chanid).state.isEmpty()) {
            setItemText(i, ColumnProgress, stateMap.value(ipcdev.chanid).state);
        } else {
            setItemText(row, ColumnProgress, "-");
        }

        row++;
    }
    reorder();
}

void MaintenanceTableView::setUpgradeProgressText(QMap<int, MaintenanceTablelItemInfo> &stateMap, int channel, const QString &text)
{
    stateMap[channel].state = text;
    for (int i = 0; i < rowCount(); ++i) {
        const resq_get_ipcdev &ipcdev = itemData(i, MaintenanceTableView::ColumnCheck, CameraInfoRole).value<resq_get_ipcdev>();
        if (ipcdev.chanid == channel) {
            setItemText(i, MaintenanceTableView::ColumnProgress, text);
        }
    }
}

void MaintenanceTableView::updateChecked(QMap<int, MaintenanceTablelItemInfo> &stateMap)
{
    for (int i = 0; i < rowCount(); ++i) {
        const resq_get_ipcdev &ipcdev = itemData(i, MaintenanceTableView::ColumnCheck, CameraInfoRole).value<resq_get_ipcdev>();
        MaintenanceTablelItemInfo &maintenanceTablelItemInfo = stateMap[ipcdev.chanid];
        if (isItemChecked(i)) {
            maintenanceTablelItemInfo.isChecked = true;
        } else {
            maintenanceTablelItemInfo.isChecked = false;
        }
    }
}
