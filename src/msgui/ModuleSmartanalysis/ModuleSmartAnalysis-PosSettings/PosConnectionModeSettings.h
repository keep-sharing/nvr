#ifndef POSCONNECTIONMODESETTINGS_H
#define POSCONNECTIONMODESETTINGS_H

#include "BaseShadowDialog.h"

extern "C" {
#include "msdb.h"
}

namespace Ui {
class PosConnectionModeSettings;
}

class PosConnectionModeSettings : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit PosConnectionModeSettings(QWidget *parent = nullptr);
    ~PosConnectionModeSettings();

    void setConfig(int index, Db_POS_CONFIG *currentConfigs, Db_POS_CONFIG *lastConfigs);

private slots:
    void onLanguageChanged() override;

    void on_pushButtonOk_clicked();
    void on_pushButtonCancel_clicked();

private:
    Ui::PosConnectionModeSettings *ui;

    int m_index = 0;
    Db_POS_CONFIG *m_lastConfigs = nullptr;
    Db_POS_CONFIG *m_currentConfigs = nullptr;
};

#endif // POSCONNECTIONMODESETTINGS_H
