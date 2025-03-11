#include "TabPtzPrivacyMask.h"
#include "ui_TabPtzPrivacyMask.h"
#include "DrawScenePtzMask.h"
#include "DrawView.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "channelcopydialog.h"
#include "ptzmaskedit.h"
#include <QScopedValueRollback>

TabPtzPrivacyMask::TabPtzPrivacyMask(QWidget *parent)
    : PtzBasePage(parent)
    , ui(new Ui::PtzPrivacyMaskPage)
{
    ui->setupUi(this);

    ui->channelGroup->setCount(qMsNvr->maxChannel());
    connect(ui->channelGroup, SIGNAL(buttonClicked(int)), this, SLOT(onChannelGroupClicked(int)));

    connect(ui->pushButton_add, SIGNAL(clicked()), this, SLOT(onPushButtonAddClicked()));

    //
    QStringList headerList;
    headerList << GET_TEXT("DISKMANAGE/72022", "ID");
    headerList << GET_TEXT("COMMON/1051", "Name");
    headerList << GET_TEXT("COMMON/1052", "Type");
    headerList << GET_TEXT("COMMON/1009", "Enable");
    headerList << GET_TEXT("PTZCONFIG/36053", "Active Zoom Ratio");
    headerList << GET_TEXT("COMMON/1019", "Edit");
    headerList << GET_TEXT("IMAGE/120001", "Area Edit");
    headerList << GET_TEXT("CHANNELMANAGE/30023", "Delete");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    ui->tableView->setHeaderCheckable(false);
    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onTableItemClicked(int, int)));
    //delegate
    ui->tableView->setItemDelegateForColumn(ColumnEnable, new ItemCheckedButtonDelegate(ItemCheckedButtonDelegate::ButtonCheckBox, this));
    ui->tableView->setItemDelegateForColumn(ColumnEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit.png"), this));
    ui->tableView->setItemDelegateForColumn(ColumnAreaEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit.png"), this));
    ui->tableView->setItemDelegateForColumn(ColumnDelete, new ItemButtonDelegate(QPixmap(":/common/common/delete.png"), this));

    ui->tableView->setSortingEnabled(false);

    ui->tableView->setColumnWidth(ColumnID, 100);
    ui->tableView->setColumnWidth(ColumnName, 175);
    ui->tableView->setColumnWidth(ColumnType, 100);
    ui->tableView->setColumnWidth(ColumnEnable, 100);
    ui->tableView->setColumnWidth(ColumnRatio, 175);
    ui->tableView->setColumnWidth(ColumnEdit, 100);
    ui->tableView->setColumnWidth(ColumnAreaEdit, 100);
    ui->tableView->setColumnWidth(ColumnDelete, 100);

    //
    m_drawItemMask = new DrawItemPTZMaskControl(nullptr);
    ui->video->addGraphicsItem(m_drawItemMask);

    ui->comboBoxRegionType->clear();
    ui->comboBoxRegionType->beginEdit();
    ui->comboBoxRegionType->addItem(GET_TEXT("IMAGE/120003", "Mask"), 0);
    ui->comboBoxRegionType->addItem(GET_TEXT("IMAGE/120004", "Mosaic"), 1);
    ui->comboBoxRegionType->endEdit();

    ui->comboBoxMaskColor->clear();
    ui->comboBoxMaskColor->beginEdit();
    ui->comboBoxMaskColor->addItem(GET_TEXT("IMAGE/37322", "White"), 0);
    ui->comboBoxMaskColor->addItem(GET_TEXT("IMAGE/37323", "Black"), 1);
    ui->comboBoxMaskColor->addItem(GET_TEXT("IMAGE/37324", "Blue"), 2);
    ui->comboBoxMaskColor->addItem(GET_TEXT("IMAGE/37403", "Yellow"), 3);
    ui->comboBoxMaskColor->addItem(GET_TEXT("IMAGE/37404", "Green"), 4);
    ui->comboBoxMaskColor->addItem(GET_TEXT("IMAGE/37405", "Brown"), 5);
    ui->comboBoxMaskColor->addItem(GET_TEXT("IMAGE/37406", "Red"), 6);
    ui->comboBoxMaskColor->addItem(GET_TEXT("IMAGE/37407", "Purple"), 7);
    ui->comboBoxMaskColor->endEdit();

    ui->labelMaskColor->hide();
    ui->labelRegionType->hide();
    ui->comboBoxMaskColor->hide();
    ui->comboBoxRegionType->hide();

    onLanguageChanged();
}

TabPtzPrivacyMask::~TabPtzPrivacyMask()
{
    m_drawItemMask->clearAll();
    m_maskList.clear();
    delete ui;
}

void TabPtzPrivacyMask::initializeData()
{
    m_drawItemMask->clearAll();
    m_drawItemMask->init();
    m_maskList.clear();
    ui->channelGroup->setCurrentIndex(currentChannel());
}

void TabPtzPrivacyMask::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_PRIVACY_MASK:
        ON_RESPONSE_FLAG_GET_PRIVACY_MASK(message);
        break;
    case RESPONSE_FLAG_SET_PRIVACY_MASK:
        ON_RESPONSE_FLAG_SET_PRIVACY_MASK(message);
        break;
    case RESPONSE_FLAG_DELETE_PRIVACY_MASK:
        ON_RESPONSE_FLAG_DELETE_PRIVACY_MASK(message);
        break;
    case RESPONSE_FLAG_GET_IPC_DIGITPOS_ZOOM:
        ON_RESPONSE_FLAG_GET_IPC_DIGITPOS_ZOOM(message);
        break;
    }
}

