#include <QMouseEvent>
#include <QtDebug>
#include "drawenhancement.h"

#define exposureAreaSize	1000

DrawEnhancement::DrawEnhancement(QWidget *parent) :
    DrawMask(parent)
{

}

void DrawEnhancement::addEnhanceArea(int left, int top, int right, int bottom)
{
	if (right == 0 || bottom == 0)
	{
		return;
	}
	
	int l = (qreal)left * width()/exposureAreaSize;
	int t = (qreal)top * height()/exposureAreaSize;
	int w = (qreal)((right-left) * width()/exposureAreaSize);
	int h = (qreal)((bottom-top) * height()/exposureAreaSize);
	
    BlcWidget *widget = new BlcWidget(this);
	widget->setRealRect(QRect(l,t, w,h));
    widget->setColor(Qt::transparent);
    widget->setBlcType(BlcCustomize);
	widget->show();
    m_maskList.append(widget);
}

void DrawEnhancement::addCentreArea()
{
    BlcWidget *widget = new BlcWidget(this);
    QRect rc;
    rc.setWidth(width() / 2);
    rc.setHeight(height() / 2);
    rc.moveCenter(rect().center());
    widget->setRealRect(rc);
    widget->setColor(Qt::transparent);
    widget->setBlcType(BlcCentre);
    widget->show();
    m_maskList.append(widget);
}

void DrawEnhancement::getEnhanceArea(image_exposure_area *area)
{
    if (m_maskList.size() > 0)
    {
        MaskWidget *widget = m_maskList.at(0);
        const QRect &rc = widget->realRect();
        area->left = (qreal)rc.x()*exposureAreaSize / width();
        area->top = (qreal)rc.y()*exposureAreaSize / height();
        area->right = (qreal)((rc.width()+rc.x())*exposureAreaSize / width());
        area->bottom = (qreal)((rc.height()+rc.y())*exposureAreaSize / height());
    }
    else
    {
        area->left = 0;
        area->top = 0;
        area->right = 0;
       	area->bottom = 0;
    }
}

void DrawEnhancement::setBlcType(BlcType type)
{
    m_blcType = type;
    for (int i = 0; i < m_maskList.size(); ++i)
    {
        BlcWidget *widget = static_cast<BlcWidget *>(m_maskList.at(i));
        widget->setBlcType(type);
    }
}

void DrawEnhancement::mousePressEvent(QMouseEvent *event)
{
    if (m_blcType == BlcCentre)
    {
        return;
    }

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

void DrawEnhancement::mouseMoveEvent(QMouseEvent *event)
{
    if (m_pressed)
    {
        if (m_operation == ModeNew)
        {
            clearAll();
        }
        if (m_currentMask == nullptr && m_maskList.size() < m_maxMaskCount)
        {
            m_currentMask = makeNewWidget();
            m_currentMask->setSelected(true);
            m_currentMask->setColor(m_selectedColor);
            m_currentMask->show();
            m_maskList.append(m_currentMask);
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
            m_currentMask->setGeometry(normalRect);
        }
    }
    QWidget::mouseMoveEvent(event);
}

MaskWidget *DrawEnhancement::makeNewWidget()
{
    BlcWidget *widget = new BlcWidget(this);
    widget->setBlcType(m_blcType);
    return widget;
}
