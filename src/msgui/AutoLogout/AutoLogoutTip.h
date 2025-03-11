#ifndef AUTOLOGOUTTIP_H
#define AUTOLOGOUTTIP_H

#include <QWidget>

namespace Ui {
class AutoLogoutTip;
}

class AutoLogoutTip : public QWidget
{
    Q_OBJECT

public:
    explicit AutoLogoutTip(QWidget *parent = nullptr);
    ~AutoLogoutTip();

    static AutoLogoutTip *instance();

    void setValue(int value);
    void clearValue();

protected:
    void paintEvent(QPaintEvent *) override;

    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;

private:
    static AutoLogoutTip *self;

    Ui::AutoLogoutTip *ui;

    bool m_isPressed = false;
    QPoint m_pressDistance;
};

#endif // AUTOLOGOUTTIP_H
