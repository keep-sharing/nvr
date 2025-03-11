#ifndef RUNNABLEGETDATABASECAMERA_H
#define RUNNABLEGETDATABASECAMERA_H

#include <QObject>
#include <QRunnable>

class RunnableGetDatabaseCamera : public QRunnable {
public:
    RunnableGetDatabaseCamera(QObject *obj, const QString &member, int channel);

    void run() override;

private:
    QObject *m_obj = nullptr;
    QString m_member;
    int m_channel = -1;
};

#endif // RUNNABLEGETDATABASECAMERA_H
