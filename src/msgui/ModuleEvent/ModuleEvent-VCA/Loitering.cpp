#include "Loitering.h"
#include "ui_Loitering.h"
#include "ActionVCA.h"
#include "DrawMultiControlPolygon.h"
#include "DrawSceneObjectSize.h"
#include "EffectiveTimeVCA.h"
#include "FaceData.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "drawmotion.h"
#include "ptzdatamanager.h"
extern "C" {
#include "ptz_public.h"
}
const MsQtVca::ObjectVcaType ObjectType = MsQtVca::Loitering;

Loitering::Loitering(QWidget *parent)
    : BaseSmartWidget(parent)
    , ui(new Ui::Loitering)
{
    ui->setupUi(this);
    ui->widget_buttonGroup->setCount(qMsNvr->maxChannel());
    ui->widget_buttonGroup->setCurrentIndex(0);
    connect(ui->widget_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onChannelButtonClicked(int)));

    m_videoDraw = new DrawMotion(this);
    m_videoDraw->hide();

    ui->radioButton_minLayout->setAutoExclusive(false);
    ui->radioButton_maxLayout->setAutoExclusive(false);
    m_drawScene = new DrawSceneObjectSize(this);

    connect(m_drawScene, SIGNAL(objectSizeChanged(QRect, MsQtVca::ObjectSizeType)), this, SLOT(onSceneObjectSizeChanged(QRect, MsQtVca::ObjectSizeType)));

    m_drawMultiControl = new DrawMultiControlPolygon();
    m_drawMultiControl->setAlarmable(false);
    connect(m_drawMultiControl, SIGNAL(conflicted()), this, SLOT(onDrawPolygonConflicted()), Qt::QueuedConnection);
    ui->commonVideo->addGraphicsItem(m_drawMultiControl);

    m_event_info = new ms_smart_event_info;
    memset(m_event_info, 0, sizeof(ms_smart_event_info));
    m_settings_info2 = new ms_vca_settings_info2;

    ui->widgetMinLineBox->setWidthRange(1, 320);
    ui->widgetMinLineBox->setHeightRange(1, 240);
    connect(ui->widgetMinLineBox, SIGNAL(widthChange()), this, SLOT(onLineEditMinWidthEditingFinished()));
    connect(ui->widgetMinLineBox, SIGNAL(heightChange()), this, SLOT(onLineEditMinHeightEditingFinished()));

    ui->widgetMaxLineBox->setWidthRange(1, 320);
    ui->widgetMaxLineBox->setHeightRange(1, 240);
    connect(ui->widgetMaxLineBox, SIGNAL(widthChange()), this, SLOT(onLineEditMaxWidthEditingFinished()));
    connect(ui->widgetMaxLineBox, SIGNAL(heightChange()), this, SLOT(onLineEditMaxHeightEditingFinished()));

    ui->loiteringTime->setCheckMode(MyLineEdit::RangeCheck, 3, 1800);

    ui->comboBoxRegionNo->beginEdit();
    ui->comboBoxRegionNo->clear();
    ui->comboBoxRegionNo->addItem("1", 0);
    ui->comboBoxRegionNo->addItem("2", 1);
    ui->comboBoxRegionNo->addItem("3", 2);
    ui->comboBoxRegionNo->addItem("4", 3);
    ui->comboBoxRegionNo->endEdit();

    ui->comboBoxEffectiveRegion->beginEdit();
    ui->comboBoxEffectiveRegion->clear();
    ui->comboBoxEffectiveRegion->addItem(GET_TEXT("SMARTEVENT/146002", "Normal"), 0);
    ui->comboBoxEffectiveRegion->addItem(GET_TEXT("SMARTEVENT/146003", "Advanced"), 1);
    ui->comboBoxEffectiveRegion->endEdit();

    ui->comboBoxEffectiveWithPreset->beginEdit();
    ui->comboBoxEffectiveWithPreset->clear();
    ui->comboBoxEffectiveWithPreset->addItem(GET_TEXT("PTZDIALOG/21015", "preset") + QString(" %1").arg(1));
    ui->comboBoxEffectiveWithPreset->addItem(GET_TEXT("PTZDIALOG/21015", "preset") + QString(" %1").arg(2));
    ui->comboBoxEffectiveWithPreset->addItem(GET_TEXT("PTZDIALOG/21015", "preset") + QString(" %1").arg(3));
    ui->comboBoxEffectiveWithPreset->addItem(GET_TEXT("PTZDIALOG/21015", "preset") + QString(" %1").arg(4));
    ui->comboBoxEffectiveWithPreset->endEdit();

    clearSettings();
    onLanguageChanged();
}

Loitering::~Loitering()
{
    if (m_event_info) {
        delete m_event_info;
        m_event_info = nullptr;
    }
    if (m_settings_info2) {
        delete m_settings_info2;
        m_settings_info2 = nullptr;
    }
    delete ui;
}

void Loitering::initializeData(int channel)
{
    ui->widget_buttonGroup->setCurrentIndex(channel);
}

