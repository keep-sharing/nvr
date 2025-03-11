#include "AutoLogoutTip.h"
#include "ui_AutoLogoutTip.h"
#include "MyDebug.h"
#include "mainwindow.h"
#include <QMouseEvent>
#include <QPainter>

AutoLogoutTip *AutoLogoutTip::self = nullptr;

AutoLogoutTip::AutoLogoutTip(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AutoLogoutTip)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

AutoLogoutTip::~AutoLogoutTip()
{
    delete ui;
}

AutoLogoutTip *AutoLogoutTip::instance()
{
    if (!self) {
        if (MainWindow::instance()) {
            MainWindow *m = MainWindow::instance();
            if (!m) {
                qMsWarning() << "MainWindow is nullptr";
                return nullptr;
            }
            self = new AutoLogoutTip(m);
        }
    }
    return self;
}

void AutoLogoutTip::setValue(int value)
{
    ui->labelTime->setText(QString("%1").arg(value));
}

void AutoLogoutTip::clearValue()
{
    ui->labelTime->setText(QString("--"));
}

void AutoLogoutTip::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(QPen(QColor(225, 225, 225), 2));
    painter.setBrush(QColor(6, 127, 172));
    painter.drawEllipse(rect().center(), 38, 38);
}

void AutoLogoutTip::mousePressEvent(QMouseEvent *event)
{
    m_isPressed = true;
    m_pressDistance = event->globalPos() - pos();
}

void AutoLogoutTip::mouseReleaseEvent(QMouseEvent *)
{
    m_isPressed = false;
}

void AutoLogoutTip::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPressed) {
        move(event->globalPos() - m_pressDistance);
    }
}