void TabPtzPrivacyMask::ON_RESPONSE_FLAG_GET_IPC_DIGITPOS_ZOOM(MessageReceive *message)
{
    //closeWait();
    ms_digitpos_zoom_state *data = (ms_digitpos_zoom_state *)message->data;
    if (!data) {
        qWarning() << "data is null, file: " << __FILE__ << " line: " << __LINE__;
        return;
    }

    if (data->state) {
        ShowMessageBox(GET_TEXT("IMAGE/37415", "Privacy Mask settings are not supported at this magnification."));
        m_drawItemMask->clear();
        return;
    }

    if (m_isReadyAdd) {
        addPrivacyMask();
        m_isReadyAdd = false;
    }
}

void TabPtzPrivacyMask::ON_RESPONSE_FLAG_GET_PRIVACY_MASK(MessageReceive *message)
{
    m_eventLoop.exit();
    memset(&m_editingArea, -1, sizeof(mask_area_ex));

    struct resp_privacy_mask *privacy_mask = (struct resp_privacy_mask *)message->data;
    memset(&m_privacy_mask, 0, sizeof(resp_privacy_mask));
    m_clearMask = 0;
    if (privacy_mask) {
        ui->checkBox_enable->setChecked(privacy_mask->enable);
        memcpy(&m_privacy_mask, privacy_mask, sizeof(resp_privacy_mask));

        STRUCT(resp_privacy_mask, privacy_mask,
               FIELD(int, id);
               FIELD(int, enable);
               FIELD(int, num);
               FIELD(int, cgiSdkFlag);
               FIELD(char *, sdkversion);
               FIELD(int, maxPtzZoomRatio);
               FIELD(int, areaMaskEnable);
               FIELD(char *, colorList);
               FIELD(int, system_mosaic_support);
               FIELD(int, imaging_settings_mosaic_num);
               FIELD(int, delMask);
               FIELD(int, colorSupport))

        if (m_privacy_mask.num < MAX_MASK_AREA_NUM || qMsNvr->isNT(currentChannel())) {
            m_maskNUM = 8;
        } else if (m_privacy_mask.system_mosaic_support) {
            m_maskNUM = MAX_MASK_AREA_NUM + MAX_MOSAIC_NUM;
        } else {
            m_maskNUM = MAX_MASK_AREA_NUM;
        }
        for (int i = 0; i < m_maskNUM; ++i) {
            const mask_area_ex &area = m_privacy_mask.area[i];
            qDebug() << QString("id: %1, name: %2, type: %3, enable: %4, ratio: %5, start_x: %6, start_y: %7, area_width: %8, area_height: %9")
                            .arg(area.area_id)
                            .arg(area.name)
                            .arg(area.fill_color)
                            .arg(area.enable)
                            .arg(area.ratio)
                            .arg(area.start_x)
                            .arg(area.start_y)
                            .arg(area.area_width)
                            .arg(area.area_height);
        }
        isSupportMosaic();
        if (m_maskNUM <= MAX_MASK_AREA_NUM) {
            if (qMsNvr->isNT(currentChannel())) {
                ui->label_note->setText(GET_TEXT("IMAGE/162000", "Note: Support up to 8 areas."));
            } else {
                ui->label_note->setText(GET_TEXT("IMAGE/120000", "Note: Support up to %1 Privacy Mask areas.").arg(m_maskNUM));
            }
        } else {
            ui->label_note->setText(GET_TEXT("IMAGE/120007", "Note: Support up to 24 mask and 4 mosaic areas."));
        }
        m_drawItemMask->clearAll();
        for (int i = 0; i < m_maskNUM; ++i) {
            if (privacy_mask->area[i].area_width == 0 && privacy_mask->area[i].area_height == 0) {
                snprintf(m_privacy_mask.area[i].name, sizeof(m_privacy_mask.area[i].name), "%s", QString("PTZ Privacy Mask %1").arg(m_privacy_mask.area[i].area_id + 1).toStdString().c_str());
                continue;
            }
            const mask_area_ex &area = privacy_mask->area[i];
            m_drawItemMask->addMask(area);
            m_maskList.append(area.area_id);
        }
        m_drawItemMask->refreshstack();
        m_drawItemMask->hideArea();

        m_drawItemMask->setMaxItemCount(m_maskNUM);

        showMaskTable();
    }
}

