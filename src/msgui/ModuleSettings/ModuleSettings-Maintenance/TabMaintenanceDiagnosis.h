#ifndef TABMAINTENANCEDIAGNOSIS_H
#define TABMAINTENANCEDIAGNOSIS_H

#include "AbstractSettingTab.h"

namespace Ui {
class TabMaintenanceDiagnosis;
}

class TabMaintenanceDiagnosis : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit TabMaintenanceDiagnosis(QWidget *parent = 0);
    ~TabMaintenanceDiagnosis();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;
    void filterMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_EXPORT_DIAGNOSTIC_LOG(MessageReceive *message);
    void ON_RESPONSE_FLAG_UDISK_OFFLINE(MessageReceive *message);

private slots:
    void onLanguageChanged() override;

    void on_myPushButtonBrowse_clicked();
    void on_myPushButtonBackup_clicked();
    void on_myPushButtonBack_clicked();

private:
    Ui::TabMaintenanceDiagnosis *ui;

    int m_currentPort = -1;
};

#endif // TABMAINTENANCEDIAGNOSIS_H
