#include "settinglistitemmain.h"
#include "QPainter"

SettingListItemMain::SettingListItemMain(const SettingItemInfo &info, QWidget *parent) :
    SettingListItem(info, parent)
{

}

void SettingListItemMain::paintEvent(QPaintEvent *event)
{
    SettingListItem::paintEvent(event);

    if (isItemMain())
    {
        QPainter painter(this);
        painter.setRenderHints(QPainter::SmoothPixmapTransform);
        QRect rc(rect().right() - 40, rect().center().y() - 6, 12, 12);
        if (isOpen())
        {
            painter.drawPixmap(rc, QPixmap(":/common/common/down-arrow.png"));
        }
        else
        {
            painter.drawPixmap(rc, QPixmap(":/common/common/right-arrow.png"));
        }
    }
}
