#ifndef EDITMANUALMODE_H
#define EDITMANUALMODE_H

#include "BaseShadowDialog.h"

class ManualModeSettings;

namespace Ui {
class EditManualMode;
}

class EditManualMode : public BaseShadowDialog {
    Q_OBJECT
public:
    enum ModeType {
        ModeAdd,
        ModeEdit
    };

    explicit EditManualMode(QWidget *parent);
    ~EditManualMode();

    int exposureTime() const;
    int gainLevel() const;
    void setExposureIndex(int index);
    void setGainLevelValue(int value);
    void setEditIndex(int index);
    void setExposureCtrl(int type);
    void setMode(ModeType type);

private slots:
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

private:
    void slotTranslateUi();
    QString exposureTimeString(int index);

private:
    Ui::EditManualMode *ui;

    ManualModeSettings *m_manualMode = nullptr;
    int m_editIndex = -1;
    int m_exposureCtrl = -1;
};

#endif // EDITMANUALMODE_H