void Loitering::saveData()
{
    if (!checkObjectSize()) {
        clearCopyInfo();
        return;
    }
    //showWait();

    gFaceData.getFaceConfig(currentChannel());
    if (ui->checkBoxEnable->isChecked() && gFaceData.isFaceConflict()) {
        const int &result = MessageBox::question(this, GET_TEXT("FACE/141053", "Face Detection will be disabled, continue?"));
        if (result == MessageBox::Yes) {
            gFaceData.setFaceDisable();
        } else {
            //closeWait();
            clearCopyInfo();
            return;
        }
    }
    ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);
    ui->pushButtonRegionEdit->setButtonState(PushButtonEditState::StateFinished);
    updateEnableState();

    int channel = currentChannel();

    if (m_effectiveTime) {
        m_effectiveTime->saveEffectiveTime();
    }
    if (m_action) {
        m_action->saveAction();
    }
    REQ_UPDATE_CHN req = { 0 };
    ms_set_bit(&req.chnMask, channel, 1);
    sendMessageOnly(REQUEST_FLAG_SET_SMART_EVENT, &req, sizeof(req));

    //
    if (m_event_info->regionType == REGION_SINGLE) {
        if (isSupportPolygon()) {
            if (m_drawMultiControl->isFinished()) {
                m_drawMultiControl->getPolygon(m_event_info->polygonX, sizeof(m_event_info->polygonX), m_event_info->polygonY, sizeof(m_event_info->polygonY));
            } else {
                m_drawMultiControl->setPolygon(m_event_info->polygonX, m_event_info->polygonY);
            }
        } else {
            m_videoDraw->getRegion(m_event_info->area);
        }
        m_event_info->min_loitering_time = ui->loiteringTime->text().toInt();
    } else {
        if (m_drawMultiControl->isFinished()) {
            m_drawMultiControl->getRegions(m_event_info->regionInfo, m_event_info->regionScene, MAX_LEN_4);
        } else {
            m_drawMultiControl->setRegions(m_event_info->regionInfo, m_event_info->regionScene, MAX_LEN_4);
            m_drawMultiControl->setCurrentIndex(m_currentRegion);
        }
    }

    sendMessage(REQUEST_FLAG_SET_VCA_LOITERING, m_event_info, sizeof(ms_smart_event_info));
    m_eventLoop.exec();
    //
    sendMessage(REQUEST_FLAG_SET_VAC_SETTINGS2, m_settings_info2, sizeof(ms_vca_settings_info2));
    m_eventLoop.exec();
    clearCopyInfo();
    //closeWait();
}

void Loitering::copyData()
{
    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(currentChannel());
    int result = copy.exec();
    if (result == ChannelCopyDialog::Accepted) {
        QList<int> copyList = copy.checkedList(false);
        quint64 copyFlags = copy.checkedFlags(false);
        for (int i = 0; i < MAX_LEN_65; i++) {
            if (copyList.contains(i)) {
                m_event_info->copyChn[i] = '1';
                m_settings_info2->copyChn[i] = '1';
            } else {
                m_event_info->copyChn[i] = '0';
                m_settings_info2->copyChn[i] = '0';
            }
        }
        saveData();
        copy_smart_event_action(LOITERING, currentChannel(), copyFlags, 1);
    }
}

void Loitering::clearCopyInfo()
{
    for (int i = 0; i < MAX_LEN_65; i++) {
        m_event_info->copyChn[i] = '0';
        m_settings_info2->copyChn[i] = '0';
    }
}

void Loitering::processMessage(MessageReceive *message)
{
    dealGlobalMessage(message);
    //
    switch (message->type()) {
    case RESPONSE_FLAG_GET_VCA_LOITERING:
        ON_RESPONSE_FLAG_GET_VCA_LOITERING(message);
        break;
    case RESPONSE_FLAG_SET_VCA_LOITERING:
        ON_RESPONSE_FLAG_SET_VCA_LOITERING(message);
        break;
    case RESPONSE_FLAG_GET_VAC_SETTINGS2:
        ON_RESPONSE_FLAG_GET_VAC_SETTINGS2(message);
        break;
    case RESPONSE_FLAG_SET_VAC_SETTINGS2:
        ON_RESPONSE_FLAG_SET_VAC_SETTINGS2(message);
    default:
        break;
    }
}

