#ifndef USERLOGIN_H
#define USERLOGIN_H

#include "BaseShadowDialog.h"
#include "msdb.h"

namespace Ui {
class UserLogin;
}

class UserLogin : public BaseShadowDialog {
    Q_OBJECT

    enum LOGIN_MODE {
        LOGIN_NORMAL,
        LOGIN_GUSTURE
    };

public:
    explicit UserLogin(QWidget *parent = nullptr);
    ~UserLogin();

    int execLogin();

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    void setGustureLockVisible(bool visible);
    void login(LOGIN_MODE mode, const QString &name, const QString &password);

private slots:
    void onLanguageChanged();

    void on_comboBox_user_activated(int index);
    void on_lineEdit_password_textChanged(const QString &text);
    void on_pushButton_login_clicked();
    void on_pushButton_cancel_clicked();
    void on_toolButtonForgetPassword_clicked();
    void on_toolButtonLoginWay_clicked();

    void onDrawStart();
    void onDrawFinished(QString text);

private:
    Ui::UserLogin *ui;

    QMap<QString, db_user> m_mapUser;
};

#endif // USERLOGIN_H
