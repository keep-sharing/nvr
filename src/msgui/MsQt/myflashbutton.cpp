#include "myflashbutton.h"
#include <QtDebug>

MyFlashButton::MyFlashButton(QWidget *parent) :
    MyToolButton(parent)
{
    m_flashTimer = new QTimer(this);
    connect(m_flashTimer, SIGNAL(timeout()), this, SLOT(onFlashTimer()));
}

void MyFlashButton::setNormalPixmap(const QPixmap &pixmap)
{
    m_normalPixmap = pixmap;
    setIcon(m_normalPixmap);
}

void MyFlashButton::setFlashPixmapList(const QList<QPixmap> &list)
{
    m_flashPixmapList = list;
}

void MyFlashButton::setFlashInterval(int msec)
{
    m_flashTimer->setInterval(msec);
}

void MyFlashButton::startFlash()
{
    if (m_flashPixmapList.isEmpty())
    {
        qWarning() << "MyFlashButton::startFlash, m_flashPixmapList isEmpty.";
    }
    m_flashIndex = 0;
    m_flashTimer->start();
    onFlashTimer();
}

void MyFlashButton::stopFlash()
{
    m_flashTimer->stop();
    setIcon(m_normalPixmap);
}

void MyFlashButton::onFlashTimer()
{
    if (m_flashIndex >= m_flashPixmapList.size())
    {
        m_flashIndex = 0;
    }
    const QPixmap &pixmap = m_flashPixmapList.at(m_flashIndex);
    setIcon(QIcon(pixmap));
    m_flashIndex++;
}
