#ifndef MYCMDPROCESS_H
#define MYCMDPROCESS_H
#include <QObject>
#include <QString>

extern "C"
{

}

class QProcess;
class QTimer;
class MyCmdProcess:public QObject
{
    Q_OBJECT
public:
    MyCmdProcess();

private  slots:
    void resultValue(QString message);
    void cmdStop();
    void readData();
    void cmdFinish(int exitCode);
    void requestTime();
private:
    bool checkIP(QString address);

signals:
    QString cmdResult(QString cmdResult);
    QString cmdText(QString cmdText);
private:
    QString m_cmdResult;
    QProcess *m_process = nullptr;
    bool m_requestTime;
    bool m_hasResult;
    bool m_firstRead;
    QTimer *m_time;
};

#endif // MYCMDPROCESS_H
