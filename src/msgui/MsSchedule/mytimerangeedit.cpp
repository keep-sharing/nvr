#include "mytimerangeedit.h"
#include <QLineEdit>
#include <QWheelEvent>
#include <QToolButton>
#include <QtDebug>
#include "MyDebug.h"

MyTimeRangeEdit::MyTimeRangeEdit(QWidget *parent) :
    QAbstractSpinBox(parent)
{
    setContextMenuPolicy(Qt::NoContextMenu);

    m_toolClear = new QToolButton(this);
    m_toolClear->setStyleSheet("border: 0px");
    m_toolClear->setIcon(QIcon(":/common/common/refresh_hover.png"));
    m_toolClear->hide();
    connect(m_toolClear, SIGNAL(clicked(bool)), this, SLOT(onToolButtonClear()));

    QLineEdit *edit = lineEdit();
    edit->setText("00:00-00:00");
}

void MyTimeRangeEdit::setTime(int begin, int end)
{
    QLineEdit *edit = lineEdit();
    m_h1 = begin / 60;
    m_m1 = begin % 60;
    m_h2 = end / 60;
    m_m2 = end % 60;
    edit->setText(QString("%1:%2-%3:%4").arg(m_h1, 2, 10, QLatin1Char('0')).arg(m_m1, 2, 10, QLatin1Char('0')).arg(m_h2, 2, 10, QLatin1Char('0')).arg(m_m2, 2, 10, QLatin1Char('0')));
}

void MyTimeRangeEdit::clearTime()
{
    setTime(0, 0);
}

void MyTimeRangeEdit::wheelEvent(QWheelEvent *event)
{
    //大于0，远离自己，增加
    //小于0，靠近自己，减少
    int numDegrees = event->delta() / 8;
    //int numSteps = numDegrees / 15;

    if (numDegrees > 0)
    {
        stepBy(1);
    }
    else
    {
        stepBy(-1);
    }

    event->accept();
}

void MyTimeRangeEdit::focusInEvent(QFocusEvent *event)
{
    QAbstractSpinBox::focusInEvent(event);

    m_toolClear->show();

    m_previousBegin = m_h1 * 60 + m_m1;
    m_previousEnd = m_h2 * 60 + m_m2;
    qMsDebug() << this << QString("begin edit: %1:%2-%3:%4")
                   .arg(m_h1, 2, 10, QLatin1Char('0')).arg(m_m1, 2, 10, QLatin1Char('0')).arg(m_h2, 2, 10, QLatin1Char('0')).arg(m_m2, 2, 10, QLatin1Char('0'));
}

void MyTimeRangeEdit::focusOutEvent(QFocusEvent *event)
{
    QAbstractSpinBox::focusOutEvent(event);

    m_toolClear->hide();

    qMsDebug() << this << QString("end edit: %1:%2-%3:%4")
                   .arg(m_h1, 2, 10, QLatin1Char('0')).arg(m_m1, 2, 10, QLatin1Char('0')).arg(m_h2, 2, 10, QLatin1Char('0')).arg(m_m2, 2, 10, QLatin1Char('0'));
    emit timeCleared(m_previousBegin, m_previousEnd);
    emit timeEditingFinished(m_h1 * 60 + m_m1, m_h2 * 60 + m_m2);
}

QAbstractSpinBox::StepEnabled MyTimeRangeEdit::stepEnabled() const
{
    return StepEnabled(StepUpEnabled | StepDownEnabled);
}

void MyTimeRangeEdit::adjustToolPosition()
{
    int toolHeight = height() / 3 * 2;
    QRect toolGeometry(0, 0, toolHeight, toolHeight);
    toolGeometry.moveCenter(rect().center());
    toolGeometry.moveRight(width() - 30);
    m_toolClear->setGeometry(toolGeometry);
}

void MyTimeRangeEdit::onToolButtonClear()
{
    setTime(0, 0);
}

