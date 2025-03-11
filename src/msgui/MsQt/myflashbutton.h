#ifndef MYFLASHBUTTON_H
#define MYFLASHBUTTON_H

#include "mytoolbutton.h"
#include <QTimer>

class MyFlashButton : public MyToolButton
{
    Q_OBJECT

    enum State
    {
        StateNormal,
        StateFlash
    };

public:
    explicit MyFlashButton(QWidget *parent = nullptr);

    void setNormalPixmap(const QPixmap &pixmap);
    void setFlashPixmapList(const QList<QPixmap> &list);

    void setFlashInterval(int msec);
    void startFlash();
    void stopFlash();

signals:

private slots:
    void onFlashTimer();

private:
    QTimer *m_flashTimer = nullptr;

    QPixmap m_normalPixmap;

    QList<QPixmap> m_flashPixmapList;
    int m_flashIndex = 0;
};

#endif // MYFLASHBUTTON_H