void Loitering::ON_RESPONSE_FLAG_GET_VCA_LOITERING(MessageReceive *message)
{
    if (message->isNull()) {
        qMsWarning() << QString("data is null.");
        m_eventLoop.exit(-1);
        return;
    }
    if (!isSupportDetectionObject(currentChannel())) {
        ui->labelDetectionObject->hide();
        ui->widgetDetectionObject->hide();
    }
    ms_smart_event_info *info = (ms_smart_event_info *)message->data;
    memcpy(m_event_info, info, sizeof(ms_smart_event_info));

    if (m_event_info->regionType == REGION_SINGLE) {
        ui->labelRegionNo->hide();
        ui->comboBoxRegionNo->hide();
        setEffectiveRegionVisible(false);

        ui->checkBoxEnable->setChecked(m_event_info->enable);
        ui->loiteringTime->setText(QString("%1").arg(info->min_loitering_time));

        ui->checkBoxHuman->setChecked(m_event_info->objtype & DETEC_HUMAN);
        ui->checkBoxVehicle->setChecked(m_event_info->objtype & DETEC_VEHICLE);

        if (isSupportPolygon()) {
            m_drawMultiControl->show();
            m_drawMultiControl->setPolygon(m_event_info->polygonX, m_event_info->polygonY);
        } else {
            ui->commonVideo->setDrawWidget(m_videoDraw);
            m_videoDraw->setRegion(m_event_info->area);
            ui->commonVideo->showDrawWidget();
        }
    } else if (m_event_info->regionType == REGION_MULTIPLE) {
        ui->labelRegionNo->show();
        ui->comboBoxRegionNo->show();
        setEffectiveRegionVisible(false);
        readRegion(SCENE_NORMAL);

    } else {
        ui->labelRegionNo->show();
        ui->comboBoxRegionNo->show();

        setEffectiveRegionVisible(true);
        //ptz preset
        gPtzDataManager->beginGetData(currentChannel());
        gPtzDataManager->waitForGetPtzOvfInfo();
        resp_ptz_ovf_info *ptz_ovf_info = gPtzDataManager->ptzOvfInfo();
        for (int i = 0; i < 4; i++) {
            const resp_ptz_preset &preset = ptz_ovf_info->preset[i];
            if (m_event_info->presetExist[i + 1]) {
                ui->comboBoxEffectiveWithPreset->setItemData(i, preset.name);
            }
        }
        //region
        readRegion(m_event_info->regionScene);

        m_isFirstChoiceAdvanced = false;
        if (m_event_info->regionScene == SCENE_NORMAL) {
            ui->comboBoxEffectiveRegion->setCurrentIndexFromData(0);
            m_isFirstChoiceAdvanced = true;

        } else {
            m_ptzNeedToCall = false;
            ui->comboBoxEffectiveWithPreset->setCurrentIndex(m_event_info->regionScene - 1);
            ui->comboBoxEffectiveRegion->setCurrentIndexFromData(1);
        }
    }

    m_eventLoop.exit();
}

void Loitering::ON_RESPONSE_FLAG_SET_VCA_LOITERING(MessageReceive *message)
{
    if (message->isNull()) {
        qMsWarning() << "data is null.";
        m_eventLoop.exit(-1);
        return;
    }
    int result = *((int *)message->data);
    if (result < 0) {
        qMsWarning() << QString("result: %1").arg(result);
    }
    m_eventLoop.exit(result);
}

void Loitering::ON_RESPONSE_FLAG_GET_VAC_SETTINGS2(MessageReceive *message)
{
    if (message->isNull()) {
        m_eventLoop.exit(-1);
        return;
    }
    ms_vca_settings_info2 *info = static_cast<ms_vca_settings_info2 *>(message->data);
    memcpy(m_settings_info2, info, sizeof(ms_vca_settings_info2));
    int minX = m_settings_info2->minobject_window_rectangle_x[ObjectType];
    int minY = m_settings_info2->minobject_window_rectangle_y[ObjectType];
    int minWidth = m_settings_info2->minobject_window_rectangle_width[ObjectType];
    int minHeight = m_settings_info2->minobject_window_rectangle_height[ObjectType];
    int maxX = m_settings_info2->maxobject_window_rectangle_x[ObjectType];
    int maxY = m_settings_info2->maxobject_window_rectangle_y[ObjectType];
    int maxWidth = m_settings_info2->maxobject_window_rectangle_width[ObjectType];
    int maxHeight = m_settings_info2->maxobject_window_rectangle_height[ObjectType];
    ui->widgetMinLineBox->setWidthValue(minWidth);
    ui->widgetMinLineBox->setHeightValue(minHeight);
    ui->widgetMaxLineBox->setWidthValue(maxWidth);
    ui->widgetMaxLineBox->setHeightValue(maxHeight);
    qMsDebug() << QString("min: %1, %2, %3 x %4").arg(minX).arg(minY).arg(minWidth).arg(minHeight);
    qMsDebug() << QString("max: %1, %2, %3 x %4").arg(maxX).arg(maxY).arg(maxWidth).arg(maxHeight);

    m_eventLoop.exit();
}

void Loitering::ON_RESPONSE_FLAG_SET_VAC_SETTINGS2(MessageReceive *message)
{
    if (message->isNull()) {
        qMsWarning() << "data is null.";
        m_eventLoop.exit(-1);
    }
    int result = *((int *)message->data);
    if (result < 0) {
        qMsWarning() << QString("result: %1").arg(result);
    }
    m_eventLoop.exit(result);
}

void Loitering::clearSettings()
{
    ui->checkBoxEnable->setChecked(false);
    ui->checkBoxHuman->setChecked(false);
    ui->checkBoxVehicle->setChecked(false);
    setEffectiveRegionVisible(false);
}

