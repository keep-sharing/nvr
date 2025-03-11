#ifndef CHANNELWIDGETPREVIEW_H
#define CHANNELWIDGETPREVIEW_H

#include "CustomLayoutKey.h"
#include <QWidget>

namespace Ui {
class ChannelWidgetPreview;
}

class ChannelWidgetPreview : public QWidget {
    Q_OBJECT

public:
    explicit ChannelWidgetPreview(const CustomLayoutKey &key, int index, QWidget *parent = nullptr);
    ~ChannelWidgetPreview();

    void setChannel(int channel);
    void updateChannel();

protected:
    void paintEvent(QPaintEvent *) override;

private:
    Ui::ChannelWidgetPreview *ui;

    CustomLayoutKey m_key;
    int m_index = -1;
    int m_channel = -1;
};

#endif // CHANNELWIDGETPREVIEW_H
