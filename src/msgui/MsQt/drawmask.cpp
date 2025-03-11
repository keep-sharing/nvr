#include "drawmask.h"
#include <QMouseEvent>
#include <QtDebug>
#include "maskwidget.h"

DrawMask::DrawMask(QWidget *parent) : QWidget(parent)
{

}

void DrawMask::setColor(const QColor &color)
{
    m_selectedColor = color;

    for (int i = 0; i < m_maskList.size(); ++i)
    {
        MaskWidget *widget = m_maskList.at(i);
		if (widget->getColor() != Qt::transparent)
        	widget->setColor(m_selectedColor);
    }
}

void DrawMask::setColorType(int type)
{
    m_colorType = type;
    switch (type)
    {
    case 0:
        setColor(Qt::white);
        break;
    case 1:
        setColor(Qt::black);
        break;
    case 2:
        setColor(Qt::blue);
        break;
	case 3:
        setColor(Qt::yellow);
        break;
	case 4:
        setColor(Qt::green);
        break;
	case 5:
        //setColor(Qt::brown);
        setColor(QColor("#7F2A2A"));
        break;
	case 6:
        setColor(Qt::red);
        break;
	case 7:
        //setColor(Qt::purple);
        setColor(QColor("#8A2CDE"));
        break;
    default:
        break;
    }
}

void DrawMask::clearSelected()
{
    if (m_currentMask)
    {
        m_maskList.removeOne(m_currentMask);
        delete m_currentMask;
        m_currentMask = nullptr;
    }
}

void DrawMask::clearAll()
{
    for (int i = m_maskList.size(); i > 0; --i)
    {
        MaskWidget *widget = m_maskList.at(i - 1);
        delete widget;
    }
    m_maskList.clear();
    m_currentMask = nullptr;
}

void DrawMask::setMaxMaskCount(int count)
{
    m_maxMaskCount = count;
}

void DrawMask::addMask(const mask_area_ex &area)
{
    if (area.area_width == 0 || area.area_height == 0)
    {
        return;
    }

    int x = (qreal)area.start_x / 100 * width();
    int y = (qreal)area.start_y / 100 * height();
    int w = (qreal)area.area_width / 100 * width();
    int h = (qreal)area.area_height / 100 * height();

    MaskWidget *widget = makeNewWidget();
	widget->setRealRect(QRect(x, y, w, h));
	widget->setIndex(area.area_id);
	if (!area.enable)
		widget->setColor(Qt::transparent);
	else
		widget->setColorType(area.fill_color);
    widget->show();
	

    m_maskList.append(widget);
}

void DrawMask::getMask(mask_area_ex *area_array, int count)
{
	int index = 0;

	for (int i = 0; i < count; i++)
	{
		
        mask_area_ex &area = area_array[i];
        area.fill_color = m_colorType;

		area.start_x = 0;
        area.start_y = 0;
        area.area_width = 0;
        area.area_height = 0;
		area.enable = 0;
	}

	for (int i = 0; i < m_maskList.size(); i++)
	{
		MaskWidget *widget = m_maskList.at(i);
		index = widget->getIndex();
        mask_area_ex &area = area_array[index];
		
        const QRect &rc = widget->realRect();
        area.start_x = (qreal)rc.x() / width() * 100;
        area.start_y = (qreal)rc.y() / height() * 100;
        area.area_width = (qreal)rc.width() / width() * 100;
        area.area_height = (qreal)rc.height() / height() * 100;
		area.enable = 1;
	}
}

void DrawMask::mousePressEvent(QMouseEvent *event)
{
    m_pressed = true;
    m_pressPoint = event->pos();
    m_currentMask = maskUnderMouse(event->pos());
    if (m_currentMask)
    {
        m_operation = ModeOld;
        m_currentMask->setSelected(true);
        m_currentMask->raise();
        //处理z轴问题
        m_maskList.removeOne(m_currentMask);
        m_maskList.append(m_currentMask);
    }
    else
    {
        m_operation = ModeNew;
    }
    for (int i = m_maskList.size(); i > 0; --i)
    {
        MaskWidget *widget = m_maskList.at(i - 1);
        if (widget != m_currentMask)
        {
            widget->setSelected(false);
        }
    }
    QWidget::mousePressEvent(event);
}

