#ifndef DRAWSCENEANPR_H
#define DRAWSCENEANPR_H

#include "DrawItemAnpr.h"

#include <QGraphicsScene>
#include <QObject>

extern "C" {
#include "msg.h"
}

class DrawSceneAnpr : public QGraphicsScene {
    Q_OBJECT

protected:
    enum OperationMode {
        ModeNew, //新建一个item
        ModeOld, //操作旧的一个item
        ModeNull
    };

public:
    explicit DrawSceneAnpr(QObject *parent = nullptr);

    void finishEdit();

    QString regionName(int index) const;
    void setRegionName(int index, const QString &name);

    void clearAt(int index);
    void clearCurrent();
    void clearSelected();
    void clearAll();

    bool hasDraw() const;
    bool isFull() const;
    void setCurrentIndex(int index);
    int getSelectedRegionIndex();
    void cancelSelected();

    void setRegionData(ms_lpr_position_info *region_array);
    void getRegionData(ms_lpr_position_info *region_array, bool jsonSupport);
    void updateRegionData(ms_lpr_position_info *region_array);
    void setBaseRegionResolution(int width, int height);
    int baseRegionWidth() const;
    int baseRegionHeight() const;

    void refreshstack();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    int availableIndex() const;
    void updateItemSizeText();

signals:

public slots:

protected:
    int m_maxRegion = 4;

    bool m_pressed = false;
    QPointF m_pressPoint;

    int m_currentIndex = -1;
    DrawItemAnpr *m_selectedRegion = nullptr;

    QList<DrawItemAnpr *> m_regionList;
    OperationMode m_operation = ModeNull;

    int m_baseRegionWidth = 0;
    int m_baseRegionHeight = 0;
};

#endif // DRAWSCENEANPR_H
