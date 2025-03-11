#ifndef TABPTZAUTOTRACKING_H
#define TABPTZAUTOTRACKING_H

#include "ptzbasepage.h"
#include <QEventLoop>
#include <QWidget>

extern "C" {
#include "msg.h"
}

class DrawMotion;

namespace Ui {
class PtzAutoTrackingPage;
}

class TabPtzAutoTracking : public PtzBasePage {
    Q_OBJECT

public:
    explicit TabPtzAutoTracking(QWidget *parent = nullptr);
    ~TabPtzAutoTracking();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_AUTO_TRACKING(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_AUTO_TRACKING(MessageReceive *message);

    void ON_RESPONSE_FLAG_GET_VCA_HUMANDETECTION(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_VCA_HUMANDETECTION(MessageReceive *message);

    void ON_RESPONSE_FLAG_GET_IPC_DIGITPOS_ZOOM(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(MessageReceive *message);

    int waitForGetAutoTracking();
    int saveAutoTracking(int channel);

    void setSettingEnable(bool enable);
    void clearSetting();
    void isSupportHumanVehicle();

private slots:
    void onLanguageChanged();

    void onChannelGroupClicked(int channel);

    void on_checkBox_enable_clicked(bool checked);
    void on_comboBox_zoomRatioMode_indexSet(int index);
    void on_comboBox_zoomRatioMode_activated(int index);
    void on_pushButton_setZoomRatio_clicked();
    void on_pushButton_editSchedule_clicked();

    void on_pushButtonSetAll_clicked();
    void on_pushButtonDeleteAll_clicked();

    void on_pushButton_copy_clicked();
    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

    void on_checkBoxHuman_clicked(bool checked);

    void on_checkBoxVehicle_clicked(bool checked);

private:
    Ui::PtzAutoTrackingPage *ui;

    DrawMotion *m_regionDraw = nullptr;

    ms_auto_tracking m_auto_tracking;
    QEventLoop m_eventLoop;
    ms_smart_event_info m_ms_smart_event_info;
    ms_digitpos_zoom_state m_zoom_state;
    resp_image_display m_image_display;

    bool m_isSetZoomClicked = false;
};

#endif // TABPTZAUTOTRACKING_H
