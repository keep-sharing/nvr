#ifndef TABHEATMAPAUTOBACKUP_H
#define TABHEATMAPAUTOBACKUP_H

#include "AbstractSettingTab.h"

namespace Ui {
class TabHeatMapAutoBackup;
}

class TabHeatMapAutoBackup : public AbstractSettingTab
{
    Q_OBJECT

public:
    explicit TabHeatMapAutoBackup(QWidget *parent = nullptr);
    ~TabHeatMapAutoBackup();
    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_SET_REPORT_AUTO_BACKUP_SETTINGS(MessageReceive *message);

private slots:
    void onLanguageChanged() override;

    void on_comboBoxAutoBackup_indexSet(int index);
    void on_comboBoxDay_indexSet(int index);
    void on_comboBoxTimeRange_indexSet(int index);

    void on_pushButtonApply_clicked();
    void on_pushButtonBack_clicked();

    void on_checkBoxTimeHeatMap_clicked(bool checked);

private:
    Ui::TabHeatMapAutoBackup *ui;
    int m_currentRangeType = 0;
    REPORT_AUTO_BACKUP_S m_settings;
};

#endif // TABHEATMAPAUTOBACKUP_H
