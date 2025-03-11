#ifndef ANPRDETECTIONSETTINGS_H
#define ANPRDETECTIONSETTINGS_H

#include "BaseShadowDialog.h"
#include <QCheckBox>

struct ms_lpr_settings;
struct ms_lpr_wildcards;

namespace Ui {
class AnprDetectionSettings;
}

class AnprDetectionSettings : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit AnprDetectionSettings(QWidget *parent = nullptr);
    ~AnprDetectionSettings();

    void showSettings(int version, ms_lpr_settings *lpr_settings, ms_lpr_wildcards *lpr_wildcards);
    void saveSettings();

signals:
    void needLicensePlateFormatChanged();

  protected:
    void showEvent(QShowEvent *event) override;

private:
    bool needLicensePlateFormat();
    void setNeedLicensePlateFormat(bool);
    void mayDisplayLicensePlateFormatPanel();

private slots:
    void onLanguageChanged() override;

    void on_comboBox_checktime_currentIndexChanged(int index);

    void updateAllCheckState();
    void on_checkBox_all_clicked(bool checked);

    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();
    void on_pushButton_editLicensePlateFormat_clicked();

    void onNeedLicensePlateFormatChanged();

private:
    Ui::AnprDetectionSettings *ui;
    int m_lpr_version = 0;
    ms_lpr_settings *m_lpr_settings = nullptr;
    ms_lpr_wildcards *m_lpr_wildcards = nullptr;
    bool m_needLicensePlateFormat = true;
    QList<QString> m_shieldingList;
    QList<QCheckBox *> m_checkBoxList;
};

#endif // ANPRDETECTIONSETTINGS_H
