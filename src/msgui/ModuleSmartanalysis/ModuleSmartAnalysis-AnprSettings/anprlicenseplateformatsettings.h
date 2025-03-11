#pragma once

#include "BaseShadowDialog.h"
#include <QWidget>

class AnprLicensePlateFormatSettingsPrivate;
struct ms_lpr_wildcards;

namespace Ui {
class AnprLicensePlateFormatSettings;
}

class AnprLicensePlateFormatSettings : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit AnprLicensePlateFormatSettings(QWidget *parent = nullptr, ms_lpr_wildcards *wildcards = nullptr);
    ~AnprLicensePlateFormatSettings();
    QObject *d_func();

private slots:
    void on_pushButton_cancel_clicked();
    void on_pushButton_ok_clicked();

private:
    Ui::AnprLicensePlateFormatSettings *ui;
    AnprLicensePlateFormatSettingsPrivate *d;
    friend class AnprLicensePlateFormatSettingsPrivate;
};
