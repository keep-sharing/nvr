#ifndef DRAWITEMANPRCONTROL_H
#define DRAWITEMANPRCONTROL_H

#include "MsGraphicsObject.h"

extern "C" {
#include "msg.h"
}

class DrawItemAnprControl: public MsGraphicsObject {
  Q_OBJECT
public:
  enum {
    Type = UserType + 4
  };

  explicit DrawItemAnprControl(QGraphicsItem *parent = nullptr);
  void init();
  void startDraw();
  int type() const override;
  void regionFinish() override;
  void setItemEnable(bool enable);
  void setCurrentItem(DrawItemFacePolygon *item) override;
  void setItemsSeleteFalse() override;
  int findNewRegionIndex();
  void findCurrentItem();

  void clearAll();
  int clear();
  void clear(int index);

  void showConflict() override;
  void addShield(const MS_POLYGON &polygon, const int index);
  void getShield(MS_POLYGON *polygon);
  void updataShield(MS_POLYGON *polygon);
  void clearSelect();
  void setSelectNull();
  void refreshstack();

protected:
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
  bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) override;

private:
  bool clickedRegion(QPointF point);

  QPointF physicalPoint(const QPoint &p) const;
  QPoint logicalPoint(const QPointF &p) const;
signals:
  void conflicted();

private:
  int m_maxRegion = 4;

  bool m_pressed = false;
  QPointF m_pressPoint;
  int m_currentIndex = -1;

  DrawItemFacePolygon *m_selectedRegion = nullptr;
  QList<DrawItemFacePolygon *> m_regionList;
};

#endif // DRAWITEMANPRCONTROL_H
