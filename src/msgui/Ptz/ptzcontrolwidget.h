#ifndef PTZCONTROLWIDGET_H
#define PTZCONTROLWIDGET_H

#include "MsWidget.h"
#include <QToolButton>
namespace Ui {
class PtzControlWidget;
}

class PtzControlWidget : public MsWidget {
    Q_OBJECT

public:
    explicit PtzControlWidget(QWidget *parent = 0);
    ~PtzControlWidget();

    void setChannel(int channel);

    void setSpeedValue(int value);
    void setAutoScanChecked(int checked);
    void setAutoScanEnabled(bool enable);

    void setAutoZoomMode(bool isAutoZoom);
    void setAutoZoomValue(int value);
    void setZoomEnable(bool enable);
    void setIrisEnable(bool enable);
    void setEnabled(bool enable);
    void setFocusEnable(bool enable);

public slots:
    void onLanguageChanged();

    void onSendPtzControl(int action, int Hrate = 0, int Vrate = 0);
    void onSendPresetControl(int action, int index, const QString &name);
    void onSendPatternControl(int action, int index);

private slots:
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
    void onSliderSpeedChanged(int value);
    void onSliderZoomChanged(int value);

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
private:
    Ui::PtzControlWidget *ui;

    int m_channel;
    QList<QToolButton *> m_buttonList;
    int m_speedValue;
    int m_zoomValue;
};

#endif // PTZCONTROLWIDGET_H