void MyTimeRangeEdit::stepBy(int steps)
{
    QLineEdit *edit = lineEdit();

    QString text = edit->text();
    QRegExp rx("(\\d\\d):(\\d\\d)-(\\d\\d):(\\d\\d)");
    if (!rx.exactMatch(text))
    {
        qWarning() << text;
    }

    m_h1 = rx.cap(1).toInt();
    m_m1 = rx.cap(2).toInt();
    m_h2 = rx.cap(3).toInt();
    m_m2 = rx.cap(4).toInt();

    int position = edit->cursorPosition();
    int startPosition = 0;

    if (position >= 0 && position <= 2)
    {
        //begin hour
        if (steps > 0)
            m_h1++;
        else
            m_h1--;
        if (m_h1 > 24)
            m_h1 = 0;
        else if (m_h1 < 0)
            m_h1 = 24;

        int beginMinutes = m_h1 * 60 + m_m1;
        int endMinutes = m_h2 * 60 + m_m2;
        if (beginMinutes >= 1440)
        {
            if (steps > 0)
            {
                m_h1 = 0;
            }
            else
            {
                m_h1 = 23;
                m_m1 = 59;
            }
            beginMinutes = m_h1 * 60 + m_m1;
        }
        if (beginMinutes >= endMinutes)
        {
            endMinutes = beginMinutes + 1;
            m_h2 = endMinutes / 60;
            m_m2 = endMinutes % 60;
        }

        startPosition = 0;
    }
    else if (position >= 3 && position <= 5)
    {
        //begin minute
        if (steps > 0)
            m_m1++;
        else
            m_m1--;
        if (m_m1 > 59)
            m_m1 = 0;
        else if (m_m1 < 0)
            m_m1 = 59;

        int beginMinutes = m_h1 * 60 + m_m1;
        int endMinutes = m_h2 * 60 + m_m2;
        if (beginMinutes >= 1440)
        {
            m_h1 = 23;
            m_m1 = 59;
            beginMinutes = m_h1 * 60 + m_m1;
        }
        if (beginMinutes >= endMinutes)
        {
            endMinutes = beginMinutes + 1;
            m_h2 = endMinutes / 60;
            m_m2 = endMinutes % 60;
        }

        startPosition = 3;
    }
    else if (position >= 6 && position <= 8)
    {
        //end hour
        if (steps > 0)
            m_h2++;
        else
            m_h2--;
        if (m_h2 > 24)
            m_h2 = 0;
        else if (m_h2 < 0)
            m_h2 = 24;

        int beginMinutes = m_h1 * 60 + m_m1;
        int endMinutes = m_h2 * 60 + m_m2;
        if (endMinutes > 1440)
        {
            m_h2 = 24;
            m_m2 = 0;
            endMinutes = m_h2 * 60 + m_m2;
        }
        else if (endMinutes <= 0)
        {
            m_h2 = 0;
            m_m2 = 1;
            endMinutes = m_h2 * 60 + m_m2;
        }
        if (beginMinutes >= endMinutes)
        {
            beginMinutes = endMinutes - 1;
            m_h1 = beginMinutes / 60;
            m_m1 = beginMinutes % 60;
        }

        startPosition = 6;
    }
    else if (position >= 9 && position <= 11)
    {
        //end minute
        if (steps > 0)
            m_m2++;
        else
            m_m2--;
        if (m_m2 > 59)
            m_m2 = 0;
        else if (m_m2 < 0)
            m_m2 = 59;

        int beginMinutes = m_h1 * 60 + m_m1;
        int endMinutes = m_h2 * 60 + m_m2;
        if (endMinutes > 1440)
        {
            if (steps > 0)
            {
                m_h2 = 0;
                m_m2 = 1;
            }
            else
            {
                m_h2 = 23;
            }
            endMinutes = m_h2 * 60 + m_m2;
        }
        else if (endMinutes <= 0)
        {
            if (steps > 0)
            {
                m_h2 = 0;
                m_m2 = 1;
            }
            else
            {
                m_h2 = 24;
                m_m2 = 0;
            }
            endMinutes = m_h2 * 60 + m_m2;
        }
        if (beginMinutes >= endMinutes)
        {
            beginMinutes = endMinutes - 1;
            m_h1 = beginMinutes / 60;
            m_m1 = beginMinutes % 60;
        }

        startPosition = 9;
    }
    else
    {

    }

    edit->setText(QString("%1:%2-%3:%4").arg(m_h1, 2, 10, QLatin1Char('0')).arg(m_m1, 2, 10, QLatin1Char('0')).arg(m_h2, 2, 10, QLatin1Char('0')).arg(m_m2, 2, 10, QLatin1Char('0')));
    edit->setSelection(startPosition, 2);
}

void MyTimeRangeEdit::showEvent(QShowEvent *event)
{
    QAbstractSpinBox::showEvent(event);

    adjustToolPosition();
}

void MyTimeRangeEdit::resizeEvent(QResizeEvent *event)
{
    QAbstractSpinBox::resizeEvent(event);

    adjustToolPosition();
}
