#include "EventPopup.h"
#include "ui_EventPopup.h"
#include "CustomLayoutData.h"
#include "EventPopupSub.h"
#include "LivePage.h"
#include "LiveView.h"
#include "LiveViewOccupancyManager.h"
#include "LiveViewTargetPlay.h"
#include "LogoutChannel.h"
#include "MsDevice.h"
#include "SubControl.h"
#include "mslogin.h"
#include <QDesktopWidget>
#include <QPainter>
#include <QtDebug>

EventPopup *EventPopup::s_eventPopup = nullptr;

EventPopup::EventPopup(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EventPopup)
{
    ui->setupUi(this);
    s_eventPopup = this;

    gEventPopupData;

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_timer->setSingleShot(true);

    const display &db_display = qMsNvr->displayInfo();
    setPopupInfo(db_display.eventPop_screen, db_display.eventPop_time);
}

EventPopup::~EventPopup()
{
    s_eventPopup = nullptr;
    delete ui;
    qDebug() << "EventPopup::~EventPopup()";
}

EventPopup *EventPopup::instance()
{
    return s_eventPopup;
}

void EventPopup::setPopupInfo(int screen, int seconds)
{
    m_popupScreen = screen;
    m_popupInterval = seconds;
    m_timer->setInterval(m_popupInterval * 1000);
}

void EventPopup::setPopupChannel(quint64 state, int layout, const QList<int> &channels)
{
    if (state <= 0) {
        return;
    }
    if (LiveViewOccupancyManager::instance()->isOccupancyMode()) {
        return;
    }
    if (MsLogin::instance() && MsLogin::instance()->isVisible()) {
        return;
    }

    //没有开辅屏
    if (!SubControl::instance()->isSubEnable()) {
        m_popupScreen = SubControl::instance()->currentScreen();
    }

    //确认要弹出的通道
    QList<int> mainRealChannels;
    QList<int> subRealChannels;
    for (int i = 0; i < channels.size(); ++i) {
        int channel = channels.at(i);
        //logout后锁定的通道不弹窗
        if (LogoutChannel::instance() && LogoutChannel::instance()->isLogout() && !LogoutChannel::instance()->isLogoutChannel(channel)) {
            mainRealChannels.append(-1);
            subRealChannels.append(-1);
            continue;
        }
        //MSHN-6706 QT:报警弹窗，在Layout里面的单视图中屏蔽通道3（视图设置同步或者不同步的情况一样），通道3不触发报警弹窗
        if (m_popupScreen == 2) {
            if (CustomLayoutData::instance()->isChannelEnable(LiveView::instance()->currentLayoutMode(SCREEN_MAIN), channel)) {
                mainRealChannels.append(channel);
            } else {
                mainRealChannels.append(-1);
            }
            if (CustomLayoutData::instance()->isChannelEnable(LiveView::instance()->currentLayoutMode(SCREEN_SUB), channel)) {
                subRealChannels.append(channel);
            } else {
                subRealChannels.append(-1);
            }
        } else if (m_popupScreen == SCREEN_MAIN) {
            if (CustomLayoutData::instance()->isChannelEnable(LiveView::instance()->currentLayoutMode(m_popupScreen), channel)) {
                mainRealChannels.append(channel);
            } else {
                mainRealChannels.append(-1);
            }
        } else if (m_popupScreen == SCREEN_SUB) {
            if (CustomLayoutData::instance()->isChannelEnable(LiveView::instance()->currentLayoutMode(m_popupScreen), channel)) {
                subRealChannels.append(channel);
            } else {
                subRealChannels.append(-1);
            }
        }
    }
    int mainChannelCount = 0;
    int subChannelCount = 0;
    for (int channel : mainRealChannels) {
        if (channel >= 0) {
            mainChannelCount++;
        }
    }
    for (int channel : subRealChannels) {
        if (channel >= 0) {
            subChannelCount++;
        }
    }
    if (mainChannelCount == 0 && subChannelCount == 0) {
        return;
    }

    //报警弹窗时要关闭的
    if (LiveViewTargetPlay::instance() && LiveViewTargetPlay::instance()->isVisible()) {
        LiveViewTargetPlay::instance()->closeAnprPlayback();
    }

    //
    if (m_popupScreen == 2) {
        /**双屏显示报警**/
        //
        if (LiveView::instance()->canShowPopup()) {
            LiveView::instance()->preparePopup(SubControl::instance()->mainLiveViewScreen(),
                                            layout,
                                            SubControl::instance()->isSubControl() ? subRealChannels : mainRealChannels);
            setGeometry(LiveView::instance()->liveviewGeometry());
            show();
            raise();
        }
        //
        LiveView::instance()->preparePopup(SubControl::instance()->subLiveViewScreen(),
                                        layout,
                                        SubControl::instance()->isSubControl() ? mainRealChannels : subRealChannels);
        EventPopupSub::instance()->showSubPopup();
    } else if (m_popupScreen == SubControl::instance()->currentScreen()) {
        /**逻辑主屏显示报警**/
        //
        if (!LiveView::instance()->canShowPopup()) {
            hide();
            return;
        }
        //
        LiveView::instance()->preparePopup(m_popupScreen,
                                        layout,
                                        m_popupScreen == SCREEN_SUB ? subRealChannels : mainRealChannels);
        setGeometry(LiveView::instance()->liveviewGeometry());
        show();
        raise();
    } else {
        /**逻辑辅屏显示报警**/
        //
        LiveView::instance()->preparePopup(m_popupScreen,
                                        layout,
                                        m_popupScreen == SCREEN_SUB ? subRealChannels : mainRealChannels);
        EventPopupSub::instance()->showSubPopup();
    }

    //
    if (m_popupInterval > 0) {
        m_timer->start(m_popupInterval * 1000);
    }
}

int EventPopup::popupScreen()
{
    return m_popupScreen;
}

void EventPopup::closePopup()
{
    switch (m_popupScreen) {
    case SCREEN_MAIN:
        LiveView::instance()->recoverLayoutBeforePopup(SCREEN_MAIN);
        break;
    case SCREEN_SUB:
        LiveView::instance()->recoverLayoutBeforePopup(SCREEN_SUB);
        break;
    case 2: //All
        LiveView::instance()->recoverLayoutBeforePopup(SCREEN_MAIN);
        LiveView::instance()->recoverLayoutBeforePopup(SCREEN_SUB);
        break;
    default:
        break;
    }
    EventPopupSub::instance()->closeSubPopup();
    close();
}

void EventPopup::hideEvent(QHideEvent *)
{
    //MSHN-6294 QT-Event：Live view设置page info为on；弹窗报警后，page info消失了。但page info仍为“on”
    LivePage::mainPage()->initializeData();
}

void EventPopup::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(Qt::red, 8));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(rect());
}

void EventPopup::mousePressEvent(QMouseEvent *)
{
    closePopup();
}

void EventPopup::onTimeout()
{
    closePopup();
}
