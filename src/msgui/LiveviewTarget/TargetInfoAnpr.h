#ifndef TARGETINFOANPR_H
#define TARGETINFOANPR_H

#include "TargetInfo.h"

class TargetInfoAnpr : public TargetInfo {
public:
    explicit TargetInfoAnpr(lpr_metadata_info *pMsg, lpr_image_info *pImage);
    ~TargetInfoAnpr() override;

    int channel() const override;
    QString timeString() const override;

    QString licenseString() const;
    QString plateTypeString() const;
    QString plateColorString() const;
    QString vehicleTypeString() const;
    QString vehicleBrandString() const;
    QString vehicleColorString() const;
    QString speedString() const;
    QString directionString() const;

    void setAnprTypeString(const QString &type);

private:
    lpr_metadata_info m_info;
};

#endif // TARGETINFOANPR_H
