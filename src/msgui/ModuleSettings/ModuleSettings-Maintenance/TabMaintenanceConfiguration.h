#ifndef TABMAINTENANCECONFIGURATION_H
#define TABMAINTENANCECONFIGURATION_H

#include "AbstractSettingTab.h"

namespace Ui {
class TabMaintenanceConfiguration;
}

class TabMaintenanceConfiguration : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit TabMaintenanceConfiguration(QWidget *parent = nullptr);
    ~TabMaintenanceConfiguration();

    void initializeData() override;

    void processMessage(MessageReceive *message) override;
    void filterMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_UDISK_OFFLINE(MessageReceive *message);

private slots:
    void onLanguageChanged();

    void on_pushButton_browse_import_clicked();
    void on_pushButton_restore_clicked();

    void on_pushButton_browse_export_clicked();
    void on_pushButton_backup_clicked();

    void on_pushButton_back_clicked();

private:
    Ui::TabMaintenanceConfiguration *ui;
    int m_importPort = -1;
    int m_exportPort = -1;
};

#endif // TABMAINTENANCECONFIGURATION_H
