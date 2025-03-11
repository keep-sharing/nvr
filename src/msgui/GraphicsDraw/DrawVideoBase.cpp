#include "DrawVideoBase.h"

DrawVideoBase::DrawVideoBase(QGraphicsItem *parent) :
    QGraphicsRectItem(parent)
{

}

void DrawVideoBase::setState(int state)
{
    if (m_state == state)
    {
        return;
    }
    m_state = state;
    update();
}

int DrawVideoBase::state() const
{
    return m_state;
}

void DrawVideoBase::clearState()
{
    m_state = StateNormal;
    update();
}
