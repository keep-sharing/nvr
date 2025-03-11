#ifndef TABMAINTENANCEONLINEUPGRADE_H
#define TABMAINTENANCEONLINEUPGRADE_H

#include "AbstractSettingTab.h"

class UpgradeThread;

namespace Ui {
class TabMaintenanceOnlineUpgrade;
}

class TabMaintenanceOnlineUpgrade : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit TabMaintenanceOnlineUpgrade(QWidget *parent = nullptr);
    ~TabMaintenanceOnlineUpgrade();

    bool canAutoLogout() override;

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_CHECK_ONLINE_NVR(MessageReceive *message);

    quint64 maxFirmwareSize() const;

private slots:
    void onLanguageChanged() override;

    void onDownloadFinished(int result);
    void onUpgradeFinished(int result);

    void on_pushButton_check_clicked();
    void on_pushButton_upgrade_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::TabMaintenanceOnlineUpgrade *ui;

    UpgradeThread *m_upgradeThread = nullptr;
    QString latest_ver, curren_ver, url;
    int upgrade_state;
    quint64 m_fileSize = 0;
    bool m_isAboutToReboot = false;
};

#endif // TABMAINTENANCEONLINEUPGRADE_H
