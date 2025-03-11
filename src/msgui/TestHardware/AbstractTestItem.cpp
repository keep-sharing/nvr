#include "AbstractTestItem.h"
#include "MsColor.h"
#include "MyDebug.h"
#include "TestHardwareData.h"
#include <QPainter>

AbstractTestItem::AbstractTestItem(QWidget *parent)
    : QWidget(parent)
{
}

QString AbstractTestItem::moduleName() const
{
    return m_moduleName;
}

void AbstractTestItem::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    switch (m_testState) {
    case TEST_NONE:
        painter.setPen(QPen(QColor(	128, 128, 128), 2));
        painter.setBrush(Qt::NoBrush);
        break;
    case TEST_PASS:
        painter.setPen(QPen(QColor(0, 200, 0), 2));
        painter.setBrush(QColor(0, 200, 0, 100));
        break;
    case TEST_NOT_PASS:
        painter.setPen(QPen(QColor(200, 0, 0), 2));
        painter.setBrush(QColor(200, 0, 0, 100));
        break;
    }
    if (m_isSelected) {
        painter.setPen(QPen(gMsColor.backgroundBlue(), 8));
    }
    painter.drawRect(rect());
}

bool AbstractTestItem::isChecked() const
{
    return m_isChecked;
}

void AbstractTestItem::setChecked(bool newChecked)
{
    m_isChecked = newChecked;
    emit checkChanged(newChecked);
}

void AbstractTestItem::setSelected(bool newSelected)
{
    m_isSelected = newSelected;
    update();
}

void AbstractTestItem::setPassed(bool pass)
{
    if (pass) {
        m_testState = TEST_PASS;
    } else {
        m_testState = TEST_NOT_PASS;
    }
    update();
}
