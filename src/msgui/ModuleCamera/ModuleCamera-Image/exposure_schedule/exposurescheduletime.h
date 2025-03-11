#ifndef EXPOSURESCHEDULETIME_H
#define EXPOSURESCHEDULETIME_H

#include <QHash>
#include <QMap>
#include <QVariant>

#define MAX_SCHEDULE_SEC (86400 + 1)

extern "C" {
#include "msdefs.h"
}

enum ExposureAction {
    ExposureActionAuto = 0,
    ExposureActionManual
};

struct ExposureManualValue {
    uint8_t actionType = 0;
    uint8_t exposureTime = 0;
    uint8_t gainLevel = 0;

    ExposureManualValue()
    {
    }
    ExposureManualValue(int action, int exposure, int gain)
    {
        actionType = static_cast<uint8_t>(action);
        exposureTime = static_cast<uint8_t>(exposure);
        gainLevel = static_cast<uint8_t>(gain);
    }

    bool isValid() const
    {
        if (actionType == 0) {
            return false;
        }
        return true;
    }

    bool operator<(const ExposureManualValue &other) const
    {
        if (actionType != other.actionType) {
            return actionType < other.actionType;
        }
        if (exposureTime != other.exposureTime) {
            return exposureTime < other.exposureTime;
        } else {
            return gainLevel < other.gainLevel;
        }
    }
    bool operator==(const ExposureManualValue &other) const
    {
        return actionType == other.actionType && exposureTime == other.exposureTime && gainLevel == other.gainLevel;
    }
    bool operator!=(const ExposureManualValue &other) const
    {
        return actionType != other.actionType || exposureTime != other.exposureTime || gainLevel != other.gainLevel;
    }
};
Q_DECLARE_METATYPE(ExposureManualValue)

class ExposureScheduleTime {
public:
    ExposureScheduleTime();
    ExposureScheduleTime(const exposure_schedule_item &item);
    ExposureScheduleTime(const ExposureManualValue &type, int beginMinute, int endMinute);

    static QList<ExposureScheduleTime> fromSecondsHash(ExposureManualValue* secArray);

    bool isValid() const;

    ExposureManualValue type() const;
    int beginMinute() const;
    void setBeginMinute(int minute);
    int endMinute() const;
    void setEndMinute(int minute);
    void getExposureItem(exposure_schedule_item *item) const;

    QMap<ExposureScheduleTime, int> merge(const ExposureScheduleTime &other) const;

    bool operator<(const ExposureScheduleTime &other) const;

private:
    int getMinutesFromTimeString(const char *time);

private:
    ExposureManualValue m_manualValue;
    int m_actionType = 0;
    int m_beginMinute = 0;
    int m_endMinute = 0;
    int m_exposureTime = 0;
    int m_gainLevel = 0;
};

#endif // EXPOSURESCHEDULETIME_H
