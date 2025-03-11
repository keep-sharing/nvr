#ifndef GRAPHICSPOSDISPLAYREGION_H
#define GRAPHICSPOSDISPLAYREGION_H

#include "GraphicsPosDisplayRegionItem.h"
#include "MsGraphicsObject.h"
#include <QMap>

class GraphicsPosDisplayRegion : public MsGraphicsObject {
    Q_OBJECT

public:
    explicit GraphicsPosDisplayRegion(QGraphicsItem *parent = nullptr);
    ~GraphicsPosDisplayRegion() override;

    void clearAll();
    void setCurrentPos(int id);
    void setPosItemRegion(int id, int x, int y, int w, int h);
    void getPosItemRegion(int id, int &x, int &y, int &w, int &h);

    void setEnabled(int current, bool enabled);

    QRect posGlobalGeometry() const;

signals:
    void mouseDragging();

private:
    int m_currentPos = 0;
    QMap<int, GraphicsPosDisplayRegionItem *> m_itemMap;
};

#endif // GRAPHICSPOSDISPLAYREGION_H
