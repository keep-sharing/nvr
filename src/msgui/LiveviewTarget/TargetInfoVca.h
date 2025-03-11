#ifndef TARGETINFOVCA_H
#define TARGETINFOVCA_H

#include "TargetInfo.h"

class TargetInfoVca : public TargetInfo {
public:
    explicit TargetInfoVca(MS_VCA_ALARM *alarm);
    ~TargetInfoVca() override;

    int channel() const override;
    QString timeString() const override;

    QString vcaEventString() const;
    QString vcaObjectString() const;

private:
    MS_VCA_ALARM m_info;
};

#endif // TARGETINFOVCA_H
