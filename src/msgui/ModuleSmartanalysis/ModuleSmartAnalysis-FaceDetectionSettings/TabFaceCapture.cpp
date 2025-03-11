#include "TabFaceCapture.h"
#include "ui_TabFaceCapture.h"
#include "ActionFace.h"
#include "EffectiveTimeFace.h"
#include "FaceCaptureSettings.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "centralmessage.h"
#include "ptzdatamanager.h"

TabFaceCapture::TabFaceCapture(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabFaceCapture)
{
    ui->setupUi(this);
    ui->widgetButtonGroup->setCount(qMsNvr->maxChannel());
    connect(ui->widgetButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onChannelButtonClicked(int)));

    //table
    QStringList headerList;
    headerList << GET_TEXT("FACE/141006", "Shield Region ID");
    headerList << GET_TEXT("COMMON/1051", "Name");
    headerList << GET_TEXT("COMMON/1009", "Enable");
    headerList << GET_TEXT("CHANNELMANAGE/30023", "Delete");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    ui->tableView->setHeaderCheckable(false);
    ui->tableView->setItemDelegate(new ItemIconDelegate(this));

    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onTableItemClicked(int, int)));

    //delegate
    ui->tableView->setItemDelegateForColumn(ColumnDelete, new ItemButtonDelegate(QPixmap(":/common/common/delete.png"), this));
    ui->tableView->setItemDelegateForColumn(ColumnEnable, new ItemCheckedButtonDelegate(ItemCheckedButtonDelegate::ButtonCheckBox, this));

    //sort
    ui->tableView->setSortableForColumn(ColumnID, false);
    ui->tableView->setSortableForColumn(ColumnName, false);
    ui->tableView->setSortableForColumn(ColumnEnable, false);
    ui->tableView->setSortableForColumn(ColumnDelete, false);

    ui->tableView->setColumnWidth(ColumnID, 200);
    ui->tableView->setColumnWidth(ColumnName, 300);
    ui->tableView->setColumnWidth(ColumnEnable, 200);
    ui->tableView->setColumnWidth(ColumnDelete, 200);

    ui->comboBoxRegionEdit->beginEdit();
    ui->comboBoxRegionEdit->addItem(GET_TEXT("FACE/141004", "Detection Region"), 0);
    ui->comboBoxRegionEdit->addItem(GET_TEXT("FACE/141003", "Shield Region"), 1);
    ui->comboBoxRegionEdit->endEdit();

    ui->horizontalSliderMinSize->setRange(30, 800);

    //
    m_drawItem = new DrawItemFaceCapture();
    ui->commonVideo->addGraphicsItem(m_drawItem);
    connect(m_drawItem, SIGNAL(conflicted()), this, SLOT(onDrawPolygonConflicted()), Qt::QueuedConnection);

    hideMessage();

    onLanguageChanged();
}

TabFaceCapture::~TabFaceCapture()
{
    delete ui;
}

void TabFaceCapture::initializeData()
{
    ui->widgetButtonGroup->setCurrentIndex(currentChannel());
}

void TabFaceCapture::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_FACE_SUPPORT:
        ON_RESPONSE_FLAG_GET_FACE_SUPPORT(message);
        break;
    case RESPONSE_FLAG_GET_FACE_CONFIG:
        ON_RESPONSE_FLAG_GET_FACE_CONFIG(message);
        break;
    case RESPONSE_FLAG_SET_FACE_CONFIG:
        ON_RESPONSE_FLAG_SET_FACE_CONFIG(message);
        break;
    case RESPONSE_FLAG_GET_IPC_HEATMAP_SUPPORT:
        ON_RESPONSE_FLAG_GET_IPC_HEATMAP_SUPPORT(message);
        break;
    default:
        break;
    }
}

void TabFaceCapture::ON_RESPONSE_FLAG_GET_FACE_SUPPORT(MessageReceive *message)
{
    qDebug() << "====FaceCapture::ON_RESPONSE_FLAG_GET_FACE_SUPPORT, begin====";
    if (message->data) {
        m_isSupported = *(static_cast<bool *>(message->data));
    }
    m_eventLoop.exit();
}

