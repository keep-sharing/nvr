#include "LiveViewOccupancyManager.h"
#include "MyDebug.h"
#include "BottomBar.h"
#include "centralmessage.h"
#include "ContentLiveView.h"
#include "LiveView.h"
#include "LiveviewBottomBar.h"
#include "LiveViewOccupancy.h"
#include "LiveViewSub.h"
#include "MsDevice.h"
#include "PeopleCountingData.h"
#include "SubControl.h"

LiveViewOccupancyManager *LiveViewOccupancyManager::self = nullptr;

LiveViewOccupancyManager::LiveViewOccupancyManager(QObject *parent)
    : MsObject(parent)
{
    const display &db_display = qMsNvr->displayInfo();
    setScreen(db_display.occupancy_screen);

    m_timerAutoReset = new QTimer(this);
    connect(m_timerAutoReset, SIGNAL(timeout()), this, SLOT(onTimerAutoReset()));
    m_timerAutoReset->start(1000);

    //TODO: LiuHuanyu 2021-07-28, 回调改为统一一个接口
}

LiveViewOccupancyManager::~LiveViewOccupancyManager()
{
    self = nullptr;
}

LiveViewOccupancyManager *LiveViewOccupancyManager::instance()
{
    if (!self) {
        QMS_ASSERT(ContentLiveView::instance());
        self = new LiveViewOccupancyManager(ContentLiveView::instance());
    }
    return self;
}

void LiveViewOccupancyManager::readyToQuit()
{
    m_timerAutoReset->stop();
}

void LiveViewOccupancyManager::setShowLater()
{
    m_isShowLater = true;
}

bool LiveViewOccupancyManager::isShowLater()
{
    return m_isShowLater;
}

bool LiveViewOccupancyManager::isOccupancyMode()
{
    return m_isVisible;
}

void LiveViewOccupancyManager::showOccupancy(Reason reason)
{
    m_isVisible = true;

    //MSHN-9884	QT-Live View-Occupancy：需显示已添加的Group No.最小的组，没有组的时候为空
    if (m_group < 0) {
        m_group = 0;
    }
    people_cnt_info *people_info = gPeopleCountingData.peopleInfo();
    const PEOPLECNT_SETTING &setting = people_info->sets[m_group];
    if (!PeopleCountingData::hasChannel(setting.tri_channels)) {
        for (int i = 0; i < MAX_PEOPLECNT_GROUP; ++i) {
            if (i == m_group) {
                continue;
            }
            const PEOPLECNT_SETTING &tempSetting = people_info->sets[i];
            if (PeopleCountingData::hasChannel(tempSetting.tri_channels)) {
                m_group = i;
                break;
            }
        }
        m_group = -1;
    }

    //
    switch (m_screen) {
    case 0:
        initializeMainOccupancy();
        if (m_mainOccupancy) {
            m_mainOccupancy->setGeometry(SubControl::instance()->physicalMainScreenGeometry());
            m_mainOccupancy->show();
            m_mainOccupancy->raise();
            m_isMainVisible = true;
        }
        if (m_subOccupancy) {
            m_subOccupancy->hide();
            m_isSubVisible = false;
        }
        break;
    case 1:
        initializeSubOccupancy();
        if (m_subOccupancy) {
            m_subOccupancy->setGeometry(SubControl::instance()->physicalSubScreenGeometry());
            m_subOccupancy->show();
            m_subOccupancy->raise();
            m_isSubVisible = true;
        }
        if (m_mainOccupancy) {
            m_mainOccupancy->hide();
            m_isMainVisible = false;
        }
        break;
    case 2:
        initializeMainOccupancy();
        if (m_mainOccupancy) {
            m_mainOccupancy->setGeometry(SubControl::instance()->physicalMainScreenGeometry());
            m_mainOccupancy->show();
            m_mainOccupancy->raise();
            m_isMainVisible = true;
        }

        initializeSubOccupancy();
        if (m_subOccupancy) {
            m_subOccupancy->setGeometry(SubControl::instance()->physicalSubScreenGeometry());
            m_subOccupancy->show();
            m_subOccupancy->raise();
            m_isSubVisible = true;
        }
        break;
    }

    if (isCurrentScreenOccupancy()) {
        BottomBar::instance()->animateHide();
    }
    LiveView::instance()->updateOccupancyMenu();
    LiveviewBottomBar::instance()->updateOccupancyState();

    switch (reason) {
    case NormalReason:
        qMsNvr->setOccupancyMode(true);
        break;
    default:
        break;
    }

    //
    if (LiveView::instance()) {
        LiveView::instance()->resetTimeInfoMode();
    }
    if (LiveViewSub::instance()) {
        LiveViewSub::instance()->resetTimeInfoMode();
    }
}

void LiveViewOccupancyManager::showOccupancy(int group, Reason reason)
{
    showOccupancy(reason);
    setGroup(group);
}

void LiveViewOccupancyManager::closeOccupancy(Reason reason)
{
    m_isVisible = false;
    m_isMainVisible = false;
    m_isSubVisible = false;

    if (m_mainOccupancy) {
        m_mainOccupancy->close();
    }
    if (m_subOccupancy) {
        m_subOccupancy->close();
    }

    LiveView::instance()->updateOccupancyMenu();
    LiveviewBottomBar::instance()->updateOccupancyState();

    switch (reason) {
    case NormalReason:
        qMsNvr->setOccupancyMode(false);
        break;
    default:
        break;
    }
}

void LiveViewOccupancyManager::resetOccupancy(Reason reason)
{
    if (m_isVisible || m_isShowLater) {
        m_isShowLater = false;
        showOccupancy(reason);
    }
}

