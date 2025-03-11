#include "TimeWidget.h"
#include "ui_TimeWidget.h"
#include <QDateTime>
#include <QPainter>
#include "LiveviewBottomBar.h"

TimeWidget::TimeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimeWidget)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_TransparentForMouseEvents);

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_timer->start(1000);
    onTimeout();
}

TimeWidget::~TimeWidget()
{
    delete ui;
}

void TimeWidget::setMode(int mode)
{
    m_mode = mode;
}

int TimeWidget::mode() const
{
    return m_mode;
}

void TimeWidget::setBackgroundVisible(bool visible)
{
    m_isBackgroundVisible = visible;
    update();
}

void TimeWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    if (m_isBackgroundVisible)
    {
        painter.setPen(QPen(QColor("#0aa8e3"), 2));
        painter.setBrush(QColor(46, 46, 46, 204));
        QRect rc = rect();
        rc.setWidth(width() - 4);
        rc.setHeight(height() - 4);
        rc.moveCenter(rect().center());
        int radius = rc.height() / 2;
        painter.drawRoundedRect(rc, radius, radius);
    }
}

void TimeWidget::onTimeout()
{
    ui->label_time->setText(QTime::currentTime().toString("HH:mm:ss"));
    ui->label_date->setText(QDate::currentDate().toString("yyyy-MM-dd"));
    //update();
}