int TabPtzPrivacyMask::waitForSaveMask(int channel)
{
    m_clearMask |= m_drawItemMask->updateMask(m_privacy_mask.area);
    m_privacy_mask.enable = ui->checkBox_enable->isChecked();
    resp_privacy_mask set_mask;
    memcpy(&set_mask, &m_privacy_mask, sizeof(resp_privacy_mask));
    set_mask.id = channel;
    set_mask.areaMaskEnable = 0xfffffff;
    set_mask.delMask = m_clearMask;
    set_mask.cgiSdkFlag = 1;
    for (int i = 0; i < m_maskNUM; ++i) {
        const mask_area_ex &area = m_privacy_mask.area[i];
        qDebug() << QString("id: %1, name: %2, type: %3, enable: %4, ratio: %5, start_x: %6, start_y: %7, area_width: %8, area_height: %9")
                        .arg(area.area_id)
                        .arg(area.name)
                        .arg(area.fill_color)
                        .arg(area.enable)
                        .arg(area.ratio)
                        .arg(area.start_x)
                        .arg(area.start_y)
                        .arg(area.area_width)
                        .arg(area.area_height);
    }
    sendMessage(REQUEST_FLAG_SET_PRIVACY_MASK, &set_mask, sizeof(struct resp_privacy_mask));

    return 0;
}

void TabPtzPrivacyMask::ON_RESPONSE_FLAG_SET_PRIVACY_MASK(MessageReceive *message)
{
    int res = 0;
    if (message->data) {
        res = *(int *)message->data;
    }
    m_eventLoop.exit(res);
}

/**
 * @brief PtzPrivacyMaskPage::waitForDeleteMask
 * @param channel
 * @param id: 1-8
 * @return
 */
int TabPtzPrivacyMask::waitForDeleteMask(int channel, int id)
{
    delete_arae_params delete_arae;
    delete_arae.chan_id = channel;
    delete_arae.area_id = id;
    Q_UNUSED(delete_arae)

    return 0;
}

void TabPtzPrivacyMask::ON_RESPONSE_FLAG_DELETE_PRIVACY_MASK(MessageReceive *message)
{
    int res = 0;
    if (message->data) {
        res = *(int *)message->data;
    }
    m_eventLoop.exit(res);
}

