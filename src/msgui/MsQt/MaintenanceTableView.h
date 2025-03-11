#ifndef MAINTENANCETABLEVIEW_H
#define MAINTENANCETABLEVIEW_H

#include "tableview.h"
#include <QWidget>
extern "C" {
#include "msg.h"
}
struct MaintenanceTablelItemInfo {
    MaintenanceTablelItemInfo() { }
    MaintenanceTablelItemInfo(bool checked, QString str)
        : isChecked(checked)
        , state(str)
    {
    }
    bool isChecked;
    QString state;
};

class MaintenanceTableView : public TableView {
    Q_OBJECT
public:
    enum CameraColumn {
        ColumnCheck,
        ColumnChannel,
        ColumnName,
        ColumnStatus,
        ColumnIP,
        ColumnChannelId,
        ColumnProtocol,
        ColumnMAC,
        ColumnFirmware,
        ColumnModel,
        ColumnProgress
    };
    explicit MaintenanceTableView(QWidget *parent = nullptr);
    void refreshTable(QList<resq_get_ipcdev> cameraList, QMap<int, MaintenanceTablelItemInfo> stateMap);
    void setUpgradeProgressText(QMap<int, MaintenanceTablelItemInfo> &stateMap, int channel, const QString &text);
    void updateChecked(QMap<int, MaintenanceTablelItemInfo> &stateMap);
};

#endif // MAINTENANCETABLEVIEW_H
