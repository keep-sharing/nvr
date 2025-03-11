#include "objectleftremoved.h"
#include "ui_objectleftremoved.h"
#include "ActionVCA.h"
#include "DrawMultiControlPolygon.h"
#include "DrawSceneObjectSize.h"
#include "DrawScenePolygon.h"
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

const MsQtVca::ObjectVcaType ObjectType = MsQtVca::ObjectLeftRemoved;

ObjectLeftRemoved::ObjectLeftRemoved(QWidget *parent)
    : BaseSmartWidget(parent)
    , ui(new Ui::ObjectLeftRemoved)
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

    m_settings_info2 = new ms_vca_settings_info2;
    connect(this, SIGNAL(needSensitivityChanged()), this, SLOT(onNeedSensitivityChanged()));
    setNeedSensitivity(false);
    ui->lineEdit_mintime->setCheckMode(MyLineEdit::RangeCheck, 5, 1800);

    m_object_info = new ms_smart_leftremove_info;
    memset(m_object_info, 0, sizeof(ms_smart_leftremove_info));

    ui->widgetMinLineBox->setWidthRange(1, 320);
    ui->widgetMinLineBox->setHeightRange(1, 240);
    connect(ui->widgetMinLineBox, SIGNAL(widthChange()), this, SLOT(onLineEditMinWidthEditingFinished()));
    connect(ui->widgetMinLineBox, SIGNAL(heightChange()), this, SLOT(onLineEditMinHeightEditingFinished()));

    ui->widgetMaxLineBox->setWidthRange(1, 320);
    ui->widgetMaxLineBox->setHeightRange(1, 240);
    connect(ui->widgetMaxLineBox, SIGNAL(widthChange()), this, SLOT(onLineEditMaxWidthEditingFinished()));
    connect(ui->widgetMaxLineBox, SIGNAL(heightChange()), this, SLOT(onLineEditMaxHeightEditingFinished()));

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

ObjectLeftRemoved::~ObjectLeftRemoved()
{
    if (m_object_info) {
        delete m_object_info;
        m_object_info = nullptr;
    }
    if (m_settings_info2) {
        delete m_settings_info2;
        m_settings_info2 = nullptr;
    }
    delete ui;
}

void ObjectLeftRemoved::initializeData(int channel)
{
    ui->widget_buttonGroup->setCurrentIndex(channel);
}

