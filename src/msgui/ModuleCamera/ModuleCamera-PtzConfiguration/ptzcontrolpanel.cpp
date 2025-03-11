#include "ptzcontrolpanel.h"
#include "ui_ptzcontrolpanel.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "msuser.h"

extern "C" {
#include "ptz_public.h"
}

PtzControlPanel::PtzControlPanel(QWidget *parent)
    : MsWidget(parent)
    , ui(new Ui::PtzControlPanel)
{
    ui->setupUi(this);

    ui->slider_speed->setRange(1, 10);
    connect(ui->slider_speed, SIGNAL(valueChanged(int)), this, SLOT(onSliderSpeedChanged(int)));

    ui->widget_zoom->hide();
    ui->slider_zoom->setRange(0, 100);
    connect(ui->slider_zoom, SIGNAL(valueChanged(int)), this, SLOT(onSliderZoomChanged(int)));
}

PtzControlPanel::~PtzControlPanel()
{
    delete ui;
}

void PtzControlPanel::setCurrentChannel(int channel)
{
    m_channel = channel;
}

int PtzControlPanel::currentChannel() const
{
    return m_channel;
}

void PtzControlPanel::setPresetVisible(bool visible)
{
    ui->widget_preset->setVisible(visible);
    adjustPos();
}

void PtzControlPanel::clearPreset()
{
    ui->widget_preset->clear();
}

void PtzControlPanel::setAutoZoomModel(bool value)
{
    if (value) {
        ui->widget_zoom->show();
        ui->toolButton_zoom_sub->hide();
        ui->label_zoom_2->hide();
        ui->toolButton_zoom_add->hide();
    } else {
        ui->widget_zoom->hide();
        ui->toolButton_zoom_sub->show();
        ui->label_zoom_2->show();
        ui->toolButton_zoom_add->show();
    }
}

void PtzControlPanel::setZoomEnable(bool enable)
{
    ui->widget_zoom->setEnabled(enable);
    ui->toolButton_zoom_sub->setEnabled(enable);
    ui->label_zoom_2->setEnabled(enable);
    ui->toolButton_zoom_add->setEnabled(enable);
}

void PtzControlPanel::showPresetData(const resp_ptz_preset *preset_array, int count)
{
    ui->widget_preset->initializeData(preset_array, count);
}

void PtzControlPanel::setAutoScanChecked(bool checked)
{
    ui->toolButton_autoScan->setChecked(checked);
}

void PtzControlPanel::stopAutoScan()
{
    if (m_autoScanClicked) {
        m_autoScanClicked = false;
    } else {
        if (ui->toolButton_autoScan->isChecked()) {
            ui->toolButton_autoScan->setChecked(false);
            on_toolButton_autoScan_clicked(false);
        }
    }
}

void PtzControlPanel::editSpeedValue(int speed)
{
    qMsDebug() << speed;
    ui->slider_speed->editValue(speed);
}

int PtzControlPanel::speedValue() const
{
    return ui->slider_speed->value();
}

void PtzControlPanel::setZoomValue(int zoom)
{
    ui->slider_zoom->editValue(zoom);
}

int PtzControlPanel::zoomValue() const
{
    return ui->slider_zoom->value();
}

void PtzControlPanel::setFocusEnable(bool enable)
{
    ui->toolButton_focus_sub->setEnabled(enable);
    ui->label_focus->setEnabled(enable);
    ui->toolButton_focus_add->setEnabled(enable);
}

void PtzControlPanel::resizeEvent(QResizeEvent *event)
{
    adjustPos();
    QWidget::resizeEvent(event);
}

void PtzControlPanel::sendPtzControl(int action)
{
    stopAutoScan();

    int speed = ui->slider_speed->value();
    gPtzDataManager->sendPtzControl(action, speed);
}

