#include "DownloadAnimate.h"
#include <QPainter>
#include <QPropertyAnimation>
#include "DownloadPanel.h"
#include "settingcontent.h"

DownloadAnimate *DownloadAnimate::self = nullptr;
DownloadAnimate::DownloadAnimate(QWidget *parent) :
    QWidget(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    m_animation = new QPropertyAnimation(this, "geometry");
    m_animation->setDuration(1000);
    m_animation->setEasingCurve(QEasingCurve::OutExpo);
    connect(m_animation, SIGNAL(finished()), this, SLOT(close()));

    resize(50, 50);
}

DownloadAnimate *DownloadAnimate::instance()
{
    if (!self)
    {
        self = new DownloadAnimate(DownloadPanel::instance());
    }
    return self;
}

void DownloadAnimate::showAnimate(const QRect &rc, const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    setGeometry(rc);
    show();
}

void DownloadAnimate::startAnimate(const QRect &rc, const QPixmap &pixmap)
{
    m_pixmap = pixmap;

    QRect rc1 = rc;
    m_animation->setStartValue(rc1);
    QPoint p = SettingContent::instance()->globalDownloadPos();
    QRect rc2(0, 0, 2, 2);
    rc2.moveCenter(p);
    m_animation->setEndValue(rc2);
    m_animation->start();
    show();
}

void DownloadAnimate::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(rect(), m_pixmap);
}
