#include "ScriptCommand.h"

ScriptCommand::ScriptCommand()
{
}

void ScriptCommand::append(QEvent::Type type, Qt::MouseButton button, QPoint point, int delay)
{
    CommandInfo info;
    info.type = type;
    info.button = button;
    info.point = point;
    info.delay = delay;
    m_infos.append(info);
}

void ScriptCommand::setIndex(int newIndex)
{
    m_index = newIndex;
}

void ScriptCommand::clear()
{
    m_infos.clear();
}

void ScriptCommand::setTimeline(qint64 msec)
{
    m_timeline = msec;
}

int ScriptCommand::size() const
{
    return m_infos.size();
}

QList<ScriptCommand::CommandInfo> ScriptCommand::infos() const
{
    return m_infos;
}

QString ScriptCommand::text() const
{
    if (m_infos.isEmpty()) {
        return QString("无效指令");
    }

    const CommandInfo &first = m_infos.first();
    QString strButton;
    switch (first.button) {
    case Qt::LeftButton:
        strButton = "左键";
        break;
    case Qt::RightButton:
        strButton = "右键";
        break;
    default:
        break;
    }

    int min = m_timeline / 60000;
    int sec = m_timeline % 60000 / 1000;
    int msec = m_timeline % 1000;
    QString time = QString("%1:%2.%3")
                       .arg(min, 2, 10, QLatin1Char('0'))
                       .arg(sec, 2, 10, QLatin1Char('0'))
                       .arg(msec, 3, 10, QLatin1Char('0'));

    if (m_infos.size() > 2) {
        return QString("%1, %2, %3双击(%4, %5)").arg(m_index + 1, 3, 10, QLatin1Char('0')).arg(time).arg(strButton).arg(first.point.x()).arg(first.point.y());
    } else {
        return QString("%1, %2, %3单击(%4, %5)").arg(m_index + 1, 3, 10, QLatin1Char('0')).arg(time).arg(strButton).arg(first.point.x()).arg(first.point.y());
    }
}
