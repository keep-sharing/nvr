#ifndef EDITUSERDIALOG_H
#define EDITUSERDIALOG_H

#include "BaseShadowDialog.h"

extern "C" {
#include "msdb.h"
}

namespace Ui {
class EditUserPassword;
}

class EditUserDialog : public BaseShadowDialog {
    Q_OBJECT

public:
    enum LogoutType {
        TypeLogout = 10,
    };
    explicit EditUserDialog(QWidget *parent = nullptr);
    ~EditUserDialog();

    int execEdit(const db_user &user);

private slots:
    void onLanguageChanged();

    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

    void on_comboBox_patternEnable_activated(int index);
    void on_pushButton_patternEdit_clicked();
    void on_comboBox_changePassword_activated(int index);

    void on_pushButtonPermissionsEdit_clicked();

private:
    Ui::EditUserPassword *ui;

    db_user m_userInfo;
    QString m_pattern_text;
    db_user m_userOperator;
    db_user m_userViewer;
    bool m_changePsw = false;
};

#endif // EDITUSERDIALOG_H
