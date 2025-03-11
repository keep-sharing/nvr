#include "ptzcontrol.h"
#include "ui_ptzcontrol.h"
#include "LiveViewTarget.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "PopupContent.h"
#include "SubControl.h"
#include "centralmessage.h"
#include "msuser.h"
#include "ptz3dcontrol.h"
#include "ptzdatamanager.h"
#include <QButtonGroup>
#include <QDesktopWidget>
#include <QMouseEvent>

#ifdef MS_FISHEYE_SOFT_DEWARP
#else
#include "FisheyeControl.h"
#endif

PtzControl *PtzControl::s_ptzWidget = nullptr;

PtzControl::PtzControl(QWidget *parent)
    : BasePopup(parent)
    , ui(new Ui::PtzControl)
{
    ui->setupUi(this);
    s_ptzWidget = this;

    setTitleWidget(ui->label_title);

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->addButton(ui->pushButton_preset, 0);
    m_buttonGroup->addButton(ui->pushButton_path, 1);
    m_buttonGroup->addButton(ui->pushButton_pattern, 2);
    connect(m_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onButtonGroupClicked(int)));

    //
    ui->comboBoxBundleStream->clear();
    ui->comboBoxBundleStream->addItem(GET_TEXT("LIVEVIEW/20001", "Channel %1").arg(1), 0);
    ui->comboBoxBundleStream->addItem(GET_TEXT("LIVEVIEW/20001", "Channel %1").arg(2), 1);
    ui->comboBoxBundleStream->addItem(GET_TEXT("LIVEVIEW/20001", "Channel %1").arg(3), 2);
    ui->comboBoxBundleStream->addItem(GET_TEXT("LIVEVIEW/20001", "Channel %1").arg(4), 3);

    //
    m_waitting = new MsWaitting(this);
    m_waitting->setWindowModality(Qt::ApplicationModal);
    m_waitting->setAttribute(Qt::WA_ShowWithoutActivating);
    m_waitting->setCustomPos(true);

    //
    connect(ui->widget_preset, SIGNAL(sendPresetControl(int, int, QString)), ui->widget_control, SLOT(onSendPresetControl(int, int, QString)));
    connect(ui->widget_pattern, SIGNAL(sendPatternControl(int, int)), ui->widget_control, SLOT(onSendPatternControl(int, int)));

    //
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();

    //
    ui->stackedWidget->setCurrentIndex(0);
    resize(350, 800);
}

PtzControl::~PtzControl()
{
    qDebug() << "PtzControl::~PtzControl()";
    s_ptzWidget = nullptr;
    delete ui;
}

PtzControl *PtzControl::instance()
{
    return s_ptzWidget;
}

QPoint PtzControl::calculatePos()
{
    QRect screenRect = SubControl::instance()->logicalMainScreenGeometry();
    QPoint p(m_videoRect.left() - width(), m_videoRect.top());
    if (LiveViewTarget::instance()->isTargetEnable()) {
        if (p.x() < screenRect.left() + screenRect.width() / 4.0) {
            p.setX(m_videoRect.right());
        }
    }
    if (p.x() < screenRect.left()) {
        p.setX(m_videoRect.right());
    }
    if (p.x() + width() > screenRect.right()) {
        if (LiveViewTarget::instance()->isTargetEnable()) {
            p.setX(screenRect.left() + screenRect.width() / 4.0);
        } else {
            p.setX(screenRect.left());
        }
    }
    if (p.y() + height() > screenRect.bottom()) {
        p.setY(screenRect.bottom() - height());
    }
    return p;
}

void PtzControl::closePopup(CloseType type)
{
    if (m_popupEventLoop.isRunning()) {
        if (ui->pushButton_preset->isEnabled()) {
            ui->pushButton_preset->setChecked(true);
            ui->stackedWidget->setCurrentIndex(0);
        }
        m_popupEventLoop.exit(type);
    }
}

void PtzControl::escapePressed()
{
    on_pushButton_close_clicked();
}

void PtzControl::showWait(const QRect &videoRect, const PtzControl::PTZ_MODE &mode)
{
    m_mode = mode;
    m_videoRect = videoRect;

    m_waitting->moveToCenter(videoRect);
    //m_waitting->//showWait();
}

void PtzControl::closeWait()
{
    //m_waitting->//closeWait();
}

