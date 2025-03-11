#include "AutoLogout.h"
#include "AutoLogoutTip.h"
#include "LiveView.h"
#include "LogoutChannel.h"
#include "MsDevice.h"
#include "MyDebug.h"
#include "PlaybackWindow.h"
#include "mainwindow.h"
#include "settingcontent.h"

AutoLogout::AutoLogout(QObject *parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimerLogout()));
    m_timer->setInterval(1000);

    resume();
}

AutoLogout::~AutoLogout()
{
}

AutoLogout &AutoLogout::instance()
{
    static AutoLogout self;
    return self;
}

int AutoLogout::readInterval()
{
    int value = get_param_int(SQLITE_FILE_NAME, PARAM_GUI_AUTO_LOGOUT, 0);
    return value;
}

void AutoLogout::writeInterval(int sec)
{
    set_param_int(SQLITE_FILE_NAME, PARAM_GUI_AUTO_LOGOUT, sec);
    //
    setInterval(sec);
}

int AutoLogout::interval() const
{
    return m_interval;
}

void AutoLogout::resume()
{
    setInterval(readInterval());
}

void AutoLogout::refreshTimeout()
{
    if (m_timer->isActive()) {
        m_count = 0;
        updateTipValue();
    }
}

void AutoLogout::setInterval(int sec)
{
    if (sec > 0) {
        qMsCDebug("qt_auto_logout") << "start:" << sec << "s";
        m_interval = sec;
        m_timer->start();
        updateTipValue();
    } else {
        qMsCDebug("qt_auto_logout") << "stop:" << sec << "s";
        m_timer->stop();
        clearTipValue();
    }
}

bool AutoLogout::isReadyLogout() const
{
    return m_isReadyLogout;
}

void AutoLogout::logout()
{
    m_isReadyLogout = false;
}

void AutoLogout::updateTipValue()
{
    if (AutoLogoutTip::instance()) {
        AutoLogoutTip::instance()->setValue(m_interval - m_count);
    }
}

void AutoLogout::clearTipValue()
{
    if (AutoLogoutTip::instance()) {
        AutoLogoutTip::instance()->clearValue();
    }
}

void AutoLogout::onTimerLogout()
{
    if (!MainWindow::instance()) {
        return;
    }

    if (LogoutChannel::instance()->isLogout()) {
        m_timer->stop();
        updateTipValue();
        return;
    }
    //Live View播放不会计时退出
    if (LiveView::instance() && LiveView::instance()->isVisible()) {
        if (LiveView::instance()->canAutoLogout()) {
            m_count++;
            if (m_count > m_interval) {
                m_isReadyLogout = true;
                m_count = 0;
                emit logouted();
                MainWindow::s_mainWindow->logout();
            }
        } else {
            m_count = 0;
        }
        updateTipValue();
        return;
    }
    //回放界面播放不会计时退出
    if (PlaybackWindow::instance() && PlaybackWindow::instance()->isVisible()) {
        if (PlaybackWindow::instance()->canAutoLogout()) {
            m_count++;
            if (m_count > m_interval) {
                PlaybackWindow::instance()->closePlayback();
                m_isReadyLogout = true;
                m_count = 0;
                emit logouted();
            }
        } else {
            m_count = 0;
        }
        updateTipValue();
        return;
    }
    //
    if (SettingContent::instance() && SettingContent::instance()->isVisible()) {
        if (SettingContent::instance()->canAutoLogout()) {
            m_count++;
            if (m_count > m_interval) {
                SettingContent::instance()->closeToLiveView();
                m_isReadyLogout = true;
                m_count = 0;
                emit logouted();
            }
        } else {
            m_count = 0;
        }
        updateTipValue();
        return;
    }
}
