#ifndef LEGENDITEM_H
#define LEGENDITEM_H

#include <QGraphicsRectItem>

class ToolTipItem;

class LegendItem : public QGraphicsRectItem
{
public:
    explicit LegendItem(QGraphicsItem *parent = nullptr);

    void setData(int channel, const QString &text, bool checked, const QImage &image);
    void setChecked(bool checked);
    void setRect(const QRectF &rect);
    void setToolTip(ToolTipItem *toolTip);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setCheckable(bool newCheckable);
    bool checkable() const;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    int m_channel = -1;
    QString m_text;
    QString m_displayText;
    bool m_checked = true;
    QImage m_image;
    bool m_checkable = true;

    ToolTipItem *m_toolTip = nullptr;
};

class ToolTipItem : public QGraphicsRectItem
{
public:
    explicit ToolTipItem(QGraphicsItem *parent = nullptr);

    void setText(const QString &text);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QString m_text;
};

#endif // LEGENDITEM_H
