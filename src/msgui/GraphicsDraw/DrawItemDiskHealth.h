#ifndef DRAWITEMDISKHEALTH_H
#define DRAWITEMDISKHEALTH_H

#include <QDateTime>
#include <QGraphicsRectItem>
#include "msfs_disk.h"

struct disk_temperature;

class DrawItemDiskHealth: public QGraphicsRectItem
{
public:
  DrawItemDiskHealth(QGraphicsItem *parent = nullptr);
  void showDiskHealthMap(struct disk_temperature *temperatureList);

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
  void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

private:
  void drawGrid(QPainter *painter);
  void drawLine(QPainter *painter);

private:

 struct disk_temperature m_values[MAX_DISK_HM_TEMPERATURE];
 QList<QRectF> m_valueRects;

  int m_left = 0;
  int m_right = 0;
  int m_top = 0;
  int m_bottom = 0;

  int m_yMax = 0;
  qreal m_xStep = 0;

  int m_hoverIndex = -1;
  qreal m_hoverValue = 0;
  uint m_hoverTime;
};

#endif // DRAWITEMDISKHEALTH_H
