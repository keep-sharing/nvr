#include "mycalendarwidget.h"
#include <QPainter>
#include <QTextCharFormat>
#include <QVBoxLayout>
#include <QToolButton>
#include <QSpinBox>
#include <QSettings>
#include <QMouseEvent>
#include <QApplication>
#include <QtDebug>
#include "MyDebug.h"

#include "msdefs.h"

MyCalendarWidget::MyCalendarWidget(QWidget *parent) :
    QCalendarWidget(parent)
{
    setFirstDayOfWeek(Qt::Sunday);
    setHorizontalHeaderFormat(QCalendarWidget::ShortDayNames);
    setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);

    //
    m_lineWidget = new QWidget(this);
    m_lineWidget->setMaximumHeight(1);
    m_lineWidget->setMinimumHeight(1);
    QVBoxLayout *vBoxLayout = qobject_cast<QVBoxLayout *>(layout());
    vBoxLayout->insertWidget(1, m_lineWidget);

    //
    QToolButton *prevBtn = findChild<QToolButton*>(QLatin1String("qt_calendar_prevmonth"));
    prevBtn->setIcon(QIcon());
    prevBtn->setText("<");
    prevBtn->setAutoRepeat(false);
    QToolButton *bextBtn = findChild<QToolButton*>(QLatin1String("qt_calendar_nextmonth"));
    bextBtn->setIcon(QIcon());
    bextBtn->setText(">");
    bextBtn->setAutoRepeat(false);

    //修复点击月份弹出月份选择，不选择月份下直接选择日期，日历自动关闭，但月份弹出框没有关闭
    m_menuList = findChildren<QMenu *>();
    connect(this, SIGNAL(clicked(QDate)), this, SLOT(onEditingFinished()));

    connect(this, SIGNAL(currentPageChanged(int,int)), this, SLOT(onCurrentPageChanged(int,int)));
    connect(this, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));

    //
    QSpinBox *spinBox = findChild<QSpinBox *>();
    if (spinBox)
    {
        spinBox->setProperty("ShowKeyboard", -12345);
        spinBox->setContextMenuPolicy(Qt::NoContextMenu);
    }

    setBlackTheme();
}

void MyCalendarWidget::setCurrentDate(const QDate &date)
{
    setSelectedDate(date);
    emit dateChanged(date);
}

void MyCalendarWidget::setDaysColor(const QMap<QDate, QColor> &dayMap)
{
    m_dayColorMap.clear();
    m_dayColorMap = dayMap;
    update();
}

void MyCalendarWidget::clearDaysColor()
{
    m_dayColorMap.clear();
    update();
}

void MyCalendarWidget::setWhiteTheme()
{
    m_lineWidget->setStyleSheet("background-color: #B9B9B9");

    QTextCharFormat format;
    format.setForeground(QColor("#4A4A4A"));
    format.setBackground(QColor("#FFFFFF"));
    setHeaderTextFormat(format);
    setWeekdayTextFormat(Qt::Saturday, format);
    setWeekdayTextFormat(Qt::Sunday,   format);
    setWeekdayTextFormat(Qt::Monday,   format);
    setWeekdayTextFormat(Qt::Tuesday,  format);
    setWeekdayTextFormat(Qt::Wednesday,format);
    setWeekdayTextFormat(Qt::Thursday, format);
    setWeekdayTextFormat(Qt::Friday,   format);

    m_currentMonthColor = QColor("#4A4A4A");
    m_otherMonthColor = QColor("#787878");
    m_backgroundColor = QColor("#FFFFFF");
    m_selectedColor = QColor("#00A2E8");
}

void MyCalendarWidget::setBlackTheme()
{
    m_lineWidget->setStyleSheet("background-color: #FFFFFF");

    QTextCharFormat format;
    format.setForeground(QColor("#FFFFFF"));
    format.setBackground(QColor("#2E2E2E"));
    setHeaderTextFormat(format);
    setWeekdayTextFormat(Qt::Saturday, format);
    setWeekdayTextFormat(Qt::Sunday,   format);
    setWeekdayTextFormat(Qt::Monday,   format);
    setWeekdayTextFormat(Qt::Tuesday,  format);
    setWeekdayTextFormat(Qt::Wednesday,format);
    setWeekdayTextFormat(Qt::Thursday, format);
    setWeekdayTextFormat(Qt::Friday,   format);

    m_currentMonthColor = QColor("#FFFFFF");
    m_otherMonthColor = QColor("#787878");
    m_backgroundColor = QColor("#2E2E2E");
    m_selectedColor = QColor("#00A2E8");
}

