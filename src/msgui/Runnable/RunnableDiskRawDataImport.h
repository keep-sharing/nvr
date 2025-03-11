#ifndef RUNNABLEDISKRAWDATAIMPORT_H
#define RUNNABLEDISKRAWDATAIMPORT_H

#include <QObject>
#include <QRunnable>

class RunnableDiskRawDataImport : public QRunnable
{
public:
    RunnableDiskRawDataImport(int port, const QString &srcPath);
    ~RunnableDiskRawDataImport() override;

    void run() override;

private:
    int m_port = 0;
    QString m_srcPath;
};

#endif // RUNNABLEDISKRAWDATAIMPORT_H