void PtzControlPanel::adjustPos()
{
    if (ui->widget_preset->isVisible()) {
        QRect rc1 = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignLeft, QSize(width() / 2 - 50, height()), rect());
        ui->widget_panel->setGeometry(rc1);
        QRect rc2 = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignRight, QSize(width() / 2 + 50, height()), rect());
        ui->widget_preset->setGeometry(rc2);
    } else {
        QRect rc1 = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, QSize(width() / 2 - 50, height()), rect());
        ui->widget_panel->setGeometry(rc1);
    }
}

void PtzControlPanel::getIpcDigitposZoom()
{
    sendMessage(REQUEST_FLAG_GET_IPC_DIGITPOS_ZOOM, (void *)&m_channel, sizeof(m_channel));
}

void PtzControlPanel::onLanguageChanged()
{
    ui->toolButton_focus_sub->setToolTip(GET_TEXT("PTZDIALOG/21001", "Focus-"));
    ui->toolButton_focus_add->setToolTip(GET_TEXT("PTZDIALOG/21002", "Focus+"));
    ui->toolButton_iris_sub->setToolTip(GET_TEXT("PTZDIALOG/21003", "Iris-"));
    ui->toolButton_iris_add->setToolTip(GET_TEXT("PTZDIALOG/21004", "Iris+"));
    ui->toolButton_LEFT_UP->setToolTip(GET_TEXT("PTZDIALOG/21006", "Left Up"));
    ui->toolButton_LEFT->setToolTip(GET_TEXT("PTZDIALOG/21007", "Left"));
    ui->toolButton_LEFT_DOWN->setToolTip(GET_TEXT("PTZDIALOG/21008", "Left Down"));
    ui->toolButton_RIGHT_UP->setToolTip(GET_TEXT("PTZDIALOG/21009", "Right Up"));
    ui->toolButton_RIGHT->setToolTip(GET_TEXT("PTZDIALOG/21010", "Right"));
    ui->toolButton_RIGHT_DOWN->setToolTip(GET_TEXT("PTZDIALOG/21011", "Right Down"));
    ui->toolButton_UP->setToolTip(GET_TEXT("PTZDIALOG/21012", "Up"));
    ui->toolButton_DOWN->setToolTip(GET_TEXT("PTZDIALOG/21013", "Down"));
    ui->toolButton_autoScan->setToolTip(GET_TEXT("PTZDIALOG/21014", "Auto Scan"));
    ui->toolButton_zoom_sub->setToolTip(GET_TEXT("PTZDIALOG/21018", "Zoom-"));
    ui->toolButton_zoom_add->setToolTip(GET_TEXT("PTZDIALOG/21019", "Zoom+"));
    ui->label_speed->setToolTip(GET_TEXT("PTZDIALOG/21020", "Speed"));
    ui->label_zoom->setToolTip(GET_TEXT("PTZDIALOG/21024", "Zoom"));
    ui->label_zoom_2->setToolTip(GET_TEXT("PTZDIALOG/21024", "Zoom"));
    ui->label_focus->setToolTip(GET_TEXT("PTZDIALOG/21022", "Focus"));
    ui->label_iris->setToolTip(GET_TEXT("PTZDIALOG/21023", "Iris"));

    ui->widget_preset->onLanguageChanged();
}

void PtzControlPanel::setEnabled(bool enable)
{
    if (!enable) {
        ui->toolButton_autoScan->setChecked(false);
    }
    QWidget::setEnabled(enable);
    ui->toolButton_iris_add->setEnabled(false);
    ui->toolButton_iris_sub->setEnabled(false);
    ui->label_iris->setEnabled(false);
}

void PtzControlPanel::onSliderSpeedChanged(int value)
{
    gPtzDataManager->writePtzSpeed(value);
}

void PtzControlPanel::onSliderZoomChanged(int value)
{
    gPtzDataManager->writePtzZoomPos(value);
}

void PtzControlPanel::on_toolButton_LEFT_pressed()
{
    sendPtzControl(PTZ_LEFT);
}

void PtzControlPanel::on_toolButton_LEFT_released()
{
    sendPtzControl(PTZ_STOP_ALL);
}

