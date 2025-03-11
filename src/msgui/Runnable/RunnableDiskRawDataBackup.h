#ifndef RUNNABLEDISKRAWDATABACKUP_H
#define RUNNABLEDISKRAWDATABACKUP_H

#include <QObject>
#include <QRunnable>

class RunnableDiskRawDataBackup : public QRunnable
{
public:
    RunnableDiskRawDataBackup(int port, quint64 fileOffset, const QString &dstPath, QObject *originatingObject, const QString &callBack);
    ~RunnableDiskRawDataBackup() override;

    void run() override;

private:
    int m_port = 0;
    quint64 m_fileOffset = 0;
    QString m_dstPath;

    QObject *m_originatingObject = nullptr;
    QString m_callBack;
};

#endif // RUNNABLEDISKRAWDATABACKUP_H
