#ifndef TABMAINTENANCELOCALUPGRADE_H
#define TABMAINTENANCELOCALUPGRADE_H

#include "AbstractSettingTab.h"

class UpgradeThread;

namespace Ui {
class TabMaintenanceLocalUpgrade;
}

class TabMaintenanceLocalUpgrade : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit TabMaintenanceLocalUpgrade(QWidget *parent = nullptr);
    ~TabMaintenanceLocalUpgrade();

    void initializeData() override;

    bool canAutoLogout() override;

    void processMessage(MessageReceive *message);
    void filterMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_UDISK_OFFLINE(MessageReceive *message);

private slots:
    void onLanguageChanged();

    void onUpgradeFinished(int result);

    void on_pushButton_browse_clicked();
    void on_pushButton_upgrade_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::TabMaintenanceLocalUpgrade *ui;

    UpgradeThread *m_upgradeThread = nullptr;
    int m_currentPort = -1;

    bool m_isAboutToReboot = false;
};

#endif // TABMAINTENANCELOCALUPGRADE_H
