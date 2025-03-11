#include "PopupContent.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QtDebug>
#include "LiveView.h"

PopupContent *PopupContent::s_popupContent = nullptr;
PopupContent::PopupContent(QWidget *parent) :
    QDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    s_popupContent = this;
}

PopupContent *PopupContent::instance()
{
    if (!s_popupContent)
    {
        s_popupContent = new PopupContent(LiveView::instance());
        s_popupContent->hide();
    }
    return s_popupContent;
}

void PopupContent::setPopupWidget(BasePopup *widget)
{
    m_popupWidget = widget;
}

void PopupContent::showPopupWidget()
{
    showMaximized();
    m_popupWidget->move(m_popupWidget->calculatePos());
    m_popupWidget->show();
}

void PopupContent::closePopupWidget(BasePopup::CloseType type)
{
    if (m_popupWidget)
    {
        m_popupWidget->closePopup(type);
        m_popupWidget->hide();
    }
    hide();
}

void PopupContent::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        closePopupWidget(BasePopup::CloseWithRightButton);
    }
    if (event->button() == Qt::LeftButton)
    {
        if (m_popupWidget && !m_popupWidget->geometry().contains(event->pos()))
        {
            closePopupWidget(BasePopup::CloseWithLeftButton);
        }
    }
}
