#include "PosData.h"
#include "MyDebug.h"
#include "PosDisplayData.h"
#include <QRect>
#include <QSize>

PosData::PosData()
{

}

PosData::PosData(PosMetadata *data, reco_frame *frame)
{
    m_posId = data->details->posId;
    m_channel = data->details->chnid;
    //0表示清空POS
    if (data->details->textLength == 0) {
        m_text.clear();
    } else {
        m_text = QString(data->details->textData);
    }
    m_fontColor = gPosDisplayData.colorFromIndex(data->details->fontColor);
    m_fontSize = gPosDisplayData.fontSizeFromIndex(data->details->fontSize);
    m_x = data->details->areaX;
    m_y = data->details->areaY;
    m_width = data->details->areaWidth;
    m_height = data->details->areaHeight;
    m_clearTime = data->details->clearTime;
    m_showTime = data->details->showTime;
    m_overlayMode = data->details->overlayMode;
    m_protocol = data->details->protocol;
    m_streamFormat = frame->stream_format;
    m_streamFrom = frame->stream_from;
    m_sid = frame->sid;
}

void PosData::clear()
{
    m_text.clear();
}

int PosData::posId() const
{
    return m_posId;
}

int PosData::channel() const
{
    return m_channel;
}

QString PosData::text() const
{
    return m_text;
}

QString PosData::richText() const
{
    QString str = text().trimmed().replace("\n", "<br>");
    QString txt = QString(R"(<html><head/><body><p><span style=" font-size:%1pt; color:%2;">%3</span></p></body></html>)")
                      .arg(fontSize())
                      .arg(fontColor().name())
                      .arg(str);
    return txt;
}

QColor PosData::fontColor() const
{
    return m_fontColor;
}

int PosData::fontSize() const
{
    return m_fontSize;
}

QRect PosData::posArea() const
{
    return QRect(m_x, m_y, m_width, m_height);
}

QRectF PosData::geometry(const QRect &videoRect) const
{
    return getPosGeometry(QRectF(m_x, m_y, m_width, m_height), videoRect);
}

int PosData::streamFormat() const
{
    return m_streamFormat;
}

int PosData::streamFrom() const
{
    return m_streamFrom;
}

int PosData::showTime() const
{
    if (m_protocol == POS_COMMON) {
        return m_showTime;
    } else {
        return m_clearTime;
    }
}

int PosData::overlayMode() const
{
    return m_overlayMode;
}

int PosData::sid() const
{
    return m_sid;
}

QRectF PosData::getPosGeometry(const QRectF &posArea, const QRect &videoRect)
{
    qreal x = posArea.x() / 1000 * videoRect.width();
    qreal y = posArea.y() / 1000 * videoRect.height();
    qreal w = posArea.width() / 1000 * videoRect.width();
    qreal h = posArea.height() / 1000 * videoRect.height();
    QRectF rc(x, y, w, h);
    return rc;
}
