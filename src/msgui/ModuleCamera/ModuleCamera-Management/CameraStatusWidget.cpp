#include "CameraStatusWidget.h"
#include "ui_CameraStatusWidget.h"
#include "CameraStatusTip.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include <QMouseEvent>

extern "C" {
#include "recortsp.h"
}

CameraStatusWidget::CameraStatusWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CameraStatusWidget)
{
    ui->setupUi(this);

    ui->toolButtonState->installEventFilter(this);
}

CameraStatusWidget::~CameraStatusWidget()
{
    delete ui;
}

void CameraStatusWidget::setState(int state)
{
    m_state = state;

    if (RTSP_CLIENT_CONNECT == m_state) {
        ui->toolButtonState->setIcon(QIcon(":/status/status/check.png"));
        return ;
    } else {
        ui->toolButtonState->setIcon(QIcon(":/status/status/warning.png"));
    }
}

void CameraStatusWidget::setStateString(const IPC_CONNECT_STATUS &status)
{
    switch (status) {
    case IPC_CONNECT_NETWORK:
        m_disconnectedStateText = GET_TEXT("CHANNELMANAGE/30081", "Network Error");
        break;
    case IPC_CONNECT_PASSWORD:
        m_disconnectedStateText = GET_TEXT("CHANNELMANAGE/30080", "Incorrect Password");
        break;
    case IPC_CONNECT_UNKNOW:
        m_disconnectedStateText = GET_TEXT("CHANNELMANAGE/30082", "Unknown Error");
        break;
    case IPC_CONNECT_BANDWIDTH:
        m_disconnectedStateText = GET_TEXT("CHANNELMANAGE/30079", "Bandwidth Limitation");
        break;
    case IPC_CONNECT_ACCESS:
        m_disconnectedStateText = GET_TEXT("CHANNELMANAGE/30078", "Access Limitation");
        break;
    default:
        m_disconnectedStateText = GET_TEXT("CHANNELMANAGE/30082", "Unknown Error");
        break;
    }
}

int CameraStatusWidget::stateValue()
{
    return m_state;
}

bool CameraStatusWidget::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type()) {
    case QEvent::Enter: {
        CameraStatusTip::instance()->setState(m_state, m_disconnectedStateText);
        CameraStatusTip::instance()->show();
        QPoint p = mapToGlobal(QPoint(width() / 2, 0));
        QRect tipRc = CameraStatusTip::instance()->geometry();
        tipRc.moveCenter(p);
        tipRc.moveBottom(p.y());
        CameraStatusTip::instance()->move(tipRc.topLeft());
        break;
    }
    case QEvent::Leave: {
        CameraStatusTip::instance()->hide();
        break;
    }
    case QEvent::MouseButtonPress:{
        QMouseEvent *mouseEvent = (QMouseEvent *)event;
        if (mouseEvent->buttons() == Qt::RightButton) {
            CameraStatusTip::instance()->hide();
        } else {
            return false;
        }
        break;
    }
    case QEvent::MouseButtonRelease: {
        return false;
    }
    case QEvent::Wheel:
        CameraStatusTip::instance()->hide();
        break;
    default:
        break;
    }
    return QWidget::eventFilter(object, event);
}