void MyCalendarWidget::paintCell(QPainter *painter, const QRect &rect, const QDate &date) const
{
    QDate selectDate = selectedDate();
    //
    if (date == selectDate)
    {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(m_backgroundColor));
        painter->drawRect(rect);
        painter->restore();

        painter->save();
        painter->setPen(QPen(m_selectedColor));
        painter->setBrush(Qt::NoBrush);
        QRect rc = rect;
        rc.setLeft(rect.left() + 5);
        rc.setTop(rect.top() + 3);
        rc.setRight(rect.right() - 6);
        rc.setBottom(rect.bottom() - 4);
        painter->drawRect(rc);
        painter->restore();
    }
    //
    if (date.month() == selectDate.month())
    {
        painter->save();

        painter->setPen(QPen(m_currentMonthColor));
        painter->drawText(rect, Qt::AlignCenter, QString::number(date.day()));

        painter->restore();
    }
    else
    {
        painter->save();

        painter->setPen(QPen(m_otherMonthColor));
        painter->drawText(rect, Qt::AlignCenter, QString::number(date.day()));

        painter->restore();
    }
    //
    if (m_dayColorMap.contains(date))
    {
        painter->save();

        painter->setPen(QPen(m_dayColorMap.value(date)));
        painter->drawText(rect, Qt::AlignCenter, QString::number(date.day()));

        painter->restore();
    }
}

void MyCalendarWidget::showEvent(QShowEvent *)
{
    QSettings setting(GUI_LANG_INI, QSettings::IniFormat);
    int lang = setting.value("language", 0).toInt();
    switch(lang)
    {
    case 1: //中文简体
        setLocale(QLocale(QLocale::Chinese, QLocale::China));
        break;
    case 2: //中文繁体
        setLocale(QLocale(QLocale::Chinese, QLocale::Taiwan));
        break;
    case 3: //匈牙利语
        setLocale(QLocale(QLocale::Hungarian));
        break;
    case 4: //俄语
        setLocale(QLocale(QLocale::Russian));
        break;
    case 5: //法语
        setLocale(QLocale(QLocale::French));
        break;
    case 6: //波兰语
        setLocale(QLocale(QLocale::Polish));
        break;
    case 7: //丹麦语
        setLocale(QLocale(QLocale::Danish));
        break;
    case 8: //意大利语
        setLocale(QLocale(QLocale::Italian));
        break;
    case 9: //德语
        setLocale(QLocale(QLocale::German));
        break;
    case 10:    //捷克语
        setLocale(QLocale(QLocale::Czech));
        break;
    case 11:    //日语
        setLocale(QLocale(QLocale::Japanese));
        break;
    case 12:    //韩语
        setLocale(QLocale(QLocale::Korean));
        break;
    case 13:    //芬兰语
        setLocale(QLocale(QLocale::Hungarian));
        break;
    case 14:    //土耳其语
        setLocale(QLocale(QLocale::Turkish));
        break;
    case 16:    //希伯来语
        setLocale(QLocale(QLocale::Hebrew));
        break;
    case 17:    //阿拉伯语
        setLocale(QLocale(QLocale::Arabic));
        break;
    case 19:    //波斯语
        setLocale(QLocale(QLocale::Persian));
        break;
    case 20:    //荷兰语
        setLocale(QLocale(QLocale::Dutch));
        break;
    default:
        setLocale(QLocale(QLocale::English, QLocale::AnyCountry));
        break;
    }
}

void MyCalendarWidget::contextMenuEvent(QContextMenuEvent *)
{
    //向父窗口传递鼠标右键事件
    QMouseEvent event(QEvent::MouseButtonPress, QPoint(0, 0), Qt::RightButton, 0, 0);
    qApp->sendEvent(this, &event);
}

void MyCalendarWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        QWidget::mousePressEvent(event);
    }
    else
    {
        QCalendarWidget::mousePressEvent(event);
    }
}

void MyCalendarWidget::onEditingFinished()
{
    for (int i = 0; i < m_menuList.size(); ++i)
    {
        m_menuList.at(i)->close();
    }
}

void MyCalendarWidget::onCurrentPageChanged(int year, int month)
{
    Q_UNUSED(year)
    Q_UNUSED(month)
    //qDebug() << this << QString("MyCalendarWidget::onCurrentPageChanged, year: %1, month: %2").arg(year).arg(month);

    setSelectedDate(QDate(year, month, 1));
}

void MyCalendarWidget::onSelectionChanged()
{
    //MsDebug() << selectedDate();
    emit dateChanged(selectedDate());
}