void ObjectLeftRemoved::saveData()
{
    if (!checkObjectSize()) {
        clearCopyInfo();
        return;
    }
    //showWait();
    gFaceData.getFaceConfig(currentChannel());
    if ((ui->checkBox_obj_left->isChecked() || ui->checkBox_obj_remove->isChecked()) && gFaceData.isFaceConflict()) {
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
    if (m_object_info->regionType == REGION_SINGLE) {
        if (isSupportPolygon()) {
            if (m_drawMultiControl->isFinished()) {
                m_drawMultiControl->getPolygon(m_object_info->polygonX, sizeof(m_object_info->polygonX), m_object_info->polygonY, sizeof(m_object_info->polygonY));
            } else {
                m_drawMultiControl->setPolygon(m_object_info->polygonX, m_object_info->polygonY);
            }
        } else {
            m_videoDraw->getRegion(m_object_info->area);
        }
        m_object_info->left_enable = ui->checkBox_obj_left->isChecked();
        m_object_info->remove_enable = ui->checkBox_obj_remove->isChecked();
    } else {
        if (m_drawMultiControl->isFinished()) {
            m_drawMultiControl->getRegions(m_object_info->regionInfo, m_object_info->regionScene, MAX_LEN_4);
        } else {
            m_drawMultiControl->setRegions(m_object_info->regionInfo, m_object_info->regionScene, MAX_LEN_4);
            m_drawMultiControl->setCurrentIndex(m_currentRegion);
        }
    }
    m_object_info->min_time = ui->lineEdit_mintime->text().toInt();
    if (needSensitivity()) {
        m_object_info->sensitivity = ui->slider_sensitivity->value();
    }
    sendMessage(REQUEST_FLAG_SET_VCA_LEFTREMOVE, m_object_info, sizeof(ms_smart_leftremove_info));
    m_eventLoop.exec();
    sendMessage(REQUEST_FLAG_SET_VAC_SETTINGS2, m_settings_info2, sizeof(ms_vca_settings_info2));
    m_eventLoop.exec();
    clearCopyInfo();
    //closeWait();
}

void ObjectLeftRemoved::copyData()
{
    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(currentChannel());
    int result = copy.exec();
    if (result == ChannelCopyDialog::Accepted) {
        QList<int> copyList = copy.checkedList(false);
        quint64 copyFlags = copy.checkedFlags(false);
        for (int i = 0; i < MAX_LEN_65; i++) {
            if (copyList.contains(i)) {
                m_object_info->copyChn[i] = '1';
                m_settings_info2->copyChn[i] = '1';
            } else {
                m_object_info->copyChn[i] = '0';
                m_settings_info2->copyChn[i] = '0';
            }
        }
        saveData();
        copy_smart_event_action(OBJECT_LEFTREMOVE, currentChannel(), copyFlags, 1);
    }
}

void ObjectLeftRemoved::clearCopyInfo()
{
    for (int i = 0; i < MAX_LEN_65; i++) {
        m_object_info->copyChn[i] = '0';
        m_settings_info2->copyChn[i] = '0';
    }
}

void ObjectLeftRemoved::processMessage(MessageReceive *message)
{
    dealGlobalMessage(message);
    //
    switch (message->type()) {
    case RESPONSE_FLAG_GET_VCA_LEFTREMOVE:
        ON_RESPONSE_FLAG_GET_VCA_LEFTREMOVE(message);
        break;
    case RESPONSE_FLAG_SET_VCA_LEFTREMOVE:
        ON_RESPONSE_FLAG_SET_VCA_LEFTREMOVE(message);
        break;
    case RESPONSE_FLAG_GET_VAC_SETTINGS2:
        ON_RESPONSE_FLAG_GET_VAC_SETTINGS2(message);
        break;
    case RESPONSE_FLAG_SET_VAC_SETTINGS2:
        ON_RESPONSE_FLAG_SET_VAC_SETTINGS2(message);
        break;
    default:
        break;
    }
}

void ObjectLeftRemoved::ON_RESPONSE_FLAG_GET_VCA_LEFTREMOVE(MessageReceive *message)
{
    if (message->isNull()) {
        qMsWarning() << QString("data is null.");
        m_eventLoop.exit(-1);
        return;
    }

    if (isSupportSensitivity(currentChannel())) {
        setNeedSensitivity(true);
    } else {
        setNeedSensitivity(false);
    }

    ms_smart_leftremove_info *info = (ms_smart_leftremove_info *)message->data;
    memcpy(m_object_info, info, sizeof(ms_smart_leftremove_info));

    if (m_object_info->regionType == REGION_SINGLE) {
        ui->labelRegionNo->hide();
        ui->comboBoxRegionNo->hide();
        setEffectiveRegionVisible(false);

        ui->checkBox_obj_left->setChecked(m_object_info->left_enable);
        ui->checkBox_obj_remove->setChecked(m_object_info->remove_enable);

        if (isSupportPolygon()) {
            m_drawMultiControl->show();
            m_drawMultiControl->setPolygon(m_object_info->polygonX, m_object_info->polygonY);
        } else {
            ui->commonVideo->setDrawWidget(m_videoDraw);
            m_videoDraw->setRegion(m_object_info->area);
            ui->commonVideo->showDrawWidget();
        }
    } else if (m_object_info->regionType == REGION_MULTIPLE) {
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
            if (m_object_info->presetExist[i + 1]) {
                ui->comboBoxEffectiveWithPreset->setItemData(i, preset.name);
            }
        }
        //region
        readRegion(m_object_info->regionScene);
        m_isFirstChoiceAdvanced = false;
        if (m_object_info->regionScene == SCENE_NORMAL) {
            ui->comboBoxEffectiveRegion->setCurrentIndexFromData(0);
            m_isFirstChoiceAdvanced = true;

        } else {
            m_ptzNeedToCall = false;
            ui->comboBoxEffectiveWithPreset->setCurrentIndex(m_object_info->regionScene - 1);
            ui->comboBoxEffectiveRegion->setCurrentIndexFromData(1);
        }
    }
    ui->slider_sensitivity->setValue(m_object_info->sensitivity);
    ui->lineEdit_mintime->setText(QString("%1").arg(m_object_info->min_time));
    m_eventLoop.exit();
}

