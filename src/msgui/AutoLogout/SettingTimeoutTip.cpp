#include "SettingTimeoutTip.h"
#include "ui_SettingTimeoutTip.h"
#include <QPainter>
#include <QMouseEvent>
#include "mainwindow.h"
#include "MyDebug.h"

SettingTimeoutTip *SettingTimeoutTip::self = nullptr;

SettingTimeoutTip::SettingTimeoutTip(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingTimeoutTip)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

SettingTimeoutTip::~SettingTimeoutTip()
{
    delete ui;
}

SettingTimeoutTip *SettingTimeoutTip::instance()
{
    if (!self)
    {
        MainWindow *m = MainWindow::instance();
        if (!m)
        {
            qMsWarning() << "MainWindow is nullptr";
            return nullptr;
        }
        self = new SettingTimeoutTip(m);
    }
    return self;
}

void SettingTimeoutTip::setValue(int value)
{
    ui->labelTime->setText(QString("%1").arg(value));
}

void SettingTimeoutTip::clearValue()
{
    ui->labelTime->setText(QString("--"));
}

void SettingTimeoutTip::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(QPen(QColor(225, 225, 225), 2));
    painter.setBrush(QColor(6, 172, 127));
    painter.drawEllipse(rect().center(), 38, 38);
}

void SettingTimeoutTip::mousePressEvent(QMouseEvent *event)
{
    m_isPressed = true;
    m_pressDistance = event->globalPos() - pos();
}

void SettingTimeoutTip::mouseReleaseEvent(QMouseEvent *)
{
    m_isPressed = false;
}

void SettingTimeoutTip::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPressed)
    {
        move(event->globalPos() - m_pressDistance);
    }
}
