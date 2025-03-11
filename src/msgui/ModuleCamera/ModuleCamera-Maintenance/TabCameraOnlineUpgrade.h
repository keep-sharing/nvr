#ifndef TABCAMERAONLINEUPGRADE_H
#define TABCAMERAONLINEUPGRADE_H

#include "AbstractSettingTab.h"
#include "camerakey.h"
#include <QEventLoop>
#include <QPersistentModelIndex>

extern "C" {
#include "msg.h"
}

class MessageReceive;

namespace Ui {
class CameraOnlineUpgrade;
}

class TabCameraOnlineUpgrade : public AbstractSettingTab {
    Q_OBJECT

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
        ColumnLatest,
        ColumnUpgrade,
        ColumnProgress
    };

    enum CheckState {
        StateNone = -2,
        StateDisconnect = -1,
        StateChecked
    };

    struct UpgradeInfo {
        resp_check_online_upgrade_ipc info;
        int checkState = StateNone;
        int upgradeState = -1;
        bool isChecked = false;
        UpgradeInfo()
        {
            memset(&info, 0, sizeof(info));
        }
        bool canUpgrade() const
        {
            if (QString(info.upInfo.pUrl).isEmpty()) {
                return false;
            }
            if (upgradeState == UPGRADE_SUCCESS || upgradeState == UPGRADE_ING || upgradeState == UPGRADE_WAITING) {
                return false;
            }
            return true;
        }
    };

public:
    explicit TabCameraOnlineUpgrade(QWidget *parent = nullptr);
    ~TabCameraOnlineUpgrade();

    void initializeData() override;

    void processMessage(MessageReceive *message) override;

    bool isCloseable() override;
    bool isChangeable() override;
    bool canAutoLogout() override;

private:
    void ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message);
    void ON_RESPONSE_FLAG_CHECK_ONLINE_IPC(MessageReceive *message);
    void ON_RESPONSE_FLAG_ONLINE_UPGRADE_CAMERA(MessageReceive *message);

    void showCameraTable();
    void setTextByChannel(int channel, int Column, const QString &text);

    void startUpgrede();
    void stopUpgrede();

    QString progressString(int state) const;

private slots:
    //table
    void onTableItemClicked(int row, int column);
    void onTableHeaderChecked(bool checked);

    void on_pushButton_check_clicked();
    void on_pushButton_upgrade_clicked();
    void on_pushButton_refresh_clicked();
    void on_pushButton_back_clicked();

protected:
    void onLanguageChanged() override;

private:
    int m_isUpgrading = 0;
    bool m_isCancel = false;
    bool m_isAboutToClose = false;

    Ui::CameraOnlineUpgrade *ui;

    QEventLoop m_upgradeLoop;
    QList<resq_get_ipcdev> m_allCameraList;

    //key: channel
    QMap<int, UpgradeInfo> m_onlineMap;
    //key: url
    QMap<QString, QList<UpgradeInfo>> m_upgradeListMap;
    QList<QString> m_upgradeIndex;
};

#endif // TABCAMERAONLINEUPGRADE_H
