#include "qrcodewidget_transparent.h"
#include <QPainter>

QRCodeWidget_transparent::QRCodeWidget_transparent(QWidget *parent) :
    QWidget(parent)
{

}

QRCodeWidget_transparent::~QRCodeWidget_transparent()
{
    if (m_code)
    {
        QRcode_free(m_code);
        m_code = nullptr;
    }
}

void QRCodeWidget_transparent::setText(const QString &text)
{
    m_text = text;
    if (m_code)
    {
        QRcode_free(m_code);
        m_code = nullptr;
    }
    m_code = QRcode_encodeString(text.toStdString().c_str(), 1, QR_ECLEVEL_L, QR_MODE_8, 1);

    //一行小方块的个数
    const int qr_width = m_code->width > 0 ? m_code->width : 1;
    //小方块的宽高
    int realWidth = m_fixedWidth / qr_width * qr_width + 2;
    int realHeight = m_fixedHeight / qr_width * qr_width + 2;
    setMinimumSize(realWidth, realHeight);
    setMaximumSize(realWidth, realHeight);

    update();
}

void QRCodeWidget_transparent::setFixedSize(int w, int h)
{
    m_fixedWidth = w;
    m_fixedHeight = h;
}

void QRCodeWidget_transparent::setBorderColor(const QColor &color)
{
    m_borderColor = color;
}

void QRCodeWidget_transparent::setBackgroundColor(const QColor &color)
{
    m_backgroundColor = color;
}

void QRCodeWidget_transparent::setDataColor(const QColor &color)
{
    m_dataColor = color;
}

void QRCodeWidget_transparent::paintEvent(QPaintEvent *)
{
    if (m_text.isEmpty()) {
        return;
    }
    if (!m_code) {
        return;
    }

    QPainter painter(this);

    int w = qMin(width(), height());
    int h = w;
    QRect rc(0, 0, w - 2, h - 2);
    rc.moveCenter(rect().center());

    painter.save();
    painter.setPen(QPen(Qt::white, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(rc);
    painter.restore();

    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    //一行小方块的个数
    const int qr_width = m_code->width > 0 ? m_code->width : 1;
    //小方块的宽高
    int scale_x = w / qr_width;
    int scale_y = h / qr_width;
    for (int y = 0; y < qr_width; y++)
    {
        for (int x = 0; x < qr_width; x++)
        {
            unsigned char b = m_code->data[y * qr_width + x];
            if (b & 0x01)
            {
                //QRectF r(x * scale_x + rc.left(), y * scale_y + rc.top(), scale_x, scale_y);
                //painter.drawRects(&r, 1);
            }
            else
            {
                QRect r(x * scale_x + rc.left(), y * scale_y + rc.top(), scale_x, scale_y);
                painter.drawRect(r);
            }
        }
    }
    painter.restore();
}
