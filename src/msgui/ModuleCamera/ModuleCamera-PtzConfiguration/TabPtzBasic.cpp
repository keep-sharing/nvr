#include "TabPtzBasic.h"
#include "ui_TabPtzBasic.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "channelcopydialog.h"
#include "LiveView.h"
#include <qmath.h>

extern "C" {
#include "msg.h"
}

TabPtzBasic::TabPtzBasic(QWidget *parent)
    : PtzBasePage(parent)
    , ui(new Ui::PtzBasicPage)
{
    ui->setupUi(this);

    ui->comboBoxChannel->clear();
    for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
        ui->comboBoxChannel->addItem(QString::number(i + 1), i);
    }

    ui->ptzControlPanel->setPresetVisible(false);

    ui->comboBoxZoomStatus->beginEdit();
    ui->comboBoxZoomStatus->clear();
    ui->comboBoxZoomStatus->addItem(GET_TEXT("PTZBASIC/111023", "2 seconds"), Basic2seconds);
    ui->comboBoxZoomStatus->addItem(GET_TEXT("PTZBASIC/111024", "5 seconds"), Basic5seconds);
    ui->comboBoxZoomStatus->addItem(GET_TEXT("PTZBASIC/111025", "10 seconds"), Basic10seconds);
    ui->comboBoxZoomStatus->addItem(GET_TEXT("PTZBASIC/111021", "Always Open"), BasicAlwaysOpen);
    ui->comboBoxZoomStatus->addItem(GET_TEXT("PTZBASIC/111022", "Always Close"), BasicAlwaysClose);
    ui->comboBoxZoomStatus->endEdit();

    ui->comboBoxPanStatus->beginEdit();
    ui->comboBoxPanStatus->clear();
    ui->comboBoxPanStatus->addItem(GET_TEXT("PTZBASIC/111023", "2 seconds"), Basic2seconds);
    ui->comboBoxPanStatus->addItem(GET_TEXT("PTZBASIC/111024", "5 seconds"), Basic5seconds);
    ui->comboBoxPanStatus->addItem(GET_TEXT("PTZBASIC/111025", "10 seconds"), Basic10seconds);
    ui->comboBoxPanStatus->addItem(GET_TEXT("PTZBASIC/111021", "Always Open"), BasicAlwaysOpen);
    ui->comboBoxPanStatus->addItem(GET_TEXT("PTZBASIC/111022", "Always Close"), BasicAlwaysClose);
    ui->comboBoxPanStatus->endEdit();

    ui->comboBoxPresetStatus->beginEdit();
    ui->comboBoxPresetStatus->clear();
    ui->comboBoxPresetStatus->addItem(GET_TEXT("PTZBASIC/111023", "2 seconds"), Basic2seconds);
    ui->comboBoxPresetStatus->addItem(GET_TEXT("PTZBASIC/111024", "5 seconds"), Basic5seconds);
    ui->comboBoxPresetStatus->addItem(GET_TEXT("PTZBASIC/111025", "10 seconds"), Basic10seconds);
    ui->comboBoxPresetStatus->addItem(GET_TEXT("PTZBASIC/111021", "Always Open"), BasicAlwaysOpen);
    ui->comboBoxPresetStatus->addItem(GET_TEXT("PTZBASIC/111022", "Always Close"), BasicAlwaysClose);
    ui->comboBoxPresetStatus->endEdit();

    ui->comboBoxPatrolStatus->beginEdit();
    ui->comboBoxPatrolStatus->clear();
    ui->comboBoxPatrolStatus->addItem(GET_TEXT("PTZBASIC/111021", "Always Open"), BasicAlwaysOpen);
    ui->comboBoxPatrolStatus->addItem(GET_TEXT("PTZBASIC/111022", "Always Close"), BasicAlwaysClose);
    ui->comboBoxPatrolStatus->endEdit();

    ui->comboBoxPatternStatus->beginEdit();
    ui->comboBoxPatternStatus->clear();
    ui->comboBoxPatternStatus->addItem(GET_TEXT("PTZBASIC/111021", "Always Open"), BasicAlwaysOpen);
    ui->comboBoxPatternStatus->addItem(GET_TEXT("PTZBASIC/111022", "Always Close"), BasicAlwaysClose);
    ui->comboBoxPatternStatus->endEdit();

    ui->comboBoxAutoStatus->beginEdit();
    ui->comboBoxAutoStatus->clear();
    ui->comboBoxAutoStatus->addItem(GET_TEXT("PTZBASIC/111021", "Always Open"), BasicAlwaysOpen);
    ui->comboBoxAutoStatus->addItem(GET_TEXT("PTZBASIC/111022", "Always Close"), BasicAlwaysClose);
    ui->comboBoxAutoStatus->endEdit();

    ui->comboBoxPreset->beginEdit();
    ui->comboBoxPreset->clear();
    ui->comboBoxPreset->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
    ui->comboBoxPreset->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBoxPreset->endEdit();

    ui->comboBoxSpeed->beginEdit();
    ui->comboBoxSpeed->clear();
    ui->comboBoxSpeed->addItem("1", 0);
    ui->comboBoxSpeed->addItem("2", 1);
    ui->comboBoxSpeed->addItem("3", 2);
    ui->comboBoxSpeed->addItem("4", 3);
    ui->comboBoxSpeed->addItem("5", 4);
    ui->comboBoxSpeed->addItem("6", 5);
    ui->comboBoxSpeed->addItem("7", 6);
    ui->comboBoxSpeed->addItem("8", 7);
    ui->comboBoxSpeed->addItem("9", 8);
    ui->comboBoxSpeed->addItem("10", 9);
    ui->comboBoxSpeed->endEdit();

    ui->comboBoxRecovering->beginEdit();
    ui->comboBoxRecovering->clear();
    ui->comboBoxRecovering->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
    ui->comboBoxRecovering->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBoxRecovering->endEdit();

    ui->lineEditRecoveryTime->setCheckMode(MyLineEdit::PTZRangeCheck, 5, 720);
    QRegExp numrx(QString("[0-9]*"));
    QValidator *numvalidator = new QRegExpValidator(numrx, this);
    ui->lineEditRecoveryTime->setValidator(numvalidator);

    ui->comboBoxFocusMode->beginEdit();
    ui->comboBoxFocusMode->clear();
    ui->comboBoxFocusMode->addItem(GET_TEXT("PTZBASIC/111012", "Auto"), 0);
    ui->comboBoxFocusMode->addItem(GET_TEXT("PTZBASIC/111013", "Semi-Auto"), 1);
    ui->comboBoxFocusMode->addItem(GET_TEXT("PTZBASIC/111014", "Manual"), 2);
    ui->comboBoxFocusMode->endEdit();

    ui->comboBoxFocusDistance->beginEdit();
    ui->comboBoxFocusDistance->clear();
    ui->comboBoxFocusDistance->addItem(GET_TEXT("PTZBASIC/111038", "1 meter"), 0);
    ui->comboBoxFocusDistance->addItem(GET_TEXT("PTZBASIC/111039", "1.5 meters"), 1);
    ui->comboBoxFocusDistance->addItem(GET_TEXT("PTZBASIC/111040", "3 meters"), 2);
    ui->comboBoxFocusDistance->addItem(GET_TEXT("PTZBASIC/111041", "6 meters"), 3);
    ui->comboBoxFocusDistance->addItem(GET_TEXT("PTZBASIC/111042", "10 meters"), 4);
    ui->comboBoxFocusDistance->addItem(GET_TEXT("PTZBASIC/111043", "20 meters"), 5);
    ui->comboBoxFocusDistance->endEdit();

    ui->comboBoxResumeTime->beginEdit();
    ui->comboBoxResumeTime->clear();
    ui->comboBoxResumeTime->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBoxResumeTime->addItem(GET_TEXT("PTZBASIC/111034", "30 seconds"), 1);
    ui->comboBoxResumeTime->addItem(GET_TEXT("PTZBASIC/111035", "60 seconds"), 2);
    ui->comboBoxResumeTime->addItem(GET_TEXT("PTZBASIC/111036", "300 seconds"), 3);
    ui->comboBoxResumeTime->addItem(GET_TEXT("PTZBASIC/111037", "600 seconds"), 4);
    ui->comboBoxResumeTime->endEdit();

    ui->comboBoxManualSpeed->beginEdit();
    ui->comboBoxManualSpeed->clear();
    ui->comboBoxManualSpeed->addItem(GET_TEXT("PTZBASIC/111026", "Low"), 0);
    ui->comboBoxManualSpeed->addItem(GET_TEXT("PTZBASIC/111027", "Medium"), 1);
    ui->comboBoxManualSpeed->addItem(GET_TEXT("PTZBASIC/111028", "High"), 2);
    ui->comboBoxManualSpeed->endEdit();

    onLanguageChanged();
}

