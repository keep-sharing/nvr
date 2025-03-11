#include "AlarmKey.h"
#include "MsDevice.h"

AlarmKey::AlarmKey()
{

}

AlarmKey::AlarmKey(int alarm_id, const QString &name)
{
    m_type = NvrAlarm;
    m_alarm_id = alarm_id;
    m_name = name;
}

AlarmKey::AlarmKey(int channel, int alarm_id, const QString &name)
{
    m_type = CameraAlarm;
    m_channel = channel;
    m_alarm_id = alarm_id;
    m_name = name;
}

AlarmKey::AlarmKey(int channel, int alarm_id, int delay, const QString &name)
{
    m_type = CameraAlarm;
    m_channel = channel;
    m_alarm_id = alarm_id;
    m_delay = delay;
    m_name = name;
}

AlarmKey::Type AlarmKey::type() const
{
    return m_type;
}

int AlarmKey::channel() const
{
    return m_channel;
}

int AlarmKey::alarmId() const
{
    return m_alarm_id;
}

int AlarmKey::delay() const
{
    return m_delay;
}

QString AlarmKey::name() const
{
    return m_name;
}

QString AlarmKey::numberName() const
{
    QString text;
    switch (m_type)
    {
    case NvrAlarm:
        text = QString("%1").arg(m_alarm_id + 1);
        break;
    case CameraAlarm:
        text = QString("CH%1_%2").arg(m_channel + 1).arg(m_alarm_id + 1);
        break;
    default:
        break;
    }
    return text;
}

QString AlarmKey::nameForSearch() const
{
    QString text;
    switch (m_type)
    {
    case NvrAlarm:
        text = QString("%1 %2").arg(RECORD_TYPE_ALARM).arg(m_alarm_id + 1);
        break;
    case CameraAlarm:
        text = QString("CH%1_%2").arg(m_channel + 1).arg(m_alarm_id + 1);
        break;
    default:
        break;
    }
    return text;
}

bool AlarmKey::operator <(const AlarmKey &other) const
{
    if (m_type != other.type())
    {
        return m_type < other.type();
    }
    else if (m_channel != other.channel())
    {
        return m_channel < other.channel();
    }
    else
    {
        return m_alarm_id < other.alarmId();
    }
}

bool AlarmKey::operator ==(const AlarmKey &other) const
{
    return m_type == other.type() && m_channel == other.channel() && alarmId() == other.alarmId();
}

bool AlarmKey::operator !=(const AlarmKey &other) const
{
    return m_type != other.type() || m_channel != other.channel() || alarmId() != other.alarmId();
}
