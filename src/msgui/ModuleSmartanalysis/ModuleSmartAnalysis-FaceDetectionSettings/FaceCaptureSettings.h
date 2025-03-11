#ifndef FACECAPTURESETTINGS_H
#define FACECAPTURESETTINGS_H

#include "BaseShadowDialog.h"
extern "C" {
#include "msg.h"
}
namespace Ui {
class FaceCaptureSettings;
}

class FaceCaptureSettings : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit FaceCaptureSettings(QWidget *parent = nullptr);
    ~FaceCaptureSettings();
    void initializeData(MsFaceConfig *faceConfig);
    void saveData();
    MsFaceConfig *faceConfig() ;

private:
    void updateVisible(bool enable);

private slots:
    void onLanguageChanged();
    void on_comboBoxCaptureMode_indexSet(int index);
    void on_pushButtonOk_clicked();
    void on_pushButtonCancel_clicked();

    void on_comboBoxSnapshot_activated(int index);
    void on_horizontalSliderCaptureQuality_valueChanged(int value);
    bool lineChecked();

private:
    Ui::FaceCaptureSettings *ui;
    MsFaceConfig m_faceConfig;
};

#endif // FACECAPTURESETTINGS_H
