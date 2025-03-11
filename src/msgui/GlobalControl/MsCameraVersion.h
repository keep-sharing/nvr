#ifndef MSCAMERAVERSION_H
#define MSCAMERAVERSION_H

#include <QString>

class MsCameraVersion
{
public:
    explicit MsCameraVersion();
    explicit MsCameraVersion(const QString &strVersion);
    //43.7.0.72-r1表示7.72-r1版本
    explicit MsCameraVersion(int major, int minor, int r = 0);

    static MsCameraVersion fromChannel(int channel);

    void setVersionString(const QString &strVersion);
    QString versionString() const;
    quint32 versionValue() const;

    bool isVaild() const;

    bool is45ChipVersion();

    int chipType() const;
    int chipVersion() const;
    int majorVersion() const;
    int oemType() const;
    int minorVersion() const;
    int rVersion() const;

    bool operator< (const MsCameraVersion &other) const;
    bool operator<= (const MsCameraVersion &other) const;
    bool operator> (const MsCameraVersion &other) const;
    bool operator>= (const MsCameraVersion &other) const;
    bool operator== (const MsCameraVersion &other) const;

private:
    bool m_isVaild = false;

    //43.7.0.72-a4
    QString m_strVersion;

    /*第一位表示主芯片*/
    //4: HiSi 系列
    int m_chipType = 0;

    /*第二位海思区分平台*/
    //海思0: Hi3516D/A 256M 内存
    //海思1: Hi3516D/A 512M 内存
    //海思2: Hi3519V100 平台
    //海思3: Hi3519V101 平台
    //海思4: Hi3516C 平台
    int m_chipVersion = 0;

    /*第三位表示文件系统的大版本号*/
    //MSHC 大版本号从 7 开始
    int m_majorVersion = 0;

    /*第四位表示 OEM 厂家*/
    //0：Milesight
    //1：中性
    //其他：100 上其他 OEM 信息
    int m_oemType = 0;

    /*后两位数字表示软件小版本号*/
    int m_minorVersion = 0;

    /*临时版本*/
    int m_rVersion = 0;
};

QDebug operator <<(QDebug dbg, const MsCameraVersion &version);

#endif // MSCAMERAVERSION_H
