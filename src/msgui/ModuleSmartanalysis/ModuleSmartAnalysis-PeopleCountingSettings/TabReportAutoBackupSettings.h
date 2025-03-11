#ifndef TABREPORTAUTOBACKUPSETTINGS_H
#define TABREPORTAUTOBACKUPSETTINGS_H

#include "AbstractSettingTab.h"

namespace Ui {
class TabReportAutoBackupSettings;
}

class TabReportAutoBackupSettings : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit TabReportAutoBackupSettings(QWidget *parent = 0);
    ~TabReportAutoBackupSettings();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_SET_REPORT_AUTO_BACKUP_SETTINGS(MessageReceive *message);

    void setChannelVisible(bool enable);
    void setGroupVisible(bool enable);
    void setLengthOfStayVisible(bool enable);
    void setLineVisible(bool enable);

    void saveSetting(int type);

private slots:
    void onLanguageChanged() override;

    void on_comboBoxType_indexSet(int index);
    void on_comboBoxStay_indexSet(int index);
    void on_comboBoxDay_indexSet(int index);

    void on_pushButtonApply_clicked();
    void on_pushButtonBack_clicked();

    void on_comboBoxAutoBackup_indexSet(int index);

private:
    Ui::TabReportAutoBackupSettings *ui;

    REPORT_AUTO_BACKUP_S m_settings;
    int m_currentType = -1;
    QMap<int, resp_usb_info> m_usbInfoMap;
    Uint8 m_lineMask = 8;
};

#endif // TABREPORTAUTOBACKUPSETTINGS_H