void Loitering::updateEnableState()
{
    ui->checkBoxEnable->setEnabled(m_isConnected && m_isSupported);
    bool isEnable = ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked();
    ui->pushButton_setAll->setEnabled(isEnable);
    ui->pushButton_clearAll->setEnabled(isEnable);
    ui->pushButtonRegionEdit->setEnabled(isEnable);
    ui->pushButton_setAll->setVisible(ui->pushButton_setAll->isEnabled()
                                      && ui->pushButtonRegionEdit->buttonState() == PushButtonEditState::StateEditing);
    ui->pushButton_clearAll->setVisible(ui->pushButton_clearAll->isEnabled()
                                        && ui->pushButtonRegionEdit->buttonState() == PushButtonEditState::StateEditing);
    ui->pushButtonObjectSizeEdit->setEnabled(isEnable);
    ui->pushButtonObjectSizeReset->setEnabled(isEnable);
    ui->pushButtonObjectSizeReset->setVisible(ui->pushButtonObjectSizeReset->isEnabled()
                                              && ui->pushButtonObjectSizeEdit->buttonState() == PushButtonEditState::StateEditing);
    ui->checkBoxHuman->setEnabled(isEnable);
    ui->checkBoxVehicle->setEnabled(isEnable);
    ui->pushButton_editTime->setEnabled(isEnable);
    ui->pushButton_editAction->setEnabled(isEnable);
    ui->loiteringTime->setEnabled(isEnable);
    ui->widgetSize->setEnabled(isEnable
                               && ui->pushButtonObjectSizeEdit->buttonState() == PushButtonEditState::StateEditing);
    setApplyButtonEnable(ui->checkBoxEnable->isEnabled());

    ui->commonVideo->setDrawViewEnable(isEnable
                                       && (ui->pushButtonRegionEdit->buttonState() == PushButtonEditState::StateEditing
                                           || ui->pushButtonObjectSizeEdit->buttonState() == PushButtonEditState::StateEditing));
    ui->commonVideo->setDrawWidgetEnable(isEnable
                                         && ui->pushButtonRegionEdit->buttonState() == PushButtonEditState::StateEditing);
    m_drawMultiControl->setEnabled(isEnable
                                   && (ui->pushButtonObjectSizeEdit->buttonState() == PushButtonEditState::StateEditing
                                       || ui->pushButtonRegionEdit->buttonState() == PushButtonEditState::StateEditing));
    if (ui->pushButtonObjectSizeEdit->buttonState() != PushButtonEditState::StateEditing && !isEnable) {
        ui->commonVideo->hideDrawView();
    }

    ui->comboBoxRegionNo->setEnabled(m_isConnected && m_isSupported);
}

bool Loitering::checkObjectSize()
{
    if (ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked()) {
        bool valid = ui->widgetMinLineBox->checkValid();
        valid &= ui->widgetMaxLineBox->checkValid();
        valid &= ui->loiteringTime->checkValid();
        if (!valid) {
            return false;
        }

        int minWidth = ui->widgetMinLineBox->widthValue();
        int minHeight = ui->widgetMinLineBox->heightValue();
        int maxWidth = ui->widgetMaxLineBox->widthValue();
        int maxHeight = ui->widgetMaxLineBox->heightValue();

        if (minWidth > maxWidth || minHeight > maxHeight) {
            ShowMessageBox(GET_TEXT("SMARTEVENT/55010", "Minimum size must be smaller than Maximum size."));
            return false;
        }
    }

    return true;
}

bool Loitering::isSupportPolygon()
{
    if (QString(m_event_info->polygonX).isEmpty()) {
        return false;
    } else {
        return true;
    }
}

void Loitering::setEffectiveRegionVisible(bool visible)
{
    if (visible) {
        ui->labelEffectiveRegion->show();
        ui->comboBoxEffectiveRegion->show();
    } else {
        ui->labelEffectiveRegion->hide();
        ui->labelEffectiveWithPresets->hide();
        ui->comboBoxEffectiveRegion->hide();
        ui->widgetPresets->hide();
    }
}

void Loitering::showRegionEdit()
{
    ui->commonVideo->hideDrawView();
    if (isSupportPolygon()) {
        m_drawMultiControl->show();
        ui->commonVideo->hideDrawWidget();
    } else {
        ui->commonVideo->showDrawWidget();
    }
}

void Loitering::readRegion(int index)
{
    m_drawMultiControl->setRegions(m_event_info->regionInfo, index, MAX_LEN_4);
    m_drawMultiControl->clearAllEditState();
    m_drawMultiControl->setCurrentIndex(m_currentRegion);
    m_drawMultiControl->show();
    ui->comboBoxRegionNo->setCurrentIndexFromData(m_currentRegion);
}

