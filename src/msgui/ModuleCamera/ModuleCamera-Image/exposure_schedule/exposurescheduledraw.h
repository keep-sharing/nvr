#ifndef EXPOSURESCHEDULEDRAW_H
#define EXPOSURESCHEDULEDRAW_H

#include "exposurescheduletime.h"
#include <QWidget>

class QPainer;

class ExposureScheduleDraw : public QWidget {
    Q_OBJECT
public:
    enum DayType {
        Sunday,
        Monday,
        Tuesday,
        Wednesday,
        Thursday,
        Friday,
        Saturday
    };

    explicit ExposureScheduleDraw(QWidget *parent = nullptr);
    ~ExposureScheduleDraw();

    void setCurrentType(const ExposureManualValue &type);
    ExposureManualValue currentType() const;

    void clearAll();
    void selectAll();

    void setSchedule(exposure_schedule_day *schedule_day_array);
    void getSchedule(exposure_schedule_day *schedule_day_array);

    void setScheduleArray(ExposureManualValue **dayArray);
    ExposureManualValue **scheduleArray() const;

    QSize sizeHint() const override;

protected:
    void showEvent(QShowEvent *) override;
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    int dayCount() const;
    QColor scheduleColor(const ExposureManualValue &value);

    void drawImage();
    void drawDaysLabel(QPainter &painter);
    void drawTimeHeader(QPainter &painter);
    void drawSchedule(QPainter &painter);
    void drawRubberBand(QPainter &painter);

    void insertTime(int day, const ExposureScheduleTime &time);

signals:

public slots:

private:
    ExposureManualValue m_currentType;
    ExposureManualValue **m_scheduleArray;

    QImage m_image;

    QRectF m_daysLabelRect;
    QRectF m_timeHeaderRect;
    QRectF m_drawRect;
    qreal m_dayHeight;

    bool m_isPressed = false;
    QPoint m_pressPoint;
    QRectF m_rubberBand;
};

#endif // EXPOSURESCHEDULEDRAW_H
