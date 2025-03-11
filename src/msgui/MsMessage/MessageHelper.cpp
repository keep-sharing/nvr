#include "MessageHelper.h"
#include "MyDebug.h"
#include <QFile>
#include <QRegExp>
#include <QTextStream>

MessageHelper::MessageHelper(QObject *parent)
    : QObject(parent)
{
}

MessageHelper &MessageHelper::instance()
{
    static MessageHelper self;
    return self;
}

void MessageHelper::initialize()
{
    QHash<QString, int> hash;

    //
    QFile messageFile("://msg.h");
    if (messageFile.open(QFile::ReadOnly)) {
        //eg: REQUEST_FLAG_SET_DISLAYOUT = 1000,
        QRegExp re(R"((\S+)=(\d+))");
        //eg: RESPONSE_FLAG_SET_CAMAPARAM = MAKE_RESPONSE_VALUE(REQUEST_FLAG_SET_CAMAPARAM),
        QRegExp re1(R"((\S+)=MAKE_RESPONSE_VALUE\((\S+)\))");
        //eg: RESPONSE_FLAG_RET_RECEXCEPT = 15000,
        QRegExp re2(R"((\S+)=(\d+))");
        QTextStream stream(&messageFile);
        QString line;
        do {
            line = stream.readLine();
            line.remove(' ');
            if (line.startsWith("REQUEST_")) {
                if (re.indexIn(line) != -1) {
                    QString text = re.cap(1);
                    QString type = re.cap(2);
                    hash.insert(text, type.toInt());
                    //
                    m_messageTypeHash.insert(type.toInt(), text);
                    m_requestMap.insert(type.toInt(), 0);
                }
            } else if (line.startsWith("RESPONSE_")) {
                if (re1.indexIn(line) != -1) {
                    QString text1 = re1.cap(1);
                    QString text2 = re1.cap(2);
                    int type = hash.value(text2) + 5000;
                    //
                    m_messageTypeHash.insert(type, text1);
                    m_responseMap.insert(type, 0);
                } else if (re2.indexIn(line) != -1) {
                    QString text = re2.cap(1);
                    QString type = re2.cap(2);
                    m_messageTypeHash.insert(type.toInt(), text);
                    m_responseMap.insert(type.toInt(), 0);
                }
            }
        } while (!line.isNull());
    } else {
        qMsWarning() << messageFile.errorString();
    }
}

bool MessageHelper::isNeedResponse(int type) const
{
    return m_requestMap.contains(type) && m_responseMap.contains(type + 5000);
}

QString MessageHelper::messageTypeString(int type) const
{
    if (m_messageTypeHash.contains(type)) {
        return QString("%1(%2)").arg(m_messageTypeHash.value(type)).arg(type);
    } else {
        return "None";
    }
}

void MessageHelper::requestCountAdd(int type)
{
    m_mutexCount.lock();
    m_requestCountMap[type]++;
    m_mutexCount.unlock();
}

void MessageHelper::responseCountAdd(int type)
{
    m_mutexCount.lock();
    m_responseCountMap[type]++;
    m_mutexCount.unlock();
}

void MessageHelper::makeMessageCountReport()
{
    QMap<int, int> requestMap = m_requestMap;
    QMap<int, int> responseMap = m_responseMap;

    m_mutexCount.lock();
    QMap<int, quint32> requestCountMap = m_requestCountMap;
    QMap<int, quint32> responseCountMap = m_responseCountMap;
    m_mutexCount.unlock();

    QFile file("/tmp/MessageReport.csv");
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        stream << "REQUEST," << "RESPONSE," << QString("发送次数,") << QString("接收次数,") << "\n";
        for (auto iter = requestMap.constBegin(); iter != requestMap.constEnd(); ++iter) {
            int request = iter.key();
            int response = request + 5000;
            QString strRequest = messageTypeString(request);
            QString strResponse = messageTypeString(response);
            int requestCount = requestCountMap.value(request, 0);
            int responseCount = responseCountMap.value(response, 0);
            stream << strRequest << ",";
            stream << strResponse << ",";
            stream << requestCount << ",";
            stream << responseCount << ",";
            stream << "\n";
            //
            responseMap.remove(response);
        }
        //没有REQUEST的RESPONSE
        for (auto iter = responseMap.constBegin(); iter != responseMap.constEnd(); ++iter) {
            int response = iter.key();
            QString strResponse = messageTypeString(response);
            int responseCount = responseCountMap.value(response, 0);
            stream << "None"
                   << ",";
            stream << strResponse << ",";
            stream << 0 << ",";
            stream << responseCount << ",";
            stream << "\n";
        }
        qMsWarning() << "make message report done.";
    } else {
        qMsWarning() << file.errorString();
    }
}
