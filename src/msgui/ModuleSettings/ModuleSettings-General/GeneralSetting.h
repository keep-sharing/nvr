#ifndef GENERALSETTING_H
#define GENERALSETTING_H

#include "AbstractSettingPage.h"
#include <QEventLoop>

class QTimer;

struct resp_esata_backup_status;

namespace Ui {
class GeneralSetting;
}

class GeneralSetting : public AbstractSettingPage {
    Q_OBJECT

    enum SettingPage {
        PageDateTime,
        PageDevice
    };

public:
    explicit GeneralSetting(QWidget *parent = nullptr);
    ~GeneralSetting();

    void initializeData() override;

    void processMessage(MessageReceive *message) override;
    void filterMessage(MessageReceive *message) override;

    static QList<QPair<QString, QString>> timezoneMap();

private:
    void ON_RESPONSE_FLAG_SET_SYSTIME(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_ESATA_BACKUP_STATUS(MessageReceive *message);

    void initGeneral();

    void initTime();
    void gotoGeneral();
    void freeGeneral();
    void readConfig();
    void saveConfig();

    void saveLanguage();

    void setMainSubResolution(int main_resolution, int sub_resolution, int spot_resolution);
    int ntpIntervalTransToIndex(int);
    int ntpIntervalTransToMinutes(int);
    int getTimeZoneOffsetSec(int);

signals:
    void sigCloseGeneral();
    void sigSendMosaicMsg(int devId, int layoutmode, int iLayout, int iPageNum, int type);
    void sigSetTimeOut(int);
    void sigClosePopUpDialog(int);
    void sigGotoWizard(int);
    void sigReboot();

private slots:
    void onLanguageChanged() override;
    void slotTimeSettingChanged(int index);

    void on_comboBox_hdmi_vga_output_activated(int index);
    void on_comboBox_screen2_enable_activated(int index);

    void onScreenSwitched();

    void onTimeout();

    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

    void on_pushButton_wizard_clicked();
    void onTabClicked(int index);
    void on_pushButton_back_2_clicked();
    void on_pushButton_apply_2_clicked();
    void on_comboBox_ntpSync_activated(int index);
    void on_comboBox_timezone_activated(int index);
    void on_dateTimeEdit_setTime_dateTimeChanged(const QDateTime &dateTime);

    void on_comboBox_screenSwitch_activated(int index);

    void on_comboBox_mouseAccel_activated(int index);

    void on_pushButtonLogoutChannel_clicked();

private:
    Ui::GeneralSetting *ui;

    QEventLoop m_eventLoop;
    QTimer *m_timer = nullptr;

    int m_homologous;
    int _localauth_enable;
    int _menuauth_enable;
    int _menu_timeout;
    int _sys_language;
    int old_language;
    int old_zone_index;
    int time_zone_changed;
    char init_time[32];

    struct display *pDisplayDb = nullptr;
    struct time *pTimeDb = nullptr;
    struct time *pTimeDbOri = nullptr;

    resp_esata_backup_status *m_esate_status = nullptr;
};

#endif // GENERALSETTING_H
