#include "mylineeditdigital.h"
#include <QRegExpValidator>
#include<QDebug>

MyLineEditDigital::MyLineEditDigital(QWidget *parent) :
    QLineEdit(parent)
{
    QRegExp rx("\\d+|^$");
    QRegExpValidator *validator = new QRegExpValidator(rx, this);
    setValidator(validator);

    setMaxLength(9);

    connect(this, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
}

void MyLineEditDigital::setRange(int min, int max)
{
    m_minValue = min;
    m_maxValue = max;

    onEditingFinished();
}
void MyLineEditDigital::setAllowZero(bool enable)
{
    m_allowZero = enable;
}

void MyLineEditDigital::setAllowEmpty(bool enable)
{
    m_allowEmpty = enable;
}

void MyLineEditDigital::setText(const QString &text)
{
    setValue(text.toInt());
}

int MyLineEditDigital::value()
{
    return m_currentValue;
}
void MyLineEditDigital::setValue(const int &value)
{
    m_currentValue = value;
    QLineEdit::setText(QString("%1").arg(value));
}

void MyLineEditDigital::onEditingFinished()
{
    if(text().isEmpty()&&m_allowEmpty) {
        QLineEdit::setText("");
    } else {
        int value = text().toInt();
        if (value < m_minValue) {
            if (value == 0 && m_allowZero && !text().isEmpty()) {
                m_currentValue = value;
                return;
            }
            m_currentValue = m_minValue;
        } else if (value > m_maxValue) {
            m_currentValue = m_maxValue;
        } else {
            m_currentValue = value;
        }
        QLineEdit::setText(QString("%1").arg(m_currentValue));
    }
}
