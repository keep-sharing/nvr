#include "EventPopupSub.h"
#include "ui_EventPopupSub.h"
#include <QPainter>
#include <QtDebug>
#include "SubControl.h"
#include "LiveViewSub.h"

EventPopupSub *EventPopupSub::s_eventpopupSub = nullptr;

EventPopupSub::EventPopupSub(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EventPopupSub)
{
    ui->setupUi(this);
}

EventPopupSub::~EventPopupSub()
{
    delete ui;
}

EventPopupSub *EventPopupSub::instance()
{
    if (!s_eventpopupSub)
    {
        s_eventpopupSub = new EventPopupSub(LiveViewSub::instance());
        s_eventpopupSub->hide();
    }
    return s_eventpopupSub;
}

void EventPopupSub::showSubPopup()
{
    QRect rc = LiveViewSub::instance()->rect();
    qDebug() << "EventPopupSub::showSubPopup" << rc;
    setGeometry(rc);
    show();
    raise();
}

void EventPopupSub::closeSubPopup()
{
    qDebug() << "EventPopupSub::closeSubPopup";
    hide();
}

void EventPopupSub::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(Qt::red, 8));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(rect());
}
