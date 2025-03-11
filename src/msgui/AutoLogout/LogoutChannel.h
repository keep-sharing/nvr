#ifndef LOGOUTCHANNEL_H
#define LOGOUTCHANNEL_H

#include "BaseShadowDialog.h"

namespace Ui {
class LogoutChannel;
}

class LogoutChannel : public BaseShadowDialog
{
    Q_OBJECT

public:

    enum Mode
    {
        RegularMode,
        TargetMode,
        OccupancyMode
    };

    explicit LogoutChannel(QWidget *parent = 0);
    ~LogoutChannel();

    static LogoutChannel *instance();

    void initializeData();

    int logoutMode();
    int logoutGroup();

    void logout();
    void clearLogout();
    bool isLogout();
    bool isLogoutChannel(int channel);

    void setTempLogin(bool login);
    bool isTempLogin();

    void showLogoutChannel();

private:
    void readLogoutMode();
    void writeLogoutMode();

    void readLogoutChannel();
    void writeLogoutChannel();

    void readLogoutGroup();
    void writeLogoutGroup();

private slots:
    void onLanguageChanged();

    void on_comboBoxDisplayMode_indexSet(int index);
    void on_pushButtonOk_clicked();
    void on_pushButtonCancel_clicked();

private:
    static LogoutChannel *self;

    Ui::LogoutChannel *ui;

    int m_mode = RegularMode;
    QString m_logoutChannel;
    int m_logoutGroup = -1;

    bool m_isLogout = false;
    bool m_isTempLogin = false;
};

#endif // LOGOUTCHANNEL_H
