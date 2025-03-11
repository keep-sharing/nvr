#ifndef SCRIPTDATA_H
#define SCRIPTDATA_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include "ScriptCommand.h"

class QTimer;

#define gScriptData ScriptData::instance()

class ScriptData : public QObject
{
    Q_OBJECT
public:
    explicit ScriptData(QObject *parent = nullptr);
    ~ScriptData() override;
    static ScriptData &instance();

    void clearCommand();
    void appendCommand(ScriptCommand *cmd);

    void startScript();
    void stopScript();

private:
    void mouse_move(int fd, int rel_x, int rel_y);
    void mouse_move(int fd, const QPoint &p, int delay);
    void mouse_press(int fd, int button, int delay);
    void mouse_release(int fd, int button, int delay);

    void mouse_click(int fd, int key);

    bool isRunning();

signals:
    void indexChanged(int index);
    void once();

private slots:
    void onStartScript();
    void onStopScript();

private:
    QThread m_thread;
    QMutex m_mutex;

    int m_fd = -1;

    QList<ScriptCommand> m_cmdList;
    int m_currentIndex = 0;
    ScriptCommand m_currentCmd;

    bool m_isRunning = false;
};

#endif // SCRIPTDATA_H
