#ifndef PLAYBACKTOOLBUTTON_H
#define PLAYBACKTOOLBUTTON_H

#include "mytoolbutton.h"

class PlaybackToolButton : public MyToolButton
{
    Q_OBJECT
public:
    explicit PlaybackToolButton(QWidget *parent = nullptr);

    void setBackgroundColor(const QColor &color);

protected:
    void paintEvent(QPaintEvent *event) override;

signals:

private:
    QColor m_backgroundColor;
};

#endif // PLAYBACKTOOLBUTTON_H
