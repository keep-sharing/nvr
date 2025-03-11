#include "MsDevice_3536g.h"

MsDevice_3536g::MsDevice_3536g(QObject *parent)
    : MsDevice(parent)
{

}

bool MsDevice_3536g::isSupportResolution_4k_60Hz() const
{
    return true;
}
