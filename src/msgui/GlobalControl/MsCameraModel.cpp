#include "MsCameraModel.h"
#include "MyDebug.h"
#include <QRegExp>
#include <QStringList>
#include <QtDebug>

MsCameraModel::MsCameraModel()
{
}

MsCameraModel::MsCameraModel(const QString &strModel)
{
    setModel(strModel);
}

void MsCameraModel::setModel(const QString &strModel)
{
    m_strModel = strModel;

    QRegExp rx("MS-C(.*)-(.*)");
    if (rx.indexIn(m_strModel) != -1) {
        m_strMiddle = rx.cap(1);
        m_strTail = rx.cap(2);

        if (m_strMiddle.size() > 0) {
            m_firstDigit = m_strMiddle.at(0).digitValue();
        }
        if (m_strMiddle.size() > 1) {
            m_secondDigit = m_strMiddle.at(1).digitValue();
        }
        if (m_strMiddle.size() > 2) {
            m_thirdDigit = m_strMiddle.at(2).digitValue();
        }
    } else {
        if (!strModel.isEmpty()) {
            qWarning() << "MsCameraModel::MsCameraModel, unknown camera model:" << strModel;
        }
    }
}

QString MsCameraModel::model() const
{
    return m_strModel;
}

void MsCameraModel::setPowerFrequency(int frequency)
{
    m_powerFrequency = static_cast<PowerFrequency>(frequency);
}

int MsCameraModel::powerFrequency() const
{
    return m_powerFrequency;
}

void MsCameraModel::setLensDistortCorrect(int value)
{
    m_lensDistortCorrect = value;
}

int MsCameraModel::maxFrameRate(const QString &frameSize)
{
    QStringList list = frameSize.split("*");
    if (list.size() == 2) {
        int width = list.at(0).toInt();
        int height = list.at(1).toInt();
        return maxFrameRate(width, height);
    } else {
        //qWarning() << "MsCameraModel::maxFrameRate, unknow string:" << frameSize;
        return 0;
    }
}

int MsCameraModel::maxFrameRate(int width, int height)
{
    int frameRate = 0;
    int size = qRound(width * height / 1024.0 / 1024.0);

    //45平台5MP@20fps
    if (m_strModel.contains(QRegExp("MS-C53..-.*S.*PC"))) {
        if (m_powerFrequency == MS_60Hz) {
            if (size > 3) {
                frameRate = 20;
            } else {
                frameRate = 30;
            }
        } else {
            if (size > 3) {
                frameRate = 20;
            } else {
                frameRate = 25;
            }
        }
        return frameRate;
    }
    //45平台5MP@30fps
    else if (m_strModel.contains(QRegExp("MS-C53..-.*PC"))) {
        if (size < 3) {
            if (m_powerFrequency == MS_60Hz) {
                frameRate = 60;
            } else {
                frameRate = 50;
            }
        } else if (size == 3) {
            frameRate = 45;
        } else if (size > 3) {
            if (m_powerFrequency == MS_60Hz) {
                frameRate = 30;
            } else {
                frameRate = 25;
            }
        }
        return frameRate;
    }
    //45平台8MP@20fps
    //MSHN-8587 兼容MS-C81xx-PC机型，最大帧率为20fps，当前选项最大为15fps，（这个机子升级后为带s的）
    if (m_strModel.contains(QRegExp("MS-C81..-.*S.*PC"))) {
        if (size == 8) {
            frameRate = 20;
            return frameRate;
        }
    }
    //45平台8MP@30fps
    else if (m_strModel.contains(QRegExp("MS-C81..-.*PC"))) {
        if (size == 8) {
            frameRate = 30;
            return frameRate;
        }
    }

    //MSHN-9128 QT：AV300/DV300兼容：MS-C2871-X20TPC机型Video参数设置中电源频率为60HZ时主码流最大帧率为90，当前为60
    if (m_strModel.contains("MS-C2871-X20TPC")) {
        if (m_powerFrequency == MS_60Hz) {
            frameRate = 90;
            return frameRate;
        }
    }

    /**特殊处理情况**/
    //兼容5MP机型1080P以下（含1080P）60fps，3MP时45fps ，3MP以上（不含3MP）；
    //MSHN-7365
    if (is5MpModel()) {
        if (size < 3) {
            if (m_powerFrequency == MS_60Hz) {
                frameRate = 60;
            } else {
                frameRate = 50;
            }
        } else if (size == 3) {
            frameRate = 45;
        } else if (size > 3) {
            if (m_powerFrequency == MS_60Hz) {
                frameRate = 30;
            } else {
                frameRate = 25;
            }
        }
        return frameRate;
    }

    /**特殊处理情况**/
    //4K机型兼容规则：
    //C82机型 按IPC所传分辨率，不进行特殊处理
    if (m_strMiddle.startsWith("82")) {
        return 0;
    }
    //MSHN-8578 QT-Camera：8176-PB机型，分辨率为3440*1936和3840*2160一样，帧率最大值为15fps，【目前最大值为30】
    if (m_strModel.contains(QRegExp("MS-C8176-PB"))) {
        if (width == 3440 && height == 1936) {
            frameRate = 15;
            return frameRate;
        }
        if (width == 3840 && height == 2160) {
            frameRate = 15;
            return frameRate;
        }
    }
    //MSHN-8577 QT-Camera：8176-HPB机型，所有分辨率 ，帧率选项最大值应为25（电源频率50hz）或30（电源频率60hz）
    if (m_strModel.contains(QRegExp("MS-C8176-HPB"))) {
        if (m_powerFrequency == MS_60Hz) {
            frameRate = 30;
        } else {
            frameRate = 25;
        }
        return frameRate;
    }
    //4K系列，全景筒8165不带H机型，不论是矫正还是未矫正模式下，主码流分辨率为6M和8M时，对应帧率最大应为15帧，其他分辨率的为25/30帧
    if (m_strMiddle.startsWith("8165")) {
        if (!m_strTail.contains("H")) {
            if (size >= 6) {
                frameRate = 15;
            } else {
                if (m_powerFrequency == MS_60Hz) {
                    frameRate = 30;
                } else {
                    frameRate = 25;
                }
            }
            return frameRate;
        }
    }
    //C81 带H机型 按IPC所传分辨率 ，不进行特殊处理
    //4K系列，普通机型81xx不带H，主码流分辨率只有8M时，对应帧率才15帧，其他的都是25/30帧
    if (m_strMiddle.startsWith("81")) {
        if (m_strTail.contains("H")) {
            frameRate = 0;
        } else {
            if (size >= 8) {
                frameRate = 15;
            } else {
                if (m_powerFrequency == MS_60Hz) {
                    frameRate = 30;
                } else {
                    frameRate = 25;
                }
            }
        }
        return frameRate;
    }

    /**特殊处理情况**/
    //MS-C5341-X30PB
    //5MP&4MP的最大帧率为20fps
    if (m_strModel.contains("MS-C5341-X30PB")) {
        if (size >= 4) {
            frameRate = 20;
            return frameRate;
        }
    }

    /**特殊处理情况**/
    //5365全景筒型
    //MSHN-7363
    if (m_strModel.contains("5365")) {
        if (m_lensDistortCorrect == 1) {
            if (size >= 3) {
                if (m_powerFrequency == MS_60Hz) {
                    frameRate = 30;
                } else {
                    frameRate = 25;
                }
            } else {
                if (m_powerFrequency == MS_60Hz) {
                    frameRate = 60;
                } else {
                    frameRate = 50;
                }
            }
        } else {
            if (m_powerFrequency == MS_60Hz) {
                frameRate = 30;
            } else {
                frameRate = 25;
            }
        }
        return frameRate;
    }

    /**基本规则**/
    //分辨率选择3MP及以上时，带H的机型，当电源频率为60Hz时，最大帧率为30
    if (m_strTail.contains("H")) {
        if (size >= 3) {
            if (m_powerFrequency == MS_60Hz) {
                frameRate = 30;
            } else {
                frameRate = 25;
            }
        } else {
            if (m_powerFrequency == MS_60Hz) {
                frameRate = 60;
            } else {
                frameRate = 50;
            }
        }
        return frameRate;
    }

    return frameRate;
}

