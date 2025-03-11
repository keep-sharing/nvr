#ifndef DRAWMULTIREGIONITEM_H
#define DRAWMULTIREGIONITEM_H

#include "GraphicsMultiRegionItem.h"

class DrawMultiRegionItem : public GraphicsMultiRegionItem {
public:
    explicit DrawMultiRegionItem(QGraphicsItem *parent = nullptr);
    void updateRegionAlarm(const RegionalAlarmInfo &info);
    void beginEdit() override;

    void setIsShowText(bool isShowText);

protected:
    void showText() override;

private:
    void showStayValue(int value, QColor color);

private:
    bool m_isShowText = false;
};

#endif // DRAWMULTIREGIONITEM_H
