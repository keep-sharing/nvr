#include "qrcodewidget.h"
#include <QPainter>

QRCodeWidget::QRCodeWidget(QWidget *parent) :
    QWidget(parent)
{

}

QRCodeWidget::~QRCodeWidget()
{
    if (m_code)
    {
        QRcode_free(m_code);
        m_code = nullptr;
    }
}

void QRCodeWidget::setText(const QString &text)
{
    m_text = text;
    if (m_code)
    {
        QRcode_free(m_code);
        m_code = nullptr;
    }
    m_code = QRcode_encodeString(m_text.toStdString().c_str(), 1, QR_ECLEVEL_L, QR_MODE_8, 1);
    update();
}

void QRCodeWidget::paintEvent(QPaintEvent *)
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
    QRect rc(0, 0, w, h);
    rc.moveCenter(rect().center());

    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.drawRect(rc);
    painter.restore();

    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    const int qr_width = m_code->width > 0 ? m_code->width : 1;
    qreal scale_x = w / qr_width;
    qreal scale_y = h / qr_width;
    for (int y = 0; y < qr_width; y++)
    {
        for (int x = 0; x < qr_width; x++)
        {
            unsigned char b = m_code->data[y * qr_width + x];
            if (b & 0x01)
            {
                QRectF r(x * scale_x + rc.left(), y * scale_y + rc.top(), scale_x, scale_y);
                painter.drawRects(&r, 1);
            }
        }
    }
    painter.restore();
}
