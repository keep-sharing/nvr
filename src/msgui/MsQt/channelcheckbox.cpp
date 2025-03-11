#include "channelcheckbox.h"

ChannelCheckBox::ChannelCheckBox(QWidget *parent) :
    MyCheckBox(parent)
{
    connect(this, SIGNAL(clicked(bool)), this, SLOT(onClicked(bool)));
}

void ChannelCheckBox::setChannel(int channel)
{
    m_channel = channel;
}

void ChannelCheckBox::onClicked(bool checked)
{
    emit clicked(m_channel, checked);
}
