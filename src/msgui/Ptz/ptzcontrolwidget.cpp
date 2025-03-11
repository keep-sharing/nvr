#include "ptzcontrolwidget.h"
#include "ui_ptzcontrolwidget.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "msuser.h"
#include "ptzdatamanager.h"
#include <QHoverEvent>
#ifdef MS_FISHEYE_SOFT_DEWARP
#else
#include "FisheyeControl.h"
#endif

extern "C" {
#include "ptz_public.h"
}

PtzControlWidget::PtzControlWidget(QWidget *parent)
    : MsWidget(parent)
    , ui(new Ui::PtzControlWidget)
{
    ui->setupUi(this);

    ui->slider_speed->setTextColor(Qt::white);
    ui->slider_speed->setRange(1, 10);
    connect(ui->slider_speed, SIGNAL(valueChanged(int)), this, SLOT(onSliderSpeedChanged(int)));

    ui->label_zoom->hide();
    ui->slider_zoom->hide();
    ui->slider_zoom->setTextColor(Qt::white);
    ui->slider_zoom->setRange(0, 100);
    connect(ui->slider_zoom, SIGNAL(valueChanged(int)), this, SLOT(onSliderZoomChanged(int)));

    m_buttonList.append(ui->toolButton_focus_sub);
    m_buttonList.append(ui->toolButton_focus_add);
    m_buttonList.append(ui->toolButton_iris_sub);
    m_buttonList.append(ui->toolButton_iris_add);
    m_buttonList.append(ui->toolButton_LEFT_UP);
    m_buttonList.append(ui->toolButton_LEFT);
    m_buttonList.append(ui->toolButton_LEFT_DOWN);
    m_buttonList.append(ui->toolButton_RIGHT_UP);
    m_buttonList.append(ui->toolButton_RIGHT);
    m_buttonList.append(ui->toolButton_RIGHT_DOWN);
    m_buttonList.append(ui->toolButton_UP);
    m_buttonList.append(ui->toolButton_DOWN);
    m_buttonList.append(ui->toolButton_autoScan);
    m_buttonList.append(ui->toolButton_zoom_sub);
    m_buttonList.append(ui->toolButton_zoom_add);
}

PtzControlWidget::~PtzControlWidget()
{
    delete ui;
}

void PtzControlWidget::setChannel(int channel)
{
    m_channel = channel;
}

void PtzControlWidget::setSpeedValue(int value)
{
    m_speedValue = value;
    ui->slider_speed->setValue(value);
}

void PtzControlWidget::setAutoScanChecked(int checked)
{
    ui->toolButton_autoScan->setChecked(checked);
}

void PtzControlWidget::setAutoScanEnabled(bool enable)
{
    ui->toolButton_autoScan->setEnabled(enable);
}

void PtzControlWidget::setAutoZoomMode(bool isAutoZoom)
{
    //MSHN-6191 QT-Live View：IPC型号中带“F”的，PTZ的控制面板中zoom为拉条形式
    if (isAutoZoom) {
        ui->label_zoom->setEnabled(true);
        ui->slider_zoom->setEnabled(true);
        ui->widget_3->setEnabled(true);
        ui->label_zoom->show();
        ui->slider_zoom->show();
        ui->toolButton_zoom_sub->hide();
        ui->label_zoom_2->hide();
        ui->toolButton_zoom_add->hide();
    } else {
        ui->label_zoom->hide();
        ui->slider_zoom->hide();
        ui->toolButton_zoom_sub->show();
        ui->label_zoom_2->show();
        ui->toolButton_zoom_add->show();
    }
}

void PtzControlWidget::setAutoZoomValue(int value)
{
    m_zoomValue = value;
    ui->slider_zoom->setValue(value);
}

void PtzControlWidget::setZoomEnable(bool enable)
{
    if (enable) {
        ui->widget_3->setEnabled(enable);
    }
    ui->label_zoom->setEnabled(enable);
    ui->slider_zoom->setEnabled(enable);
    ui->toolButton_zoom_sub->setEnabled(enable);
    ui->label_zoom_2->setEnabled(enable);
    ui->toolButton_zoom_add->setEnabled(enable);
}

