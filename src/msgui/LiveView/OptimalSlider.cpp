#include "OptimalSlider.h"

OptimalSlider::OptimalSlider(QWidget *parent) :
                                                BaseSlider(parent)
{

}

QString OptimalSlider::tipText()
{
  double value = valueUnderMouse(m_mouseMovePos);
  return QString::number(value / 2);
}

QString OptimalSlider::valueText()
{
  return QString::number(static_cast<double>(m_value) / 2);
}
