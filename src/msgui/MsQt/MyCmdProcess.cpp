#include "MyCmdProcess.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include <QDebug>
#include <QHostAddress>
#include <QProcess>
#include <QTimer>
#include <csignal>

MyCmdProcess::MyCmdProcess()
{
}

void MyCmdProcess::resultValue(QString message)
{
    m_time = new QTimer(this);
    m_process = new QProcess(this);
    connect(m_time, SIGNAL(timeout()), this, SLOT(requestTime()));
    connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readData()));
    connect(m_process, SIGNAL(finished(int)), this, SLOT(cmdFinish(int)));
    m_process->setReadChannel(QProcess::StandardOutput);
    m_cmdResult = "";
    m_requestTime = false;
    m_hasResult = false;
    m_firstRead = false;
    m_process->start(message);
    if (!m_process->waitForStarted(3000)) {
        cmdStop();
        return;
    }
    m_time->start(4000);
}

void MyCmdProcess::cmdStop()
{
    if (m_process->state() != QProcess::NotRunning) {
        kill(m_process->pid(), SIGINT);
    }
}

void MyCmdProcess::readData()
{
    m_requestTime = true;
    QString strText = m_process->readAllStandardOutput();

    QRegExp rx("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
    if (!m_firstRead && rx.indexIn(strText)) {
        m_firstRead = true;
        if (!checkIP(rx.cap(0))) {
            m_hasResult = true;
            m_process->kill();
            emit cmdResult("Network is unreachable.");
            return;
        }
    }
    rx.setPattern("\\b(ttl)\\b");
    if (rx.indexIn(strText) == -1) {
        m_cmdResult = strText;
        rx.setPattern("Unreachable");
        if (rx.indexIn(strText) != -1) {
            cmdStop();
        }
    } else {
        rx.setPattern("/(([1-9]\\d*\\.\\d*|0\\.\\d*[1-9]\\d*)|([0-9]*))(?=/)");
        if (rx.indexIn(strText) != -1) {
            emit cmdResult(strText);
            m_hasResult = true;
        } else {
            emit cmdText(strText);
        }
    }
}

void MyCmdProcess::cmdFinish(int exitCode)
{
    Q_UNUSED(exitCode);

    m_time->stop();
    if (!m_hasResult) {
        m_requestTime = true;
        emit cmdResult(m_cmdResult);
    }
}

void MyCmdProcess::requestTime()
{
    if (!m_requestTime) {
        cmdStop();
    }
}

bool MyCmdProcess::checkIP(QString address)
{
    int enable = get_param_int(SQLITE_FILE_NAME, PARAM_ACCESS_FILTER_ENABLE, 0);
    if (enable == 0) {
        return true;
    }
    int type = get_param_int(SQLITE_FILE_NAME, PARAM_ACCESS_FILTER_TYPE, 1);
    bool result = false;
    quint32 ipAddress = QHostAddress(address).toIPv4Address();

    struct access_list info[MAX_LIMIT_ADDR];
    int cnt = 0;
    read_access_filter(SQLITE_FILE_NAME, info, &cnt);
    for (int i = 0; i < cnt; i++) {
        if (info[i].type == 1) {
            quint32 tempAddress = QHostAddress(info[i].address).toIPv4Address();
            if (type == 0) {
                if (ipAddress == tempAddress) {
                    return false;
                }
            } else {
                if (ipAddress == tempAddress) {
                    result = true;
                }
            }
        } else if (info[i].type == 2) {
            QStringList strList;
            QString rangAdress = QString::fromLocal8Bit(info[i].address);
            strList = rangAdress.split("-");
            quint32 startAddress = QHostAddress(strList[0]).toIPv4Address();
            quint32 endAddress = QHostAddress(strList[1]).toIPv4Address();
            if (type == 0) {
                if (ipAddress >= startAddress && ipAddress <= endAddress) {
                    return false;
                }
            } else {
                if (ipAddress >= startAddress || ipAddress <= endAddress) {
                    result = true;
                }
            }
        } else if (info[i].type == 0) {
            char tempAddress[32];
            snprintf(tempAddress, sizeof(tempAddress), "%s", address.toStdString().c_str());
        }
    }
    if (type == 1 && !result) {
        return false;
    }
    return true;
}
