#ifndef HOTSPARE_H
#define HOTSPARE_H

#include "AbstractSettingPage.h"
#include "QEventLoop"
#include "msuser.h"

extern "C" {
#include "msdefs.h"
#include "msg.h"
}

class QEventLoop;
namespace Ui {
class HotSpare;
}
const int HotSpareMaxCameraNumberRole = Qt::UserRole + 110;
class HotSpare : public AbstractSettingPage {
    Q_OBJECT

public:
    struct HotSpareIPCInfo {
        QString ipAddr;
        QString mac;
        QString model;
        int maxCamera;
        HotSpareIPCInfo()
            : ipAddr()
            , mac()
            , model()
            , maxCamera()
        {
        }
        HotSpareIPCInfo(QString ip, QString mac, QString model, int maxCamera)
            : ipAddr(ip)
            , mac(mac)
            , model(model)
            , maxCamera(maxCamera)
        {
        }
    };
    explicit HotSpare(QWidget *parent = nullptr);
    ~HotSpare();
    void hideEvent(QHideEvent *) override;

    static bool isSlaveMode();

    void initHotSpare();
    void gotoHotSparePage();
    int getSelectMaster();
    void writeLog(int action, char *ip);
    bool isSlaveParaValid();
    bool failoversContains(QString ipAddr);
    QString getConnectStatusString(int status);
    QString getWorkStatusString(int status, float percent);
    void saveDisableMode(int beforeModel);
    void saveMasterMode(int beforeModel);
    void saveSlaveMode(int beforeModel);
    void waitSecond();
    void stopStatusTimer();
    void startStatusTimer();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_FAILOVER_GET_SLAVE_STATUS(MessageReceive *message);
    void ON_RESPONSE_FLAG_FAILOVER_GET_MASTER_STATUS(MessageReceive *message);
    void ON_RESPONSE_FLAG_FAILOVER_SEARCH_NVR(MessageReceive *message);
    void ON_RESPONSE_FLAG_FAILOVER_CHANGE_MODE(MessageReceive *message);
    void ON_RESPONSE_FLAG_FAILOVER_UPDATE_MASTER(MessageReceive *message);

signals:

public slots:

private slots:
    void onLanguageChanged() override;
    void onListItemClicked(int row, int column);
    void onStatusItemClicked(int row, int column);

    void on_comboBox_mode_activated(int index);

    void on_pushButton_search_clicked();
    void on_pushButton_add_clicked();

    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

    void onStatusTimeout();

private:
    Ui::HotSpare *ui;

    int m_failoverMode = 0;
    int m_slaveStatus = 0;
    struct ms_failover_master_status m_failoverBak[MAX_FAILOVER];
    char m_bondIp[MAX_LEN_64] = { '\0' };
    char m_lan1Ip[MAX_LEN_64] = { '\0' };
    char m_lan2Ip[MAX_LEN_64] = { '\0' };
    QEventLoop m_eventLoop;
    struct failover_list m_failover;
    struct failover_list m_failoverList[MAX_FAILOVER];

    QMap<QString, HotSpareIPCInfo> m_addIpc;
    QMap<QString, HotSpareIPCInfo> m_deleteIpc;

    QTimer *statusTimer;
};

#endif // HOTSPARE_H
