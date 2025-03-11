#include "MsApplication.h"
#include "AutoLogout.h"
#include "LiveViewOccupancyManager.h"
#include "LogoutChannel.h"
#include "MyDebug.h"
#include "Script.h"
#include "SubControl.h"
#include "autotest.h"
#include "centralmessage.h"
#include "mainwindow.h"
#include "settingcontent.h"
#include "tde.h"
#include <QMouseEvent>
#include <QScopedValueRollback>
#include <QScreenCursor>
#include <QWSEvent>
#include <QWSServer>

MsApplication::MsApplication(int &argc, char **argv)
    : QApplication(argc, argv)
{
    m_timerHideCursor = new QTimer(this);
    connect(m_timerHideCursor, SIGNAL(timeout()), this, SLOT(onTimerHideCursor()));
    m_timerHideCursor->setSingleShot(true);
    m_timerHideCursor->start(1000 * 60);
}

MsApplication::~MsApplication()
{
    qMsDebug();
    m_timerHideCursor->stop();
}

void MsApplication::setMainWindow(MainWindow *window)
{
    m_mainWindow = window;
}

void MsApplication::setInitializeFinished(bool finished)
{
    m_initializeFinished = finished;
}

bool MsApplication::isInitializeFinished() const
{
    return m_initializeFinished;
}

void MsApplication::setAboutToReboot(bool value)
{
    m_isAboutToReboot = value;
}

bool MsApplication::notify(QObject *receiver, QEvent *e)
{
    if (Script::instance()) {
        Script::instance()->appendCommond(receiver, e);
    }

    //
    switch (e->type()) {
    case QEvent::MouseMove: {
        if (m_isAboutToReboot) {
            return true;
        }
        //
        m_timerHideCursor->start();
        if (!QWSServer::instance()->isCursorVisible()) {
            QWSServer::instance()->setCursorVisible(true);
        }
        if (LogoutChannel::instance()) {
            if (LogoutChannel::instance()->isLogout()) {
                if (LogoutChannel::instance()->isTempLogin()) {
                    return QApplication::notify(receiver, e);
                } else {
                    return true;
                }
            }
        }
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
        if (m_mainWindow) {
            m_mainWindow->dealMouseMove(mouseEvent->globalPos());
        }
        if (SettingContent::instance()) {
            SettingContent::instance()->dealMouseMove(mouseEvent->globalPos());
        }
        //
        mouseActive();
        break;
    }
    case QEvent::MouseButtonPress: {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
        yCDebug("qt_mouse") << receiver << e << "pos:" << mouseEvent->pos() << "global pos:" << mouseEvent->globalPos();

        if (m_isAboutToReboot) {
            return true;
        }
        //qWarning() << receiver << mouseEvent;
        if (mouseEvent->buttons() & Qt::LeftButton && mouseEvent->buttons() & Qt::RightButton) {
            return true;
        }
        if (mouseEvent->button() == Qt::MidButton) {
            if (AutoTest::instance()) {
                AutoTest::instance()->stopAllTest();
            }
        }
        //
        if (LogoutChannel::instance() && LogoutChannel::instance()->isLogout()) {
            if (LogoutChannel::instance()->isTempLogin()) {

            } else {
                if (mouseEvent->button() == Qt::LeftButton || mouseEvent->button() == Qt::RightButton) {
                    LogoutChannel::instance()->setTempLogin(true);
                    if (m_mainWindow) {
                        m_mainWindow->showLogin();
                    }
                }
                return true;
            }
        }
        //
        mouseActive();
        //
        m_timerHideCursor->start();
        if (!QWSServer::instance()->isCursorVisible()) {
            QWSServer::instance()->setCursorVisible(true);
        }
        break;
    }
    case QEvent::MouseButtonRelease: {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
        yCDebug("qt_mouse") << receiver << e << "pos:" << mouseEvent->pos() << "global pos:" << mouseEvent->globalPos();

        if (m_isAboutToReboot) {
            return true;
        }
        //
        if (mouseEvent->buttons() & Qt::LeftButton && mouseEvent->buttons() & Qt::RightButton) {
            return true;
        }
        //
        if (LogoutChannel::instance()) {
            if (LogoutChannel::instance()->isLogout() && !LogoutChannel::instance()->isTempLogin()) {
                return true;
            }
        }
        //
        mouseActive();
        break;
    }
    case QEvent::MouseButtonDblClick: {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
        yCDebug("qt_mouse") << receiver << e << "pos:" << mouseEvent->pos() << "global pos:" << mouseEvent->globalPos();

        if (m_isAboutToReboot) {
            return true;
        }
        //
        if (mouseEvent->buttons() & Qt::LeftButton && mouseEvent->buttons() & Qt::RightButton) {
            return true;
        }
        //
        if (LogoutChannel::instance()) {
            if (LogoutChannel::instance()->isLogout() && !LogoutChannel::instance()->isTempLogin()) {
                return true;
            }
        }
        //
        if (mouseEvent->button() == Qt::MidButton) {
            qDebug() << "MsApplication::notify, receiver:" << receiver << ", event:" << e;
            emit middleButtonDoubleClicked();
        }
        //
        mouseActive();
        break;
    }
    case QEvent::Wheel: {
        if (m_isAboutToReboot) {
            return true;
        }
        //
        m_timerHideCursor->start();
        if (!QWSServer::instance()->isCursorVisible()) {
            QWSServer::instance()->setCursorVisible(true);
        }
        if (LogoutChannel::instance()) {
            if (LogoutChannel::instance()->isLogout() && !LogoutChannel::instance()->isTempLogin()) {
                return true;
            }
        }
        //
        mouseActive();
        break;
    }
    case QEvent::ContextMenu: {
        if (LogoutChannel::instance()) {
            if (LogoutChannel::instance()->isLogout() && !LogoutChannel::instance()->isTempLogin()) {
                return true;
            }
        }
        break;
    }
    default:
        break;
    }
    return QApplication::notify(receiver, e);
}

void MsApplication::mouseActive()
{
    gAutoLogout.refreshTimeout();
    //
    if (SettingContent::instance()) {
        SettingContent::instance()->refreshTimeout();
    }
}

void MsApplication::onTimerHideCursor()
{
    QWSServer::instance()->setCursorVisible(false);
}
