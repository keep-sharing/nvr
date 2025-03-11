#ifndef MANUALMODESETTINGS_H
#define MANUALMODESETTINGS_H

#include "BaseShadowDialog.h"
#include "exposurescheduletime.h"
#include "msdb.h"

class MessageReceive;
class MsWaitting;

struct exposure_schedule;

namespace Ui {
class ManualModeSettings;
}

class ManualModeSettings : public BaseShadowDialog {
    Q_OBJECT

    enum TableColumn {
        ColumnCheck,
        ColumnModeIndex,
        ColumnExposureTime,
        ColumnGainLevel,
        ColumnEdit,
        ColumnDelete
    };

    enum ModeType {
        ModeFromIpc,
        ModeFromNvr,
        ModeFromAll
    };

public:
    struct exposureType {
        struct exposure exposure;
        exposureType(){}
        exposureType (int exposureTime, int gainLevel) {
            exposure.exposureTime = exposureTime;
            exposure.gainLevel = gainLevel;
        }
        bool operator==(const exposureType &other) const
        {
            return exposure.exposureTime== other.exposure.exposureTime && exposure.gainLevel == other.exposure.gainLevel;
        }
        bool operator<(const exposureType &other) const
        {
            if (exposure.exposureTime != other.exposure.exposureTime) {
                return exposure.exposureTime < other.exposure.exposureTime;
            } else {
                return exposure.gainLevel < other.exposure.gainLevel;
            }
        }
    };
    explicit ManualModeSettings(QWidget *parent = nullptr);
    ~ManualModeSettings();

    void initializeData(int channel);

    bool isModeExist(int exposure_time, int gain_level, int edit_index = -1) const;

    void dealMessage(MessageReceive *message);
    void setExposureCtrl(int type);

    void clearCache();
    void apply();

    void setExposureShce(exposure_schedule *exposureShce);
    exposure_schedule *exposureShce() const;
    int channel() const;
    QList<ExposureManualValue> getManualModeList();

protected:
    void resizeEvent(QResizeEvent *) override;
    void closeEvent(QCloseEvent *) override;

private:
    void ON_RESPONSE_FLAG_GET_EXPOSURE_SCHE(MessageReceive *message);

    void updateTable();
    QString exposureTimeString(int index) const;

    void editSchedule(int exposure_time, int gain_level, int exposure_time_new, int gain_level_new);
    void clearSchedule(int exposure_time, int gain_level);

    bool saveData();

signals:
    void settingFinish();
private slots:
    void onLanguageChanged() override;

    void onTableItemClicked(int row, int column);
    void onTableItemDoubleClicked(int row, int column);

    void on_pushButton_add_clicked();
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::ManualModeSettings *ui;
    MsWaitting *m_waitting;

    int m_channel = 0;

    int m_exposureCtrl = -1;
    exposure_schedule *m_exposureShce = nullptr;
    QList<exposureType> m_exposureList;

    exposure_schedule *m_exposureShceCache = nullptr;
    QList<exposureType> m_exposureListCache;
};

#endif // MANUALMODESETTINGS_H
