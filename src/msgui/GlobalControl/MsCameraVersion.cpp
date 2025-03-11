#include "MsCameraVersion.h"
#include <QRegExp>
#include <QtDebug>

QDebug operator <<(QDebug dbg, const MsCameraVersion &version)
{
    dbg.nospace() << QString("MsCameraVersion(string:%1, major: %2, minor: %3, r: %4)")
                         .arg(version.versionString()).arg(version.majorVersion()).arg(version.minorVersion()).arg(version.rVersion());
    return dbg.space();
}

MsCameraVersion::MsCameraVersion()
{

}

MsCameraVersion::MsCameraVersion(const QString &strVersion)
{
    setVersionString(strVersion);
}

MsCameraVersion::MsCameraVersion(int major, int minor, int r)
{
    m_isVaild = true;
    m_majorVersion = major;
    m_minorVersion = minor;
    m_rVersion = r;
}

MsCameraVersion MsCameraVersion::fromChannel(int channel)
{
    Q_UNUSED(channel)
    char version[64];
    MsCameraVersion cameraVersion(version);
    return cameraVersion;
}

void MsCameraVersion::setVersionString(const QString &strVersion)
{
    m_isVaild = false;
    m_strVersion = strVersion;

    QRegExp rx("(\\d+)\\.(\\d+)\\.(\\d+)\\.(\\d+)");
    if (rx.indexIn(strVersion) != -1)
    {
        QString chip = rx.cap(1);
        if (chip.size() == 2)
        {
            m_chipType = chip.at(0).digitValue();
            m_chipVersion = chip.at(1).digitValue();
        }

        m_majorVersion = rx.cap(2).toInt();
        m_oemType = rx.cap(3).toInt();
        m_minorVersion = rx.cap(4).toInt();

        m_isVaild = true;
    }
    QRegExp rx2(".*-r(\\d+)");
    if (rx2.indexIn(strVersion) != -1)
    {
        QString r = rx2.cap(1);
        m_rVersion = r.toInt();
    }
}

QString MsCameraVersion::versionString() const
{
    return m_strVersion;
}

quint32 MsCameraVersion::versionValue() const
{
    quint32 value = 0;
    value |= (m_majorVersion << 24);
    value |= (m_minorVersion << 16);
    value |= (m_rVersion << 8);
    return value;
}

bool MsCameraVersion::isVaild() const
{
    return m_isVaild;
}

bool MsCameraVersion::is45ChipVersion()
{
    return chipType() == 4 && chipVersion() == 5;
}

int MsCameraVersion::chipType() const
{
    return m_chipType;
}

int MsCameraVersion::chipVersion() const
{
    return m_chipVersion;
}

int MsCameraVersion::majorVersion() const
{
    return m_majorVersion;
}

int MsCameraVersion::oemType() const
{
    return m_oemType;
}

int MsCameraVersion::minorVersion() const
{
    return m_minorVersion;
}

int MsCameraVersion::rVersion() const
{
    return m_rVersion;
}

bool MsCameraVersion::operator<(const MsCameraVersion &other) const
{
    return versionValue() < other.versionValue();
}

bool MsCameraVersion::operator<=(const MsCameraVersion &other) const
{
    return versionValue() <= other.versionValue();
}

bool MsCameraVersion::operator>(const MsCameraVersion &other) const
{
    return versionValue() > other.versionValue();
}

bool MsCameraVersion::operator>=(const MsCameraVersion &other) const
{
    return versionValue() >= other.versionValue();
}

bool MsCameraVersion::operator==(const MsCameraVersion &other) const
{
    return versionValue() == other.versionValue();
}
