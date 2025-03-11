#ifndef PAGELAYOUT_H
#define PAGELAYOUT_H

#include "ChannelWidget.h"
#include "ChannelWidgetPreview.h"
#include "CustomLayoutInfo.h"
#include <QWidget>

namespace Ui {
class PageLayout;
}

class PageLayout : public QWidget {
    Q_OBJECT

public:
    enum Mode {
        ModePreview,
        ModeDetail
    };

    explicit PageLayout(Mode mode, QWidget *parent = nullptr);
    ~PageLayout();

    void initializeData(const CustomLayoutInfo &info, int page);
    int page() const;
    int layoutMode() const;
    void setChecked(bool checked);

    void updateChannelPreview();

    void updateChannel();
    void setChannel(int channel);

protected:
    void enterEvent(QEvent *) override;
    void leaveEvent(QEvent *) override;
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;

private:
signals:
    void clicked(const CustomLayoutKey &key, int page);

private slots:
    void onChannelWidgetClicked(int index, int channel);

private:
    Ui::PageLayout *ui;

    Mode m_mode;

    int m_layoutMode;
    int m_page;
    int m_currentIndex = -1;

    bool m_isSelected = false;
    bool m_isHover = false;

    CustomLayoutKey m_key;
    //key: index
    QMap<int, ChannelWidget *> m_mapChannelWidget;
    QList<ChannelWidgetPreview *> m_channelWidgetPreviewList;
};

#endif // PAGELAYOUT_H
