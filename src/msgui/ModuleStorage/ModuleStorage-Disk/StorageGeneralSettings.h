#ifndef STORAGEGENERALSETTINGS_H
#define STORAGEGENERALSETTINGS_H

#include "AbstractSettingTab.h"

extern "C" {
#include "msg.h"
}

namespace Ui {
class StorageGeneralSettings;
}

class StorageGeneralSettings : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit StorageGeneralSettings(QWidget *parent = 0);
    ~StorageGeneralSettings();

    void initializeData() override;

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_REC_ADVANCED(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_REC_RECYCLEMODE(MessageReceive *message);

    void initializeAdvancedPage();

private slots:
    void onLanguageChanged();
    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::StorageGeneralSettings *ui;
    RECORD_ADVANCED m_recordAdvanced;
};

#endif // STORAGEGENERALSETTINGS_H
