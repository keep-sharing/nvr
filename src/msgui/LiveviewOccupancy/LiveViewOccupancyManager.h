#ifndef LIVEVIEWOCCUPANCYMANAGER_H
#define LIVEVIEWOCCUPANCYMANAGER_H

#include <QMap>
#include "MsObject.h"


class QTimer;
class MessageReceive;
class LiveViewOccupancy;

class LiveViewOccupancyManager : public MsObject {
    Q_OBJECT

public:
    enum Reason {
        NormalReason,
        LogoutReason,
    };

    explicit LiveViewOccupancyManager(QObject *parent = 0);
    ~LiveViewOccupancyManager();

    static LiveViewOccupancyManager *instance();

    void readyToQuit();

    void setShowLater();
    bool isShowLater();
    bool isOccupancyMode();

    void showOccupancy(Reason reason);
    void showOccupancy(int group, Reason reason);
    void closeOccupancy(Reason reason);
    void resetOccupancy(Reason reason);

    bool isCurrentScreenOccupancy();

    void setScreen(int value);

    void updateData();
    void setGroup(int group);
    int group() const;

    void sendLiveViewReset(int group);
    void sendDatabaseReset(int group);

    void processMessage(MessageReceive *message);

private:
    void ON_RESPONSE_FLAG_SET_PEOPLECNT_LIVEVIEW_RESET(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_PEOPLECNT_DB_RESET(MessageReceive *message);

    void initializeMainOccupancy();
    void initializeSubOccupancy();

signals:

public slots:
    void onUpdateData();

private slots:
    void onTimerAutoReset();

private:
    static LiveViewOccupancyManager *self;

    int m_screen = 0;
    int m_group = 0;
    bool m_isShowLater = false;
    bool m_isVisible = false;
    bool m_isMainVisible = false;
    bool m_isSubVisible = false;
    LiveViewOccupancy *m_mainOccupancy = nullptr;
    LiveViewOccupancy *m_subOccupancy = nullptr;

    QMap<int, bool> m_isReadyReset;
    QTimer *m_timerAutoReset = nullptr;
};

#endif // LIVEVIEWOCCUPANCYMANAGER_H
