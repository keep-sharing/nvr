#include "msuser.h"
#include "MsDevice.h"
#include "MyDebug.h"
#include <QMutex>

QMutex mutex(QMutex::Recursive);

MsUser::MsUser(QObject *parent)
    : QObject(parent)
{
    memset(&m_currentUser, 0, sizeof(db_user));
}

MsUser::~MsUser()
{
    qMsDebug();
}

MsUser &MsUser::instance()
{
    static MsUser self;
    return self;
}

bool MsUser::isNeedMenuAuthentication()
{
    char tmp[20] = { 0 };
    get_param_value(SQLITE_FILE_NAME, PARAM_GUI_MENU_AUTH, tmp, sizeof(tmp), "");
    int menuAuthEnable = atoi(tmp);
    return menuAuthEnable;
}

void MsUser::initializeData(QString userName)
{
    QMutexLocker locker(&mutex);
    qMsDebug() << QString("userName: %1").arg(userName);
    read_user_by_name(SQLITE_FILE_NAME, &m_currentUser, (char *)userName.toStdString().c_str());
}

void MsUser::updateData()
{
    QMutexLocker locker(&mutex);
    read_user_by_name(SQLITE_FILE_NAME, &m_currentUser, m_currentUser.username);
}

bool MsUser::isAdmin()
{
    QMutexLocker locker(&mutex);
    return m_currentUser.type == USERLEVEL_ADMIN;
}

QString MsUser::userName()
{
    QMutexLocker locker(&mutex);
    return QString(m_currentUser.username);
}

db_user MsUser::currentUserInfo()
{
    QMutexLocker locker(&mutex);
    return m_currentUser;
}

void MsUser::setCurrentUserInfo(const db_user &user)
{
    QMutexLocker locker(&mutex);
    qMsDebug() << QString("userName: %1").arg(user.username);
    memcpy(&m_currentUser, &user, sizeof(struct db_user));
}

bool MsUser::isGustureAvailable()
{
    QMutexLocker locker(&mutex);
    return QString(m_currentUser.pattern_psw).size() > 3;
}

bool MsUser::checkBasicPermission(int mode, int perssion)
{
    QMutexLocker locker(&mutex);
    if (perssion < 0) {
        return true;
    }
    if (isAdmin()) {
        return true;
    }

    bool result = false;
    switch (mode) {
    case PERM_MODE_NONE:
        result = false;
        break;
    case PERM_MODE_LIVE:
        if (m_currentUser.perm_local_live == -1) {
            result = true;
        } else if (m_currentUser.perm_local_live == 0) {
            result = false;
        } else {
            result = m_currentUser.perm_local_live & perssion;
        }
        break;
    case PERM_MODE_PLAYBACK:
        if (m_currentUser.perm_local_playback == -1) {
            result = true;
        } else if (m_currentUser.perm_local_playback == 0) {
            result = false;
        } else {
            result = m_currentUser.perm_local_playback & perssion;
        }
        break;
    case PERM_MODE_RETRIEVE:
        if (m_currentUser.perm_local_retrieve < 0) {
            result = true;
        } else {
            result = false;
        }
        break;
    case PERM_MODE_SMART:
        if (m_currentUser.perm_local_smart < 0) {
            result = true;
        } else {
            result = false;
        }
        break;
    case PERM_MODE_CAMERA:
        if (m_currentUser.perm_local_camera < 0) {
            result = true;
        } else {
            result = false;
        }
        break;
    case PERM_MODE_STORAGE:
        if (m_currentUser.perm_local_storage < 0) {
            result = true;
        } else {
            result = false;
        }
        break;
    case PERM_MODE_EVENT:
        if (m_currentUser.perm_local_event < 0) {
            result = true;
        } else {
            result = false;
        }
        break;
    case PERM_MODE_SETTINGS:
        if (m_currentUser.perm_local_settings < 0) {
            result = true;
        } else {
            result = false;
        }
        break;
    case PERM_MODE_STATUS:
        if (m_currentUser.perm_local_status < 0) {
            result = true;
        } else {
            result = false;
        }
        break;
    case PERM_MODE_LOGOUT:
        if (m_currentUser.perm_local_shutdown < 0) {
            result = true;
        } else {
            result = false;
        }
        break;
    }
    return result;
}

