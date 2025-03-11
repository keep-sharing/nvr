#ifndef PAGEUSERSETTINGS_H
#define PAGEUSERSETTINGS_H

#include "AbstractSettingPage.h"
#include "itembuttonwidget.h"
#include <QWidget>

extern "C" {
#include "msdb.h"
}

Q_DECLARE_METATYPE(db_user)

namespace Ui {
class UserSetting;
}

class PageUserSettings : public AbstractSettingPage {
    Q_OBJECT

    enum PageIndex {
        PageUser,
        PageSecurityQuestion
    };

public:
    enum UserColumn {
        ColumnCheck,
        ColumnID,
        ColumnUserName,
        ColumnUserLevel,
        ColumnEdit,
        ColumnDelete
    };

    explicit PageUserSettings(QWidget *parent = nullptr);
    ~PageUserSettings();

    void initializeData() override;

    void processMessage(MessageReceive *message) override;

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateTable();
    //
    void initializeSecurityQuestion();

private slots:
    void onLanguageChanged() override;

    void onTableItemClicked(int row, int column);
    void onTabBarClicked(int index);

    void on_pushButton_add_clicked();
    void on_pushButton_back_clicked();

    void on_pushButton_question_apply_clicked();
    void on_pushButton_question_back_clicked();

private:
    Ui::UserSetting *ui;

    QMap<int, db_user> m_userMap;

    bool m_isQuestionSet = false;
};

#endif // PAGEUSERSETTINGS_H
