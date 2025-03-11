#ifndef SETTINGTIMEOUTTIP_H
#define SETTINGTIMEOUTTIP_H

#include <QWidget>

namespace Ui {
class SettingTimeoutTip;
}

class SettingTimeoutTip : public QWidget
{
    Q_OBJECT

public:
    explicit SettingTimeoutTip(QWidget *parent = nullptr);
    ~SettingTimeoutTip();

    static SettingTimeoutTip *instance();

    void setValue(int value);
    void clearValue();

protected:
    void paintEvent(QPaintEvent *) override;

    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;

private:
    static SettingTimeoutTip *self;

    Ui::SettingTimeoutTip *ui;

    bool m_isPressed = false;
    QPoint m_pressDistance;
};

#endif // SETTINGTIMEOUTTIP_H