void DrawMask::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressed = false;
    m_operation = ModeNull;
    QWidget::mouseReleaseEvent(event);
}

void DrawMask::mouseMoveEvent(QMouseEvent *event)
{
	int newId = 0;
	int i = 0, j = 0;
	
    if (m_pressed)
    {
    	if (m_currentMask == nullptr && m_maskList.size() < m_maxMaskCount)
        {
        	//get id
			for (i = 0; i < m_maxMaskCount; i++)
			{
				for (j = 0; j < m_maskList.size(); j++)
			    {
			        MaskWidget *widget = m_maskList.at(j);
					if (widget->getIndex() == i)
					{
						break;
					}
			    }

				if (j >= m_maskList.size())
				{
					newId = i;
					break;
				}
			}
            m_currentMask = makeNewWidget();
            if (m_currentMask)
            {
                m_currentMask->setSelected(true);
                m_currentMask->setColor(m_selectedColor);
                m_currentMask->setIndex(newId);
                m_currentMask->show();
                m_maskList.append(m_currentMask);
            }
        }
        if (m_currentMask && m_operation == ModeNew)
        {
            QRect rc(m_pressPoint, event->pos());
            QRect normalRect = rc.normalized();
            int margin = m_currentMask->margin();
            if (normalRect.left() < -margin)
            {
                normalRect.setLeft(-margin);
            }
            if (normalRect.top() < -margin)
            {
                normalRect.setTop(-margin);
            }
            if (normalRect.right() > width() + margin)
            {
                normalRect.setRight(width() + margin);
            }
            if (normalRect.bottom() > height() + margin)
            {
                normalRect.setBottom(height() + margin);
            }
            //设置最小值
            if (normalRect.width() < 20)
            {
                normalRect.setWidth(20);
            }
            if (normalRect.height() < 20)
            {
                normalRect.setHeight(20);
            }
            if (normalRect.right() > width() + margin)
            {
                normalRect.moveRight(width() + margin);
            }
            if (normalRect.bottom() > height() + margin)
            {
                normalRect.moveBottom(height() + margin);
            }
            //
            m_currentMask->setGeometry(normalRect);
        }
    }
    QWidget::mouseMoveEvent(event);
}

MaskWidget *DrawMask::maskUnderMouse(const QPoint &pos) const
{
    for (int i = m_maskList.size(); i > 0; --i)
    {
        MaskWidget *widget = m_maskList.at(i - 1);
        if (widget->isUnderMouse(pos))
        {
            return widget;
        }
    }
    return nullptr;
}

MaskWidget *DrawMask::makeNewWidget()
{
    MaskWidget *widget = new MaskWidget(this);
    return widget;
}

void DrawMask::setMaskMaxCount(int count)
{
	m_maxMaskCount = count;
}

void DrawMask::setMaskEnable(int id, bool enable)
{
	for (int i = m_maskList.size(); i > 0; --i)
    {
        MaskWidget *widget = m_maskList.at(i - 1);
		if (widget->getIndex() == id)
		{
			if (enable)
				widget->setColorType(m_colorType);
			else
				widget->setColor(Qt::transparent);
			break;
		}
    }
}

void DrawMask::hideAll()
{
    for (int i = m_maskList.size(); i > 0; --i)
    {
        MaskWidget *widget = m_maskList.at(i - 1);
        widget->hide();
    }
}

void DrawMask::showAll()
{
    for (int i = m_maskList.size(); i > 0; --i)
    {
        MaskWidget *widget = m_maskList.at(i - 1);
        widget->show();
    }
}

