#include "mylineedittime.h"
#include <QtDebug>

MyLineEditTime::MyLineEditTime(QWidget *parent) :
    MyLineEdit(parent)
{
    setReadOnly(true);

    m_timeEdit = new TimeEditWidget(this);
    m_timeEdit->hide();
    connect(m_timeEdit, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

    updateTime();
}

int MyLineEditTime::beginMinute() const
{
    return m_beginMinute;
}

void MyLineEditTime::setBeginMinute(int minute)
{
    m_beginMinute = minute;
    showTime();
}

int MyLineEditTime::endMinute() const
{
    return m_endMinute;
}

void MyLineEditTime::setEndMinute(int minute)
{
    m_endMinute = minute;
    showTime();
}

void MyLineEditTime::clearTime()
{
    m_beginMinute = 0;
    m_endMinute = 0;

    showTime();
}

bool MyLineEditTime::isValid() const
{
    if (m_beginMinute == 0 && m_endMinute == 0)
    {
        return true;
    }
    else
    {
        return m_beginMinute < m_endMinute;
    }
}

void MyLineEditTime::focusInEvent(QFocusEvent *)
{
    clear();

    m_timeEdit->setGeometry(rect());
    m_timeEdit->show();
    m_timeEdit->setBeginMinute(m_beginMinute);
    m_timeEdit->setEndMinute(m_endMinute);
}

void MyLineEditTime::focusOutEvent(QFocusEvent *)
{
    if (!m_timeEdit->hasFocus())
    {
        m_timeEdit->hide();
        updateTime();
    }
}

void MyLineEditTime::updateTime()
{
    m_beginMinute = m_timeEdit->beginMinute();
    m_endMinute = m_timeEdit->endMinute();
    showTime();
}

void MyLineEditTime::showTime()
{
    QString strBegin = QString("%1:%2").arg(m_beginMinute / 60, 2, 10, QLatin1Char('0')).arg(m_beginMinute % 60, 2, 10, QLatin1Char('0'));
    QString strEnd = QString("%1:%2").arg(m_endMinute / 60, 2, 10, QLatin1Char('0')).arg(m_endMinute % 60, 2, 10, QLatin1Char('0'));
    setText(QString("%1-%2").arg(strBegin).arg(strEnd));
}

void MyLineEditTime::onEditingFinished()
{
    if (!m_timeEdit->hasFocus())
    {
        m_timeEdit->hide();
        updateTime();

        emit timeEditingFinished(beginMinute(), endMinute());
    }
}