void Loitering::onLanguageChanged()
{
    ui->label_loitering->setText(GET_TEXT("SMARTEVENT/55006", "Loitering"));
    ui->label_setRegion->setText(GET_TEXT("ANPR/103021", "Region"));
    ui->labelObjectSizeLimits->setText(GET_TEXT("SMARTEVENT/55063", "Object Size Limits"));
    ui->pushButtonObjectSizeReset->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->label_minTime->setText(GET_TEXT("SMARTEVENT/55023", "Min. Loitering Time (3-1800s)"));
    ui->labelDetectionObject->setText(GET_TEXT("TARGETMODE/103200", "Detection Object"));
    ui->checkBoxHuman->setText(GET_TEXT("TARGETMODE/103201", "Human"));
    ui->checkBoxVehicle->setText(GET_TEXT("TARGETMODE/103202", "Vehicle"));
    ui->label_effectiveTime->setText(GET_TEXT("SMARTEVENT/55015", "Effective Time"));
    ui->label_action->setText(GET_TEXT("VIDEOLOSS/50002", "Action"));
    ui->pushButton_setAll->setText(GET_TEXT("MOTION/51004", "Set All"));
    ui->pushButton_clearAll->setText(GET_TEXT("PLAYBACK/80119", "Delete All"));
    ui->pushButton_editAction->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->pushButton_editTime->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->checkBoxEnable->setText(GET_TEXT("COMMON/1009", "Enable"));

    ui->labelEffectiveRegion->setText(GET_TEXT("SMARTEVENT/146001", "Effective Region"));
    ui->labelEffectiveWithPresets->setText(GET_TEXT("SMARTEVENT/146007", "Effective with presets"));
    ui->labelRegionNo->setText(GET_TEXT("SMARTEVENT/146000", "Region No."));
    ui->pushButtonEffectiveWithPreset->setText(GET_TEXT("SMARTEVENT/146008", "call"));
}

void Loitering::onDrawPolygonConflicted()
{
    MessageBox::information(this, GET_TEXT("SMARTEVENT/55066", "The boundaries of the area cannot intersect. Please redraw."));
    m_drawMultiControl->clearPolygon();
}

void Loitering::onChannelButtonClicked(int index)
{
    if (!isVisible()) {
        return;
    }
    if (m_action) {
        m_action->clearCache();
    }
    if (m_effectiveTime) {
        m_effectiveTime->clearCache();
    }
    ui->labelRegionNo->show();
    ui->comboBoxRegionNo->show();
    ui->labelDetectionObject->show();
    ui->widgetDetectionObject->show();
    ui->widgetMinLineBox->setValid(true);
    ui->widgetMaxLineBox->setValid(true);

    int channel = index;
    setCurrentChannel(channel);
    ui->commonVideo->playVideo(channel);
    ui->commonVideo->setBanOnBack(true);
    //mutil region
    m_drawMultiControl->hide();
    m_drawMultiControl->clearAll();
    m_currentRegion = 0;

    m_isConnected = false;
    m_isSupported = false;
    ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);
    ui->pushButtonRegionEdit->editButtonState(PushButtonEditState::StateFinished);
    ui->commonVideo->hideDrawWidget();
    emit hideMessage();
    //检查通道是否连接
    m_isConnected = isChannelEnable(channel);
    if (!m_isConnected) {
        showNotConnectedMessage();
        clearSettings();
        updateEnableState();
        return;
    }
    //showWait();
    //检查鱼眼是否支持
    if (qMsNvr->isFisheye(channel)) {
        int fisheyeSupport = waitForCheckFisheeyeSupport(channel);
        if (fisheyeSupport < 0) {
            showNotSupportMessage();
            clearSettings();
            updateEnableState();
            return;
        }
    }
    //检查VCA是否支持
    m_isSupported = (waitForCheckVcaSupport(channel) >= 0);
    if (!m_isSupported) {
        showNotSupportMessage();
        clearSettings();
        updateEnableState();
        return;
    }
    //获取数据
    MsCameraVersion cameraVersion = MsCameraVersion::fromChannel(currentChannel());
    if ((cameraVersion >= MsCameraVersion(8, 3, 9) && qMsNvr->isNT(currentChannel()))
    || cameraVersion >= MsCameraVersion(8, 4)) {
        m_objectSizeType = DrawSceneObjectSize::After_8_0_3_r9;
        ui->widgetMinLineBox->setWidthRange(8, 608);
        ui->widgetMinLineBox->setHeightRange(8, 352);
        ui->widgetMaxLineBox->setWidthRange(8, 608);
        ui->widgetMaxLineBox->setHeightRange(8, 352);
        ui->radioButton_minLayout->setText(GET_TEXT("STATUS/177014", "Minimum Size(%1x%2~%3x%4)").arg(8).arg(8).arg(608).arg(352));
        ui->radioButton_maxLayout->setText(GET_TEXT("STATUS/177015", "Maximum Size(%1x%2~%3x%4)").arg(8).arg(8).arg(608).arg(352));
    } else {
        m_objectSizeType = DrawSceneObjectSize::Before_8_0_3_r9;
        ui->widgetMinLineBox->setWidthRange(1, 320);
        ui->widgetMinLineBox->setHeightRange(1, 240);
        ui->widgetMaxLineBox->setWidthRange(1, 320);
        ui->widgetMaxLineBox->setHeightRange(1, 240);
        ui->radioButton_minLayout->setText(GET_TEXT("SMARTEVENT/55052", "Minimum Size(1x1~%1x%2)").arg(320).arg(240));
        ui->radioButton_maxLayout->setText(GET_TEXT("SMARTEVENT/55053", "Maximum Size(1x1~%1x%2)").arg(320).arg(240));
    }
    // int result = 0;
    // sendMessage(REQUEST_FLAG_GET_VCA_LOITERING, (void *)&channel, sizeof(int));
    // result = m_eventLoop.exec();
    // if (result < 0) {
    //     m_isConnected = false;
    //     updateEnableState();
    //     showDataError();
    //     return;
    // }
    // sendMessage(REQUEST_FLAG_GET_VAC_SETTINGS2, (void *)&channel, sizeof(int));
    // result = m_eventLoop.exec();
    // if (result < 0) {
    //     m_isConnected = false;
    //     updateEnableState();
    //     showDataError();
    //     return;
    // }

    // //closeWait();
    // updateEnableState();
}

