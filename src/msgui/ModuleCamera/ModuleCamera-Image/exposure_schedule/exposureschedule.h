#ifndef EXPOSURESCHEDULE_H
#define EXPOSURESCHEDULE_H

#include "BaseShadowDialog.h"
#include "exposurescheduledraw.h"
#include "exposurescheduleedit.h"

class MessageReceive;

struct exposure_schedule;

namespace Ui {
class ExposureSchedule;
}

class ExposureSchedule : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit ExposureSchedule(QWidget *parent = 0);
    ~ExposureSchedule();

    void setSchedule(exposure_schedule_day *schedule_day_array);
    void getSchedule(exposure_schedule_day *schedule_day_array);
    void showAction(int channel);

    void ON_RESPONSE_FLAG_GET_EXPOSURE_SCHE(MessageReceive *message);

    void apply();

    int getChannel() const;
    exposure_schedule *getExposureShce() const;
    void setExposureShce(exposure_schedule *exposureShce);
    void clearCache();
    void resetCache();
    void setManualMode(QList<ExposureManualValue> manualModeList);

private:
    void initializeManualMode();
signals:
    void scheduleAccept();
private slots:
    void onLanguageChanged();

    void onEditingFinished();

    void onButtonGroupClicked(int index);

    void on_pushButton_default_clicked();
    void on_pushButton_selectAll_clicked();
    void on_pushButton_editTime_clicked();

    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

    void on_pushButton_manualMod_clicked();
    void on_pushButton_autoMode_clicked();
    void on_comboBox_manualModeEvent_activated(int index);

private:
    Ui::ExposureSchedule *ui;

    ExposureScheduleEdit *m_schedultEdit;
    int m_channel;
    int m_mode;
    exposure_schedule *m_exposureShce = nullptr;
    exposure_schedule *m_exposureShceCache = nullptr;

    //合并后的
    QMap<int, ExposureManualValue> m_mapAllManualMode;

    bool m_isModeSetting = false;
};

#endif // EXPOSURESCHEDULE_H
