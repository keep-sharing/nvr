#ifndef MSCAMERAMODEL_H
#define MSCAMERAMODEL_H

/***************************************************************************
 * MS-C4463-FIPB
 *
 * 第一位数字代表 分辨率
 * 2—分辨率1080P【Hi3516A、3516D】
 * 3—分辨率3MP【Hi3516A、3516D】
 * 4—分辨率4MP【Hi3516A、3516D】
 * 5—分辨率5MP【Hi3516A、3516D】
 * 8—分辨率8MP【Hi3519】
 * 9—分辨率12MP【Hi3519】
 *
 * 第二位数字代表 CMOS（Sensor）
 * 1—IMX274
 * 2—IMX274/IMX334（最高支持8MP）
 * 3—IMX326（最高支持6MP）
 * 4—OV4689(最高支持4MP)
 * 6—IMX226（最高支持12MP）
 * 7—IMX123(最高支持3MP)
 * 8—IMX185(最高支持1080P)
 * 9—IMX290/IMX291/IMX307/IMX327(最高支持1080P)
 *
 * 第三位数字代表外壳型号
 * 4—高速球
 * 5—枪型
 * 6—筒型(大筒型，小筒型，PTZ筒型)
 * 7—半球（大半球，防爆小半球，鱼眼，迷你变焦）
 * 8—MiniDome（minidome，非红外minidome，鱼眼）
 *
 * 型号尾部字母
 * F  电动变焦/罗姆电动变焦，可选配
 * I  P光圈，一般都和电动变焦一起FI，可选配
 * P  POE功能
 * W  WiFi功能
 * A  安霸IPC
 * B  海思IPC，没有A，没有B则代表Ti IPC
 * E  旗舰版，①目前大部分AF变焦镜头都有此标识（高速球除外），②枪机的E代表ABF，③注：TI的E指外部调焦。
 * L  LPR
 * H  4MP、5MP@30fps机型
 * R  高帧率60fps【2MP Sensor支持】
 * T  高帧率90fps【IMX290支持】
 * Q  高帧率120fps【IMX290支持】
 ****************************************************************************/

#include <QString>

enum PowerFrequency {
    MS_60Hz = 0,
    MS_50Hz = 1,
    MS_Unknow
};

enum MaximumResolution {
    RES_2M,
    RES_3M,
    RES_4M,
    RES_5M,
    RES_8M,
    RES_12M,
    RES_130M
};

class MsCameraModel {
public:
    MsCameraModel();
    MsCameraModel(const QString &strModel);

    void setModel(const QString &strModel);
    QString model() const;
    void setPowerFrequency(int frequency);
    int powerFrequency() const;
    void setLensDistortCorrect(int value);

    int maxFrameRate(const QString &frameSize);
    int maxFrameRate(int width, int height);

    bool isHighFrameRate_60() const;
    bool isHighFrameRate_90() const;
    bool isHighFrameRate_120() const;

    bool is5MpModel() const;
    bool isSupportReduceVideoStuttering() const;
    bool isType65() const;
    bool isType63() const;

    bool isStartsWith(const QString &s) const;
    bool isMatch(const QRegExp &rx) const;

    int getMaximumResolution();

    int opticalZoom();

private:
    QString m_strModel;

    QString m_strHeader;

    QString m_strMiddle;
    int m_firstDigit = 0;
    int m_secondDigit = 0;
    int m_thirdDigit = 0;

    QString m_strTail;

    PowerFrequency m_powerFrequency = MS_Unknow;
    int m_lensDistortCorrect = 0;
};

#endif // MSCAMERAMODEL_H
