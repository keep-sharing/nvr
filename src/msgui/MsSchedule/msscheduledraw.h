#ifndef MSSCHEDULEDRAW_H
#define MSSCHEDULEDRAW_H

#include "scheduletime.h"
#include <QWidget>
#include <array>

extern "C" {
#include "msdefs.h"
}

class QPainer;

class MsScheduleDraw : public QWidget {
    Q_OBJECT
public:
    enum DayType {
        Sunday,
        Monday,
        Tuesday,
        Wednesday,
        Thursday,
        Friday,
        Saturday,
        Holiday
    };
    enum ScheduleMode {
        Mode_Normal,
        Mode_NoHoliday,
        Mode_OnlySunday
    };

    explicit MsScheduleDraw(QWidget *parent = nullptr);
    ~MsScheduleDraw() override;

    void setCurrentType(const int &type);
    int currentType() const;

    void setTypeColor(int type, QColor color);
    void clearTypeColor();

    void setScheduleMode(int mode);
    void setCheckDayEnable(bool enable);

    void clearCurrent();
    void clearAll();
    void selectAll();

    void setSchedule(schedule_day *schedule_day_array);
    void getSchedule(schedule_day *schedule_day_array);

    void setScheduleArray(uint16_t **dayArray);
    uint16_t **scheduleArray() const;

    QSize sizeHint() const override;

protected:
    void showEvent(QShowEvent *) override;
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    int dayCount() const;

    QColor scheduleColor(int type);

    void drawImage();
    void drawDaysLabel(QPainter &painter);
    void drawTimeHeader(QPainter &painter);
    void drawSchedule(QPainter &painter);
    void drawRubberBand(QPainter &painter);

private:
    int m_currentType = 0;
    //key1: day, key2: second(0-86400), value2: action_type
    uint16_t **m_scheduleArray;

    QImage m_image;

    QRectF m_daysLabelRect;
    QRectF m_timeHeaderRect;
    QRectF m_drawRect;
    qreal m_dayHeight;

    bool m_isPressed = false;
    QPoint m_pressPoint;
    QRectF m_rubberBand;

    //image改版后有些type重复但是颜色不一样，这里自己设置type对应的颜色
    QMap<int, QColor> m_typeColorMap;

    //兼容IPC
    int m_scheduleMode = 0;

    //是否检查enable，ipc的schedule有些需要设置，不设置会无效，nvr的schedule又不需要设置，设置了也会无效
    bool m_checkDayEnable = false;
};

#endif // MSSCHEDULEDRAW_H
