#include "MsSdkVersion.h"
#include <QRegExp>
#include <QtDebug>

MsSdkVersion::MsSdkVersion(const QString &strVersion) :
    m_strVersion(strVersion)
{
    QRegExp rx("(\\d+)\\.(\\d+)\\.(\\d+)");
    if (rx.indexIn(strVersion) != -1)
    {
        m_majorVersion = rx.cap(1).toInt();
        m_minorVersion = rx.cap(2).toInt();
        m_revisionNumber = rx.cap(3).toInt();

        m_isVaild = true;
    }
    else
    {
        //qWarning() << QString("MsSdkVersion, invalid version: %1").arg(strVersion);
    }
}

QString MsSdkVersion::versionString() const
{
    return m_strVersion;
}

int MsSdkVersion::majorVersion() const
{
    return m_majorVersion;
}

int MsSdkVersion::minorVersion() const
{
    return m_minorVersion;
}

int MsSdkVersion::revisionNumber() const
{
    return m_revisionNumber;
}

bool MsSdkVersion::operator<(const MsSdkVersion &other) const
{
    if (m_majorVersion == other.majorVersion())
    {
        if (m_minorVersion == other.minorVersion())
        {
            return m_revisionNumber < other.revisionNumber();
        }
        else
        {
            return m_minorVersion < other.minorVersion();
        }
    }
    else
    {
        return m_majorVersion < other.majorVersion();
    }
}

bool MsSdkVersion::operator<=(const MsSdkVersion &other) const
{
    if (m_majorVersion == other.majorVersion())
    {
        if (m_minorVersion == other.minorVersion())
        {
            return m_revisionNumber <= other.revisionNumber();
        }
        else
        {
            return m_minorVersion <= other.minorVersion();
        }
    }
    else
    {
        return m_majorVersion <= other.majorVersion();
    }
}

bool MsSdkVersion::operator>(const MsSdkVersion &other) const
{
    if (m_majorVersion == other.majorVersion())
    {
        if (m_minorVersion == other.minorVersion())
        {
            return m_revisionNumber > other.revisionNumber();
        }
        else
        {
            return m_minorVersion > other.minorVersion();
        }
    }
    else
    {
        return m_majorVersion > other.majorVersion();
    }
}

bool MsSdkVersion::operator>=(const MsSdkVersion &other) const
{
    if (m_majorVersion == other.majorVersion())
    {
        if (m_minorVersion == other.minorVersion())
        {
            return m_revisionNumber >= other.revisionNumber();
        }
        else
        {
            return m_minorVersion >= other.minorVersion();
        }
    }
    else
    {
        return m_majorVersion >= other.majorVersion();
    }
}

bool MsSdkVersion::operator==(const MsSdkVersion &other) const
{
    return m_majorVersion == other.majorVersion() &&
           m_minorVersion == other.minorVersion() &&
           m_revisionNumber == other.revisionNumber();
}
