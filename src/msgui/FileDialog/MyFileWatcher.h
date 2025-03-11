#ifndef MYFILEWATCHER_H
#define MYFILEWATCHER_H

#include <QObject>
#include <QTimer>
#include <QStringList>
#include <QFileInfo>
#include <QMap>
#include <QDateTime>
#include <QThread>

class DirInfo
{
public:
    DirInfo();
    DirInfo(const QString &dir);

    void setDir(const QString &dir);

    bool operator ==(const DirInfo &other) const;

    QStringList entryList;
    QMap<QString, QDateTime> fileModifiedMap;

private:
};

class MyFileWatcher : public QObject
{
    Q_OBJECT
public:
    explicit MyFileWatcher(QObject *parent = 0);
    ~MyFileWatcher();

    void stopThread();
    void setPath(const QString &path);
    void clearPath();

signals:
    void directoryChanged(const QString &path);

private slots:
    void onThreadStart();
    void onThreadFinish();

    void onSetPath(const QString &path);
    void onClearPath();

    void onFileWatcherTimer();

private:
    QThread m_thread;
    QTimer *m_timer;

    QString m_watchPath;
    DirInfo m_dirInfo;
};

#endif // MYFILEWATCHER_H
