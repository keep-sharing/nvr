#include "mystatetoolbutton.h"

MyStateToolButton::MyStateToolButton(QWidget *parent) :
    MyToolButton(parent)
{

}

void MyStateToolButton::setStateIcon(int state, const QString &icon)
{
    m_stateMap.insert(state, QIcon(icon));
}

void MyStateToolButton::setState(int state)
{
    m_state = state;
    update();
}

void MyStateToolButton::paintEvent(QPaintEvent *event)
{
    if (!isEnabled())
    {
        if (m_state != m_lastState)
        {
            setIcon(m_stateMap.value(StateDisabled));
            m_lastState = StateDisabled;
        }
    }
    else
    {
        if (m_state != m_lastState)
        {
            setIcon(m_stateMap.value(m_state));
            m_lastState = m_state;
        }
    }
    MyToolButton::paintEvent(event);
}
