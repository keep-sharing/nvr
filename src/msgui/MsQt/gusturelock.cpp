#include "gusturelock.h"
#include "MyDebug.h"
#include <QMouseEvent>
#include <QPainter>

GustureLock::GustureLock(QWidget *parent)
    : QWidget(parent)
{
    m_mapColor.insert(StateNone, QColor(255, 255, 255));
    m_mapColor.insert(StateSelected, QColor(87, 210, 254));
}

void GustureLock::setEnabled(bool enable)
{
    QWidget::setEnabled(enable);

    update();
}

void GustureLock::setStyle(Style style)
{
    switch (style) {
    case WhiteStyle:
        m_mapColor.insert(StateNone, QColor(255, 255, 255));
        break;
    case GrayStyle:
        m_mapColor.insert(StateNone, QColor(100, 100, 100));
        break;
    }
}

void GustureLock::resizeEvent(QResizeEvent *)
{
    m_listPoint.clear();
    int value = 0;
    int l = qMin(width(), height());
    QRect rc(0, 0, l, l);
    rc.moveCenter(rect().center());
    int w = l / 3;
    int h = l / 3;
    int length = qMin(w, h);
    int position_w, position_h;
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            position_w = column * w + w / 2 + rc.left();
            position_h = row * h + h / 2 + rc.top();
            if (row == 0) {
                position_h = h * 3 / 8 + rc.top();
            } else if (row == 2) {
                position_h = row * h + h * 5 / 8 + rc.top();
            }

            if (column == 0) {
                position_w = w * 3 / 8 + rc.left();
            } else if (column == 2) {
                position_w = column * w + w * 5 / 8 + rc.left();
            }

            QPoint center(position_w, position_h);

            QRect outerRect;
            outerRect.setWidth(length / 2);
            outerRect.setHeight(length / 2);
            outerRect.moveCenter(center);

            QRect innerRect;
            innerRect.setWidth(length / 8);
            innerRect.setHeight(length / 8);
            innerRect.moveCenter(center);

            m_lineWidth = innerRect.width() / 4;

            PointInfo info;
            info.value = ++value;
            info.outerRect = outerRect;
            info.innerRect = innerRect;
            m_listPoint.append(info);
        }
    }
}

void GustureLock::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton && !m_isPressed) {
        QWidget::mousePressEvent(event);
        return;
    }
    if (isEnabled()) {
        if (event->button() == Qt::LeftButton && !m_isPressed) {
            m_isPressed = true;
            m_listSelected.clear();

            PointInfo info = infoUnderMouse(event->pos());
            if (info.isValid()) {
                m_listPoint[info.value - 1].state = StateSelected;
                m_listSelected.append(info.value);
                m_polylinePoints.append(info.outerRect.center());
                m_lastSelectedPoint = info.outerRect.center();
                update();

                emit drawStart();
            }
        } else {
            if (m_isPressed) {
                drawEnd();
            }
        }
    }
}

void GustureLock::drawEnd()
{
    m_isPressed = false;
    clearSelected();
    update();

    if (m_listSelected.size()) {
        emit drawFinished(selectedText());
    }
}

void GustureLock::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    if (isEnabled()) {
        if (m_isPressed) {
            drawEnd();
        }
    }
}

void GustureLock::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPressed) {
        m_movePoint = event->pos();

        //不能重复
        //选择的两个点中间不能跳过
        PointInfo info = infoUnderMouse(m_movePoint);
        if (info.isValid()) {
            if (!m_listSelected.contains(info.value)) {
                if (!m_lastSelectedPoint.isNull()) {
                    QPoint a = m_lastSelectedPoint;
                    QPoint b = info.outerRect.center();
                    QLineF ab(a, b);
                    for (int i = 0; i < m_listPoint.size(); ++i) {
                        PointInfo &cInfo = m_listPoint[i];
                        QPoint c = cInfo.outerRect.center();
                        QLineF ac(a, c);
                        QLineF bc(b, c);
                        //ac + bc = ab, 则c在ab上
                        if (qFuzzyCompare(ac.length() + bc.length(), ab.length())) {
                            if (ac.length() != 0 && bc.length() != 0) {
                                if (cInfo.state != StateSelected) {
                                    cInfo.state = StateSelected;
                                    m_listSelected.append(cInfo.value);
                                    m_polylinePoints.append(cInfo.outerRect.center());
                                }
                            }
                        }
                    }
                }
                m_listPoint[info.value - 1].state = StateSelected;
                m_listSelected.append(info.value);
                m_polylinePoints.append(info.outerRect.center());

                m_lastSelectedPoint = info.outerRect.center();
            }
        }

        update();
    }
}

void GustureLock::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    for (int i = 0; i < m_listPoint.size(); ++i) {
        const PointInfo &info = m_listPoint.at(i);
        //外圈
        QColor color;
        if (isEnabled()) {
            color = m_mapColor.value(info.state);
        } else {
            color = QColor(185, 185, 185);
        }
        painter.save();
        painter.setPen(QPen(color, m_lineWidth));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(info.outerRect);
        painter.restore();
        //内圈
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawEllipse(info.innerRect);
        painter.restore();
    }
    //线
    if (m_isPressed) {
        painter.save();
        painter.setPen(QPen(m_mapColor.value(StateSelected), m_lineWidth));
        QVector<QPoint> points = m_polylinePoints;
        if (!m_movePoint.isNull()) {
            points.append(m_movePoint);
        }
        painter.drawPolyline(points.constData(), points.size());
        painter.restore();
    }
}

void GustureLock::clearSelected()
{
    m_polylinePoints.clear();
    m_movePoint = QPoint();
    m_lastSelectedPoint = QPoint();

    for (int i = 0; i < m_listPoint.size(); ++i) {
        PointInfo &info = m_listPoint[i];
        info.state = StateNone;
    }
}

QString GustureLock::selectedText()
{
    QString text;
    for (int i = 0; i < m_listSelected.size(); ++i) {
        text.append(QString::number(m_listSelected.at(i)));
    }
    m_listSelected.clear();
    return text;
}

GustureLock::PointInfo GustureLock::infoUnderMouse(const QPoint &p)
{
    for (int i = 0; i < m_listPoint.size(); ++i) {
        const PointInfo &info = m_listPoint.at(i);
        if (QRegion(info.outerRect, QRegion::Ellipse).contains(p)) {
            return info;
        }
    }
    return PointInfo();
}
