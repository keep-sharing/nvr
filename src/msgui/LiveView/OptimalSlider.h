#ifndef OPTIMALSLIDER_H
#define OPTIMALSLIDER_H

#include "baseslider.h"
class OptimalSlider : public BaseSlider
{
  Q_OBJECT
public:
  explicit OptimalSlider(QWidget *parent = 0);

protected:
  QString tipText() override;
  QString valueText() override;
signals:

public slots:
};

#endif // OPTIMALSLIDER_H
