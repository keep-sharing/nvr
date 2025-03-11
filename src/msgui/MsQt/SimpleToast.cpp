#include "SimpleToast.h"
#include "ui_SimpleToast.h"
#include "mainwindow.h"
#include <QTimer>

SimpleToast::SimpleToast(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SimpleToast)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_timer->setInterval(2000);
}

SimpleToast::~SimpleToast()
{
    delete ui;
}

void SimpleToast::showToast(const QString &text, const QRect &globalRc)
{
    static SimpleToast toast(MainWindow::instance());
    QFontMetrics fm(toast.font());
    QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, QSize(fm.width(text) + 60, 64), globalRc);
    toast.setGeometry(rc);
    toast.setText(text);
    toast.show();
}

void SimpleToast::setText(const QString &text)
{
    ui->label->setText(text);
    m_timer->start();
}

void SimpleToast::onTimeout()
{
    close();
}