int PtzControl::waitForGetPTZSupport(int channel, const QRect &videoRect, const PTZ_MODE &mode)
{
    setChannel(channel);
    m_mode = mode;
    m_videoRect = videoRect;

    m_waitting->moveToCenter(videoRect);
    //m_waitting->//showWait();

    int result = STATE_NONSUPPORT;

    gPtzDataManager->beginGetData(channel);
    gPtzDataManager->waitForGetCameraModelType();
    if (qMsNvr->isFisheye(channel)) {
        m_mode = MODE_FISHEYE;
        result = gPtzDataManager->waitForGetFisheyeInfo();
        if (result == STATE_NONSUPPORT) {
            //m_waitting->//closeWait();
            return result;
        }
    }
    gPtzDataManager->waitForGetCameraModelInfo();
    gPtzDataManager->waitForGetPtzSupport();

    int support = -1;
    bool isMsCamera = qMsNvr->isMsCamera(channel);
    if (isMsCamera) {
        support = gPtzDataManager->ptzSupport();
    } else {
        support = PTZ_SUPPORT_ALL;
    }
    if (!(support & PTZ_SUPPORT_YES)) {
        setPtzEnable(false);
        ui->pushButton_lensInitialize->setEnabled(true);
        ui->pushButton_autoFocus->setEnabled(true);
        result = STATE_NONSUPPORT;
    } else {
        setPtzEnable(true);
        //
        ui->widget_control->setAutoScanEnabled(true);
        ui->pushButton_lensInitialize->setEnabled(true);
        ui->pushButton_autoFocus->setEnabled(true);
        //
        ui->pushButton_preset->setEnabled(true);
        ui->pushButton_path->setEnabled(true);
        ui->pushButton_pattern->setEnabled(true);
        //
        ui->widget_preset->clear();
        ui->widget_path->clear();
        ui->widget_pattern->clear();
        if (m_mode == MODE_FISHEYE) {
            int bundleStream = ui->comboBoxBundleStream->itemData(0).toInt();
            gPtzDataManager->waitForGetPtzHttpFishInfo(bundleStream);
            //
            ui->widget_control->setAutoScanEnabled(gPtzDataManager->isFisheyeSupportAutoScan());
            ui->pushButton_lensInitialize->setEnabled(false);
            ui->pushButton_autoFocus->setEnabled(false);

            //
            bool isSupportPreset = gPtzDataManager->isFisheyeSupportPreset();
            ui->pushButton_preset->setEnabled(isSupportPreset);
            bool isSupportPath = gPtzDataManager->isFisheyeSupportPath();
            ui->pushButton_path->setEnabled(isSupportPath);
            ui->pushButton_pattern->setEnabled(false);
        } else {
            gPtzDataManager->waitForGetPtzOvfInfo();
        }
        if (gPtzDataManager->hasPtzInfo()) {
            result = dealPtzInfo();
        } else {
            result = STATE_NONSUPPORT;
        }
        if (!isMsCamera) {
            m_mode = MODE_PTZ;
            result = STATE_PTZ_NORMAL;
        }
    }
    if ((support & PTZ_SUPPORT_ZOOM_SLIDER || support & PTZ_SUPPORT_ZOOM_KEY) && result == STATE_NONSUPPORT) {
        result = STATE_PTZ_NORMAL;
    }
    //MSHN-6191 QT-Live View：IPC型号中带“F”的，PTZ的控制面板中zoom为拉条形式
    ui->widget_control->setAutoZoomMode(support & PTZ_SUPPORT_ZOOM_SLIDER);
    //枪机zoom置灰
    ui->widget_control->setZoomEnable(support & PTZ_SUPPORT_ZOOM_SLIDER || support & PTZ_SUPPORT_ZOOM_KEY);
    //PTZ机型均不支持IRIS
    ui->widget_control->setIrisEnable(support & PTZ_SUPPORT_IRIS);
    //【【旧】PTZ：测试设备为普通枪机（MS-C8152-PA），Lens Type选择P-Iris，Iris+/-置灰无法操作】https://www.tapd.cn/21417271/bugtrace/bugs/view?bug_id=1121417271001104365
    ui->widget_control->setFocusEnable(support & PTZ_SUPPORT_FOCUS);
    ui->pushButton_autoFocus->setEnabled(support & PTZ_SUPPORT_AUXFOCUS);
    ui->pushButton_lensInitialize->setEnabled(support & PTZ_SUPPORT_LENSINIT);

    ui->widgetBundleStream->hide();
    if (qMsNvr->isFisheye(channel)) {
        if (gPtzDataManager->isShowBundleStream()) {
            ui->widgetBundleStream->show();
            setBundleFisheyeStream(gPtzDataManager->fisheyeStream());
        }
    }
    //led status
    gPtzDataManager->waitForGetLedParams();
    ms_ipc_led_params_resp led_params = gPtzDataManager->ledParams();
    if (led_params.led_manual == 1) {
        ui->pushButton_light->setEnabled(true);

        gPtzDataManager->waitForGetLedStatus();
        int ledStatus = gPtzDataManager->ledStatus();
        ui->pushButton_light->setChecked(ledStatus == 1);
    } else {
        ui->pushButton_light->setEnabled(false);
        ui->pushButton_light->setChecked(false);
    }
    //zoom&&speed
    int speed = 5;
    if (isMsCamera) {
        gPtzDataManager->waitForGetPtzZoomPos();
        ui->widget_control->setAutoZoomValue(gPtzDataManager->zoomPos());
        gPtzDataManager->waitForGetPtzSpeed();
        speed = gPtzDataManager->ptzSpeed();
    }
    ui->widget_control->setSpeedValue(speed);
    //auto scan
    if (gPtzDataManager->isFisheye()) {
        ui->widget_control->setAutoScanChecked(gPtzDataManager->isFisheyeAutoScan(ui->comboBoxBundleStream->currentData().toInt()));
    } else {
        gPtzDataManager->waitForGetAutoScan();
        ui->widget_control->setAutoScanChecked(gPtzDataManager->autoScan());
    }
    //
    //m_waitting->//closeWait();
    return result;
}

