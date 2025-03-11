#ifndef CHANNELWIDGET_H
#define CHANNELWIDGET_H

#include "CustomLayoutKey.h"
#include <QWidget>

namespace Ui {
class ChannelWidget;
}

class ChannelWidget : public QWidget {
    Q_OBJECT

public:
    enum State {
        StateNone,
        StateHover,
        StateSelected
    };

    explicit ChannelWidget(const CustomLayoutKey &key, int index, QWidget *parent = nullptr);
    ~ChannelWidget() override;

    int channel() const;
    void setChannel(int channel);
    void updateChannel();
    int index() const;
    void setChecked(bool checked);

protected:
    void resizeEvent(QResizeEvent *) override;
    void showEvent(QShowEvent *) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *) override;

private:
    void adjustAllWidgets();

signals:
    void clicked(int index, int channel);

private slots:
    void on_toolButton_clear_clicked();

private:
    Ui::ChannelWidget *ui;

    QColor m_backgroundColor;
    QColor m_selectedBorderColor;
    QColor m_hoverBorderColor;

    State m_state = StateNone;

    CustomLayoutKey m_key;
    int m_index = -1;
    int m_channel = -1;
};

#endif // CHANNELWIDGET_H
