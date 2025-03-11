#ifndef ALARMKEY_H
#define ALARMKEY_H

#include <QString>
#include <QVariant>

class AlarmKey
{
public:
    enum Type
    {
        NvrAlarm,
        CameraAlarm
    };

    AlarmKey();
    AlarmKey(int alarm_id, const QString &name = QString());
    AlarmKey(int channel, int alarm_id, const QString &name = QString());
    AlarmKey(int channel, int alarm_id, int delay, const QString &name);

    Type type() const;
    int channel() const;
    int alarmId() const;
    int delay() const;
    QString name() const;
    //用于表格显示
    QString numberName() const;
    //用于Event检索
    QString nameForSearch() const;

    bool operator <(const AlarmKey &other) const;
    bool operator ==(const AlarmKey &other) const;
    bool operator !=(const AlarmKey &other) const;

private:
    Type m_type = NvrAlarm;
    int m_channel = -1;
    int m_alarm_id = -1;
    int m_delay = 0;
    QString m_name;
};

Q_DECLARE_METATYPE(AlarmKey)

#endif // ALARMKEY_H
