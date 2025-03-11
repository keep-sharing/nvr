#ifndef TIMESLIDER_H
#define TIMESLIDER_H

#include "baseslider.h"
#include <QDateTime>

class TimeSlider : public BaseSlider
{
    Q_OBJECT
public:
    explicit TimeSlider(QWidget *parent = 0);

    void setTimeRange(const QDateTime &begin, const QDateTime &end);
    void setCurrentDateTime(const QDateTime &datetime);

protected:
    QString tipText() override;

signals:

public slots:
};

#endif // TIMESLIDER_H
