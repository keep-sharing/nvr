#ifndef NETWORKCOMMOND_H
#define NETWORKCOMMOND_H

#define Keyboard_Simulation

#include "MsObject.h"
#include "commondthread.h"
#include <QtDebug>

#ifdef Keyboard_Simulation
#include <QTcpServer>
#include <QTcpSocket>
#endif

class MessageReceive;

enum PageMode {
    MOD_WIZARD = 0,
    MOD_LOGIN,
    MOD_LIVEVIEW,
    MOD_LAYOUT,
    MOD_PLAYBACK,
    MOD_RETRIEVE,
    MOD_CAMERA,
    MOD_RECORD,
    MOD_EVENT,
    MOD_SYSTEM,
    MAD_STATUS,
    MOD_SHUTDOWN,
    MOD_MAX
};

enum RockerDirection {
    RockerUp,
    RockerDown,
    RockerLeft,
    RockerRight,
    RockerUpLeft,
    RockerUpRight,
    RockerDownLeft,
    RockerDownRight,
    RockerStop,
    RockerNone
};

enum NetworkResult {
    //没有处理，继续传递
    NetworkReject = 0,
    //已经处理，不再传递
    NetworkAccept = 1,
    //交由系统处理，不再传递
    NetworkTakeOver = 2
};
QDebug operator<<(QDebug debug, const NetworkResult &r);

class NetworkCommond : public MsObject {
    Q_OBJECT

public:
    explicit NetworkCommond(QObject *parent = nullptr);
    ~NetworkCommond();

    static NetworkCommond *instance();

    void setPageMode(PageMode mode);
    void sendKeyEvent(int key, KeyEventType type);

    void ON_RESPONSE_FLAG_SENDTO_QT_CMD(MessageReceive *message);

private:
    bool dealCommond(const QString &strCommond);

    bool cmd_KeyEvent(const QString &strCommond);
    bool cmd_Enter(const QString &strCommond);
    bool cmd_Page(const QString &strCommond);
    bool cmd_SelChannel(const QString &strCommond);
    bool cmd_PlayBack(const QString &strCommond);
    bool cmd_Mouse(const QString &strCommond);
    bool cmd_TabPage(const QString &strCommond);
    bool cmd_TabPage_Prev(const QString &strCommond);
    bool cmd_ChangeFocus(const QString &strCommond);
    bool cmd_ChangeFocus_Prev(const QString &strCommond);
    bool cmd_Ptz(const QString &strCommond);
    bool cmd_Dir(const QString &strCommond);
    bool cmd_Esc(const QString &strCommond);
    bool cmd_R_Click(const QString &strCommond);
    //通用解析
    bool cmd_GeneralResolver(const QString &strCommond);

    //
    QString currentModeName();

signals:

private slots:
#ifdef Keyboard_Simulation
    void onTcpNewConnection();
    void onTcpReadyRead();
#endif

private:
    static NetworkCommond *s_networkCommond;

    CommondThread *m_commondThread;

    PageMode m_mode;

    //
#ifdef Keyboard_Simulation
    QTcpServer *m_tcpServer = nullptr;
#endif
};

#endif // NETWORKCOMMOND_H