TabPtzBasic::~TabPtzBasic()
{
    delete ui;
}

void TabPtzBasic::initializeData()
{
    ui->comboBoxChannel->setCurrentIndex(currentChannel());
    on_comboBoxChannel_activated(ui->comboBoxChannel->currentIndex());
}

void TabPtzBasic::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_PTZ_BASIC:
        ON_RESPONSE_FLAG_GET_PTZ_BASIC(message);
        break;
    case RESPONSE_FLAG_SET_PTZ_BASIC:
        ON_RESPONSE_FLAG_SET_PTZ_BASIC(message);
        break;
    case RESPONSE_FLAG_GET_IPC_SYSTEM_INFO:
        ON_RESPONSE_FLAG_GET_IPC_SYSTEM_INFO(message);
        break;
    }
}

void TabPtzBasic::setPtzEnable(bool enable)
{
    ui->ptzControlPanel->setEnabled(enable);
    if (enable) {
        //某些不支持preset的机型，PTZ的控制面板中preset ，path，pattern图标需置灰
        if (!gPtzDataManager->isPresetEnable()) {
            enable = false;
            //
            int zoom = gPtzDataManager->waitForGetPtzZoomPos();
            ui->ptzControlPanel->setZoomValue(zoom);
        }
        //MSHN-6191 QT-Live View：IPC型号中带“F”的，PTZ的控制面板中zoom为拉条形式
        ui->ptzControlPanel->setAutoZoomModel(gPtzDataManager->isAutoZoomModel());
    }
    ui->comboBoxSpeed->setEnabled(enable);
    ui->comboBoxPreset->setEnabled(enable);
    ui->comboBoxFocusMode->setEnabled(enable);
    ui->comboBoxPanStatus->setEnabled(enable);
    ui->comboBoxAutoStatus->setEnabled(enable);
    ui->comboBoxRecovering->setEnabled(enable);
    ui->comboBoxResumeTime->setEnabled(enable);
    ui->comboBoxZoomStatus->setEnabled(enable);
    ui->comboBoxManualSpeed->setEnabled(enable);
    ui->comboBoxPatrolStatus->setEnabled(enable);
    ui->comboBoxPresetStatus->setEnabled(enable);
    ui->comboBoxDehumidifying->setEnabled(enable);
    ui->comboBoxFocusDistance->setEnabled(enable);
    ui->comboBoxPatternStatus->setEnabled(enable);
    ui->lineEditRecoveryTime->setEnabled(enable && ui->comboBoxRecovering->currentIntData());
}