bool MsUser::hasLiveViewChannelPermission(int channel)
{
    QMutexLocker locker(&mutex);
    if (channel < 0 || channel >= qMsNvr->maxChannel()) {
        return true;
    }
    if (isAdmin()) {
        return true;
    }
    if (m_currentUser.local_live_view_ex[channel] == '1') {
        return true;
    }

    return false;
}

bool MsUser::checkPlaybackChannelPermission(int channel)
{
    QMutexLocker locker(&mutex);
    if (channel < 0) {
        return true;
    }
    if (isAdmin()) {
        return true;
    }

    if (m_currentUser.local_playback_ex[channel] == '1') {
        return true;
    } else {
        return false;
    }
}

QList<int> MsUser::accessiblePlaybackChannelList()
{
    QMutexLocker locker(&mutex);
    QList<int> list;
    int count = qMsNvr->maxChannel();
    for (int i = 0; i < count; ++i) {
        if (isAdmin()) {
            list.append(i);
        } else if (m_currentUser.local_playback_ex[i] == '1') {
            list.append(i);
        }
    }
    return list;
}

bool MsUser::isUserChanged(const db_user &user1, const db_user &user2)
{
    if (user1.enable != user2.enable) {
        return true;
    }
    if (QString(user1.username) != QString(user2.username)) {
        return true;
    }
    if (QString(user1.password) != QString(user2.password)) {
        return true;
    }
    if (user1.type != user2.type) {
        return true;
    }
    if (QString(user1.local_live_view_ex) != QString(user2.local_live_view_ex)) {
        return true;
    }
    if (QString(user1.local_playback_ex) != QString(user2.local_playback_ex)) {
        return true;
    }
    if (QString(user1.remote_live_view_ex) != QString(user2.remote_live_view_ex)) {
        return true;
    }
    if (QString(user1.remote_playback_ex) != QString(user2.remote_playback_ex)) {
        return true;
    }
    if (QString(user1.password_ex) != QString(user2.password_ex)) {
        return true;
    }
    if (QString(user1.pattern_psw) != QString(user2.pattern_psw)) {
        return true;
    }
    if (QString(user1.perm_local_live) != QString(user2.perm_local_live)) {
        return true;
    }
    if (QString(user1.perm_local_playback) != QString(user2.perm_local_playback)) {
        return true;
    }
    if (QString(user1.perm_local_retrieve) != QString(user2.perm_local_retrieve)) {
        return true;
    }
    if (QString(user1.perm_local_smart) != QString(user2.perm_local_smart)) {
        return true;
    }
    if (QString(user1.perm_local_event) != QString(user2.perm_local_event)) {
        return true;
    }
    if (QString(user1.perm_local_camera) != QString(user2.perm_local_camera)) {
        return true;
    }
    if (QString(user1.perm_local_storage) != QString(user2.perm_local_storage)) {
        return true;
    }
    if (QString(user1.perm_local_settings) != QString(user2.perm_local_settings)) {
        return true;
    }
    if (QString(user1.perm_local_status) != QString(user2.perm_local_status)) {
        return true;
    }
    if (QString(user1.perm_local_shutdown) != QString(user2.perm_local_shutdown)) {
        return true;
    }
    if (QString(user1.perm_remote_live) != QString(user2.perm_remote_live)) {
        return true;
    }
    if (QString(user1.perm_remote_playback) != QString(user2.perm_remote_playback)) {
        return true;
    }
    if (QString(user1.perm_remote_retrieve) != QString(user2.perm_remote_retrieve)) {
        return true;
    }
    if (QString(user1.perm_remote_smart) != QString(user2.perm_remote_smart)) {
        return true;
    }
    if (QString(user1.perm_remote_event) != QString(user2.perm_remote_event)) {
        return true;
    }
    if (QString(user1.perm_remote_camera) != QString(user2.perm_remote_camera)) {
        return true;
    }
    if (QString(user1.perm_remote_storage) != QString(user2.perm_remote_storage)) {
        return true;
    }
    if (QString(user1.perm_remote_settings) != QString(user2.perm_remote_settings)) {
        return true;
    }
    if (QString(user1.perm_remote_status) != QString(user2.perm_remote_status)) {
        return true;
    }
    if (QString(user1.perm_remote_shutdown) != QString(user2.perm_remote_shutdown)) {
        return true;
    }

    return false;
}
