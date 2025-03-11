#include "LineEditDigit.h"
#include "MsLanguage.h"
#include <QRegExpValidator>

LineEditDigit::LineEditDigit(QWidget *parent)
    : LineEdit(parent)
{
    QRegExp rx(R"(\d*)");
    QRegExpValidator *validator = new QRegExpValidator(rx, this);
    setValidator(validator);
}

void LineEditDigit::setRange(int min, int max)
{
    m_minValue = min;
    m_maxValue = max;

    QString str = QString::number(max);
    setMaxLength(str.size());
}

bool LineEditDigit::check()
{
    int value = text().toInt();
    if (value < m_minValue || value > m_maxValue) {
        return false;
    }
    return true;
}

QString LineEditDigit::tipString()
{
    return GET_TEXT("MYLINETIP/112003", "Valid range: %1-%2.").arg(m_minValue).arg(m_maxValue);
}
