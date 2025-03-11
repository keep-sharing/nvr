#include "baseslider.h"
#include "MyDebug.h"
#include <QMouseEvent>
#include <QPainter>

BaseSlider::BaseSlider(QWidget *parent)
    : QSlider(parent)
{
    setMouseTracking(true);
    setMinimumHeight(20);

    m_value = 30;
}

void BaseSlider::setValue(int value)
{
    m_value = value;
    if (m_value < m_minValue) {
        m_value = m_minValue;
    }
    if (m_value > m_maxValue) {
        m_value = m_maxValue;
    }
    update();
    emit valueChanged(m_value);
}

void BaseSlider::editValue(int value)
{
    m_value = value;
    if (m_value < m_minValue) {
        m_value = m_minValue;
    }
    if (m_value > m_maxValue) {
        m_value = m_maxValue;
    }
    update();
}

quint32 BaseSlider::value() const
{
    return m_value;
}

void BaseSlider::setRange(quint32 min, quint32 max)
{
    m_minValue = min;
    m_maxValue = max;
    m_value = m_minValue;

    update();
}

QPair<quint32, quint32> BaseSlider::range()
{
    return qMakePair(m_minValue, m_maxValue);
}

void BaseSlider::setMaximum(int value)
{
    m_maxValue = value;
    if (m_value > m_maxValue) {
        m_value = m_maxValue;
    }

    update();
}

void BaseSlider::setMinimum(int value)
{
    m_minValue = value;
    if (m_value < m_minValue) {
        m_value = m_minValue;
    }

    update();
}

quint32 BaseSlider::maximunValue() const
{
    return m_maxValue;
}

quint32 BaseSlider::minimumValue() const
{
    return m_minValue;
}

void BaseSlider::setTextColor(const QColor &color)
{
    m_textColor = color;

    update();
}

void BaseSlider::setShowValue(bool show)
{
    m_isShowValue = show;

    update();
}

void BaseSlider::enterEvent(QEvent *event)
{
    m_drawTip = true;
    update();
    QSlider::enterEvent(event);
}

void BaseSlider::leaveEvent(QEvent *event)
{
    m_drawTip = false;
    update();
    QSlider::leaveEvent(event);
}

void BaseSlider::resizeEvent(QResizeEvent *event)
{
    QSlider::resizeEvent(event);
}

void BaseSlider::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    //painter.setRenderHint(QPainter::Antialiasing);

    //    //background
    //    painter.save();
    //    painter.setPen(Qt::NoPen);
    //    painter.setBrush(QBrush("#FFFFFF"));
    //    painter.drawRect(rect());
    //    painter.restore();

    quint32 value = m_value;
    if (m_isDragging) {
        value = m_dragValue;
    }

    //groove
    QColor grooveColor;
    if (isEnabled()) {
        grooveColor = grooveNormalColor();
    } else {
        grooveColor = grooveDisableColor();
    }
    QPoint widgetCenter = rect().center();
    m_grooveRect.setLeft(m_marginLeft);
    m_grooveRect.setTop(widgetCenter.y() - m_grooveHeight / 2 + 1);
    m_grooveRect.setRight(width() - rightMargin());
    m_grooveRect.setBottom(widgetCenter.y() + m_grooveHeight / 2 + 1);
    painter.save();
    painter.setPen(QPen(grooveColor, 1));
    painter.setBrush(QBrush(grooveColor));
    painter.drawRect(m_grooveRect);
    painter.restore();

    //value groove
    QColor valueGrooveColor;
    if (isEnabled()) {
        valueGrooveColor = grooveValueNormalColor();
    } else {
        valueGrooveColor = grooveValueDisableColor();
    }
    m_valueGrooveRect = m_grooveRect;
    m_valueGrooveRect.setWidth(qreal(value - m_minValue) / (m_maxValue - m_minValue) * m_grooveRect.width());
    painter.save();
    painter.setPen(QPen(valueGrooveColor, 1));
    painter.setBrush(QBrush(valueGrooveColor));
    painter.drawRect(m_valueGrooveRect);
    painter.restore();

    //handle
    QPoint handlePos = QPoint(m_valueGrooveRect.left() + m_valueGrooveRect.right() - m_handleWidth / 2, m_valueGrooveRect.center().y());
    QRect handleRect = QRect(handlePos, QSize(m_handleWidth, m_handleHeight));
    handleRect.moveCenter(handlePos);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(valueGrooveColor));
    painter.drawEllipse(handleRect);
    painter.restore();

    //value
    drawValue(painter);

    //tip
    drawTip(painter);
}

