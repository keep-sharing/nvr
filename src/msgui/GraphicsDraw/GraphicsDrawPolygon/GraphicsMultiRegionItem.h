#ifndef GRAPHICSMULTIREGIONITEM_H
#define GRAPHICSMULTIREGIONITEM_H

#include "DrawItemPolygon.h"

class MsIpcRegionalPeople;
class RegionalAlarmInfo;

class GraphicsMultiRegionItem : public DrawItem_Polygon
{
public:
    explicit GraphicsMultiRegionItem(QGraphicsItem *parent = nullptr);

    void updateRegionAlarm(const RegionalAlarmInfo &info);
    void showItem(MsIpcRegionalPeople *info, int index);

    void beginEdit() override;
    void endEdit() override;

    bool alarm() const;
    void setAlarm(bool alarm);

protected:
    QColor lineColor() override;

    void showText() override;
    void clearText() override;

    QPointF physicalPoint(const QPoint &p) const;
    QPoint logicalPoint(const QPointF &p) const;

private:
    void showStayValue(int value, QColor color);
    void hideStayValue();

protected:
    int m_channel = -1;
    int m_stayValue = 0;
    bool m_alarm = false;
    QGraphicsTextItem *m_textItem;
};

#endif // GRAPHICSMULTIREGIONITEM_H