void TabFaceCapture::ON_RESPONSE_FLAG_GET_FACE_CONFIG(MessageReceive *message)
{
    qDebug() << "====FaceCapture::ON_RESPONSE_FLAG_GET_FACE_CONFIG, begin====";
    memset(&m_faceConfig, 0, sizeof(MsFaceConfig));
    struct MsFaceConfig *faceConfig = static_cast<MsFaceConfig *>(message->data);
    if (!faceConfig) {
        qWarning() << "data is null.";
        return;
    }
    memcpy(&m_faceConfig, faceConfig, sizeof(MsFaceConfig));
    qDebug() << QString("FaceCapturePage::ON_RESPONSE_FLAG_GET_FACE_CONFIG, channel: %1, enable: %2, min size: %3, captureMode: %4,mutuallyExclusive: %5")
                    .arg(m_faceConfig.chnId)
                    .arg(m_faceConfig.enable)
                    .arg(m_faceConfig.minPixel)
                    .arg(m_faceConfig.captureMode)
                    .arg(m_faceConfig.mutuallyExclusive);
    ui->checkBoxEnable->setChecked(m_faceConfig.enable);
    m_hasEnable = m_faceConfig.enable;
    ui->horizontalSliderMinSize->setValue(m_faceConfig.minPixel);
    m_drawItem->setMinSize(m_faceConfig.minPixel);
    m_drawItem->setDetection(m_faceConfig.detection.polygonX, m_faceConfig.detection.polygonY);
    QString strEmpty = "-1:-1:-1:-1:-1:-1:-1:-1:-1:-1:";
    for (int i = 0; i < MAX_FACE_SHIELD; ++i) {
        if (QString(m_faceConfig.shield[i].region.polygonX).isEmpty() || QString(m_faceConfig.shield[i].region.polygonY).isEmpty()) {
            snprintf(m_faceConfig.shield[i].region.polygonX, sizeof(m_faceConfig.shield[i].region.polygonX), "%s", strEmpty.toStdString().c_str());
            snprintf(m_faceConfig.shield[i].region.polygonY, sizeof(m_faceConfig.shield[i].region.polygonY), "%s", strEmpty.toStdString().c_str());
        }
        m_drawItem->addShield(m_faceConfig.shield[i].region, i);
    }
    m_drawItem->refreshstack();
    on_checkBoxEnable_stateChanged(0);
    on_comboBoxRegionEdit_currentIndexChanged(0);
    refreshTable();
    m_eventLoop.exit();
}

void TabFaceCapture::ON_RESPONSE_FLAG_SET_FACE_CONFIG(MessageReceive *message)
{
    Q_UNUSED(message)
}

void TabFaceCapture::ON_RESPONSE_FLAG_GET_IPC_HEATMAP_SUPPORT(MessageReceive *message)
{
    int result = *((int *)message->data);
    m_eventLoop.exit(result);
}

void TabFaceCapture::hideMessage()
{
    ui->widgetMessage->hide();
}

void TabFaceCapture::showNotConnectedMessage()
{
    //closeWait();
    ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
}

void TabFaceCapture::showNotSupportMessage()
{
    //closeWait();
    ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
}

void TabFaceCapture::updateEnableState()
{
    ui->checkBoxEnable->setEnabled(m_isConnected && m_isSupported);

    bool isEnable = ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked();
    m_drawItem->setItemEnable(isEnable);
    ui->horizontalSliderMinSize->setEnabled(isEnable);
    ui->comboBoxRegionEdit->setEnabled(isEnable);
    ui->pushButtonAdd->setEnabled(isEnable && ui->tableView->rowCount() < MAX_FACE_SHIELD);
    ui->pushButtonClear->setEnabled(isEnable);
    ui->pushButtonDeleteAll->setEnabled(isEnable);
    ui->pushButtonFaceCaptureSetting->setEnabled(isEnable);
    ui->pushButtonEffective->setEnabled(isEnable);
    ui->pushButtonAction->setEnabled(isEnable);
    ui->pushButtonSetAll->setEnabled(isEnable);
    ui->pushButtonRegionDelete->setEnabled(isEnable);
    ui->pushButtonApply->setEnabled(m_isConnected && m_isSupported);
    setTableEnabledStatus();
}

void TabFaceCapture::clearSetting()
{
    ui->checkBoxEnable->setChecked(false);
    m_drawItem->hide();
}

void TabFaceCapture::refreshTable()
{
    int row = 0;
    ui->tableView->clearContent();
    for (int i = 0; i < MAX_FACE_SHIELD; i++) {
        MS_FACE_SHIELD &shield = m_faceConfig.shield[i];
        QStringList xs = QString(shield.region.polygonX).split(":", QString::SkipEmptyParts);
        QStringList ys = QString(shield.region.polygonY).split(":", QString::SkipEmptyParts);
        if (xs.isEmpty() || ys.isEmpty()) {
            return;
        }
        int x = xs.at(0).toInt();
        int y = ys.at(0).toInt();
        if (x < 0 || y < 0) {
            continue;
        }
        ui->tableView->setItemIntValue(row, ColumnID, i + 1);
        QString maskName = QString("Shield Region%1").arg(i + 1);
        ui->tableView->setItemText(row, ColumnName, maskName);
        ui->tableView->setItemData(row, ColumnEnable, shield.enable, ItemCheckedRole);

        row++;
    }
    ui->pushButtonAdd->setEnabled(ui->checkBoxEnable->isChecked()
                                  && ui->tableView->rowCount() < MAX_FACE_SHIELD);
    setTableEnabledStatus();
}

