#ifndef MYCALENDARWIDGET_H
#define MYCALENDARWIDGET_H

#include <QCalendarWidget>
#include <QMap>
#include <QMenu>

class MyCalendarWidget : public QCalendarWidget
{
    Q_OBJECT
public:
    explicit MyCalendarWidget(QWidget *parent = nullptr);

    void setCurrentDate(const QDate &date);

    void setDaysColor(const QMap<QDate, QColor> &dayMap);
    void clearDaysColor();

    //白色主题
    void setWhiteTheme();
    //黑色主题
    void setBlackTheme();

protected:
    void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const;
    void showEvent(QShowEvent *) override;
    void contextMenuEvent(QContextMenuEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void dateChanged(const QDate &date);

public slots:

private slots:
    void onEditingFinished();
    void onCurrentPageChanged(int year, int month);
    void onSelectionChanged();

private:
    QMap<QDate, QColor> m_dayColorMap;

    QList<QMenu *> m_menuList;

    QWidget *m_lineWidget = nullptr;

    QColor m_currentMonthColor;
    QColor m_otherMonthColor;
    QColor m_backgroundColor;
    QColor m_selectedColor;
};
#endif // MYCALENDARWIDGET_H