void Loitering::on_pushButton_setAll_clicked()
{
    if (isSupportPolygon()) {
        m_drawMultiControl->selectAll();
    } else {
        m_videoDraw->selectAll();
    }
}

void Loitering::on_pushButton_clearAll_clicked()
{
    if (isSupportPolygon()) {
        m_drawMultiControl->clearPolygon();
    } else {
        m_videoDraw->clearAll();
    }
}

void Loitering::on_pushButton_editTime_clicked()
{
    if (!m_effectiveTime) {
        m_effectiveTime = new EffectiveTimeVCA(this);
    }
    m_effectiveTime->showEffectiveTime(currentChannel(), LOITERING);
}

void Loitering::on_pushButton_editAction_clicked()
{
    if (!m_action) {
        m_action = new ActionSmartEvent(this);
    }
    m_action->showAction(currentChannel(), LOITERING);
}

void Loitering::onSceneObjectSizeChanged(const QRect &rc, MsQtVca::ObjectSizeType sizeType)
{
    switch (sizeType) {
    case MsQtVca::MinSize:
        m_settings_info2->minobject_window_rectangle_x[ObjectType] = rc.x();
        m_settings_info2->minobject_window_rectangle_y[ObjectType] = rc.y();
        m_settings_info2->minobject_window_rectangle_width[ObjectType] = rc.width();
        m_settings_info2->minobject_window_rectangle_height[ObjectType] = rc.height();

        ui->widgetMinLineBox->setWidthValue(rc.width());
        ui->widgetMinLineBox->setHeightValue(rc.height());
        break;
    case MsQtVca::MaxSize:
        m_settings_info2->maxobject_window_rectangle_x[ObjectType] = rc.x();
        m_settings_info2->maxobject_window_rectangle_y[ObjectType] = rc.y();
        m_settings_info2->maxobject_window_rectangle_width[ObjectType] = rc.width();
        m_settings_info2->maxobject_window_rectangle_height[ObjectType] = rc.height();

        ui->widgetMaxLineBox->setWidthValue(rc.width());
        ui->widgetMaxLineBox->setHeightValue(rc.height());
        break;
    default:
        break;
    }
}

void Loitering::on_radioButton_minLayout_clicked(bool checked)
{
    if (!checked) {
        ui->radioButton_minLayout->setChecked(true);
        return;
    } else {
        ui->radioButton_maxLayout->setChecked(false);
    }
    m_drawScene->showMinimumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
}

void Loitering::on_radioButton_maxLayout_clicked(bool checked)
{
    if (!checked) {
        ui->radioButton_maxLayout->setChecked(true);
    } else {
        ui->radioButton_minLayout->setChecked(false);
    }
    m_drawScene->showMaximumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
}

void Loitering::onLineEditMinWidthEditingFinished()
{
    int value = ui->widgetMinLineBox->widthValue();
    if (value < 1) {
        return;
    }
    m_settings_info2->minobject_window_rectangle_width[ObjectType] = value;
    if (ui->radioButton_minLayout->isChecked()) {
        m_drawScene->showMinimumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
    }
}

void Loitering::onLineEditMinHeightEditingFinished()
{
    int value = ui->widgetMinLineBox->heightValue();
    if (value < 1) {
        return;
    }
    m_settings_info2->minobject_window_rectangle_height[ObjectType] = value;
    if (ui->radioButton_minLayout->isChecked()) {
        m_drawScene->showMinimumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
    }
}

void Loitering::onLineEditMaxWidthEditingFinished()
{
    int value = ui->widgetMaxLineBox->widthValue();
    if (value < 1) {
        return;
    }
    m_settings_info2->maxobject_window_rectangle_width[ObjectType] = value;
    if (ui->radioButton_maxLayout->isChecked()) {
        m_drawScene->showMaximumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
    }
}

void Loitering::onLineEditMaxHeightEditingFinished()
{
    int value = ui->widgetMaxLineBox->heightValue();
    if (value < 1) {
        return;
    }
    m_settings_info2->maxobject_window_rectangle_height[ObjectType] = value;
    if (ui->radioButton_maxLayout->isChecked()) {
        m_drawScene->showMaximumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
    }
}

void Loitering::on_pushButtonRegionEdit_clicked()
{
    switch (ui->pushButtonRegionEdit->buttonState()) {
    case PushButtonEditState::StateFinished:
        ui->pushButtonRegionEdit->setButtonState(PushButtonEditState::StateEditing);
        ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);
        break;
    case PushButtonEditState::StateEditing:
        ui->pushButtonRegionEdit->setButtonState(PushButtonEditState::StateFinished);
        break;
    }
    updateEnableState();
}

