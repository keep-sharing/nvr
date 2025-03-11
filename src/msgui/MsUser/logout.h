#ifndef LOGOUT_H
#define LOGOUT_H

#include "BaseDialog.h"

namespace Ui {
class Logout;
}

class Logout : public BaseDialog {
    Q_OBJECT

public:
    enum LogoutType {
        TypeLogout = 10,
        TypeReboot = 11,
        TypeShutdown = 12
    };

    explicit Logout(QWidget *parent = 0);
    ~Logout();

protected:
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;

    bool isMoveToCenter() override;
    bool isAddToVisibleList() override;

    void escapePressed() override;

private slots:
    void on_toolButton_logout_clicked();
    void on_toolButton_reboot_clicked();
    void on_toolButton_shutdown_clicked();

private:
    Ui::Logout *ui;
};

#endif // LOGOUT_H