int PtzControl::execPtz()
{
    switch (m_mode) {
    case MODE_PTZ:
    case MODE_FISHEYE:
        PopupContent::instance()->setPopupWidget(this);
        PopupContent::instance()->showPopupWidget();
        break;
    case MODE_PTZ3D:
        break;
    }

    int result = m_popupEventLoop.exec();
    return result;
}

void PtzControl::exitPtz(PtzExitReason reason)
{
    if (m_popupEventLoop.isRunning()) {
        m_popupEventLoop.exit(reason);
    }
}

void PtzControl::setPtzMode(PtzControl::PTZ_MODE mode)
{
    m_mode = mode;
    switch (mode) {
    case MODE_PTZ:
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        ui->pushButton_pattern->setEnabled(true);
        break;
    case MODE_PTZ3D:
        setWindowFlags(Qt::Widget);
        raise();
        show();
        break;
    case MODE_FISHEYE:
        ui->pushButton_pattern->setEnabled(false);
        break;
    }
}

PtzControl::PTZ_MODE PtzControl::ptzMode()
{
    return m_mode;
}

bool PtzControl::isFisheye() const
{
    return qMsNvr->isFisheye(m_channel);
}

void PtzControl::setChannel(int channel)
{
    m_channel = channel;
    ui->widget_control->setChannel(m_channel);
}

int PtzControl::currentChannel()
{
    return m_channel;
}

void PtzControl::setBundleFisheyeStream(int stream)
{
    ui->comboBoxBundleStream->setCurrentIndexFromData(stream);
    on_comboBoxBundleStream_activated(ui->comboBoxBundleStream->currentIndex());
}

PathWidget *PtzControl::pathWidget()
{
    return ui->widget_path;
}

PatternWidget *PtzControl::patternWidget()
{
    return ui->widget_pattern;
}

void PtzControl::sendPtzControl(int action, int Hrate, int Vrate)
{
    ui->widget_control->onSendPtzControl(action, Hrate, Vrate);
}

void PtzControl::sendPresetControl(int action, int index)
{
    QString name = QString("Preset %1").arg(index + 1);
    ui->widget_control->onSendPresetControl(action, index, name);
}

void PtzControl::sendPatrolControl(int action, int index)
{
    ui->widget_path->sendPatrolControl(action, index + 1);
}

void PtzControl::sendPatternControl(int action, int index)
{
    ui->widget_control->onSendPatternControl(action, index);
}

void PtzControl::sendManualControl(int channel, PTZ_CONTROL_CMD action)
{
    ms_ipc_ptz_control ptz_control;
    memset(&ptz_control, 0, sizeof(ms_ipc_ptz_control));
    ptz_control.chnid = channel;
    ptz_control.ptzCmd = action;
    qDebug() << QString("REQUEST_FLAG_SET_IPC_PTZ_CONTROL, channel: %1, manual: %2").arg(channel).arg(action);
    sendMessage(REQUEST_FLAG_SET_IPC_PTZ_CONTROL, &ptz_control, sizeof(ms_ipc_ptz_control));
}

