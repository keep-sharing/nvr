#include "TargetInfo.h"

TargetInfo::TargetInfo()
{

}

TargetInfo::~TargetInfo()
{

}

void TargetInfo::makeImage()
{
    m_smallImage = QImage::fromData(m_smallImageData);
    m_smallImageData.clear();
}

TargetInfo::TYPE TargetInfo::type() const
{
    return m_type;
}

QImage TargetInfo::smallImage() const
{
    return m_smallImage;
}
