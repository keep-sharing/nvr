#ifndef MSUSER_H
#define MSUSER_H

#include <QObject>
#include "msdb/msdb.h"

#define gMsUser MsUser::instance()

class MsUser : public QObject
{
    Q_OBJECT
public:
    explicit MsUser(QObject *parent = nullptr);
    ~MsUser();

    static MsUser &instance();
    static bool isNeedMenuAuthentication();

    void initializeData(QString userName);
    void updateData();

    bool isAdmin();
    QString userName();

    db_user currentUserInfo();
    void setCurrentUserInfo(const db_user &user);

    bool isGustureAvailable();

    bool checkBasicPermission(int mode, int perssion);
    bool hasLiveViewChannelPermission(int channel);
    bool checkPlaybackChannelPermission(int channel);
    QList<int> accessiblePlaybackChannelList();

    bool isUserChanged(const db_user &user1, const db_user &user2);

signals:

public slots:

private:
    db_user m_currentUser;
};

#endif // MSUSER_H
