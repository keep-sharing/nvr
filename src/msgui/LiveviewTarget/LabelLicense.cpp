#include "LabelLicense.h"
#include "MsDevice.h"
#include <QPainter>

QImage LabelLicense::s_imageRed;
QImage LabelLicense::s_imageBlue;

LabelLicense::LabelLicense(QWidget *parent)
    : MyLabel(parent)
{
    s_imageRed = QImage(":/liveview/liveview/car_red.png");
    s_imageBlue = QImage(":/liveview/liveview/car_green.png");
}

void LabelLicense::setLicenseType(const QString &type)
{
    m_licenseType = type;
    update();
}

void LabelLicense::clear()
{
    QLabel::clear();
    m_licenseType.clear();
    update();
}

void LabelLicense::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QRect rc;
    rc.setWidth(height() / 3.0 * 2.0);
    rc.setHeight(height() / 3.0 * 2.0);
    rc.moveCenter(rect().center());
    rc.moveRight(rect().right());
    if (m_licenseType == QString(PARAM_MS_ANPR_TYPE_WHITE)) {
        painter.drawImage(rc, s_imageBlue);
    } else if (m_licenseType == QString(PARAM_MS_ANPR_TYPE_BLACK)) {
        painter.drawImage(rc, s_imageRed);
    }
}
