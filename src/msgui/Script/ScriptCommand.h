#ifndef SCRIPTCOMMAND_H
#define SCRIPTCOMMAND_H

#include <QEvent>
#include <QList>
#include <QPoint>
#include <QString>

class ScriptCommand {
public:
    enum CommandType {
        CmdNone,
        CmdMouseMove,
        CmdMouseClicked,
        CmdMouseDbClicked
    };

    struct CommandInfo {
        QEvent::Type type = QEvent::None;
        Qt::MouseButton button = Qt::NoButton;
        QPoint point;
        int delay = 0;
    };

    explicit ScriptCommand();

    void append(QEvent::Type type, Qt::MouseButton button, QPoint point, int delay);
    void setIndex(int newIndex);
    void clear();

    void setTimeline(qint64 msec);

    int size() const;
    QString text() const;
    QList<CommandInfo> infos() const;

private:
    int m_index = -1;
    qint64 m_timeline;
    QList<CommandInfo> m_infos;
};

#endif // SCRIPTCOMMAND_H
