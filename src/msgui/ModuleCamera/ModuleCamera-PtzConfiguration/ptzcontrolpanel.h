#ifndef PTZCONTROLPANEL_H
#define PTZCONTROLPANEL_H

#include "MsWidget.h"
#include "ptzdatamanager.h"

namespace Ui {
class PtzControlPanel;
}

class PtzControlPanel : public MsWidget
{
    Q_OBJECT

public:
    explicit PtzControlPanel(QWidget *parent = nullptr);
    ~PtzControlPanel();

    void setCurrentChannel(int channel);
    int currentChannel() const;

    void setPresetVisible(bool visible);
    void clearPreset();

    //MSHN-6191 QT-Live View：IPC型号中带“F”的，PTZ的控制面板中zoom为拉条形式
    void setAutoZoomModel(bool value);
    //枪机IPC，隐藏zoom
    void setZoomEnable(bool enable);
    //
    void showPresetData(const resp_ptz_preset *preset_array, int count);

    void setAutoScanChecked(bool checked);
    void stopAutoScan();

    void editSpeedValue(int speed);
    int speedValue() const;

    void setZoomValue(int zoom);
    int zoomValue() const;

    void setFocusEnable(bool enable);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void sendPtzControl(int action);
    void adjustPos();

    void getIpcDigitposZoom();

public slots:
    void onLanguageChanged();

    void setEnabled(bool);

private slots:
    void onSliderSpeedChanged(int value);
    void onSliderZoomChanged(int value);

    //
    void on_toolButton_LEFT_pressed();
    void on_toolButton_LEFT_released();
    void on_toolButton_LEFT_UP_pressed();
    void on_toolButton_LEFT_UP_released();
    void on_toolButton_UP_pressed();
    void on_toolButton_UP_released();
    void on_toolButton_RIGHT_UP_pressed();
    void on_toolButton_RIGHT_UP_released();
    void on_toolButton_RIGHT_pressed();
    void on_toolButton_RIGHT_released();
    void on_toolButton_RIGHT_DOWN_pressed();
    void on_toolButton_RIGHT_DOWN_released();
    void on_toolButton_DOWN_pressed();
    void on_toolButton_DOWN_released();
    void on_toolButton_LEFT_DOWN_pressed();
    void on_toolButton_LEFT_DOWN_released();
    void on_toolButton_autoScan_clicked(bool checked);
    void on_toolButton_zoom_sub_pressed();
    void on_toolButton_zoom_sub_released();
    void on_toolButton_zoom_add_pressed();
    void on_toolButton_zoom_add_released();
    void on_toolButton_focus_sub_pressed();
    void on_toolButton_focus_sub_released();
    void on_toolButton_focus_add_pressed();
    void on_toolButton_focus_add_released();
    void on_toolButton_iris_sub_pressed();
    void on_toolButton_iris_sub_released();
    void on_toolButton_iris_add_pressed();
    void on_toolButton_iris_add_released();

private:
    Ui::PtzControlPanel *ui;

    int m_channel = -1;
    bool m_autoScanClicked = false;
};

#endif // PTZCONTROLPANEL_H