void TabPtzBasic::ON_RESPONSE_FLAG_GET_PTZ_BASIC(MessageReceive *message)
{
    PtzBasicInfo *basic = (PtzBasicInfo *)message->data;
    if (!basic) {
        m_eventLoop.exit(-1);
        return;
    }

    memset(&m_basic, 0, sizeof(PtzBasicInfo));
    memcpy(&m_basic, basic, sizeof(PtzBasicInfo));
    //PTZ OSD
    ui->comboBoxZoomStatus->setCurrentIndexFromData(m_basic.zoomStatus);
    ui->comboBoxPanStatus->setCurrentIndexFromData(m_basic.panTiltStatus);
    ui->comboBoxPresetStatus->setCurrentIndexFromData(m_basic.presetStatus);
    ui->comboBoxPatrolStatus->setCurrentIndexFromData(m_basic.patrolStatus);
    ui->comboBoxPatternStatus->setCurrentIndexFromData(m_basic.patternStatus);
    ui->comboBoxAutoStatus->setCurrentIndexFromData(m_basic.autoScanStatus);

    //Preset
    ui->comboBoxPreset->setCurrentIndexFromData(m_basic.presetFreezing);

    //Speed
    ui->comboBoxSpeed->setCurrentIndexFromData(m_basic.presetSpeed - 1);
    if (m_basic.manualSpeed > 0) {
        ui->comboBoxManualSpeed->setCurrentIndexFromData(m_basic.manualSpeed - 1);
    }
    ui->labelManualSpeed->setVisible(m_basic.manualSpeed > 0);
    ui->comboBoxManualSpeed->setVisible(m_basic.manualSpeed > 0);

    //Patrol
    ui->comboBoxRecovering->setCurrentIndexFromData(m_basic.patrolRecovering);
    ui->lineEditRecoveryTime->setText(QString("%1").arg(m_basic.patrolRecoveryTime));

    //Focus
    ui->comboBoxFocusMode->setCurrentIndexFromData(m_basic.focusMode);
    ui->comboBoxFocusDistance->setCurrentIndexFromData(m_basic.minFocusDistance);

    //Power Off Memory
    ui->comboBoxResumeTime->setCurrentIndexFromData(m_basic.resumeTime);

    //Dehumidifying
    ui->comboBoxDehumidifying->beginEdit();
    ui->comboBoxDehumidifying->clear();
    ui->comboBoxDehumidifying->addItem(GET_TEXT("PTZBASIC/111029", "General"), 0);
    ui->comboBoxDehumidifying->addItem(GET_TEXT("PTZBASIC/111030", "Enhancement"), 1);
    ui->comboBoxDehumidifying->addItem(GET_TEXT("PTZBASIC/111031", "Constant"), 2);
    MsCameraModel cameraMode = qMsNvr->cameraRealModel(m_channel);
    MsCameraVersion version = qMsNvr->cameraVersion(m_channel);
    QRegExp rx1("MS-C..61");
    QRegExp rx2("MS-C..67");
    //xx61一直都有Off，xx67 8.0.1版本以上才有Off
    if (cameraMode.model().contains(rx1) || (cameraMode.model().contains(rx2) && version >= MsCameraVersion(8, 1))) {
        ui->comboBoxDehumidifying->addItem(GET_TEXT("DISPLAYSETTINGS/108003", "Off"), 3);
    }
    ui->comboBoxDehumidifying->endEdit();
    if (m_basic.fanWorkingMode >= 0) {
        ui->comboBoxDehumidifying->setCurrentIndexFromData(m_basic.fanWorkingMode);
    }
    ui->widgetDehumidifying->setVisible(m_basic.fanWorkingMode >= 0);
    ui->labelFanWorkingMode->setVisible(m_basic.fanWorkingMode >= 0);
    ui->comboBoxDehumidifying->setVisible(m_basic.fanWorkingMode >= 0);
    m_eventLoop.exit(m_basic.ret);
}

