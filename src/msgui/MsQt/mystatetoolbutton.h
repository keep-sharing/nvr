#ifndef MYSTATETOOLBUTTON_H
#define MYSTATETOOLBUTTON_H

#include "mytoolbutton.h"
#include <QMap>

class MyStateToolButton : public MyToolButton
{
    Q_OBJECT
public:
    enum State
    {
        StateNone,
        StateNormal,
        StateChecked,
        StateDisabled
    };

    explicit MyStateToolButton(QWidget *parent = nullptr);

    void setStateIcon(int state, const QString &icon);
    void setState(int state);

protected:
    void paintEvent(QPaintEvent *event) override;

signals:

private:
    int m_state = StateNone;
    int m_lastState = StateNone;
    QMap<int, QIcon> m_stateMap;
};

#endif // MYSTATETOOLBUTTON_H
