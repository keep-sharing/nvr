#include "popupdialog.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QtDebug>

int PopupDialog::popupWindowCount = 0;

PopupDialog::PopupDialog(QWidget *parent) :
    BaseDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

void PopupDialog::move(const QPoint &p)
{
    move(p.x(), p.y());
}

void PopupDialog::move(int x, int y)
{
    if (m_mainWidget)
    {
        QPoint p(x - m_mainWidget->width() / 2, y - m_mainWidget->height());
        m_mainWidget->move(p);
    }
}

void PopupDialog::setMainWidget(QWidget *widget)
{
    m_mainWidget = widget;
}

bool PopupDialog::hasPopupWindow()
{
    return popupWindowCount > 0;
}

void PopupDialog::mousePressEvent(QMouseEvent *event)
{
    if (m_mainWidget)
    {
        if (!m_mainWidget->geometry().contains(event->pos()))
        {
            close();
        }
    }
    BaseDialog::mousePressEvent(event);
}

void PopupDialog::showEvent(QShowEvent *event)
{
    popupWindowCount++;
    BaseDialog::showEvent(event);
}

void PopupDialog::hideEvent(QHideEvent *event)
{
    popupWindowCount--;
    BaseDialog::hideEvent(event);
}

bool PopupDialog::isMoveToCenter()
{
    return false;
}

bool PopupDialog::isAddToVisibleList()
{
    return true;
}

NetworkResult PopupDialog::dealRockerNvr(const RockerDirection &direction)
{
    NetworkResult accept = NetworkReject;

    switch (direction)
    {
    case RockerUp:
        accept = NetworkTakeOver;
        break;
    case RockerDown:
        accept = NetworkTakeOver;
        break;
    default:
        break;
    }

    return accept;
}

void PopupDialog::escapePressed()
{
    qDebug() << "PopupDialog::escapePressed";
    close();
}