void TabPtzBasic::ON_RESPONSE_FLAG_SET_PTZ_BASIC(MessageReceive *message)
{
    Q_UNUSED(message);
    //closeWait();
}

void TabPtzBasic::ON_RESPONSE_FLAG_GET_IPC_SYSTEM_INFO(MessageReceive *message)
{
    ipc_system_info *info = static_cast<ipc_system_info *>(message->data);
    if (info) {
        ui->labelManualSpeed->setVisible(info->system_manual_speed_support);
        ui->comboBoxManualSpeed->setVisible(info->system_manual_speed_support);
    }
}

void TabPtzBasic::onLanguageChanged()
{
    ui->labelChannel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));

    ui->labelPtzOSD->setText(GET_TEXT("PTZBASIC/111001", "PTZ OSD"));
    ui->labelZoomStatus->setText(GET_TEXT("PTZBASIC/111002", "Zoom Status"));
    ui->labelPanStatus->setText(GET_TEXT("PTZBASIC/111003", "Pan & Tilt Status"));
    ui->labelPresetStatus->setText(GET_TEXT("PTZBASIC/111004", "Preset Status"));
    ui->labelPatrolStatus->setText(GET_TEXT("PTZBASIC/111005", "Patrol Status"));
    ui->labelPatternStatus->setText(GET_TEXT("PTZBASIC/111006", "Pattern Status"));
    ui->labelAutoStatus->setText(GET_TEXT("PTZBASIC/111007", "Auto Scan Status"));

    ui->labelPreset->setText(GET_TEXT("PTZDIALOG/21015", "Preset"));
    ui->labelFreezing->setText(GET_TEXT("PTZBASIC/111008", "Preset Freezing"));

    ui->labelSpeedWidget->setText(GET_TEXT("PTZDIALOG/21020", "Speed"));
    ui->labelSpeed->setText(GET_TEXT("PTZBASIC/111009", "Preset Speed"));
    ui->labelManualSpeed->setText(GET_TEXT("PTZBASIC/111010", "Manual Speed"));

    ui->labelPatrol->setText(GET_TEXT("PTZDIALOG/21016", "Patrol"));
    ui->labelRecovering->setText(GET_TEXT("PTZBASIC/111032", "Patrol Recovering"));
    ui->labelRecoveryTime->setText(GET_TEXT("PTZBASIC/111033", "Patrol Recovery Time"));
    ui->labelPatrolRecoveryTimeNote->setText(QString("5~720%1").arg(GET_TEXT("HEATMAP/104024", "s")));

    ui->labelFocus->setText(GET_TEXT("PTZDIALOG/21022", "Focus"));
    ui->labelFocusMode->setText(GET_TEXT("PTZBASIC/111011", "Focus Mode"));
    ui->labelFocusDistance->setText(GET_TEXT("PTZBASIC/111015", "Minimum Focus Distance"));

    ui->labelPowerOffMemory->setText(GET_TEXT("PTZBASIC/111016", "Power Off Memory"));
    ui->labelResumeTime->setText(GET_TEXT("PTZBASIC/111017", "Set Resume Time"));

    ui->labelDehumidifying->setText(GET_TEXT("PTZBASIC/111018", "Dehumidifying"));
    ui->labelFanWorkingMode->setText(GET_TEXT("PTZBASIC/111019", "Fan Working Mode"));

    ui->pushButtonApply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButtonCopy->setText(GET_TEXT("COMMON/1005", "Copy"));
}

