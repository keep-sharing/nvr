#ifndef COMBOBOXMAXFRAMERATE_H
#define COMBOBOXMAXFRAMERATE_H

#include "combobox.h"

extern "C" {
#include "msg.h"
}

class ComboBoxMaxFrameRate : public ComboBox {
    Q_OBJECT
public:
    explicit ComboBoxMaxFrameRate(QWidget *parent = nullptr);

    void setCameraInfo(const CAM_MODEL_INFO &info);
    void setMaxFrameRate(int value);

signals:

public slots:

private:
    CAM_MODEL_INFO m_cameraInfo;
    int m_maxFrameRate = 0;
};

#endif // COMBOBOXMAXFRAMERATE_H
