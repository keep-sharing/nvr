#include "LabelPosInfo.h"
#include "MyDebug.h"
#include <QPainter>
#include <QTimer>

LabelPosInfo::LabelPosInfo(QWidget *parent)
    : QLabel(parent)
{
    setStyleSheet("background-color:transparent;padding:0px;");

    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setWordWrap(true);

    m_timeout = new QTimer(this);
    connect(m_timeout, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_timeout->setInterval(100);
}

void LabelPosInfo::appendText(const PosData &data)
{
    setText(data.richText());

    //
    m_timeoutValue = 0;
    m_timeoutInterval = data.showTime() * 10;
    if (!m_timeout->isActive()) {
        m_timeout->start();
    }
}

void LabelPosInfo::setPaused(bool pause)
{
    if (pause) {
        m_timeout->stop();
    } else {
        m_timeout->start();
    }
}

void LabelPosInfo::setPosGeometry(const PosData &data, const QRect &videoRect)
{
    m_posArea = data.posArea();
    QRect rc = data.geometry(videoRect).toRect();
    setGeometry(rc);
}

void LabelPosInfo::resetPosGeometry(const QRect &videoRect)
{
    QRect rc = PosData::getPosGeometry(m_posArea, videoRect).toRect();
    setGeometry(rc);
}

void LabelPosInfo::paintEvent(QPaintEvent *e)
{
    QLabel::paintEvent(e);

#if 1
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 0, 0, 40));
    painter.drawRect(rect());
#endif
}

void LabelPosInfo::onTimeout()
{
    m_timeoutValue++;
    if (m_timeoutValue > m_timeoutInterval) {
        clear();
        m_timeoutValue = 0;
        m_timeout->stop();
    }
}