void PtzControlWidget::setIrisEnable(bool enable)
{
    ui->label_iris->setEnabled(enable);
    ui->toolButton_iris_add->setEnabled(enable);
    ui->toolButton_iris_sub->setEnabled(enable);
}

void PtzControlWidget::setEnabled(bool enable)
{
    ui->widget_1->setEnabled(enable);
    ui->label_zoom->setEnabled(enable);
    ui->slider_zoom->setEnabled(enable);
    ui->label_speed->setEnabled(enable);
    ui->slider_speed->setEnabled(enable);
    ui->widget_3->setEnabled(enable);
}

void PtzControlWidget::setFocusEnable(bool enable)
{
    ui->label_focus->setEnabled(enable);
    ui->toolButton_focus_add->setEnabled(enable);
    ui->toolButton_focus_sub->setEnabled(enable);
}

void PtzControlWidget::onLanguageChanged()
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
}

/**
 * @brief PtzControlWidget::onSendPresetControl
 * @param action
 * @param index, 0-254
 * @param name
 */
void PtzControlWidget::onSendPresetControl(int action, int index, const QString &name)
{
    if (index < 0 || index > 299) {
        return;
    }
    if (action != PTZ_AUTO_SCAN) {
        if (ui->toolButton_autoScan->isChecked()) {
            ui->toolButton_autoScan->setChecked(false);
        }
    }

    struct req_ptz_action ptz_action;
    memset(&ptz_action, 0, sizeof(req_ptz_action));

    ptz_action.chn = m_channel;
    ptz_action.action = action;
    ptz_action.param = index + 1;
    snprintf(ptz_action.name, sizeof(ptz_action.name), "%s", name.toStdString().c_str());

    int speed = ui->slider_speed->value();
    ptz_action.speed.pan = speed;
    ptz_action.speed.tilt = speed;
    ptz_action.speed.zoom = speed;
    ptz_action.speed.focus = speed;

    //fisheye
    if (gPtzDataManager->isFisheye()) {
        ptz_action.type = 1;
        ptz_action.stream_id = gPtzDataManager->fisheyeStream();
        qMsDebug() << QString("channel: %1, action: %2, name: %3, fisheye stream: %4").arg(m_channel).arg(action).arg(name).arg(ptz_action.stream_id);
    } else {
        ptz_action.type = 0;
        qMsDebug() << QString("action: %1, preset id: %2, speed: %3, name: %4").arg(action).arg(ptz_action.param).arg(speed).arg(name);
    }

    sendMessageOnly(REQUEST_FLAG_PTZ_PRESET_ACTION, (void *)&ptz_action, sizeof(req_ptz_action));
}

/**
 * @brief PtzControlWidget::onSendPatternControl
 * @param action
 * @param index, 0-3
 */
void PtzControlWidget::onSendPatternControl(int action, int index)
{
    if (index < 0 || index > 3) {
        return;
    }
    if (action != PTZ_AUTO_SCAN) {
        if (ui->toolButton_autoScan->isChecked()) {
            ui->toolButton_autoScan->setChecked(false);
        }
    }

    struct req_ptz_action ptz_action;
    memset(&ptz_action, 0, sizeof(req_ptz_action));

    ptz_action.chn = m_channel;
    ptz_action.action = action;
    ptz_action.param = index + 1;

    int speed = ui->slider_speed->value();
    ptz_action.speed.pan = speed;
    ptz_action.speed.tilt = speed;
    ptz_action.speed.zoom = speed;
    ptz_action.speed.focus = speed;

    //fisheye
    if (gPtzDataManager->isFisheye()) {
        ptz_action.type = 1;
        ptz_action.stream_id = gPtzDataManager->fisheyeStream();
    } else {
        ptz_action.type = 0;
    }

    sendMessageOnly(REQUEST_FLAG_PTZ_TRACK_ACTION, (void *)&ptz_action, sizeof(req_ptz_action));
}

