#ifndef MSSDKVERSION_H
#define MSSDKVERSION_H

#include <QString>

class MsSdkVersion
{
public:
    MsSdkVersion(const QString &strVersion);

    QString versionString() const;

    int majorVersion() const;
    int minorVersion() const;
    int revisionNumber() const;

    bool operator< (const MsSdkVersion &other) const;
    bool operator<= (const MsSdkVersion &other) const;
    bool operator> (const MsSdkVersion &other) const;
    bool operator>= (const MsSdkVersion &other) const;
    bool operator== (const MsSdkVersion &other) const;

private:
    bool m_isVaild = false;

    //7.0.72
    QString m_strVersion;
    int m_majorVersion = 0;    //主版本号
    int m_minorVersion = 0;    //子版本号
    int m_revisionNumber = 0;  //修正版本号
};

#endif // MSSDKVERSION_H
