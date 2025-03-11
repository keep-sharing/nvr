#ifndef POSDATA_H
#define POSDATA_H

#include <QMetaType>
#include <QColor>

extern "C" {
#include "msdefs.h"
}

class PosData
{
public:
    explicit PosData();
    explicit PosData(PosMetadata *data, reco_frame *frame);

    void clear();

    int posId() const;
    int channel() const;
    bool isEmpty() const;
    QString text() const;
    QString richText() const;
    QColor fontColor() const;
    int fontSize() const;
    //原生的pos区域
    QRect posArea() const;
    //根据视频大小计算后的
    QRectF geometry(const QRect &videoRect) const;
    //主次码流
    int streamFormat() const;
    //预览或回放
    int streamFrom() const;
    //显示时间，秒
    int showTime() const;
    int overlayMode() const;
    //
    int sid() const;

    //
    static QRectF getPosGeometry(const QRectF &posArea, const QRect &videoRect);

private:
    int m_posId = -1;
    int m_channel = -1;
    QString m_text;
    QColor m_fontColor;
    int m_fontSize = 0;
    int m_x = 0;
    int m_y = 0;
    int m_width = 0;
    int m_height = 0;
    int m_clearTime = 600;
    int m_showTime = 5;
    int m_overlayMode = 0;
    POS_PROTOCOL_E m_protocol = POS_COMMON;

    int m_streamFormat = 0;
    int m_streamFrom = 0;
    int m_sid = 0;
};
Q_DECLARE_METATYPE(PosData)

#endif // POSDATA_H
