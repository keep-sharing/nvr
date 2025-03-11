#ifndef CORETHREAD_H
#define CORETHREAD_H

#include <QThread>
#include <QTimer>

class CoreThread : public QObject
{
    Q_OBJECT
public:
    explicit CoreThread(QObject *parent = nullptr);

    static CoreThread &instance();

    void startCore();
    void stopCore();
    void prepareQuit();

    void stopThread();

signals:
    void coreStarted();
    void coreStoped();

private slots:
    void onStartCore();
    void onStopCore();
    void onPrepareQuit();

    void onTimerCheck();

private:
    QThread m_thread;
    QTimer *m_timerCheck = nullptr;
};

#endif // CORETHREAD_H
