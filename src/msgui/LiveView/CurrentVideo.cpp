#include "CurrentVideo.h"
#include "ui_CurrentVideo.h"
#include "mainwindow.h"
#include <QPainter>

CurrentVideo::CurrentVideo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CurrentVideo)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

CurrentVideo::~CurrentVideo()
{
    delete ui;
}

void CurrentVideo::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor("#FFFFFF"), m_borderWidth));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(rect());
}

void CurrentVideo::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}