bool MsCameraModel::isHighFrameRate_60() const
{
    return m_strTail.contains("R");
}

bool MsCameraModel::isHighFrameRate_90() const
{
    return m_strTail.contains("T");
}

bool MsCameraModel::isHighFrameRate_120() const
{
    return m_strTail.contains("Q");
}

bool MsCameraModel::is5MpModel() const
{
    if (m_strMiddle.startsWith("5") && m_strTail.contains("H")) {
        return true;
    }

    return false;
}

bool MsCameraModel::isSupportReduceVideoStuttering() const
{
    QRegExp rx("MS-C53.*");
    if (rx.exactMatch(m_strModel)) {
        if (m_strTail.contains("B") && !m_strTail.contains("H")) {
            return true;
        }
    }
    return false;
}

bool MsCameraModel::isType65() const
{
    return m_strMiddle.endsWith("65");
}

bool MsCameraModel::isType63() const
{
    return m_strMiddle.endsWith("63");
}

bool MsCameraModel::isStartsWith(const QString &s) const
{
    return m_strModel.startsWith(s);
}

bool MsCameraModel::isMatch(const QRegExp &rx) const
{
    return rx.exactMatch(m_strModel);
}

int MsCameraModel::getMaximumResolution()
{
    QList<QString> ms_2mpList { "3862", "3863", "3872" /*old chip*/
                                ,
                                "2862", "2863", "2872", "2962", "2963", "2972", "2982", "2973", "2951", "2851" };
    QList<QString> ms_3mpList { "3762", "3763", "3772", "3773", "3751", "3782" };
    QList<QString> ms_4mpList { "4462", "4463", "4472", "4473", "4482", "4451" };
    QList<QString> ms_8mpList { "8662" };
    QList<QString> ms_12mpList { "9662", "9683", "9674" };
    if (ms_2mpList.contains(m_strMiddle)) {
        return RES_2M;
    }
    if (ms_3mpList.contains(m_strMiddle)) {
        return RES_3M;
    }
    if (ms_4mpList.contains(m_strMiddle)) {
        return RES_4M;
    }
    if (ms_8mpList.contains(m_strMiddle)) {
        return RES_8M;
    }
    if (ms_12mpList.contains(m_strMiddle)) {
        return RES_12M;
    }
    int firstNum = QString(m_strMiddle[0]).toInt();
    int maximumResolution = RES_130M;
    switch (firstNum) {
    case 1:
        maximumResolution = RES_130M;
        break;
    case 2:
        maximumResolution = RES_2M;
        break;
    case 3:
        maximumResolution = RES_3M;
        break;
    case 4:
        maximumResolution = RES_4M;
        break;
    case 5:
        maximumResolution = RES_5M;
        break;
    case 9:
        maximumResolution = RES_12M;
        break;
    case 8:
        maximumResolution = RES_8M;
        break;
    }
    return  maximumResolution;
}

int MsCameraModel::opticalZoom()
{
    //高速球 MS-CXXXX-XAAXX, AA代表倍数, 20及以上表示支持
    QRegExp rx1(R"(MS-C.*-X(\d\d).*)");
    if (rx1.exactMatch(m_strModel)) {
        QString value = rx1.cap(1);
        return value.toInt();
    }
    //
    return 0;
}
