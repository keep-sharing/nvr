#ifndef CHANNELCHECKBOX_H
#define CHANNELCHECKBOX_H

#include "MyCheckBox.h"

class ChannelCheckBox : public MyCheckBox
{
    Q_OBJECT
public:
    explicit ChannelCheckBox(QWidget *parent = nullptr);

    void setChannel(int channel);

signals:
    void clicked(int channel, bool checked);

private slots:
    void onClicked(bool checked);

private:
    int m_channel = 0;
};

#endif // CHANNELCHECKBOX_H
