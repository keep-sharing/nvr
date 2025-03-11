#ifndef MSDEVICE_3536G_H
#define MSDEVICE_3536G_H

#include "MsDevice.h"

class MsDevice_3536g : public MsDevice
{
    Q_OBJECT
public:
    explicit MsDevice_3536g(QObject *parent = nullptr);

    bool isSupportResolution_4k_60Hz() const override;

signals:

};

#endif // MSDEVICE_3536G_H
