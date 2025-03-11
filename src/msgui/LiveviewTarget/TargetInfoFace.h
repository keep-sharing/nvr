#ifndef TARGETINFOFACE_H
#define TARGETINFOFACE_H

#include "TargetInfo.h"

class ToolButtonFace;

class TargetInfoFace : public TargetInfo
{
public:
    explicit TargetInfoFace(int channel, QString time, const MS_FACE_IMAGE *face);
    ~TargetInfoFace() override;

    int channel() const override;
    QString timeString() const override;

    //性别
    FACE_GENDE male() const;
    void updateMaleInfo(ToolButtonFace *button);
    //年龄
    FACE_AGE age() const;
    void updateAgeInfo(ToolButtonFace *button);
    //眼镜
    FACE_GLASSES glasses() const;
    void updateGlassesInfo(ToolButtonFace *button);
    //口罩
    FACE_MASK mask() const;
    void updateMaskInfo(ToolButtonFace *button);
    //帽子
    FACE_CAP cap() const;
    void updateCapInfo(ToolButtonFace *button);

private:
    int m_channel = 0;
    QString m_time;
    MS_FACE_PRIVATE m_info;
};

#endif // TARGETINFOFACE_H
