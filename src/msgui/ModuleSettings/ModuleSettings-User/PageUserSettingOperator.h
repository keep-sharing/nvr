#ifndef PAGEUSERSETTINGOPERATOR_H
#define PAGEUSERSETTINGOPERATOR_H

#include "AbstractSettingPage.h"

extern "C" {
#include "msdb.h"
}

namespace Ui {
class UserSettingOperator;
}

class PageUserSettingOperator : public AbstractSettingPage {
    Q_OBJECT

public:
    explicit PageUserSettingOperator(QWidget *parent = nullptr);
    ~PageUserSettingOperator();

    void initializeData() override;

    void processMessage(MessageReceive *message) override;

private slots:
    void onLanguageChanged() override;

    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

    void on_pushButtonEdit_clicked();

    void on_comboBoxChangePw_activated(int index);

    void on_comboBoxUnlockPattern_activated(int index);

private:
    Ui::UserSettingOperator *ui;

    QString m_pattern_text;
    struct db_user m_user;
    bool m_changePsw = false;
};

#endif // PAGEUSERSETTINGOPERATOR_H