void PtzControlWidget::onSendPtzControl(int action, int Hrate, int Vrate)
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_PTZCONTROL)) {
        //清除因为弹窗导致按钮状态保持hover
        for (auto iter : m_buttonList) {
            iter->setAttribute(Qt::WA_UnderMouse, false);
            iter->setDown(false);
            iter->setChecked(false);
        }
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }

    if (action != PTZ_AUTO_SCAN) {
        if (ui->toolButton_autoScan->isChecked()) {
            ui->toolButton_autoScan->setChecked(false);
        }
    }

    struct req_ptz_action ptz_action;
    memset(&ptz_action, 0, sizeof(req_ptz_action));

    ptz_action.chn = m_channel;
    ptz_action.action = action;

    int speed = ui->slider_speed->value();
    ptz_action.speed.pan = speed;
    ptz_action.speed.tilt = speed;
    ptz_action.speed.zoom = speed;
    ptz_action.speed.focus = speed;

    if (Hrate) {
        ptz_action.speed.pan = Hrate;
    }
    if (Vrate) {
        ptz_action.speed.tilt = Vrate;
    }
    //qDebug()<<QString("PtzControlWidget::onSendPtzControl channel:[%1], action:[%2], Hrate:[%3], Vrate:[%4], speed:[%5], pan:[%6], tilt:[%7]").arg(ptz_action.chn).arg(action).arg(Hrate).arg(Vrate).arg(speed).arg(ptz_action.speed.pan).arg(ptz_action.speed.tilt);

    //fisheye
    if (gPtzDataManager->isFisheye()) {
        ptz_action.type = 1;
        ptz_action.stream_id = gPtzDataManager->fisheyeStream();
    } else {
        ptz_action.type = 0;
    }

    qDebug() << QString("REQUEST_FLAG_PTZ_ACTION, channel: %1, action: %2, Hrate:[%3], Vrate:[%4]").arg(m_channel).arg(action).arg(Hrate).arg(Vrate);
    sendMessageOnly(REQUEST_FLAG_PTZ_ACTION, (void *)&ptz_action, sizeof(req_ptz_action));
}

void PtzControlWidget::on_toolButton_LEFT_pressed()
{
    onSendPtzControl(PTZ_LEFT);
}

void PtzControlWidget::on_toolButton_LEFT_released()
{
    onSendPtzControl(PTZ_STOP_ALL);
}

void PtzControlWidget::on_toolButton_LEFT_UP_pressed()
{
    onSendPtzControl(PTZ_LEFT_UP);
}

void PtzControlWidget::on_toolButton_LEFT_UP_released()
{
    onSendPtzControl(PTZ_STOP_ALL);
}

void PtzControlWidget::on_toolButton_UP_pressed()
{
    onSendPtzControl(PTZ_UP);
}

void PtzControlWidget::on_toolButton_UP_released()
{
    onSendPtzControl(PTZ_STOP_ALL);
}

void PtzControlWidget::on_toolButton_RIGHT_UP_pressed()
{
    onSendPtzControl(PTZ_RIGHT_UP);
}

void PtzControlWidget::on_toolButton_RIGHT_UP_released()
{
    onSendPtzControl(PTZ_STOP_ALL);
}

void PtzControlWidget::on_toolButton_RIGHT_pressed()
{
    onSendPtzControl(PTZ_RIGHT);
}

void PtzControlWidget::on_toolButton_RIGHT_released()
{
    onSendPtzControl(PTZ_STOP_ALL);
}

void PtzControlWidget::on_toolButton_RIGHT_DOWN_pressed()
{
    onSendPtzControl(PTZ_RIGHT_DOWN);
}

void PtzControlWidget::on_toolButton_RIGHT_DOWN_released()
{
    onSendPtzControl(PTZ_STOP_ALL);
}

void PtzControlWidget::on_toolButton_DOWN_pressed()
{
    onSendPtzControl(PTZ_DOWN);
}

void PtzControlWidget::on_toolButton_DOWN_released()
{
    onSendPtzControl(PTZ_STOP_ALL);
}

