#include "TargetImageWidget.h"
#include "LogoutChannel.h"
#include "TargetInfoManager.h"
#include <QPainter>
#include <QtDebug>

TargetImageWidget::TargetImageWidget(QWidget *parent)
    : QWidget(parent)
{
}

void TargetImageWidget::setIndex(int index)
{
    m_index = index;
}

void TargetImageWidget::paintEvent(QPaintEvent *)
{
    if (m_index < 0) {
        return;
    }

    gTargetInfoManager.lock();
    const TargetInfo *info = gTargetInfoManager.getTargetInfo(m_index);
    if (info) {
        QPainter painter(this);
        //painter.setRenderHint(QPainter::SmoothPixmapTransform);

        QRect rc = rect();
        qreal ratio = 16.0 / 9;
        switch (info->type()) {
        case TargetInfo::TARGET_FACE:
            ratio = 1;
            break;
        default:
            break;
        }

        if ((qreal)rc.width() / rc.height() > ratio) {
            rc.setWidth(rc.height() * ratio);
        } else {
            rc.setHeight(rc.width() / ratio);
        }
        rc.moveCenter(rect().center());
        painter.drawImage(rc, info->smallImage());
    }
    gTargetInfoManager.unlock();
}