void TabFaceCapture::setTableEnabledStatus()
{
    int rowCount = ui->tableView->rowCount();
    bool enabled = ui->checkBoxEnable->isChecked();
    for (int i = 0; i < rowCount; i++) {
        ui->tableView->setRowEnable(i, enabled);
    }
}

bool TabFaceCapture::isPTZCamera()
{
    if (!qMsNvr->isMsCamera(m_currentChannel)) {
        return false;
    }

    //showWait();
    gPtzDataManager->beginGetData(m_currentChannel);
    gPtzDataManager->waitForGetCameraModelType();
    if (qMsNvr->isFisheye(m_currentChannel)) {
        //closeWait();
        return false;
    }
    gPtzDataManager->waitForGetCameraModelInfo();
    gPtzDataManager->waitForGetPtzSupport();
    if (!gPtzDataManager->isPtzSupport()) {
        //closeWait();
        return false;
    }
    if (!gPtzDataManager->isPresetEnable()) {
        //closeWait();
        return false;
    }
    //closeWait();
    return true;
}

void TabFaceCapture::onLanguageChanged()
{
    ui->checkBoxEnable->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->pushButtonDeleteAll->setText(GET_TEXT("PLAYBACK/80119", "Delete All"));
    ui->pushButtonAdd->setText(GET_TEXT("COMMON/1000", "Add"));
    ui->pushButtonClear->setText(GET_TEXT("PTZCONFIG/36039", "Clear"));

    ui->pushButtonFaceCaptureSetting->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->pushButtonEffective->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->pushButtonAction->setText(GET_TEXT("COMMON/1019", "Edit"));

    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButtonApply->setText(GET_TEXT("COMMON/1003", "Apply"));

    ui->pushButtonSetAll->setText(GET_TEXT("MOTION/51004", "Set All"));
    ui->pushButtonRegionDelete->setText(GET_TEXT("PLAYBACK/80119", "Delete All"));

    ui->labelFaceDetectionEnable->setText(GET_TEXT("FACE/141000", "Face Detection"));
    ui->labelMinDetectionSize->setText(GET_TEXT("FACE/141001", "Min. Detection Size"));
    ui->labelRegionEdit->setText(GET_TEXT("FACE/141002", "Region Edit"));
    ui->labelfaceCaptureSeetings->setText(GET_TEXT("FACE/141007", "Face Capture Settings"));
    ui->labelEffectiveTime->setText(GET_TEXT("SMARTEVENT/55015", "Effective Time"));
    ui->labelAction->setText(GET_TEXT("VIDEOLOSS/50002", "Action"));
    ui->labelRegionOperation->setText(GET_TEXT("FACE/141005", "Edit Operation"));
    ui->labelEditOperation->setText(GET_TEXT("FACE/141005", "Edit Operation"));
}

void TabFaceCapture::onChannelButtonClicked(int index)
{
    if (!isVisible()) {
        return;
    }
    //
    m_currentChannel = index;
    setCurrentChannel(m_currentChannel);
    ui->commonVideo->playVideo(m_currentChannel);
    m_drawItem->show();
    m_drawItem->init();
    ui->comboBoxRegionEdit->setCurrentIndex(0);
    m_isConnected = false;
    m_isSupported = false;
    ui->tableView->clearContent();

    //检查通道是否连接
    m_isConnected = LiveView::instance()->isChannelConnected(m_currentChannel);
    if (!m_isConnected) {
        showNotConnectedMessage();
        clearSetting();
        updateEnableState();
        return;
    }
    //showWait();
    sendMessage(REQUEST_FLAG_GET_FACE_SUPPORT, &m_currentChannel, sizeof(int));
    //m_eventLoop.exec();
    if (!m_isSupported) {
        showNotSupportMessage();
        clearSetting();
        updateEnableState();
        //closeWait();
        return;
    }
    //    m_isSupported = true;
    hideMessage();
    sendMessage(REQUEST_FLAG_GET_FACE_CONFIG, &m_currentChannel, sizeof(int));
    //m_eventLoop.exec();

    if (m_action) {
        m_action->clearCache();
    }
    if (m_effectiveTime) {
        m_effectiveTime->clearCache();
    }
    //closeWait();
}

