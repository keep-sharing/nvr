#include "comboboxmaxframerate.h"

ComboBoxMaxFrameRate::ComboBoxMaxFrameRate(QWidget *parent)
    : ComboBox(parent)
{
    memset(&m_cameraInfo, 0, sizeof(CAM_MODEL_INFO));
}

void ComboBoxMaxFrameRate::setCameraInfo(const CAM_MODEL_INFO &info)
{
    memcpy(&m_cameraInfo, &info, sizeof(CAM_MODEL_INFO));
}

void ComboBoxMaxFrameRate::setMaxFrameRate(int value)
{
    m_maxFrameRate = value;
}