void PtzControl::mousePressEvent(QMouseEvent *event)
{
    event->accept();
}

void PtzControl::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();
}

void PtzControl::mouseDoubleClickEvent(QMouseEvent *event)
{
    event->accept();
}

void PtzControl::mouseMoveEvent(QMouseEvent *event)
{
    event->accept();
}

void PtzControl::wheelEvent(QWheelEvent *event)
{
    event->accept();
}

void PtzControl::hideEvent(QHideEvent *event)
{
    if (ui->pushButton_preset->isEnabled()) {
        ui->pushButton_preset->setChecked(true);
        ui->stackedWidget->setCurrentIndex(0);
    }
    QWidget::hideEvent(event);
}

int PtzControl::dealPtzInfo()
{
    if (gPtzDataManager->isPresetEnable()) {
        resp_ptz_ovf_info *ptz_ovf_info = gPtzDataManager->ptzOvfInfo();
        ui->widget_preset->initializeData(ptz_ovf_info->preset, gPtzDataManager->maxPresetCount(m_channel));
        ui->widget_path->initializeData(m_channel, ptz_ovf_info->tour);
        ui->widget_pattern->initializeData(ptz_ovf_info->pattern);
    } else {
        ui->widget_preset->clear();
        ui->widget_path->clear();
        ui->widget_pattern->clear();
    }

    //Fisheye Mode - 1P, 2P模式下，不支持preset & path
    if (gPtzDataManager->isFisheye() && gPtzDataManager->isBundleFisheye()) {
        int mode = gPtzDataManager->fisheyeDisplayMode();
        switch (mode) {
        case MsQt::FISHEYE_DISPLAY_1P:
        case MsQt::FISHEYE_DISPLAY_2P:
            ui->pushButton_preset->setEnabled(false);
            ui->pushButton_path->setEnabled(false);
            ui->widget_preset->clear();
            ui->widget_path->clear();
            break;
        default:
            ui->pushButton_preset->setEnabled(true);
            ui->pushButton_path->setEnabled(true);
            break;
        }
    }

    //
    if (PTZ3DControl::isSupportPTZ3D(QString(gPtzDataManager->cameraModelInfo().model))) {
        m_mode = MODE_PTZ3D;
        return STATE_PTZ_3D;
    } else {
        return STATE_PTZ_NORMAL;
    }
}

void PtzControl::setPtzEnable(bool enable)
{
    ui->widget_control->setEnabled(enable);
    ui->widget_tab->setEnabled(enable);
    ui->stackedWidget->setEnabled(enable);

    if (enable) {
        //某些不支持preset的机型，PTZ的控制面板中preset ，path，pattern图标需置灰
        if (!gPtzDataManager->isPresetEnable()) {
            ui->widget_tab->setEnabled(false);
            ui->stackedWidget->setEnabled(false);
        }
    } else {
        ui->widget_preset->clear();
        ui->widget_path->clear();
        ui->widget_pattern->clear();
    }
}

void PtzControl::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("LIVEVIEW/20021", "PTZ"));
    ui->pushButton_close->setText(GET_TEXT("PTZDIALOG/21005", "Close"));

    ui->comboBoxBundleStream->setItemText(0, GET_TEXT("LIVEVIEW/20001", "Channel %1").arg(1));
    ui->comboBoxBundleStream->setItemText(1, GET_TEXT("LIVEVIEW/20001", "Channel %1").arg(2));
    ui->comboBoxBundleStream->setItemText(2, GET_TEXT("LIVEVIEW/20001", "Channel %1").arg(3));
    ui->comboBoxBundleStream->setItemText(3, GET_TEXT("LIVEVIEW/20001", "Channel %1").arg(4));

    ui->pushButton_light->setToolTip(GET_TEXT("PTZDIALOG/21025", "Lighting for 30s"));
    ui->pushButton_lensInitialize->setToolTip(GET_TEXT("PTZDIALOG/21026", "Lens Initialization"));
    ui->pushButton_autoFocus->setToolTip(GET_TEXT("PTZDIALOG/21027", "Auxiliary Focus"));

    ui->pushButton_preset->setToolTip(GET_TEXT("PTZDIALOG/21015", "Preset"));
    ui->pushButton_path->setToolTip(GET_TEXT("PTZDIALOG/21016", "Patrol"));
    ui->pushButton_pattern->setToolTip(GET_TEXT("PTZDIALOG/21017", "Pattern"));

    ui->widget_control->onLanguageChanged();
}

