#ifndef WIDGETBUTTON_H
#define WIDGETBUTTON_H

#include <QWidget>

class WidgetButton : public QWidget
{
    Q_OBJECT

    enum State
    {
        StateNormal,
        StateHover,
        StatePressed
    };

public:
    explicit WidgetButton(QWidget *parent = nullptr);

protected:
    void enterEvent(QEvent *) override;
    void leaveEvent(QEvent *) override;

    void focusInEvent(QFocusEvent *) override;
    void focusOutEvent(QFocusEvent *) override;

    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;

    void paintEvent(QPaintEvent *) override;

signals:
    void clicked();

public slots:

private:
    State m_state = StateNormal;
};

#endif // WIDGETBUTTON_H
