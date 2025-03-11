#ifndef POSPRIVACYSETTINGS_H
#define POSPRIVACYSETTINGS_H

#include "BaseShadowDialog.h"

extern "C" {
#include "msdb.h"
}

namespace Ui {
class PosPrivacySettings;
}

class PosPrivacySettings : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit PosPrivacySettings(QWidget *parent = nullptr);
    ~PosPrivacySettings();

    void setConfig(Db_POS_CONFIG *config);

private slots:
    void onLanguageChanged() override;
    void on_pushButtonOk_clicked();
    void on_pushButtonCancel_clicked();

private:
    Ui::PosPrivacySettings *ui;

    Db_POS_CONFIG *m_config = nullptr;
};

#endif // POSPRIVACYSETTINGS_H
