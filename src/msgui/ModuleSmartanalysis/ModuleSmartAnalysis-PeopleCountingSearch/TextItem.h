#ifndef TEXTITEM_H
#define TEXTITEM_H

#include <QGraphicsRectItem>

class TextItem : public QGraphicsRectItem
{
public:
    TextItem(QGraphicsItem *parent = nullptr);

    void setTextColor(const QColor &color);
    void setText(const QString &text);
    void adjustPos(const QPointF &pos);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QString m_text;
    QColor m_textColor = QColor(10, 168, 227);
    QColor m_backgroundColor = QColor(255, 255, 255);
    QColor m_borderColor = QColor(10, 168, 227);
};

#endif // TEXTITEM_H
