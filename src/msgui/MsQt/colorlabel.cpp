#include "colorlabel.h"

ColorLabel::ColorLabel(QWidget *parent) :
    QLabel(parent)
{

}

void ColorLabel::setColor(const QColor &color)
{
    setStyleSheet(QString("background-color: %1;").arg(color.name()));
}

void ColorLabel::setColor(const QString &color)
{
    setStyleSheet(QString("background-color: %1;").arg(color));
}
