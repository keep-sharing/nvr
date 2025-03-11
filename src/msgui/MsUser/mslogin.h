#ifndef MSLOGIN_H
#define MSLOGIN_H

#include "BaseWidget.h"

extern "C" {
#include "msdb.h"
}

namespace Ui {
class MsLogin;
}

class MsLogin : public BaseWidget {
    Q_OBJECT

    struct login_left {
        int attempt;
        time_t lockTime;
    };

public:
    explicit MsLogin(QWidget *parent = nullptr);
    ~MsLogin();

    static MsLogin *instance();

    void showLogin();

    void menuLogin(const QString &name, int &second);
    void menuLoginSuccess(const QString &name);
    void menuLoginFailed(const QString &name, int &attempt);

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *) override;
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;

    void returnPressed() override;

signals:
    void loginFinished();

private slots:
    void onLanguageChanged();

    void on_pushButton_login_clicked();
    void on_comboBox_user_activated(int index);
    void on_lineEdit_password_textChanged(const QString &text);

    void onDrawStart();
    void onDrawFinished(QString text);
    void on_toolButton_forgot_clicked();
    void on_toolButton_loginWay_clicked();

private:
    void adjustGeometry();
    void getUsers(int fill = 0);
    void login_success(const db_user &user);
    void login_failed(QString userName);
    void setPatternShow(bool visible);

private:
    static MsLogin *s_msLogin;

    Ui::MsLogin *ui;

    QMap<QString, db_user> m_mapUser;
    QMap<QString, struct login_left> m_mapFailed;
};

#endif // MSLOGIN_H
