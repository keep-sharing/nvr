#ifndef RUNNABLEPLAYBACKREALTIME_H
#define RUNNABLEPLAYBACKREALTIME_H

#include <QRunnable>

class QObject;

class RunnablePlaybackRealTime : public QRunnable {
public:
    RunnablePlaybackRealTime(QObject *obj);

    void run() override;

private:
    QObject *m_obj = nullptr;
};

#endif // RUNNABLEPLAYBACKREALTIME_H
