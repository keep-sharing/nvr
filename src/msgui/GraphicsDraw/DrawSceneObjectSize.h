#ifndef DRAWSCENEOBJECTSIZE_H
#define DRAWSCENEOBJECTSIZE_H

#include <QGraphicsScene>
#include "msqtvca.h"

class DrawItemObjectSize;
struct ms_vca_settings_info2;
struct MsIpcRegionalPeople;

class DrawSceneObjectSize : public QGraphicsScene {
    Q_OBJECT

public:
    enum LogicSizeType{
     Before_8_0_3_r9,
     After_8_0_3_r9,
    };
    explicit DrawSceneObjectSize(QObject *parent = nullptr);

    void showMinimumObjectSize(ms_vca_settings_info2 *info, MsQtVca::ObjectVcaType type, int sizeType);
    void showMaximumObjectSize(ms_vca_settings_info2 *info, MsQtVca::ObjectVcaType type, int sizeType);

    void showMinimumObjectSize(MsIpcRegionalPeople *regionInfo, int sizeType);
    void showMaximumObjectSize(MsIpcRegionalPeople *regionInfo, int sizeType);

    void showMinimumObjectSize(int x, int y, int w, int h, int sizeType);
    void showMaximumObjectSize(int x, int y, int w, int h, int sizeType);

    //item调用
    void updateSize(MsQtVca::ObjectSizeType sizeType);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QPointF physicalPos(qreal x, qreal y);
    QSizeF physicalSize(qreal w, qreal h);
    qreal physicalWidth(qreal w);
    qreal physicalHeight(qreal h);

    QPoint logicalPos(const QPointF &pos);
    int logicalWidth(qreal w);
    int logicalHeight(qreal h);

signals:
    void objectSizeChanged(const QRect &rc, MsQtVca::ObjectSizeType sizeType);

private:
    DrawItemObjectSize *m_item = nullptr;
    LogicSizeType m_logicSizeType;
};

#endif // DRAWSCENEOBJECTSIZE_H
