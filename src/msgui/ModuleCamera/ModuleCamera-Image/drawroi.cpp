#include <QMouseEvent>
#include <QtDebug>
#include "drawroi.h"

#define MAX_WIDTH 100.0
#define MAX_HEIGHT 100.0

DrawRoi::DrawRoi(QWidget *parent) :
    DrawMask(parent)
{
	
}

void DrawRoi::addRoiArea(const image_roi_area &area, int index)
{
	if (area.right == 0 || area.bottom == 0)
	{
		return;
	}
	
    int l = (qreal)area.left * width() / MAX_WIDTH;
    int t = (qreal)area.top * height() / MAX_HEIGHT;
    int w = (qreal)area.right * width() / MAX_WIDTH;
    int h = (qreal)area.bottom * height() / MAX_HEIGHT;
	
    MaskWidget *widget = makeNewWidget();
    widget->setRealRect(QRect(l, t, w, h));
    widget->setColor(Qt::transparent);
	widget->setIndex(index);
#if 0	
	if (area.enable)
		widget->show();
	else
		widget->hide();
#endif
	widget->show();
    m_maskList.append(widget);
}

void DrawRoi::getRoiArea(image_roi_area *area_array, int count)
{
	int index = 0;
	for (int i = 0; i < count; i++)
    {
        image_roi_area &area = area_array[i];
		area.enable = 0;
		area.left = 0;
        area.top = 0;
        area.right = 0;
        area.bottom = 0;
    }

	for (int i = 0; i < m_maskList.size() ; i++)
    {
    	MaskWidget *widget = m_maskList.at(i);
		index = widget->getIndex();
        image_roi_area &area = area_array[index];
		const QRect &rc = widget->realRect();
        area.left = (qreal)rc.x() * MAX_WIDTH / width();
        area.top = (qreal)rc.y() * MAX_HEIGHT / height();
        area.right = (qreal)rc.width() * MAX_WIDTH / width();
        area.bottom = (qreal)rc.height() * MAX_HEIGHT / height();
		area.enable = 1;
    }
}

MaskWidget *DrawRoi::makeNewWidget()
{
    RoiWidget *widget = new RoiWidget(this);
    //widget->setIndex(m_maskList.size());
    return widget;
}

void DrawRoi::setMaxCount(int count)
{
	setMaskMaxCount(count);
}

void DrawRoi::setAreaEnable(int id, bool enable)
{
	int index = 0;
	for (int i = 0; i < m_maskList.size() ; i++)
    {
    	MaskWidget *widget = m_maskList.at(i);
		index = widget->getIndex();
		if (index == id)
		{
			if (enable)
				widget->show();
			else
				widget->hide();
			break;
		}
    }
}

int DrawRoi::getRoiWidth(const image_roi_area &area)
{
    int w = (qreal)area.right * width() / MAX_WIDTH;
	return w;
}

int DrawRoi::getRoiHeight(const image_roi_area &area)
{
    int h = (qreal)area.bottom * height() / MAX_HEIGHT;
	return h;
}