void TabPtzBasic::on_comboBoxChannel_activated(int index)
{
    m_channel = ui->comboBoxChannel->itemData(index).toInt();
    setCurrentChannel(m_channel);
    ui->widget_video->playVideo(m_channel);

    do {
        if (!LiveView::instance()->isChannelConnected(m_channel)) {
            ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
            break;
        }

        //showWait();

        sendMessage(REQUEST_FLAG_GET_PTZ_BASIC, &m_channel, sizeof(int));
        // int result = m_eventLoop.exec();
        // if (result != 0) {
        //     ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        //     break;
        // }

        //
        gPtzDataManager->beginGetData(m_channel);
        gPtzDataManager->waitForGetCameraModelType();
        if (qMsNvr->isFisheye(m_channel)) {
            ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
            break;
        }
        gPtzDataManager->waitForGetCameraModelInfo();
        gPtzDataManager->waitForGetPtzSupport();
        if (!gPtzDataManager->isPtzSupport()) {
            ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
            break;
        }

        gPtzDataManager->waitForGetPtzOvfInfo();
        //
        gPtzDataManager->waitForGetPtzSpeed();
        ui->ptzControlPanel->editSpeedValue(gPtzDataManager->ptzSpeed());

        gPtzDataManager->waitForGetAutoScan();
        ui->ptzControlPanel->setAutoScanChecked(gPtzDataManager->autoScan());

        sendMessage(REQUEST_FLAG_GET_IPC_SYSTEM_INFO, &m_channel, sizeof(int));

        //
        ui->widgetMessage->hide();
        setPtzEnable(true);
        ui->pushButtonCopy->setEnabled(true);
        ui->pushButtonApply->setEnabled(true);
        //closeWait();
        return;
    } while (0);
    setPtzEnable(false);
    ui->pushButtonCopy->setEnabled(false);
    ui->pushButtonApply->setEnabled(false);
    //closeWait();
}