void TabFaceCapture::onTableItemClicked(int row, int column)
{
    if (ui->checkBoxEnable->isChecked()) {
        int enable = 0;
        int id = ui->tableView->itemText(row, ColumnID).toInt() - 1;
        switch (column) {
        case ColumnEnable:
            if (ui->checkBoxEnable->isChecked()) {
                enable = ui->tableView->itemData(row, column, ItemCheckedRole).toInt();
                if (enable) {
                    ui->tableView->setItemData(row, column, false, ItemCheckedRole);
                } else {
                    ui->tableView->setItemData(row, column, true, ItemCheckedRole);
                }
                m_faceConfig.shield[id].enable = ui->tableView->itemData(row, ColumnEnable, ItemCheckedRole).toInt();
            }
            break;
        case ColumnDelete:
            enable = ui->checkBoxEnable->isChecked();
            if (!enable) {
                break;
            }
            MS_FACE_SHIELD &shield = m_faceConfig.shield[id];
            QString strEmpty = "-1:-1:-1:-1:-1:-1:-1:-1:-1:-1:";
            snprintf(shield.region.polygonX, sizeof(shield.region.polygonX), "%s", strEmpty.toStdString().c_str());
            snprintf(shield.region.polygonY, sizeof(shield.region.polygonY), "%s", strEmpty.toStdString().c_str());
            shield.enable = false;
            ui->tableView->removeRow(row);
            m_drawItem->clear(id);

            ui->pushButtonAdd->setEnabled(ui->checkBoxEnable->isChecked() && ui->tableView->rowCount() < MAX_FACE_SHIELD);
            break;
        }
    }
}

void TabFaceCapture::on_comboBoxRegionEdit_currentIndexChanged(int index)
{
    bool isEditOperationVisible = static_cast<bool>(index);
    if (isEditOperationVisible) {
        ui->widgetRegionOperation->hide();
        ui->labelRegionOperation->hide();
        ui->widgetEditOperation->show();
        ui->labelEditOperation->show();
        if (m_drawItem) {
            m_drawItem->setFaceMode(DrawItemFaceCapture::ShieldMode);
        }

    } else {
        ui->widgetEditOperation->hide();
        ui->labelEditOperation->hide();
        ui->widgetRegionOperation->show();
        ui->labelRegionOperation->show();
        if (m_drawItem) {
            m_drawItem->setFaceMode(DrawItemFaceCapture::DetectionMode);
        }
    }
}

void TabFaceCapture::on_pushButtonApply_clicked()
{
    return ;
    if (!m_hasEnable && ui->checkBoxEnable->isChecked()) {
        MessageBox::information(this, GET_TEXT("FACE/141030", "Save failed. Reach the maximum channel of ANPR and Face Detection supported."));
        return;
    }
    if (ui->checkBoxEnable->isChecked() && m_faceConfig.mutuallyExclusive == 1) {
        int result = 0;
        sendMessage(REQUEST_FLAG_GET_IPC_HEATMAP_SUPPORT, &m_currentChannel, sizeof(int));
        int heatmapSupport = m_eventLoop.exec();
        if (qMsNvr->isNT(m_currentChannel) && heatmapSupport) {
            if (isPTZCamera()) {
                result = MessageBox::question(this, GET_TEXT("FACE/165000", "VCA/People Counting/Regional People Counting/Corridor Mode/Auto Tracking/Heat Map will be disabled,continue?"));
            } else {
                result = MessageBox::question(this, GET_TEXT("FACE/165001", "VCA/People Counting/Regional People Counting/Corridor Mode/Heat Map will be disabled,continue?"));
            }
        } else {
            if (isPTZCamera()) {
                result = MessageBox::question(this, GET_TEXT("FACE/141054", "VCA/People Counting/Regional People Counting/Corridor Mode/Auto Tracking will be disabled, continue?"));
            } else {
                result = MessageBox::question(this, GET_TEXT("FACE/141052", "VCA/People Counting/Regional People Counting/Corridor Mode will be disabled, continue?"));
            }
        }

        if (result == MessageBox::Yes) {
            m_faceConfig.mutuallyExclusive = 0;
        } else {
            return;
        }
    }
    if (m_effectiveTime) {
        m_effectiveTime->saveEffectiveTime();
    }
    if (m_action) {
        m_action->saveAction();
    }
    sendMessageOnly(REQUEST_FLAG_SET_FACE_EVENT, &m_currentChannel, sizeof(int));
    //showWait();
    m_faceConfig.enable = ui->checkBoxEnable->isChecked();
    m_drawItem->getDetection(&m_faceConfig.detection);
    m_drawItem->updataShield(m_faceConfig.shield);
    m_faceConfig.minPixel = static_cast<int>(ui->horizontalSliderMinSize->value());
    sendMessage(REQUEST_FLAG_SET_FACE_CONFIG, &m_faceConfig, sizeof(MsFaceConfig));

    //显示等待，给ipc时间进入重启
    QEventLoop eventLoop;
    QTimer::singleShot(2000, &eventLoop, SLOT(quit()));
    eventLoop.exec();
    initializeData();
    //closeWait();
}