void Loitering::on_pushButtonRegionEdit_stateSet(PushButtonEditState::State state)
{
    if (ui->checkBoxEnable->isEnabled()) {
        switch (state) {
        case PushButtonEditState::StateFinished:
            if (isSupportPolygon()) {
                if (m_event_info->regionType != REGION_SINGLE) {
                    m_drawMultiControl->setEditable(false);
                }
                if (!m_drawMultiControl->isFinished()) {
                    m_drawMultiControl->setPolygon(m_event_info->polygonX, m_event_info->polygonY);
                }
            }
            break;
        case PushButtonEditState::StateEditing:
            showRegionEdit();
            if (isSupportPolygon()) {
                m_drawMultiControl->setEditable(true);
            }
            break;
        }
    }
}
void Loitering::on_pushButtonObjectSizeEdit_clicked()
{
    switch (ui->pushButtonObjectSizeEdit->buttonState()) {
    case PushButtonEditState::StateFinished:
        ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateEditing);
        ui->pushButtonRegionEdit->setButtonState(PushButtonEditState::StateFinished);
        break;
    case PushButtonEditState::StateEditing:
        ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);
        break;
    }
    updateEnableState();
}

void Loitering::on_pushButtonObjectSizeEdit_stateSet(PushButtonEditState::State state)
{
    if (ui->checkBoxEnable->isEnabled()) {
        switch (state) {
        case PushButtonEditState::StateFinished:

            ui->radioButton_minLayout->setChecked(false);
            ui->radioButton_maxLayout->setChecked(false);
            ui->commonVideo->removeDrawScene();
            showRegionEdit();
            break;
        case PushButtonEditState::StateEditing:

            ui->radioButton_minLayout->setChecked(true);
            ui->radioButton_maxLayout->setChecked(false);
            ui->commonVideo->showDrawScene(m_drawScene);
            m_drawScene->showMinimumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
            m_drawMultiControl->hide();
            break;
        }
    }
}

void Loitering::on_pushButtonObjectSizeReset_clicked()
{
    int minw,minh,maxw,maxh;
    if (m_objectSizeType == DrawSceneObjectSize::Before_8_0_3_r9) {
        minw = 3;
        minh = 3;
        maxw = 320;
        maxh = 240;
    } else {
        minw = 8;
        minh = 8;
        maxw = 608;
        maxh = 352;
    }
    m_settings_info2->minobject_window_rectangle_x[ObjectType] = 0;
    m_settings_info2->minobject_window_rectangle_y[ObjectType] = 0;
    m_settings_info2->minobject_window_rectangle_width[ObjectType] = minw;
    m_settings_info2->minobject_window_rectangle_height[ObjectType] = minh;
    m_settings_info2->maxobject_window_rectangle_x[ObjectType] = 0;
    m_settings_info2->maxobject_window_rectangle_y[ObjectType] = 0;
    m_settings_info2->maxobject_window_rectangle_width[ObjectType] = maxw;
    m_settings_info2->maxobject_window_rectangle_height[ObjectType] = maxh;

    ui->widgetMinLineBox->setWidthValue(minw);
    ui->widgetMinLineBox->setHeightValue(minh);
    ui->widgetMaxLineBox->setWidthValue(maxw);
    ui->widgetMaxLineBox->setHeightValue(maxh);


    if (ui->radioButton_minLayout->isChecked()) {
        m_drawScene->showMinimumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
    }
    if (ui->radioButton_maxLayout->isChecked()) {
        m_drawScene->showMaximumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
    }
}

void Loitering::on_checkBoxEnable_clicked(bool checked)
{
    Q_UNUSED(checked)
    if (m_event_info->regionType == REGION_SINGLE) {
        m_event_info->enable = ui->checkBoxEnable->isChecked();
    } else {
        m_event_info->regionInfo[m_currentRegion].enable = ui->checkBoxEnable->isChecked();
    }

    if (!checked) {
        ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);
    }
    updateEnableState();
}

void Loitering::on_comboBoxEffectiveRegion_indexSet(int index)
{
    if (index == 1 && !m_event_info->presetExist[ui->comboBoxEffectiveWithPreset->currentIndex() + 1]) {
        ui->labelEffectiveWithPresets->show();
        ui->widgetPresets->show();
        MessageBox::information(this, GET_TEXT("SMARTEVENT/146006", "Please set up the preset first."));
        if (m_event_info->regionScene == SCENE_NORMAL) {
            ui->comboBoxEffectiveRegion->setCurrentIndexFromData(0);
        }
        return;
    }
    if (m_isFirstChoiceAdvanced && index == 1) {
        m_isFirstChoiceAdvanced = false;
        const int &result = MessageBox::question(this, GET_TEXT("SMARTEVENT/146004", "The camera will jump to preset 1, continue?"));
        if (result == MessageBox::Yes) {
            ui->comboBoxEffectiveWithPreset->setCurrentIndex(0);
        } else {
            ui->comboBoxEffectiveRegion->setCurrentIndexFromData(0);
            return;
        }
    }

    m_drawMultiControl->getRegions(m_event_info->regionInfo, m_event_info->regionScene, MAX_LEN_4);
    if (index == 0) {
        m_event_info->regionScene = SCENE_NORMAL;
        ui->labelEffectiveWithPresets->hide();
        ui->widgetPresets->hide();
    } else {
        m_event_info->regionScene = static_cast<SMART_REGION_SCENE>(ui->comboBoxEffectiveWithPreset->currentIndex() + 1);
        ui->labelEffectiveWithPresets->show();
        ui->widgetPresets->show();
        if (!m_event_info->presetExist[m_event_info->regionScene]) {
            MessageBox::information(this, GET_TEXT("SMARTEVENT/146006", "Please set up the preset first."));
            ui->comboBoxEffectiveWithPreset->setCurrentIndex(m_event_info->regionScene - 1);
            return;
        }
        if (m_ptzNeedToCall) {
            gPtzDataManager->sendPresetControl(PTZ_PRESET_GOTO, m_event_info->regionScene, 5, ui->comboBoxEffectiveWithPreset->currentData().toString());
        }
        m_ptzNeedToCall = true;
    }
    readRegion(m_event_info->regionScene);
}