int TabPtzPrivacyMask::waitForGetPrivacyMask()
{
    int channel = currentChannel();

    Q_UNUSED(channel)
    return 0;
}

void TabPtzPrivacyMask::showMaskTable()
{
    int row = 0;
    ui->tableView->clearContent();
    for (int i = 0; i < m_maskNUM; i++) {
        mask_area_ex &area = m_privacy_mask.area[i];
        if (area.area_height == 0 && area.area_width == 0) {
            continue;
        }

        ui->tableView->setItemIntValue(row, ColumnID, area.area_id + 1);
        QString maskName(area.name);
        if (maskName.isEmpty()) {
            maskName = QString("%1 %2").arg("PTZ Privacy Mask").arg(area.area_id + 1);
            snprintf(area.name, sizeof(area.name), "%s", maskName.toStdString().c_str());
        }
        ui->tableView->setItemText(row, ColumnName, maskName);
        ui->tableView->setItemText(row, ColumnType, typeString(area.fill_color));
        ui->tableView->setItemChecked(row, ColumnEnable, area.enable);
        ui->tableView->setItemIntValue(row, ColumnRatio, area.ratio);

        row++;
    }

    if (ui->tableView->rowCount() >= m_maskNUM) {
        m_drawItemMask->setEnabled(false);
        ui->pushButton_add->setEnabled(false);
    } else {
        if (ui->checkBox_enable->isChecked()) {
            m_drawItemMask->setEnabled(true);
            ui->pushButton_add->setEnabled(true);
        }
    }
    bool enabled = ui->checkBox_enable->isChecked() && ui->checkBox_enable->isEnabled();
    int rowCount = ui->tableView->rowCount();
    for (int i = 0; i < rowCount; i++) {
        ui->tableView->setRowEnable(i, enabled);
    }
}

