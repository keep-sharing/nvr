#ifndef ADDUSERDIALOG_H
#define ADDUSERDIALOG_H

#include "BaseShadowDialog.h"

extern "C" {
#include "msdb.h"
}

namespace Ui {
class AddUser;
}

class AddUserDialog : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit AddUserDialog(QWidget *parent = nullptr);
    ~AddUserDialog();

private slots:
    void onLanguageChanged();

    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

    void on_comboBox_patternEnable_activated(int index);
    void on_pushButton_patternEdit_clicked();

    void on_pushButtonPermissionsEdit_clicked();

private:
    Ui::AddUser *ui;
    QString m_pattern_text;
    db_user m_userOperator;
    db_user m_userViewer;
};

#endif // ADDUSERDIALOG_H
