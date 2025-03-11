#include "ChannelWidgetPreview.h"
#include "ui_ChannelWidgetPreview.h"
#include "LayoutSettings.h"
#include "MsDevice.h"
#include <QPainter>

ChannelWidgetPreview::ChannelWidgetPreview(const CustomLayoutKey &key, int index, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChannelWidgetPreview)
    , m_key(key)
    , m_index(index)
{
    ui->setupUi(this);

    updateChannel();
}

ChannelWidgetPreview::~ChannelWidgetPreview()
{
    delete ui;
}

void ChannelWidgetPreview::setChannel(int channel)
{
    m_channel = channel;
    if (channel < 0 || m_index >= qMsNvr->maxChannel()) {
        ui->label_channel->clear();
    } else {
        ui->label_channel->setText(QString::number(channel + 1));
    }
}

void ChannelWidgetPreview::updateChannel()
{
    int channel = LayoutSettings::instance()->channelFromGlobalIndex(m_key, m_index);
    setChannel(channel);
}

void ChannelWidgetPreview::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#5E5E5E"));
    painter.drawRect(rect());
}
