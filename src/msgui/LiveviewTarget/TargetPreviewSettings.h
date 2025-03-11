#ifndef TARGETPREVIEWSETTINGS_H
#define TARGETPREVIEWSETTINGS_H

#include "BaseShadowDialog.h"

namespace Ui {
class TargetPreviewSettings;
}

class TargetPreviewSettings : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit TargetPreviewSettings(QWidget *parent = 0);
    ~TargetPreviewSettings();

private slots:
    void on_pushButtonOk_clicked();
    void on_pushButtonCancel_clicked();

private:
    Ui::TargetPreviewSettings *ui;
};

#endif // TARGETPREVIEWSETTINGS_H