void TabPtzBasic::on_pushButtonCopy_clicked()
{
    m_copyList.clear();

    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(m_channel);
    int result = copy.exec();
    if (result == ChannelCopyDialog::Accepted) {
        m_copyList = copy.checkedList();
        QTimer::singleShot(0, this, SLOT(on_pushButtonApply_clicked()));
    }
}

void TabPtzBasic::on_pushButtonApply_clicked()
{
    if (ui->lineEditRecoveryTime->isEnabled() && !ui->lineEditRecoveryTime->checkValid()) {
        return;
    }
    //showWait();
    m_basic.patrolRecoveryTime = ui->lineEditRecoveryTime->text().trimmed().toInt();
    if (m_copyList.isEmpty()) {
        m_basic.chanid = m_channel;
    } else {
        m_basic.chanid = -1;
        memset(m_basic.channelMask, 0, sizeof(m_basic.channelMask));
        for (int i = 0; i < m_copyList.size(); ++i) {
            int c = m_copyList.at(i);
            m_basic.channelMask[c] = '1';
        }
    }
    sendMessage(REQUEST_FLAG_SET_PTZ_BASIC, &m_basic, sizeof(PtzBasicInfo));
    //显示等待，避免频繁点击
    QEventLoop eventLoop;
    QTimer::singleShot(500, &eventLoop, SLOT(quit()));
    eventLoop.exec();
}

void TabPtzBasic::on_pushButtonBack_clicked()
{
    back();
}

void TabPtzBasic::on_comboBoxRecovering_currentIndexChanged(int index)
{
    switch (index) {
    case 0:
        ui->lineEditRecoveryTime->setEnabled(true);
        break;
    case 1:
        ui->lineEditRecoveryTime->setEnabled(false);
        break;
    default:
        break;
    }
    m_basic.patrolRecovering = ui->comboBoxRecovering->currentData().toInt();
}

void TabPtzBasic::on_comboBoxZoomStatus_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    m_basic.zoomStatus = ui->comboBoxZoomStatus->currentData().toInt();
}

void TabPtzBasic::on_comboBoxPanStatus_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    m_basic.panTiltStatus = ui->comboBoxPanStatus->currentData().toInt();
}

void TabPtzBasic::on_comboBoxPresetStatus_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    m_basic.presetStatus = ui->comboBoxPresetStatus->currentData().toInt();
}

void TabPtzBasic::on_comboBoxPatrolStatus_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    m_basic.patrolStatus = ui->comboBoxPatrolStatus->currentData().toInt();
}

void TabPtzBasic::on_comboBoxPatternStatus_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    m_basic.patternStatus = ui->comboBoxPatternStatus->currentData().toInt();
}

void TabPtzBasic::on_comboBoxAutoStatus_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    m_basic.autoScanStatus = ui->comboBoxAutoStatus->currentData().toInt();
}

void TabPtzBasic::on_comboBoxPreset_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    m_basic.presetFreezing = ui->comboBoxPreset->currentData().toInt();
}

void TabPtzBasic::on_comboBoxSpeed_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    m_basic.presetSpeed = ui->comboBoxSpeed->currentData().toInt() + 1;
}

void TabPtzBasic::on_comboBoxManualSpeed_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    m_basic.manualSpeed = ui->comboBoxManualSpeed->currentData().toInt() + 1;
}

void TabPtzBasic::on_comboBoxFocusMode_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    m_basic.focusMode = ui->comboBoxFocusMode->currentData().toInt();
}

void TabPtzBasic::on_comboBoxFocusDistance_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    m_basic.minFocusDistance = ui->comboBoxFocusDistance->currentData().toInt();
}

void TabPtzBasic::on_comboBoxResumeTime_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    m_basic.resumeTime = ui->comboBoxResumeTime->currentData().toInt();
}

void TabPtzBasic::on_comboBoxDehumidifying_indexSet(int index)
{
    Q_UNUSED(index);
    m_basic.fanWorkingMode = ui->comboBoxDehumidifying->currentData().toInt();
}
