#ifndef DRAWVIDEOBASE_H
#define DRAWVIDEOBASE_H

#include <QGraphicsRectItem>

class DrawVideoBase : public QGraphicsRectItem
{
public:
    enum State
    {
        StateNormal,
        StateAlarm
    };

    DrawVideoBase(QGraphicsItem *parent = nullptr);

    void setState(int state);
    int state() const;
    virtual void clearState();

protected:
    int m_state = StateNormal;
};

#endif // DRAWVIDEOBASE_H
