#include "MsApplication.h"
#include "tde.h"
#include "vapi.h"
#include <QMouseEvent>
#include <QWSEvent>
#include <QWSServer>
#include <QtDebug>

#define EVENT_FILTER_TIMES (3)
#define MOUSE_FILTER_TIMES (6)

MsApplication::MsApplication(int &argc, char **argv) : QApplication(argc, argv) {
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    m_timer->start(50);
}

MsApplication::~MsApplication() {
}

bool MsApplication::notify(QObject *receiver, QEvent *e) {
    switch (e->type()) {
    case QEvent::MouseMove: {
        // qDebug() << "MsApplication::notify, receiver:" << receiver << ", event:" << e;
        // QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
        break;
    }
    case QEvent::MouseButtonPress: {
        break;
    }
    case QEvent::MouseButtonDblClick: {
        break;
    }
    case QEvent::Wheel: {
        break;
    }
    case QEvent::Paint: {
        m_needUpdate = true;
        break;
    }
    default:
        break;
    }
    return QApplication::notify(receiver, e);
}

bool MsApplication::qwsEventFilter(QWSEvent *e) {
    if (e->asMouse()) {
        m_needUpdate = true;
    }
    return QApplication::qwsEventFilter(e);
}

void MsApplication::onTimer() {
    if (m_needUpdate) {
        tde_fb_scale(SCREEN_MAIN, 0, 0, 1920, 1080, NULL);
        m_needUpdate = false;
    }
}