void TabPtzPrivacyMask::onLanguageChanged()
{
    ui->ptzControlPanel->onLanguageChanged();

    ui->label_enable->setText(GET_TEXT("PTZCONFIG/36051", "PTZ Privacy Mask"));
    ui->checkBox_enable->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->label_region->setText(GET_TEXT("PTZCONFIG/36056", "Region"));
    ui->pushButton_add->setText(GET_TEXT("COMMON/1000", "Add"));
    ui->pushButton_clear->setText(GET_TEXT("ANPR/103027", "Clear"));
    ui->pushButton_clearAll->setText(GET_TEXT("PLAYBACK/80119", "Delete All"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->labelRegionType->setText(GET_TEXT("IMAGE/37220", "Region Type"));
    ui->labelMaskColor->setText(GET_TEXT("IMAGE/120005", "Mask Color"));
}

void TabPtzPrivacyMask::setSettingEnable(bool enable)
{
    m_drawItemMask->setEnabled(enable);
    ui->pushButton_add->setEnabled(enable);
    ui->pushButton_clear->setEnabled(enable);
    ui->pushButton_clearAll->setEnabled(enable);
    ui->comboBoxRegionType->setEnabled(enable);
    ui->comboBoxMaskColor->setEnabled(enable && ui->comboBoxRegionType->currentIntData() == 0);
    if (ui->checkBox_enable->isEnabled()) {
        ui->label_note->show();
    } else {
        ui->label_note->hide();
    }
    bool enabled = ui->checkBox_enable->isChecked() && ui->checkBox_enable->isEnabled();
    int rowCount = ui->tableView->rowCount();
    for (int i = 0; i < rowCount; i++) {
        ui->tableView->setRowEnable(i, enabled);
    }
}

void TabPtzPrivacyMask::setFunctionEnable(bool enable)
{
    ui->ptzControlPanel->setEnabled(enable);
    ui->widget_maskContent->setEnabled(enable);
    ui->checkBox_enable->setEnabled(enable);
    ui->pushButton_apply->setEnabled(enable);
    setSettingEnable(false);
}

void TabPtzPrivacyMask::clearSetting()
{
    ui->ptzControlPanel->clearPreset();

    m_drawItemMask->clearAll();

    ui->checkBox_enable->setChecked(false);
    ui->tableView->clearContent();
    m_privacy_mask.system_mosaic_support = true;
    isSupportMosaic();
}

QString TabPtzPrivacyMask::typeString(int type)
{
    QString text;
    switch (type) {
    case 0:
        text = GET_TEXT("IMAGE/37322", "White");
        break;
    case 1:
        text = GET_TEXT("IMAGE/37323", "Black");
        break;
    case 2:
        text = GET_TEXT("IMAGE/37324", "Blue");
        break;
    case 3:
        text = GET_TEXT("IMAGE/37403", "Yellow");
        break;
    case 4:
        text = GET_TEXT("IMAGE/37404", "Green");
        break;
    case 5:
        text = GET_TEXT("IMAGE/37405", "Brown");
        break;
    case 6:
        text = GET_TEXT("IMAGE/37406", "Red");
        break;
    case 7:
        text = GET_TEXT("IMAGE/37407", "Purple");
        break;
    case 8:
        text = GET_TEXT("IMAGE/120004", "Mosaic");
    }
    return text;
}

void TabPtzPrivacyMask::onChannelGroupClicked(int channel)
{
    setCurrentChannel(channel);
    ui->video->playVideo(channel);

    clearSetting();

    ui->widgetMessage->hideMessage();

    if (!LiveView::instance()->isChannelConnected(channel)) {
        setFunctionEnable(false);
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
        return;
    }

    if (!qMsNvr->isMsCamera(channel)) {
        setFunctionEnable(false);
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        return;
    }

    //showWait();
    gPtzDataManager->beginGetData(channel);
    gPtzDataManager->waitForGetCameraModelType();
    if (qMsNvr->isFisheye(channel)) {
        //closeWait();
        setFunctionEnable(false);
        ui->widgetMessage->showMessage(GET_TEXT("PTZCONFIG/36059", "This channel just support Privacy Mask, please configure on Image interface."));
        return;
    }
    gPtzDataManager->waitForGetCameraModelInfo();
    gPtzDataManager->waitForGetPtzSupport();
    if (!gPtzDataManager->isPtzSupport()) {
        //closeWait();
        setFunctionEnable(false);
        ui->widgetMessage->showMessage(GET_TEXT("PTZCONFIG/36059", "This channel just support Privacy Mask, please configure on Image interface."));
        return;
    }
    if (!gPtzDataManager->isPresetEnable()) {
        //closeWait();
        setFunctionEnable(false);
        ui->widgetMessage->showMessage(GET_TEXT("PTZCONFIG/36059", "This channel just support Privacy Mask, please configure on Image interface."));
        return;
    }

    setFunctionEnable(true);

    gPtzDataManager->waitForGetPtzOvfInfo();
    resp_ptz_ovf_info *ptz_ovf_info = gPtzDataManager->ptzOvfInfo();
    ui->ptzControlPanel->showPresetData(ptz_ovf_info->preset, gPtzDataManager->maxPresetCount(channel));

    gPtzDataManager->waitForGetPtzSpeed();
    ui->ptzControlPanel->editSpeedValue(gPtzDataManager->ptzSpeed());

    gPtzDataManager->waitForGetAutoScan();
    ui->ptzControlPanel->setAutoScanChecked(gPtzDataManager->autoScan());

    waitForGetPrivacyMask();

    //closeWait();
}

void TabPtzPrivacyMask::onTableItemClicked(int row, int column)
{
    if (ui->checkBox_enable->isChecked()) {
        int index = ui->tableView->itemIntValue(row, ColumnID) - 1;
        mask_area_ex &area = m_privacy_mask.area[index];
        switch (column) {
        case ColumnEnable: {
            bool enable = ui->tableView->isItemChecked(row, ColumnEnable);
            ui->tableView->setItemChecked(row, ColumnEnable, !enable);
            area.enable = !enable;
            showMaskTable();
            break;
        }
        case ColumnEdit: {
            ms_privary_mask_edit edit_privary;
            edit_privary.chnid = currentChannel();
            edit_privary.maskid = area.area_id;
            qMsDebug() << QString("REQUEST_FLAG_SET_PRIVACY_MASK_EDIT, channel: %1, mask id: %2").arg(edit_privary.chnid).arg(edit_privary.maskid);
            sendMessage(REQUEST_FLAG_SET_PRIVACY_MASK_EDIT, &edit_privary, sizeof(struct ms_privary_mask_edit));
            //
            m_drawItemMask->editArea(area);
            m_drawItemMask->setEnabled(true);
            m_drawItemMask->setCurrentItemSelected(true);

            PtzMaskEdit edit(this);
            int result = edit.execEdit(&area, m_privacy_mask.maxPtzZoomRatio);
            if (result == PtzMaskEdit::Accepted) {
                m_drawItemMask->setItemRatio(area.area_id, area.ratio);
                if (m_privacy_mask.colorSupport && area.fill_color != 8) {
                    m_drawItemMask->updateSingleColor(m_privacy_mask.area, area.fill_color);
                }
                showMaskTable();
            }
            if (m_maskList.contains(area.area_id)) {
                m_drawItemMask->hideArea(area.area_id);
                m_drawItemMask->cancelCurrentItem();
            } else {
                m_drawItemMask->setCurrentItemSelected(false);
                m_drawItemMask->cancelCurrentItem();
            }
            break;
        }
        case ColumnAreaEdit: {
            m_clearMask |= m_drawItemMask->updateMask(m_privacy_mask.area, m_clearMask);
            if (m_editingArea.area_id != area.area_id) {
                if (m_maskList.contains(m_editingArea.area_id) && m_editingArea.area_id != -1) {
                    m_drawItemMask->hideArea(m_editingArea.area_id);
                }
                memcpy(&m_editingArea, &area, sizeof(mask_area_ex));
            }

            ms_privary_mask_edit edit_privary;
            edit_privary.chnid = currentChannel();
            edit_privary.maskid = area.area_id;
            qMsDebug() << QString("REQUEST_FLAG_SET_PRIVACY_MASK_EDIT, channel: %1, mask id: %2").arg(edit_privary.chnid).arg(edit_privary.maskid);
            sendMessage(REQUEST_FLAG_SET_PRIVACY_MASK_EDIT, &edit_privary, sizeof(struct ms_privary_mask_edit));
            //
            m_drawItemMask->editArea(area);
            m_drawItemMask->setEnabled(true);
            m_drawItemMask->setCurrentItemSelected(true);
            break;
        }
        case ColumnDelete: {
            int id = area.area_id;
            char name[MAX_LEN_64];
            memcpy(name, area.name, sizeof(area.name));
            memset(&area, 0, sizeof(mask_area_ex));
            area.area_id = id;
            area.ratio = 1;
            snprintf(area.name, sizeof(area.name), "%s", QString("PTZ Privacy Mask %1").arg(id + 1).toStdString().c_str());

            ms_set_bit(&m_privacy_mask.delMask, id, 1);
            m_drawItemMask->clear(id);
            m_maskList.removeAll(id);
            showMaskTable();
            break;
        }
        default:
            break;
        }
    }
}

void TabPtzPrivacyMask::on_checkBox_enable_checkStateSet(int checked)
{
    qMsDebug() << checked;
    setSettingEnable(checked);
}

void TabPtzPrivacyMask::onPushButtonAddClicked()
{
    qMsDebug() << "";

    int channel = currentChannel();
    m_isReadyAdd = true;

    Q_UNUSED(channel)
}

void TabPtzPrivacyMask::addPrivacyMask()
{
    if (!qMsNvr->isNT(currentChannel()) && !m_drawItemMask->checkRegionNum(m_privacy_mask.area)) {
        ShowMessageBox(GET_TEXT("IMAGE/120007", "Only support up to 24 mask and 4 mosaic areas."));
        return;
    }
    m_clearMask |= m_drawItemMask->getMask(m_privacy_mask.area);
    if (m_privacy_mask.colorSupport && ui->comboBoxMaskColor->currentIntData() != 8) {
        m_drawItemMask->updateSingleColor(m_privacy_mask.area, ui->comboBoxMaskColor->currentIntData());
    }

    m_privacy_mask.id = currentChannel();
    m_privacy_mask.enable = ui->checkBox_enable->isChecked();
    showMaskTable();
}

void TabPtzPrivacyMask::isSupportMosaic()
{
    bool isSupport = m_privacy_mask.system_mosaic_support || qMsNvr->isNT(currentChannel());
    m_drawItemMask->setIsSupportMosaic(isSupport);
    if (isSupport) {
        ui->tableView->setMinimumHeight(212);
    } else {
        ui->tableView->setMinimumHeight(272);
    }
    ui->labelMaskColor->setVisible(isSupport);
    ui->labelRegionType->setVisible(isSupport);
    ui->comboBoxMaskColor->setVisible(isSupport);
    ui->comboBoxRegionType->setVisible(isSupport);

    ui->comboBoxMaskColor->setCurrentIndex(0);
    ui->comboBoxRegionType->setCurrentIndex(0);
    on_comboBoxRegionType_activated(0);
    on_comboBoxMaskColor_activated(0);
}

void TabPtzPrivacyMask::hideEvent(QHideEvent *event)
{
    m_drawItemMask->clearAll();
    QWidget::hideEvent(event);
}

void TabPtzPrivacyMask::on_pushButton_clear_clicked()
{
    int index = m_drawItemMask->getCurrentItemID();
    if (index == -1) {
        return;
    }
    mask_area_ex &area = m_privacy_mask.area[index];
    int id = area.area_id;
    memset(&area, 0, sizeof(mask_area_ex));
    area.area_id = id;
    area.ratio = 1;
    area.enable = false;
    snprintf(area.name, sizeof(area.name), "%s", QString("PTZ Privacy Mask %1").arg(id + 1).toStdString().c_str());

    ms_set_bit(&m_privacy_mask.delMask, id, 1);
    m_drawItemMask->clear(id);
    m_maskList.removeAll(id);
    showMaskTable();
}

void TabPtzPrivacyMask::on_pushButton_clearAll_clicked()
{
    for (int i = 0; i < m_maskNUM; ++i) {
        mask_area_ex &area = m_privacy_mask.area[i];
        int id = area.area_id;
        memset(&area, 0, sizeof(mask_area_ex));
        area.area_id = id;
        area.ratio = 1;
        area.enable = false;
        snprintf(area.name, sizeof(area.name), "%s", QString("PTZ Privacy Mask %1").arg(i + 1).toStdString().c_str());
    }
    m_privacy_mask.delMask = 0xfffffff;
    m_drawItemMask->clearAll();
    m_maskList.clear();
    m_clearMask |= m_drawItemMask->getMask(m_privacy_mask.area);

    m_privacy_mask.id = currentChannel();
    m_privacy_mask.enable = ui->checkBox_enable->isChecked();
    showMaskTable();
}

void TabPtzPrivacyMask::on_pushButton_apply_clicked()
{
    //showWait();
    waitForSaveMask(currentChannel());
    //closeWait();

    if (ui->tableView->rowCount() >= m_maskNUM) {
        m_drawItemMask->setEnabled(false);
    } else {
        m_drawItemMask->setEnabled(true);
    }
    //刚设置完马上获取会获得错误的数据，等待半秒
    QEventLoop eventLoop;
    QTimer::singleShot(500, &eventLoop, SLOT(quit()));
    eventLoop.exec();
    initializeData();
}

void TabPtzPrivacyMask::on_pushButton_back_clicked()
{
    back();
}

void TabPtzPrivacyMask::on_comboBoxRegionType_activated(int index)
{
    switch (index) {
    case 0:
        ui->comboBoxMaskColor->setEnabled(ui->checkBox_enable->isChecked());
        m_drawItemMask->setColorType(ui->comboBoxMaskColor->currentIntData());
        break;
    case 1:
        ui->comboBoxMaskColor->setEnabled(false);
        m_drawItemMask->setColorType(8);
        break;
    }
}

void TabPtzPrivacyMask::on_comboBoxMaskColor_activated(int index)
{
    m_drawItemMask->setColorType(ui->comboBoxMaskColor->itemData(index).toInt());
}