void ObjectLeftRemoved::ON_RESPONSE_FLAG_SET_VCA_LEFTREMOVE(MessageReceive *message)
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

void ObjectLeftRemoved::ON_RESPONSE_FLAG_GET_VAC_SETTINGS2(MessageReceive *message)
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

void ObjectLeftRemoved::ON_RESPONSE_FLAG_SET_VAC_SETTINGS2(MessageReceive *message)
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

void ObjectLeftRemoved::clearSettings()
{
    ui->checkBox_obj_left->setChecked(false);
    ui->checkBox_obj_remove->setChecked(false);
    setEffectiveRegionVisible(false);
}

void ObjectLeftRemoved::updateEnableState()
{
    ui->checkBox_obj_left->setEnabled(m_isConnected && m_isSupported);
    ui->checkBox_obj_remove->setEnabled(m_isConnected && m_isSupported);

    bool m_widgetEnabled = (ui->checkBox_obj_left->isEnabled() && ui->checkBox_obj_left->isChecked())
        || (ui->checkBox_obj_remove->isEnabled() && ui->checkBox_obj_remove->isChecked());

    ui->pushButton_setAll->setEnabled(m_widgetEnabled);
    ui->pushButton_clearAll->setEnabled(m_widgetEnabled);
    ui->pushButtonRegionEdit->setEnabled(m_widgetEnabled);
    ui->pushButton_setAll->setVisible(ui->pushButton_setAll->isEnabled()
                                      && ui->pushButtonRegionEdit->buttonState() == PushButtonEditState::StateEditing);
    ui->pushButton_clearAll->setVisible(ui->pushButton_clearAll->isEnabled()
                                        && ui->pushButtonRegionEdit->buttonState() == PushButtonEditState::StateEditing);
    ui->pushButtonObjectSizeEdit->setEnabled(m_widgetEnabled);
    ui->pushButtonObjectSizeReset->setEnabled(m_widgetEnabled);
    ui->pushButtonObjectSizeReset->setVisible(ui->pushButtonObjectSizeReset->isEnabled()
                                              && ui->pushButtonObjectSizeEdit->buttonState() == PushButtonEditState::StateEditing);
    ui->slider_sensitivity->setEnabled(m_widgetEnabled);
    ui->lineEdit_mintime->setEnabled(m_widgetEnabled);
    ui->pushButton_editTime->setEnabled(m_widgetEnabled);
    ui->pushButton_editAction->setEnabled(m_widgetEnabled);
    ui->widgetSize->setEnabled(m_widgetEnabled
                               && ui->pushButtonObjectSizeEdit->buttonState() == PushButtonEditState::StateEditing);
    setApplyButtonEnable(ui->checkBox_obj_left->isEnabled() || ui->checkBox_obj_remove->isEnabled());

    ui->commonVideo->setDrawViewEnable(m_widgetEnabled
                                       && (ui->pushButtonRegionEdit->buttonState() == PushButtonEditState::StateEditing
                                           || ui->pushButtonObjectSizeEdit->buttonState() == PushButtonEditState::StateEditing));
    ui->commonVideo->setDrawWidgetEnable(m_widgetEnabled
                                         && ui->pushButtonRegionEdit->buttonState() == PushButtonEditState::StateEditing);
    m_drawMultiControl->setEnabled(m_widgetEnabled
                                   && (ui->pushButtonObjectSizeEdit->buttonState() == PushButtonEditState::StateEditing
                                       || ui->pushButtonRegionEdit->buttonState() == PushButtonEditState::StateEditing));
    if (ui->pushButtonObjectSizeEdit->buttonState() != PushButtonEditState::StateEditing && !m_widgetEnabled) {
        ui->commonVideo->hideDrawView();
    }

    ui->comboBoxRegionNo->setEnabled(m_isConnected && m_isSupported);
}

