#ifndef GRAPHICSMULTIREGIONSCENE_H
#define GRAPHICSMULTIREGIONSCENE_H

#include "GraphicsMultiRegionItem.h"
#include "DrawScenePolygon.h"

class MsIpcRegionalArea;
class RegionalAlarmInfo;

class GraphicsMultiRegionScene : public DrawScenePolygon {
    Q_OBJECT
public:
    explicit GraphicsMultiRegionScene(QObject *parent = nullptr);

    void setCurrentIndex(int index);
    void setEditable(bool enable);
    void clearAllEditState();

    void setRegions(MsIpcRegionalArea *regions, int size);
    void getRegions(MsIpcRegionalArea *regions, int size);
    void getRegion(MsIpcRegionalArea *regions, int index);

    void clearAll();

    //
    void setAlarmable(bool enable);
    void showRegionAlarm(const RegionalAlarmInfo &info);

protected:
    GraphicsMultiRegionItem *currentItem() override;

private:
    int m_currentIndex = 0;
    QMap<int, GraphicsMultiRegionItem *> m_itemMap;

    bool m_isAlarmable = true;
};

#endif // GRAPHICSMULTIREGIONSCENE_H