void PtzControl::onButtonGroupClicked(int index)
{
    switch (index) {
    case 0:
    case 1:
    case 2:
        ui->stackedWidget->setCurrentIndex(index);
        break;
    default:
        break;
    }
}

void PtzControl::onTimerCloseWhiteLight()
{
    for (auto iter = m_ledTimerMap.begin(); iter != m_ledTimerMap.end();) {
        int channel = iter.key();
        QTimer *timer = iter.value();
        if (!timer->isActive()) {
            sendManualControl(channel, PTZ_LEN_OFF);
            if (isVisible() && channel == m_channel) {
                ui->pushButton_light->setChecked(false);
            }
            iter = m_ledTimerMap.erase(iter);
        } else {
            iter++;
        }
    }
}

void PtzControl::on_comboBoxBundleStream_activated(int index)
{
    int bundleStream = ui->comboBoxBundleStream->itemData(index).toInt();
    //bundle O视图不支持ptz
    if (gPtzDataManager->fisheyeDisplayMode() == MsQt::FISHEYE_DISPLAY_1O3R && bundleStream == 0) {
        ui->widget_control->setEnabled(false);
        ui->widget_tab->setEnabled(false);
        ui->stackedWidget->setEnabled(false);
    } else {
        ui->widget_control->setEnabled(true);
        ui->widget_tab->setEnabled(true);
        ui->stackedWidget->setEnabled(true);
        //
        //m_waitting->//showWait();
        gPtzDataManager->waitForGetPtzHttpFishInfo(bundleStream);
        if (gPtzDataManager->hasPtzInfo()) {
            dealPtzInfo();
        }
        //m_waitting->//closeWait();
        //
        ui->widget_control->setAutoScanEnabled(gPtzDataManager->isFisheyeSupportAutoScan());
        ui->pushButton_lensInitialize->setEnabled(false);
        ui->pushButton_autoFocus->setEnabled(false);
        //
        bool isSupportPreset = gPtzDataManager->isFisheyeSupportPreset();
        ui->pushButton_preset->setEnabled(isSupportPreset);
        if (!isSupportPreset) {
            ui->widget_preset->clear();
        }
        bool isSupportPath = gPtzDataManager->isFisheyeSupportPath();
        ui->pushButton_path->setEnabled(isSupportPath);
        if (!isSupportPath) {
            ui->widget_path->clear();
        }
        ui->pushButton_pattern->setEnabled(false);
        ui->widget_pattern->clear();
    }
    //
    ui->widget_control->setAutoScanChecked(gPtzDataManager->isFisheyeAutoScan(bundleStream));
}

void PtzControl::on_pushButton_light_clicked(bool checked)
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_PTZCONTROL)) {
        ui->pushButton_light->setAttribute(Qt::WA_UnderMouse, false);
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    if (checked) {
        sendManualControl(m_channel, PTZ_LEN_ON);

        QTimer *timer = nullptr;
        if (m_ledTimerMap.contains(m_channel)) {
            timer = m_ledTimerMap.value(m_channel);
        } else {
            timer = new QTimer(this);
            timer->setSingleShot(true);
            connect(timer, SIGNAL(timeout()), this, SLOT(onTimerCloseWhiteLight()));
        }
        timer->start(30 * 1000);
        m_ledTimerMap.insert(m_channel, timer);
    } else {
        sendManualControl(m_channel, PTZ_LEN_OFF);

        if (m_ledTimerMap.contains(m_channel)) {
            QTimer *timer = m_ledTimerMap.value(m_channel);
            timer->deleteLater();
            m_ledTimerMap.remove(m_channel);
        }
    }
}

void PtzControl::on_pushButton_lensInitialize_clicked()
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_PTZCONTROL)) {
        ui->pushButton_lensInitialize->setAttribute(Qt::WA_UnderMouse, false);
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    sendManualControl(m_channel, LENS_INIT);
}

void PtzControl::on_pushButton_autoFocus_clicked()
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_PTZCONTROL)) {
        ui->pushButton_autoFocus->setAttribute(Qt::WA_UnderMouse, false);
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }
    sendManualControl(m_channel, AUX_FOCUS);
}

void PtzControl::on_pushButton_close_clicked()
{
    switch (m_mode) {
    case MODE_PTZ:
    case MODE_FISHEYE:
        PopupContent::instance()->closePopupWidget(BasePopup::CloseNormal);
        break;
    case MODE_PTZ3D:
        close();
        break;
    }

    ui->pushButton_close->clearFocus();
}
