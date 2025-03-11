#ifndef QRCODEWIDGET_TRANSPARENT_H
#define QRCODEWIDGET_TRANSPARENT_H

#include <QWidget>
#include "qrencode/qrencode.h"

class QRCodeWidget_transparent : public QWidget
{
    Q_OBJECT
public:
    explicit QRCodeWidget_transparent(QWidget *parent = nullptr);
    ~QRCodeWidget_transparent();

    void setText(const QString &text);
    void setFixedSize(int w, int h);

    void setBorderColor(const QColor &color);
    void setBackgroundColor(const QColor &color);
    void setDataColor(const QColor &color);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    int m_fixedWidth = 150;
    int m_fixedHeight = 150;

    QString m_text;
    QRcode *m_code = nullptr;

    QColor m_borderColor = Qt::white;
    QColor m_backgroundColor = Qt::transparent;
    QColor m_dataColor = Qt::white;
};

#endif // QRCODEWIDGET_TRANSPARENT_H
