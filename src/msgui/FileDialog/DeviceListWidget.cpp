#include "DeviceListWidget.h"
#include <QListWidgetItem>
#include <QtDebug>
#include "DeviceInfoWidget.h"

DeviceListWidget::DeviceListWidget(QWidget *parent) :
    QListWidget(parent)
{

}

void DeviceListWidget::addDeviceWidget(DeviceInfoWidget *widget)
{
    QListWidgetItem *item = new QListWidgetItem(this);
    item->setSizeHint(QSize(item->sizeHint().width(), widget->height()));
    addItem(item);
    setItemWidget(item, widget);
}
