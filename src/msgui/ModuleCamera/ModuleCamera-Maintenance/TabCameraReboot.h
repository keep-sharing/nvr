#ifndef TABCAMERAREBOOT_H
#define TABCAMERAREBOOT_H

#include "AbstractSettingTab.h"
#include <QEventLoop>
#include "MaintenanceTableView.h"
extern "C" {
#include "msg.h"
}

namespace Ui {
class TabCameraReboot;
}

class TabCameraReboot : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit TabCameraReboot(QWidget *parent = nullptr);
    ~TabCameraReboot();
    void initializeData() override;

    void processMessage(MessageReceive *message) override;

    bool isCloseable() override;
    bool isChangeable() override;
    bool canAutoLogout() override;

protected:
    void onLanguageChanged() override;

private:
    void ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPC_REBOOT(MessageReceive *message);

    void startUpgrede();
    void stopUpgrede();

private slots:
    void on_pushButtonReboot_clicked();
    void on_pushButtonRefresh_clicked();
    void on_pushButtonBack_clicked();


private:
    Ui::TabCameraReboot *ui;
    QList<resq_get_ipcdev> m_allCameraList;

    QString m_filePath;
    int m_currentChannel = -1;
    QMap<int, int> m_channelMap;
    QMap<int, MaintenanceTablelItemInfo> m_stateMap;

    QEventLoop m_eventLoop;

    bool m_isCancel = false;
    bool m_isAboutToClose = false;
    bool m_isUpgrading = false;
};

#endif // TABCAMERAREBOOT_H