void PtzControlWidget::on_toolButton_LEFT_DOWN_pressed()
{
    onSendPtzControl(PTZ_LEFT_DOWN);
}

void PtzControlWidget::on_toolButton_LEFT_DOWN_released()
{
    onSendPtzControl(PTZ_STOP_ALL);
}

void PtzControlWidget::on_toolButton_autoScan_clicked(bool checked)
{
    if (checked) {
        onSendPtzControl(PTZ_AUTO_SCAN);
    } else {
        onSendPtzControl(PTZ_STOP_ALL);
    }
}

void PtzControlWidget::on_toolButton_zoom_sub_pressed()
{
    onSendPtzControl(PTZ_ZOOM_MINUS);
}

void PtzControlWidget::on_toolButton_zoom_sub_released()
{
    onSendPtzControl(PTZ_STOP_ALL);
}

void PtzControlWidget::on_toolButton_zoom_add_pressed()
{
    onSendPtzControl(PTZ_ZOOM_PLUS);
}

void PtzControlWidget::on_toolButton_zoom_add_released()
{
    onSendPtzControl(PTZ_STOP_ALL);
}

void PtzControlWidget::on_toolButton_focus_sub_pressed()
{
    onSendPtzControl(PTZ_FOCUS_MINUS);
}

void PtzControlWidget::on_toolButton_focus_sub_released()
{
    onSendPtzControl(PTZ_STOP_ALL);
}

void PtzControlWidget::on_toolButton_focus_add_pressed()
{
    onSendPtzControl(PTZ_FOCUS_PLUS);
}

void PtzControlWidget::on_toolButton_focus_add_released()
{
    onSendPtzControl(PTZ_STOP_ALL);
}

void PtzControlWidget::on_toolButton_iris_sub_pressed()
{
    onSendPtzControl(PTZ_IRIS_MINUS);
}

void PtzControlWidget::on_toolButton_iris_sub_released()
{
    onSendPtzControl(PTZ_STOP_ALL);
}

void PtzControlWidget::on_toolButton_iris_add_pressed()
{
    onSendPtzControl(PTZ_IRIS_PLUS);
}

void PtzControlWidget::on_toolButton_iris_add_released()
{
    onSendPtzControl(PTZ_STOP_ALL);
}

void PtzControlWidget::onSliderSpeedChanged(int value)
{
    if (!isVisible()) {
        return;
    }
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_PTZCONTROL)) {
        ui->slider_speed->editValue(m_speedValue);
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }

    m_speedValue = value;
    struct req_http_common_param common_param;
    memset(&common_param, 0, sizeof(struct req_http_common_param));
    common_param.chnid = m_channel;
    snprintf(common_param.info.httpname, sizeof(common_param.info.httpname), "%s", "vsetting.html");
    snprintf(common_param.info.key, sizeof(common_param.info.key), "%s", "ptzspeed");
    snprintf(common_param.info.value, sizeof(common_param.info.value), "%d", (value > 0) ? (value - 1) : 1);

    sendMessage(REQUEST_FLAG_SET_IPC_COMMON_PARAM, &common_param, sizeof(struct req_http_common_param));
}

void PtzControlWidget::onSliderZoomChanged(int value)
{
    if (!isVisible()) {
        return;
    }

    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_PTZCONTROL)) {
        ui->slider_zoom->editValue(value);
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }

    m_zoomValue = value;
    struct req_http_common_param common_param;
    memset(&common_param, 0, sizeof(struct req_http_common_param));
    common_param.chnid = m_channel;
    snprintf(common_param.info.httpname, sizeof(common_param.info.httpname), "%s", "vsetting.html");
    snprintf(common_param.info.key, sizeof(common_param.info.key), "%s", "ipncptz=zoompos");
    snprintf(common_param.info.value, sizeof(common_param.info.value), "%d", value);

    sendMessage(REQUEST_FLAG_SET_IPC_COMMON_PARAM, &common_param, sizeof(struct req_http_common_param));
}