void Loitering::on_comboBoxEffectiveWithPreset_activated(int index)
{
    if (!m_event_info->presetExist[index + 1]) {
        MessageBox::information(this, GET_TEXT("SMARTEVENT/146006", "Please set up the preset first."));
        ui->comboBoxEffectiveWithPreset->setCurrentIndex(m_event_info->regionScene - 1);
        return;
    }
    m_drawMultiControl->getRegions(m_event_info->regionInfo, m_event_info->regionScene, MAX_LEN_4);
    m_event_info->regionScene = static_cast<SMART_REGION_SCENE>(index + 1);
    readRegion(m_event_info->regionScene);

    gPtzDataManager->sendPresetControl(PTZ_PRESET_GOTO, index + 1, 5, ui->comboBoxEffectiveWithPreset->currentData().toString());
}

void Loitering::on_comboBoxRegionNo_indexSet(int index)
{
    if (ui->loiteringTime->isEnabled() && !ui->loiteringTime->checkValid() && index != m_currentRegion) {
        ui->comboBoxRegionNo->setCurrentIndexFromData(m_currentRegion);
        return;
    }
    ui->pushButtonRegionEdit->setButtonState(PushButtonEditState::StateFinished);
    ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);

    m_currentRegion = index;
    m_drawMultiControl->setCurrentIndex(index);
    ui->checkBoxEnable->setChecked(m_event_info->regionInfo[index].enable);
    if (m_event_info->regionInfo[index].minTime >= 0) {
        ui->loiteringTime->setText(QString("%1").arg(m_event_info->regionInfo[index].minTime));
    } else {
        ui->loiteringTime->setText("");
    }

    ui->checkBoxHuman->setChecked(m_event_info->regionInfo[index].objType & DETEC_HUMAN);
    ui->checkBoxVehicle->setChecked(m_event_info->regionInfo[index].objType & DETEC_VEHICLE);
    updateEnableState();
}

void Loitering::on_pushButtonEffectiveWithPreset_clicked()
{
    int index = ui->comboBoxEffectiveWithPreset->currentIndex();
    gPtzDataManager->sendPresetControl(PTZ_PRESET_GOTO, index + 1, 5, ui->comboBoxEffectiveWithPreset->currentData().toString());
}

void Loitering::on_checkBoxHuman_clicked()
{
    if (m_event_info->regionType == REGION_SINGLE) {
        if (ui->checkBoxHuman->isChecked()) {
            m_event_info->objtype |= DETEC_HUMAN;
        } else {
            m_event_info->objtype &= ~(DETEC_HUMAN);
        }
    } else {
        int objType = static_cast<int>(m_event_info->regionInfo[m_currentRegion].objType);
        if (ui->checkBoxHuman->isChecked()) {
            objType |= DETEC_HUMAN;
        } else {
            objType &= ~(DETEC_HUMAN);
        }
        m_event_info->regionInfo[m_currentRegion].objType = static_cast<DETEC_OBJ_EN>(objType);
    }
}

void Loitering::on_checkBoxVehicle_clicked()
{
    if (m_event_info->regionType == REGION_SINGLE) {
        if (ui->checkBoxVehicle->isChecked()) {
            m_event_info->objtype |= DETEC_VEHICLE;
        } else {
            m_event_info->objtype &= ~(DETEC_VEHICLE);
        }
    } else {
        int objType = static_cast<int>(m_event_info->regionInfo[m_currentRegion].objType);
        if (ui->checkBoxVehicle->isChecked()) {
            objType |= DETEC_VEHICLE;
        } else {
            objType &= ~(DETEC_VEHICLE);
        }
        m_event_info->regionInfo[m_currentRegion].objType = static_cast<DETEC_OBJ_EN>(objType);
    }
}

void Loitering::on_loiteringTime_editingFinished()
{
    if (ui->loiteringTime->text().isEmpty()) {
        m_event_info->regionInfo[m_currentRegion].minTime = -1;
    } else {
        m_event_info->regionInfo[m_currentRegion].minTime = ui->loiteringTime->text().toInt();
    }
}