bool ObjectLeftRemoved::checkObjectSize()
{
    if (ui->checkBox_obj_left->isEnabled() && (ui->checkBox_obj_left->isChecked() || ui->checkBox_obj_remove->isChecked())) {
        bool valid = ui->widgetMinLineBox->checkValid();
        valid &= ui->widgetMaxLineBox->checkValid();
        valid &= ui->lineEdit_mintime->checkValid();
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

bool ObjectLeftRemoved::isSupportPolygon()
{
    if (QString(m_object_info->polygonX).isEmpty()) {
        return false;
    } else {
        return true;
    }
}

void ObjectLeftRemoved::setEffectiveRegionVisible(bool visible)
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

void ObjectLeftRemoved::showRegionEdit()
{
    ui->commonVideo->hideDrawView();
    if (isSupportPolygon()) {
        m_drawMultiControl->show();
        ui->commonVideo->hideDrawWidget();
    } else {
        ui->commonVideo->showDrawWidget();
    }
}

void ObjectLeftRemoved::readRegion(int index)
{
    m_drawMultiControl->setRegions(m_object_info->regionInfo, index, MAX_LEN_4);
    m_drawMultiControl->clearAllEditState();
    m_drawMultiControl->setCurrentIndex(m_currentRegion);
    m_drawMultiControl->show();
    ui->comboBoxRegionNo->setCurrentIndexFromData(m_currentRegion);
}

void ObjectLeftRemoved::onLanguageChanged()
{
    ui->label_object_left->setText(GET_TEXT("SMARTEVENT/55060", "Object Left"));
    ui->label_object_removed->setText(GET_TEXT("SMARTEVENT/55061", "Object Removed"));
    ui->label_setRegion->setText(GET_TEXT("ANPR/103021", "Region"));
    ui->labelObjectSizeLimits->setText(GET_TEXT("SMARTEVENT/55063", "Object Size Limits"));
    ui->pushButtonObjectSizeReset->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->label_mintime->setText(GET_TEXT("SMARTEVENT/55062", "Min.Time(5~1800s)"));
    ui->label_effectiveTime->setText(GET_TEXT("SMARTEVENT/55015", "Effective Time"));
    ui->label_action->setText(GET_TEXT("VIDEOLOSS/50002", "Action"));
    ui->pushButton_setAll->setText(GET_TEXT("MOTION/51004", "Set All"));
    ui->pushButton_clearAll->setText(GET_TEXT("PLAYBACK/80119", "Delete All"));
    ui->pushButton_editAction->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->pushButton_editTime->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->label_sensitivity->setText(GET_TEXT("MOTION/51003", "Sensitivity"));
    ui->checkBox_obj_left->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->checkBox_obj_remove->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->labelEffectiveRegion->setText(GET_TEXT("SMARTEVENT/146001", "Effective Region"));
    ui->labelEffectiveWithPresets->setText(GET_TEXT("SMARTEVENT/146007", "Effective with presets"));
    ui->labelRegionNo->setText(GET_TEXT("SMARTEVENT/146000", "Region No."));
    ui->pushButtonEffectiveWithPreset->setText(GET_TEXT("SMARTEVENT/146008", "call"));
}

void ObjectLeftRemoved::onChannelButtonClicked(int index)
{
    if (!isVisible()) {
        emit showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        return;
    } else {
        emit hideMessage();
    }
    if (m_action) {
        m_action->clearCache();
    }
    if (m_effectiveTime) {
        m_effectiveTime->clearCache();
    }
    //
    ui->labelRegionNo->show();
    ui->comboBoxRegionNo->show();
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
    if (!m_isSupported || vcaType() != 1) {
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
    // sendMessage(REQUEST_FLAG_GET_VCA_LEFTREMOVE, (void *)&channel, sizeof(int));
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

void ObjectLeftRemoved::on_pushButton_setAll_clicked()
{
    if (isSupportPolygon()) {
        m_drawMultiControl->selectAll();
    } else {
        m_videoDraw->selectAll();
    }
}

void ObjectLeftRemoved::on_pushButton_clearAll_clicked()
{
    if (isSupportPolygon()) {
        m_drawMultiControl->clearPolygon();
    } else {
        m_videoDraw->clearAll();
    }
}

void ObjectLeftRemoved::on_pushButton_editTime_clicked()
{
    if (!m_effectiveTime) {
        m_effectiveTime = new EffectiveTimeVCA(this);
    }
    m_effectiveTime->showEffectiveTime(currentChannel(), OBJECT_LEFTREMOVE);
}

void ObjectLeftRemoved::on_pushButton_editAction_clicked()
{
    if (!m_action) {
        m_action = new ActionSmartEvent(this);
    }
    m_action->showAction(currentChannel(), OBJECT_LEFTREMOVE);
}

void ObjectLeftRemoved::onSceneObjectSizeChanged(const QRect &rc, MsQtVca::ObjectSizeType sizeType)
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

void ObjectLeftRemoved::on_radioButton_minLayout_clicked(bool checked)
{
    if (!checked) {
        ui->radioButton_minLayout->setChecked(true);
        return;
    } else {
        ui->radioButton_maxLayout->setChecked(false);
    }
    m_drawScene->showMinimumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
}

void ObjectLeftRemoved::on_radioButton_maxLayout_clicked(bool checked)
{
    if (!checked) {
        ui->radioButton_maxLayout->setChecked(true);
    } else {
        ui->radioButton_minLayout->setChecked(false);
    }
    m_drawScene->showMaximumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
}

void ObjectLeftRemoved::onLineEditMinWidthEditingFinished()
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

void ObjectLeftRemoved::onLineEditMinHeightEditingFinished()
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

void ObjectLeftRemoved::onLineEditMaxWidthEditingFinished()
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

void ObjectLeftRemoved::onLineEditMaxHeightEditingFinished()
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
void ObjectLeftRemoved::on_pushButtonRegionEdit_clicked()
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

void ObjectLeftRemoved::on_pushButtonRegionEdit_stateSet(PushButtonEditState::State state)
{
    if (ui->checkBox_obj_left->isEnabled() || ui->checkBox_obj_remove->isEnabled()) {
        switch (state) {
        case PushButtonEditState::StateFinished:
            if (isSupportPolygon()) {
                if (m_object_info->regionType != REGION_SINGLE) {
                    m_drawMultiControl->setEditable(false);
                }
                if (!m_drawMultiControl->isFinished()) {
                    m_drawMultiControl->setPolygon(m_object_info->polygonX, m_object_info->polygonY);
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
void ObjectLeftRemoved::on_pushButtonObjectSizeEdit_clicked()
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

void ObjectLeftRemoved::on_pushButtonObjectSizeEdit_stateSet(PushButtonEditState::State state)
{
    if (ui->checkBox_obj_left->isEnabled() || ui->checkBox_obj_remove->isEnabled()) {
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

void ObjectLeftRemoved::on_pushButtonObjectSizeReset_clicked()
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

void ObjectLeftRemoved::on_checkBox_obj_left_clicked(bool checked)
{
    Q_UNUSED(checked)
    onCheckBoxEnableClicked();
}

void ObjectLeftRemoved::on_checkBox_obj_remove_clicked(bool checked)
{
    Q_UNUSED(checked)
    onCheckBoxEnableClicked();
}

void ObjectLeftRemoved::onCheckBoxEnableClicked()
{
    if (m_object_info->regionType == REGION_SINGLE) {
        m_object_info->left_enable = ui->checkBox_obj_left->isChecked();
        m_object_info->remove_enable = ui->checkBox_obj_remove->isChecked();
    } else {
        m_object_info->regionInfo[m_currentRegion].enable = ui->checkBox_obj_left->isChecked();
        m_object_info->regionInfo[m_currentRegion].removeEnable = ui->checkBox_obj_remove->isChecked();
    }
    if (!ui->checkBox_obj_left->isChecked() && !ui->checkBox_obj_remove->isChecked()) {
        ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);
        ui->pushButtonRegionEdit->setButtonState(PushButtonEditState::StateFinished);
    }
    updateEnableState();
}

bool ObjectLeftRemoved::needSensitivity()
{
    return m_needSensitivity;
}

void ObjectLeftRemoved::setNeedSensitivity(bool b)
{
    if (m_needSensitivity == b) {
        return;
    }
    m_needSensitivity = b;
    emit needSensitivityChanged();
}

bool ObjectLeftRemoved::isSupportSensitivity(int channel)
{
    MsCameraVersion cameraVersion = MsCameraVersion::fromChannel(channel);
    if (cameraVersion >= MsCameraVersion(7, 74)) {
        return true;
    } else {
        return false;
    }
}

void ObjectLeftRemoved::onNeedSensitivityChanged()
{
    ui->label_sensitivity->setVisible(needSensitivity());
    ui->slider_sensitivity->setVisible(needSensitivity());
    if (ui->slider_sensitivity->range() != qMakePair(quint32(1), quint32(10))) {
        ui->slider_sensitivity->setRange(1, 10);
    }
}

void ObjectLeftRemoved::onDrawPolygonConflicted()
{
    MessageBox::information(this, GET_TEXT("SMARTEVENT/55066", "The boundaries of the area cannot intersect. Please redraw."));
    m_drawMultiControl->clearPolygon();
}

void ObjectLeftRemoved::on_comboBoxEffectiveRegion_indexSet(int index)
{
    if (index == 1 && !m_object_info->presetExist[ui->comboBoxEffectiveWithPreset->currentIndex() + 1]) {
        ui->labelEffectiveWithPresets->show();
        ui->widgetPresets->show();
        MessageBox::information(this, GET_TEXT("SMARTEVENT/146006", "Please set up the preset first."));
        if (m_object_info->regionScene == SCENE_NORMAL) {
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

    m_drawMultiControl->getRegions(m_object_info->regionInfo, m_object_info->regionScene, MAX_LEN_4);
    if (index == 0) {
        m_object_info->regionScene = SCENE_NORMAL;
        ui->labelEffectiveWithPresets->hide();
        ui->widgetPresets->hide();
    } else {
        m_object_info->regionScene = static_cast<SMART_REGION_SCENE>(ui->comboBoxEffectiveWithPreset->currentIndex() + 1);
        ui->labelEffectiveWithPresets->show();
        ui->widgetPresets->show();
        if (!m_object_info->presetExist[m_object_info->regionScene]) {
            MessageBox::information(this, GET_TEXT("SMARTEVENT/146006", "Please set up the preset first."));
            ui->comboBoxEffectiveWithPreset->setCurrentIndex(m_object_info->regionScene - 1);
            return;
        }
        if (m_ptzNeedToCall) {
            gPtzDataManager->sendPresetControl(PTZ_PRESET_GOTO, m_object_info->regionScene, 5, ui->comboBoxEffectiveWithPreset->currentData().toString());
        }
        m_ptzNeedToCall = true;
    }
    readRegion(m_object_info->regionScene);
}

void ObjectLeftRemoved::on_comboBoxEffectiveWithPreset_activated(int index)
{
    if (!m_object_info->presetExist[index + 1]) {
        MessageBox::information(this, GET_TEXT("SMARTEVENT/146006", "Please set up the preset first."));
        ui->comboBoxEffectiveWithPreset->setCurrentIndex(m_object_info->regionScene - 1);
        return;
    }
    m_drawMultiControl->getRegions(m_object_info->regionInfo, m_object_info->regionScene, MAX_LEN_4);
    m_object_info->regionScene = static_cast<SMART_REGION_SCENE>(index + 1);
    readRegion(m_object_info->regionScene);

    gPtzDataManager->sendPresetControl(PTZ_PRESET_GOTO, index + 1, 5, ui->comboBoxEffectiveWithPreset->currentData().toString());
}

void ObjectLeftRemoved::on_comboBoxRegionNo_indexSet(int index)
{
    if (ui->lineEdit_mintime->isEnabled() && !ui->lineEdit_mintime->checkValid() && index != m_currentRegion) {
        ui->comboBoxRegionNo->setCurrentIndexFromData(m_currentRegion);
        return;
    }
    ui->pushButtonRegionEdit->setButtonState(PushButtonEditState::StateFinished);
    ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);

    m_currentRegion = index;
    m_drawMultiControl->setCurrentIndex(index);
    ui->checkBox_obj_left->setChecked(m_object_info->regionInfo[m_currentRegion].enable);
    ui->checkBox_obj_remove->setChecked(m_object_info->regionInfo[m_currentRegion].removeEnable);

    updateEnableState();
}

void ObjectLeftRemoved::on_pushButtonEffectiveWithPreset_clicked()
{
    int index = ui->comboBoxEffectiveWithPreset->currentIndex();
    gPtzDataManager->sendPresetControl(PTZ_PRESET_GOTO, index + 1, 5, ui->comboBoxEffectiveWithPreset->currentData().toString());
}

void ObjectLeftRemoved::on_lineEdit_mintime_editingFinished()
{
    m_object_info->regionInfo[m_currentRegion].minTime = ui->lineEdit_mintime->text().toInt();
}