void BaseSlider::mousePressEvent(QMouseEvent *event)
{
    m_pressed = true;
    m_drawTip = false;
    update();
    QSlider::mousePressEvent(event);
}

void BaseSlider::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressed = false;
    m_drawTip = false;
    m_isDragging = false;
    m_value = valueUnderMouse(event->pos());
    emit valueChanged(m_value);
    emit valueEdited(m_value);
    update();
    QSlider::mouseReleaseEvent(event);
}

void BaseSlider::mouseMoveEvent(QMouseEvent *event)
{
    m_mouseMovePos = event->pos();
    if (m_pressed) {
        m_value = valueUnderMouse(event->pos());

        m_isDragging = true;
        m_dragValue = m_value;

        emit sliderMoved(m_value);
    } else {
        m_drawTip = true;
    }
    update();
    QSlider::mouseMoveEvent(event);
}

int BaseSlider::rightMarginWithoutText() const
{
    return m_handleWidth / 2 + 5;
}

int BaseSlider::rightMarginWithText() const
{
    return m_handleWidth / 2 + 30;
}

QString BaseSlider::tipText()
{
    return QString::number(valueUnderMouse(m_mouseMovePos));
}

QString BaseSlider::valueText()
{
    return QString::number(m_value);
}

QColor BaseSlider::grooveNormalColor()
{
    return QColor("#949494");
}

QColor BaseSlider::grooveDisableColor()
{
    return QColor("#BEBEBE");
}

QColor BaseSlider::grooveValueNormalColor()
{
    return QColor("#0AA9E3");
}

QColor BaseSlider::grooveValueDisableColor()
{
    return QColor("#BEBEBE");
}

QColor BaseSlider::tipNormalColor()
{
    return QColor("#7A7A7A");
}

QColor BaseSlider::tipDisableColor()
{
    return QColor("#7A7A7A");
}

QColor BaseSlider::valueNormalColor()
{
    return m_textColor;
}

QColor BaseSlider::valueDisableColor()
{
    return m_disabledColor;
}

void BaseSlider::drawValue(QPainter &painter)
{
    if (m_isShowValue) {
        QColor textColor;
        if (isEnabled()) {
            textColor = valueNormalColor();
        } else {
            textColor = valueDisableColor();
        }
        QRect textRect = rect();
        textRect.setLeft(m_grooveRect.right());
        painter.save();
        painter.setPen(QPen(textColor));
        painter.drawText(textRect, Qt::AlignCenter, valueText());
        painter.restore();
    }
}

void BaseSlider::drawTip(QPainter &painter)
{
    if (m_drawTip && isEnabled()) {
        QFont tipFont = painter.font();
        if (tipFont.pixelSize() > m_grooveRect.top()) {
            tipFont.setPixelSize(m_grooveRect.top());
        }
        QString strText = tipText();
        QFontMetrics fm(tipFont);
        int tipWidth = fm.width(strText);
        int tipHeight = fm.height();
        int tipX = m_mouseMovePos.x();
        if (tipX > m_grooveRect.right()) {
            tipX = m_grooveRect.right();
        }
        if (tipX - tipWidth / 2 < 0) {
            tipX = tipWidth / 2;
        }
        QRect tipTextRect;
        tipTextRect.setTopLeft(QPoint(tipX - tipWidth / 2, m_grooveRect.top() - tipHeight));
        tipTextRect.setBottomRight(QPoint(tipX + tipWidth / 2, m_grooveRect.top()));
        if (tipTextRect.right() > m_grooveRect.right()) {
            tipTextRect.moveRight(m_grooveRect.right());
        }
        painter.save();
        painter.setPen(tipNormalColor());
        painter.setFont(tipFont);
        painter.drawText(tipTextRect, Qt::AlignCenter, strText);
        painter.restore();
    }
}

quint32 BaseSlider::valueUnderMouse(const QPoint &pos)
{
    quint32 value = 0;
    if (pos.x() < 0) {
        value = m_minValue;
        return value;
    }

    value = qRound(qreal(pos.x()) / (width() - rightMargin()) * (m_maxValue - m_minValue)) + m_minValue;
    if (value < m_minValue) {
        value = m_minValue;
    }
    if (value > m_maxValue) {
        value = m_maxValue;
    }
    return value;
}

int BaseSlider::rightMargin() const
{
    if (m_isShowValue) {
        return rightMarginWithText();
    } else {
        return rightMarginWithoutText();
    }
}
