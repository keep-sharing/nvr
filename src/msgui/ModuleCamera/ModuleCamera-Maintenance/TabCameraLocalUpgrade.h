#ifndef TABCAMERALOCALUPGRADE_H
#define TABCAMERALOCALUPGRADE_H

#include "AbstractSettingTab.h"
#include "camerakey.h"
#include <QEventLoop>
#include <QPersistentModelIndex>
#include "MaintenanceTableView.h"

extern "C" {
#include "msg.h"
}

class MessageReceive;

namespace Ui {
class CameraLocalUpgrade;
}

class TabCameraLocalUpgrade : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit TabCameraLocalUpgrade(QWidget *parent = nullptr);
    ~TabCameraLocalUpgrade();

    void initializeData() override;

    void processMessage(MessageReceive *message) override;

    bool isCloseable() override;
    bool isChangeable() override;
    bool canAutoLogout() override;

protected:
    void onLanguageChanged() override;

private:
    void ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message);
    void ON_RESPONSE_FLAG_UPGRADE_IPC_IMAGE(MessageReceive *message);

    void setUpgradeEnabled(bool enable);

private slots:
    //table
    void onTableItemClicked(int row, int column);
    void onTableHeaderChecked(bool checked);

    void on_pushButton_browse_clicked();
    void on_pushButton_upgrade_clicked();
    void on_pushButton_refresh_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::CameraLocalUpgrade *ui;

    QList<resq_get_ipcdev> m_allCameraList;

    QString m_filePath;
    int m_currentChannel = -1;
    QMap<int, int> m_upgradeMap;
    //key MAC
    QMap<int, MaintenanceTablelItemInfo> m_stateMap;

    QEventLoop m_eventLoop;
    QEventLoop m_quitLoop;

    bool m_isCancel = false;
    bool m_isAboutToClose = false;
};

#endif // TABCAMERALOCALUPGRADE_H
