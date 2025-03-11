#include "RtspInfo.h"
#include "myqt.h"
#include <QDebug>
#include <QRegExp>
RtspInfo::RtspInfo()
{
}

RtspInfo::RtspInfo(const QString &str)
{
    setRtsp(str);
}

void RtspInfo::setRtsp(const QString &str)
{
    m_valid = false;
    m_ip.clear();
    m_path.clear();
    m_port = -1;

    QString rtsp = str;
    int index = rtsp.indexOf("@");
    if (rtsp.contains("@")) {
        rtsp.remove(0, index + 1);
    } else {
        rtsp.remove("rtsp://");
    }
    if (rtsp.isEmpty()) {
        return;
    }

    //
    QRegExp rx("(\\[)([^\\]]*)(\\])");
    QRegExp rxIp("(^[^\\/\\:]*)(\\:?)([^\\/\\:]*)(\\/?)(.*)");
    QRegExp rxDdns("(^ddns\\.milesight\\.com\\/[^\\/]*)(\\/?)(.*)");
    rx.indexIn(rtsp);
    if (!rx.cap(2).isEmpty()) {
        //ipv6
        m_ip = rx.cap(2);
        int index_colon = rtsp.indexOf(":");
        int index_slash = rtsp.indexOf("/");
        if (index_colon != -1) {
            m_port = 554;
        } else {
            m_port = rtsp.mid(index_colon + 1, index_slash - index_colon - 1).toInt();
        }
        if (index_slash != -1) {
            m_path = rtsp.right(rtsp.size() - index_slash - 1);
        }
        //
        m_valid = MyQt::isValidIPv6(m_ip);
    } else {
        //ipv4
        bool ddnsValid = false;
        if (rxDdns.indexIn(rtsp) != -1) {
            m_ip = rxDdns.cap(1);
            m_port = 554;
            m_path = rxDdns.cap(3);
            ddnsValid = true;
        } else {
            rxIp.indexIn(rtsp);
            m_ip = rxIp.cap(1);
            if (rxIp.cap(3).isEmpty()) {
                m_port = 554;
            } else {
                m_port = rxIp.cap(3).toInt();
            }
            m_path = rxIp.cap(5);
        }
        //检查合法性
        m_valid = MyQt::isValidIP(m_ip) || MyQt::isValidDomain(m_ip) || ddnsValid;
    }
    if (!str.contains("@")) {
        QRegExp rx("^rtsp://");
        if (rx.indexIn(str) == -1) {
            m_valid = false;
        }
    }
    m_url = str;
}

bool RtspInfo::isValid() const
{
    return m_valid;
}

const QString &RtspInfo::url() const
{
    return m_url;
}

const QString &RtspInfo::ip() const
{
    return m_ip;
}

void RtspInfo::setIp(const QString &newIp)
{
    m_ip = newIp;
}

const QString &RtspInfo::path() const
{
    return m_path;
}

void RtspInfo::setPath(const QString &newPath)
{
    m_path = newPath;
}

int RtspInfo::port() const
{
    return m_port;
}

void RtspInfo::setPort(int newPort)
{
    m_port = newPort;
}
