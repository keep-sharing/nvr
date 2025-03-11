#ifndef RTSPINFO_H
#define RTSPINFO_H

#include <QString>

class RtspInfo
{
public:
    explicit RtspInfo();
    explicit RtspInfo(const QString &str);

    void setRtsp(const QString &str);
    bool isValid() const;

    const QString &url() const;

    const QString &ip() const;
    void setIp(const QString &newIp);

    const QString &path() const;
    void setPath(const QString &newPath);

    int port() const;
    void setPort(int newPort);

private:
    bool m_valid = false;

    QString m_url;
    QString m_ip;
    QString m_path;
    int m_port = -1;
};

#endif // RTSPINFO_H
