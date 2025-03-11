#ifndef DRAWMULTICONTROLPOLYGON_H
#define DRAWMULTICONTROLPOLYGON_H

#include "DrawControlPolygon.h"
#include "DrawMultiRegionItem.h"

class DrawMultiControlPolygon : public DrawControlPolygon
{
    Q_OBJECT
public:
    explicit DrawMultiControlPolygon(QGraphicsItem *parent = nullptr);

    void setCurrentIndex(int index);
    void setEditable(bool enable);
    void clearAllEditState();

    void setPolygon(const QString &xList, const QString &yList);
    void setRegions(SMART_REGION_INFO *regions,int index, int size);
    void getRegions(SMART_REGION_INFO *regions,int index, int size);
    void getRegion(MsIpcRegionalArea *regions, int index);

    void clearAll();
    void setRect(const QRectF &rect);
    //
    void setAlarmable(bool enable);
    void showRegionAlarm(const RegionalAlarmInfo &info);

    void setAlarmState(int state, int index);
    void clearAlarmState();

protected:
    DrawMultiRegionItem *currentItem() override;

private:
    int m_currentIndex = 0;
    QMap<int, DrawMultiRegionItem *> m_itemMap;

    bool m_isAlarmable = true;

    SMART_REGION_INFO m_regions[MAX_LEN_4];
    int m_size = 0;
    int m_scene = 0;
};

#endif // DRAWMULTICONTROLPOLYGON_H