void TabFaceCapture::on_pushButtonBack_clicked()
{
    back();
}

void TabFaceCapture::on_checkBoxEnable_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    m_drawItem->clearSelect();
    updateEnableState();
}

void TabFaceCapture::on_pushButtonFaceCaptureSetting_clicked()
{
    if (!m_faceSetting) {
        m_faceSetting = new FaceCaptureSettings(this);
    }
    m_faceSetting->initializeData(&m_faceConfig);
    int result = m_faceSetting->exec();
    if (result == QDialog::Accepted) {
        memcpy(&m_faceConfig, m_faceSetting->faceConfig(), sizeof(MsFaceConfig));
    }
    ui->pushButtonFaceCaptureSetting->clearUnderMouse();
}
void TabFaceCapture::on_horizontalSliderMinSize_valueChanged(int value)
{
    m_drawItem->setMinSize(value);
}

void TabFaceCapture::on_horizontalSliderMinSize_sliderMoved(int position)
{
    m_drawItem->setMinSize(position);
}

void TabFaceCapture::on_pushButtonSetAll_clicked()
{
    m_drawItem->detectionSetAll();
}

void TabFaceCapture::on_pushButtonRegionDelete_clicked()
{
    m_drawItem->clear();
}

void TabFaceCapture::on_pushButtonAdd_clicked()
{
    m_drawItem->getShield(m_faceConfig.shield);
    refreshTable();
}

void TabFaceCapture::on_pushButtonClear_clicked()
{
    int index = m_drawItem->clear();
    if (index > -1) {
        MS_FACE_SHIELD &shield = m_faceConfig.shield[index];
        QString strEmpty = "-1:-1:-1:-1:-1:-1:-1:-1:-1:-1:";
        snprintf(shield.region.polygonX, sizeof(shield.region.polygonX), "%s", strEmpty.toStdString().c_str());
        snprintf(shield.region.polygonY, sizeof(shield.region.polygonY), "%s", strEmpty.toStdString().c_str());
        shield.enable = false;
        for (int i = 0; i < ui->tableView->rowCount(); i++) {
            if (ui->tableView->itemIntValue(i, ColumnID) == index + 1) {
                ui->tableView->removeRow(i);
                break;
            }
        }
        ui->pushButtonAdd->setEnabled(ui->checkBoxEnable->isChecked() && ui->tableView->rowCount() < MAX_FACE_SHIELD);
    }
}

void TabFaceCapture::on_pushButtonDeleteAll_clicked()
{
    QString strEmpty = "-1:-1:-1:-1:-1:-1:-1:-1:-1:-1:";
    for (int i = 0; i < MAX_FACE_SHIELD; ++i) {
        snprintf(m_faceConfig.shield[i].region.polygonX, sizeof(m_faceConfig.shield[i].region.polygonX), "%s", strEmpty.toStdString().c_str());
        snprintf(m_faceConfig.shield[i].region.polygonY, sizeof(m_faceConfig.shield[i].region.polygonY), "%s", strEmpty.toStdString().c_str());
        m_faceConfig.shield[i].enable = false;
    }

    m_drawItem->clearAll();
    on_pushButtonAdd_clicked();
}

void TabFaceCapture::onDrawPolygonConflicted()
{
    MessageBox::information(this, GET_TEXT("SMARTEVENT/55066", "The boundaries of the area cannot intersect. Please redraw."));
    m_drawItem->clear();
}

void TabFaceCapture::on_pushButtonEffective_clicked()
{
    if (!m_effectiveTime) {
        m_effectiveTime = new EffectiveTimeFace(this);
    }
    m_effectiveTime->showEffectiveTime(m_currentChannel);
    m_effectiveTime->show();
    ui->pushButtonEffective->clearUnderMouse();
}

void TabFaceCapture::on_pushButtonAction_clicked()
{
    if (!m_action) {
        m_action = new ActionFace(this);
    }
    m_action->showAction(m_currentChannel);
    ui->pushButtonAction->clearUnderMouse();
}