bool LiveViewOccupancyManager::isCurrentScreenOccupancy()
{
    if (SubControl::instance()->isSubControl()) {
        return m_isSubVisible;
    } else {
        return m_isMainVisible;
    }
}

void LiveViewOccupancyManager::setScreen(int value)
{
    m_screen = value;
    switch (m_screen) {
    case 0:
        break;
    case 1:
        break;
    case 2:
        break;
    }
}

void LiveViewOccupancyManager::updateData()
{
    QMetaObject::invokeMethod(this, "onUpdateData", Qt::QueuedConnection);
}

void LiveViewOccupancyManager::setGroup(int group)
{
    m_group = group;
    if (m_mainOccupancy) {
        m_mainOccupancy->setGroup(group);
    }
    if (m_subOccupancy) {
        m_subOccupancy->setGroup(group);
    }
}

int LiveViewOccupancyManager::group() const
{
    return m_group;
}

void LiveViewOccupancyManager::sendLiveViewReset(int group)
{
    sendMessage(REQUEST_FLAG_SET_PEOPLECNT_LIVEVIEW_RESET, &group, sizeof(group));
}

void LiveViewOccupancyManager::sendDatabaseReset(int group)
{
    sendMessage(REQUEST_FLAG_SET_PEOPLECNT_DB_RESET, &group, sizeof(group));
}

void LiveViewOccupancyManager::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SET_PEOPLECNT_LIVEVIEW_RESET:
        ON_RESPONSE_FLAG_SET_PEOPLECNT_LIVEVIEW_RESET(message);
        break;
    case RESPONSE_FLAG_SET_PEOPLECNT_DB_RESET:
        ON_RESPONSE_FLAG_SET_PEOPLECNT_DB_RESET(message);
        break;
    }
}

void LiveViewOccupancyManager::ON_RESPONSE_FLAG_SET_PEOPLECNT_LIVEVIEW_RESET(MessageReceive *message)
{
    Q_UNUSED(message)

    //
    updateData();
}

void LiveViewOccupancyManager::ON_RESPONSE_FLAG_SET_PEOPLECNT_DB_RESET(MessageReceive *message)
{
    Q_UNUSED(message)

    //
    updateData();
}

void LiveViewOccupancyManager::initializeMainOccupancy()
{
    if (!m_mainOccupancy) {
        QWidget *p = static_cast<QWidget *>(parent());
        m_mainOccupancy = new LiveViewOccupancy(p);
    }
    m_mainOccupancy->setGroup(m_group);
}

void LiveViewOccupancyManager::initializeSubOccupancy()
{
    if (!m_subOccupancy) {
        QWidget *p = static_cast<QWidget *>(parent());
        m_subOccupancy = new LiveViewOccupancy(p);
    }
    m_subOccupancy->setGroup(m_group);
}

void LiveViewOccupancyManager::onUpdateData()
{
    //MSHN-9884	QT-Live View-Occupancy：需显示已添加的Group No.最小的组，没有组的时候为空
    if (m_group < 0) {
        m_group = 0;
    }
    people_cnt_info *people_info = gPeopleCountingData.peopleInfo();
    const PEOPLECNT_SETTING &setting = people_info->sets[m_group];
    if (!PeopleCountingData::hasChannel(setting.tri_channels)) {
        for (int i = 0; i < MAX_PEOPLECNT_GROUP; ++i) {
            if (i == m_group) {
                continue;
            }
            const PEOPLECNT_SETTING &tempSetting = people_info->sets[i];
            if (PeopleCountingData::hasChannel(tempSetting.tri_channels)) {
                setGroup(i);
                return;
            }
        }
        setGroup(-1);
    }

    //
    if (m_mainOccupancy) {
        m_mainOccupancy->updateData();
    }
    if (m_subOccupancy) {
        m_subOccupancy->updateData();
    }
}

void LiveViewOccupancyManager::onTimerAutoReset()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    people_cnt_info *people_info = gPeopleCountingData.peopleInfo();
    for (int i = 0; i < MAX_PEOPLECNT_GROUP; ++i) {
        const peoplecnt_setting &setting = people_info->sets[i];
        if (setting.liveview_auto_reset) {
            switch (setting.auto_day) {
            case 0:
                break;
            case 1: //sunday
                if (dateTime.date().dayOfWeek() != Qt::Sunday) {
                    continue;
                }
                break;
            case 2:
                if (dateTime.date().dayOfWeek() != Qt::Monday) {
                    continue;
                }
                break;
            case 3:
                if (dateTime.date().dayOfWeek() != Qt::Tuesday) {
                    continue;
                }
                break;
            case 4:
                if (dateTime.date().dayOfWeek() != Qt::Wednesday) {
                    continue;
                }
                break;
            case 5:
                if (dateTime.date().dayOfWeek() != Qt::Thursday) {
                    continue;
                }
                break;
            case 6:
                if (dateTime.date().dayOfWeek() != Qt::Friday) {
                    continue;
                }
                break;
            case 7:
                if (dateTime.date().dayOfWeek() != Qt::Saturday) {
                    continue;
                }
                break;
            }
            QTime time = QTime::fromString(QString(setting.auto_day_time), "HH:mm:ss");
            int msec = dateTime.time().msecsTo(time);
            if (m_isReadyReset.value(i)) {
                if (msec <= 0) {
                    sendMessage(REQUEST_FLAG_SET_PEOPLECNT_LIVEVIEW_RESET, &i, sizeof(i));
                    m_isReadyReset.insert(i, false);
                }
            } else {
                if (msec > 0 && msec < 5000) {
                    m_isReadyReset.insert(i, true);
                }
            }
        }
    }
}