void PtzControlPanel::on_toolButton_LEFT_UP_pressed()
{
    sendPtzControl(PTZ_LEFT_UP);
}

void PtzControlPanel::on_toolButton_LEFT_UP_released()
{
    sendPtzControl(PTZ_STOP_ALL);
}

void PtzControlPanel::on_toolButton_UP_pressed()
{
    sendPtzControl(PTZ_UP);
}

void PtzControlPanel::on_toolButton_UP_released()
{
    sendPtzControl(PTZ_STOP_ALL);
}

void PtzControlPanel::on_toolButton_RIGHT_UP_pressed()
{
    sendPtzControl(PTZ_RIGHT_UP);
}

void PtzControlPanel::on_toolButton_RIGHT_UP_released()
{
    sendPtzControl(PTZ_STOP_ALL);
}

void PtzControlPanel::on_toolButton_RIGHT_pressed()
{
    sendPtzControl(PTZ_RIGHT);
}

void PtzControlPanel::on_toolButton_RIGHT_released()
{
    sendPtzControl(PTZ_STOP_ALL);
}

void PtzControlPanel::on_toolButton_RIGHT_DOWN_pressed()
{
    sendPtzControl(PTZ_RIGHT_DOWN);
}

void PtzControlPanel::on_toolButton_RIGHT_DOWN_released()
{
    sendPtzControl(PTZ_STOP_ALL);
}

void PtzControlPanel::on_toolButton_DOWN_pressed()
{
    sendPtzControl(PTZ_DOWN);
}

void PtzControlPanel::on_toolButton_DOWN_released()
{
    sendPtzControl(PTZ_STOP_ALL);
}

void PtzControlPanel::on_toolButton_LEFT_DOWN_pressed()
{
    sendPtzControl(PTZ_LEFT_DOWN);
}

void PtzControlPanel::on_toolButton_LEFT_DOWN_released()
{
    sendPtzControl(PTZ_STOP_ALL);
}

void PtzControlPanel::on_toolButton_autoScan_clicked(bool checked)
{
    m_autoScanClicked = true;
    if (checked) {
        sendPtzControl(PTZ_AUTO_SCAN);
    } else {
        sendPtzControl(PTZ_STOP_ALL);
    }
}

void PtzControlPanel::on_toolButton_zoom_sub_pressed()
{
    sendPtzControl(PTZ_ZOOM_MINUS);
}

void PtzControlPanel::on_toolButton_zoom_sub_released()
{
    sendPtzControl(PTZ_STOP_ALL);
}

void PtzControlPanel::on_toolButton_zoom_add_pressed()
{
    sendPtzControl(PTZ_ZOOM_PLUS);
}

void PtzControlPanel::on_toolButton_zoom_add_released()
{
    sendPtzControl(PTZ_STOP_ALL);
}

void PtzControlPanel::on_toolButton_focus_sub_pressed()
{
    sendPtzControl(PTZ_FOCUS_MINUS);
}

void PtzControlPanel::on_toolButton_focus_sub_released()
{
    sendPtzControl(PTZ_STOP_ALL);
}

void PtzControlPanel::on_toolButton_focus_add_pressed()
{
    sendPtzControl(PTZ_FOCUS_PLUS);
}

void PtzControlPanel::on_toolButton_focus_add_released()
{
    sendPtzControl(PTZ_STOP_ALL);
}

void PtzControlPanel::on_toolButton_iris_sub_pressed()
{
    sendPtzControl(PTZ_IRIS_MINUS);
}

void PtzControlPanel::on_toolButton_iris_sub_released()
{
    sendPtzControl(PTZ_STOP_ALL);
}

void PtzControlPanel::on_toolButton_iris_add_pressed()
{
    sendPtzControl(PTZ_IRIS_PLUS);
}

void PtzControlPanel::on_toolButton_iris_add_released()
{
    sendPtzControl(PTZ_STOP_ALL);
}
