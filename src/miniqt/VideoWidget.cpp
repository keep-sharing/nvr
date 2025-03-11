#include "VideoWidget.h"
#include "ui_VideoWidget.h"
#include <QPainter>
#include "CameraInfo.h"

VideoWidget::VideoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoWidget)
{
    ui->setupUi(this);
}

VideoWidget::~VideoWidget()
{
    delete ui;
}

void VideoWidget::setChannel(int channel)
{
    m_channel = channel;

    update();
}

void VideoWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    painter.setPen(Qt::white);
    painter.drawText(20, 20, QString("%1").arg(m_channel + 1));

    painter.drawRect(rect());
}
