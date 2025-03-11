#include "imagepagemask.h"
#include "ui_imagepagemask.h"
#include "EditImageRegionType.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "channelcopydialog.h"
#include "itembuttoncheckbox.h"
#include "ptzdatamanager.h"
#include <QScopedValueRollback>
#include <QTimer>

extern "C" {
#include "msg.h"
}
#define MASK_VERSION_SUPPORT_8AREAS 2402
#define MASK_VERSION_SUPPORT_8AREASC_CHAR "2.4.02"

ImagePageMask::ImagePageMask(QWidget *parent)
    : AbstractImagePage(parent)
    , ui(new Ui::ImagePageMask)
{
    ui->setupUi(this);

    ui->tableView->clear();
    QStringList headerList;
    headerList << GET_TEXT("DISKMANAGE/72022", "ID");
    headerList << GET_TEXT("COMMON/1051", "Name");
    headerList << GET_TEXT("COMMON/1052", "Type");
    headerList << GET_TEXT("COMMON/1009", "Enable");
    headerList << GET_TEXT("COMMON/1019", "Edit");
    headerList << GET_TEXT("CHANNELMANAGE/30023", "Delete");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    ui->tableView->setHeaderCheckable(false);
    ui->tableView->setItemDelegate(new ItemIconDelegate(this));

    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onTableItemClicked(int, int)));

    //delegate
    ui->tableView->setItemDelegateForColumn(ColumnEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit.png"), this));
    ui->tableView->setItemDelegateForColumn(ColumnDelete, new ItemButtonDelegate(QPixmap(":/common/common/delete.png"), this));
    ui->tableView->setItemDelegateForColumn(ColumnEnable, new ItemCheckedButtonDelegate(ItemCheckedButtonDelegate::ButtonCheckBox, this));

    //sort
    ui->tableView->setSortableForColumn(ColumnID, false);
    ui->tableView->setSortableForColumn(ColumnName, false);
    ui->tableView->setSortableForColumn(ColumnType, false);
    ui->tableView->setSortableForColumn(ColumnEnable, false);
    ui->tableView->setSortableForColumn(ColumnEdit, false);
    ui->tableView->setSortableForColumn(ColumnDelete, false);

    ui->widgetMessage->hideMessage();

    ui->comboBoxRegionType->clear();
    ui->comboBoxRegionType->beginEdit();
    ui->comboBoxRegionType->addItem(GET_TEXT("IMAGE/120003", "Mask"), 0);
    ui->comboBoxRegionType->addItem(GET_TEXT("IMAGE/120004", "Mosaic"), 1);
    ui->comboBoxRegionType->endEdit();

    m_drawItemMask = new DrawItemMaskControl(nullptr);
    setDrawItem(m_drawItemMask);
    m_drawItemMask->init();

    onLanguageChanged();
    setMaskNewVersion(false);
}

ImagePageMask::~ImagePageMask()
{
    delete ui;
}

void ImagePageMask::initializeData(int channel)
{
    m_currentRow = -1;
    m_drawItemMask->clearAll();

    m_currentChannel = channel;

    setMaskNewVersion(false);
    ui->tableView->clearContent();

    do {
        if (!isChannelConnected()) {
            ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
            break;
        }
        if (!qMsNvr->isMsCamera(m_currentChannel)) {
            ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
            break;
        }

        //MsWaitting::showGlobalWait();
        gPtzDataManager->beginGetData(m_currentChannel);
        gPtzDataManager->waitForGetCameraModelType();
        gPtzDataManager->waitForGetCameraModelInfo();
        gPtzDataManager->waitForGetPtzSupport();
        if (gPtzDataManager->isPtzSupport() && !qMsNvr->isFisheye(m_currentChannel) && gPtzDataManager->isPresetEnable()) {
            //MsWaitting::closeGlobalWait();
            ui->widgetMessage->showMessage(GET_TEXT("IMAGE/37410", "This channel just support PTZ Privacy Mask, please configure on PTZ Configuration interface."));
            break;
        } else {
            sendMessage(REQUEST_FLAG_GET_IPCIMAGE_DISPLAY, &m_currentChannel, sizeof(int));
            //m_eventLoop.exec();
            //
            // int result = 1;
            // if (qMsNvr->isFisheye(m_currentChannel)) {
            //     sendMessage(REQUEST_FLAG_GET_FISHEYE_MODE, (void *)&m_currentChannel, sizeof(int));
            //     result = m_eventLoop.exec();
            // }
            // if (result == 0 || result == -1) {
            //     ui->widgetMessage->showMessage(GET_TEXT("IMAGE/37342", "This channel does not support this function."));
            //     break;
            // }
            // if (result == -2) {
            //     ui->widgetMessage->showMessage(GET_TEXT("IMAGE/37325", "Privacy Mask takes effect only in Original display mode for Fisheye camera"));
            //     break;
            // }
            // //
            // if (result == 1) {
            //     sendMessage(REQUEST_FLAG_GET_PRIVACY_MASK, (void *)&m_currentChannel, sizeof(int));
            //     m_eventLoop.exec();
            // }
        }
        //MsWaitting::closeGlobalWait();
        ui->widgetMessage->hideMessage();
        setSettingEnable(true);
        return;
    } while (0);

    clearSetting();
    setSettingEnable(false);
    //MsWaitting::closeGlobalWait();
}

void ImagePageMask::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_FISHEYE_MODE:
        ON_RESPONSE_FLAG_GET_FISHEYE_MODE(message);
        break;
    case RESPONSE_FLAG_GET_PRIVACY_MASK:
        ON_RESPONSE_FLAG_GET_PRIVACY_MASK(message);
        break;
    case RESPONSE_FLAG_DELETE_PRIVACY_MASK:
        ON_RESPONSE_FLAG_DELETE_PRIVACY_MASK(message);
        break;
    case RESPONSE_FLAG_SET_PRIVACY_MASK:
        ON_RESPONSE_FLAG_SET_PRIVACY_MASK(message);
        break;
    case RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY:
        ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(message);
        break;
    default:
        break;
    }
}

void ImagePageMask::hideEvent(QHideEvent *event)
{
    m_drawItemMask->clearAll();
    m_drawItemMask->setEnabled(false);
    QWidget::hideEvent(event);
}

void ImagePageMask::ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(MessageReceive *message)
{
    m_eventLoop.exit();

    resp_image_display *data = reinterpret_cast<resp_image_display *>(message->data);
    if (!data) {
        qWarning() << "ImagePageRoi::ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY, data is null.";
        return;
    }
}

void ImagePageMask::ON_RESPONSE_FLAG_GET_IPC_MODEL_TYPE(MessageReceive *message)
{
    if (message->data) {
        int result = *((int *)message->data);
        if (qMsNvr->isFisheye(m_currentChannel)) {
            result = FISHEYE_IPC;
        }
        switch (result) {
        case -1:
        case -3:
        case MS_SPEEDOME:
            //MsWaitting::closeGlobalWait();
            if (isChannelConnected()) {
                ui->widgetMessage->hide();
            } else {
                ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
            }
            break;
        case FISHEYE_IPC:
            sendMessage(REQUEST_FLAG_GET_FISHEYE_MODE, (void *)&m_currentChannel, sizeof(int));
            break;
        default:
            sendMessage(REQUEST_FLAG_GET_PRIVACY_MASK, (void *)&m_currentChannel, sizeof(int));
            break;
        }
    }
}

void ImagePageMask::ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message)
{
    Q_UNUSED(message)
}

void ImagePageMask::ON_RESPONSE_FLAG_GET_PRIVACY_MASK(MessageReceive *message)
{
    m_eventLoop.exit();

    memset(&m_privacy_mask, 0, sizeof(resp_privacy_mask));
    struct resp_privacy_mask *privacy_mask = (struct resp_privacy_mask *)message->data;
    bool setColor = false;
    int currentColor = 0;
    int flag = 0;

    if (!privacy_mask) {
        qWarning() << "ImagePageMask::ON_RESPONSE_FLAG_GET_PRIVACY_MASK, data is null.";
        return;
    }

    setSettingEnable(true);
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
    if (m_privacy_mask.system_mosaic_support && !qMsNvr->isNT(m_currentChannel)) {
        m_maskNUM = MAX_MASK_AREA_NUM + MAX_MOSAIC_NUM;
    } else {
        m_maskNUM = m_privacy_mask.num;
    }
    isSupportMosaic();
    if (m_maskNUM <= MAX_MASK_AREA_NUM) {
        if (qMsNvr->isNT(m_currentChannel)) {
            ui->label_note->setText(GET_TEXT("IMAGE/162000", "Note: Support up to 8 areas."));
            ui->label_type->setText(GET_TEXT("IMAGE/120005", "Mask Color"));
        } else {
            ui->label_note->setText(GET_TEXT("IMAGE/120000", "Note: Support up to %1 Privacy Mask areas.").arg(m_maskNUM));
            ui->label_type->setText(GET_TEXT("COMMON/1052", "Type"));
        }
    } else {
        ui->label_note->setText(GET_TEXT("IMAGE/120007", "Note: Support up to 24 mask and 4 mosaic areas."));
        ui->label_type->setText(GET_TEXT("IMAGE/120005", "Mask Color"));
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
    qDebug() << getImageSdkVersion(privacy_mask->sdkversion, strlen(privacy_mask->sdkversion));
    if (m_privacy_mask.cgiSdkFlag == 1 || compareSdkVersions(privacy_mask->sdkversion, MASK_VERSION_SUPPORT_8AREASC_CHAR) >= 0) {
        //sdk version >= 2.4.02 support 8 areas
        setMaskNewVersion(true);
        flag = 1;
    } else {
        setMaskNewVersion(false);
    }
    currentColor = privacy_mask->area[0].fill_color;

    m_drawItemMask->clearAll();
    for (int i = 0; i < m_maskNUM; ++i) {
        if (privacy_mask->area[i].area_width == 0 && privacy_mask->area[i].area_height == 0) {
            continue;
        }
        if (!setColor) {
            setColor = true;
            currentColor = privacy_mask->area[i].fill_color;
        }
        const mask_area_ex &area = privacy_mask->area[i];
        m_drawItemMask->addMask(area);
    }
    m_drawItemMask->refreshstack();

    ui->checkBox_enable->setChecked(privacy_mask->enable);
    on_checkBox_enable_clicked(privacy_mask->enable);

    if (!flag && currentColor >= 3) {
        currentColor = 0;
    }
    if (ui->comboBox_type->findData(currentColor) >= 0 && !privacy_mask->system_mosaic_support) {
        ui->comboBox_type->setCurrentIndexFromData(currentColor);
        on_comboBox_type_activated(currentColor);
    } else {
        ui->comboBox_type->setCurrentIndex(0);
        on_comboBox_type_activated(0);
    }

    refreshTable();
}

void ImagePageMask::setMaskEnable(bool enable)
{
    Q_UNUSED(enable);
}

void ImagePageMask::onLanguageChanged()
{
    ui->label_mask->setText(GET_TEXT("CHANNELMANAGE/30005", "Privacy Mask"));
    ui->checkBox_enable->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->labelRegionType->setText(GET_TEXT("IMAGE/37220", "Region Type"));
    ui->labelRegion->setText(GET_TEXT("ANPR/103021", "Region"));
    ui->pushButtonDeleteAll->setText(GET_TEXT("PLAYBACK/80119", "Delete All"));
    ui->pushButtonAdd->setText(GET_TEXT("COMMON/1000", "Add"));
    ui->pushButtonClear->setText(GET_TEXT("PTZCONFIG/36039", "Clear"));

    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));
}

void ImagePageMask::on_checkBox_enable_clicked(bool checked)
{
    m_drawItemMask->setEnabled(checked);
    int typeNumber = ui->comboBox_type->count();
    ui->comboBoxRegionType->setEnabled(checked);
    ui->comboBox_type->setEnabled(checked && typeNumber > 1 && !ui->comboBoxRegionType->currentIntData());
    ui->pushButtonDeleteAll->setEnabled(checked);
    ui->pushButtonAdd->setEnabled(checked && ui->tableView->rowCount() < m_maskNUM);
    ui->pushButtonClear->setEnabled(checked);
    setTableEnabledStatus();
}

void ImagePageMask::on_pushButtonDeleteAll_clicked()
{
    for (int i = 0; i < m_maskNUM; ++i) {
        mask_area_ex &area = m_privacy_mask.area[i];
        int id = area.area_id;
        memset(&area, 0, sizeof(mask_area_ex));
        area.area_id = id;
        area.ratio = 1;
        area.enable = false;
    }
    m_drawItemMask->clearAll();
    //MSHN-7569
    //QT：图像-隐私遮挡，点击clear all后，Type颜色建议一律变成White，同步Web.
    if (ui->comboBoxRegionType->currentIntData() != 1) {
        ui->comboBox_type->setCurrentIndex(0);
        on_comboBox_type_activated(ui->comboBox_type->currentIndex());
    }
    on_pushButtonAdd_clicked();
}

void ImagePageMask::on_comboBox_type_activated(int index)
{
    m_drawItemMask->setColorType(ui->comboBox_type->itemData(index).toInt());
}

void ImagePageMask::on_pushButton_copy_clicked()
{
    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(m_currentChannel);
    int result = copy.exec();
    if (result == QDialog::Accepted) {
        QList<int> copyList = copy.checkedList();
        //MsWaitting::showGlobalWait();
        waitForSaveMask(m_currentChannel);
        qDebug() << "ImagePageMask::on_pushButton_copy_clicked, copy begin.";

        for (int i = 0; i < copyList.size(); ++i) {
            int channel = copyList.at(i);
            if (channel != m_currentChannel) {
                gPtzDataManager->beginGetData(channel);
                gPtzDataManager->waitForGetCameraModelInfo();
                gPtzDataManager->waitForGetPtzSupport();
                if (!qMsNvr->isFisheye(channel) && gPtzDataManager->isPtzSupport() && gPtzDataManager->isPresetEnable()) {
                    qDebug() << "channel:" << channel << "doesn't support copy.";
                } else {
                    //                    //copy前清空区域
                    waitForSaveMask(channel);
                    qDebug() << "channel:" << channel << "support copy.";
                }
            }
        }
        qDebug() << "ImagePageMask::on_pushButton_copy_clicked, copy end.";
        //MsWaitting::closeGlobalWait();
    }
}

void ImagePageMask::on_pushButton_apply_clicked()
{
    //MsWaitting::showGlobalWait();
    waitForSaveMask(m_currentChannel);
    //MsWaitting::closeGlobalWait();

    initializeData(m_currentChannel);
}

void ImagePageMask::on_pushButton_back_clicked()
{
    back();
}

void ImagePageMask::onTableItemClicked(int row, int column)
{
    if (ui->checkBox_enable->isChecked()) {
        int enable = 0;
        int id = ui->tableView->itemText(row, ColumnID).toInt() - 1;
        switch (column) {
        case ColumnEnable:
            if (ui->checkBox_enable->isChecked()) {
                enable = ui->tableView->itemData(row, column, ItemCheckedRole).toInt();
                if (enable) {
                    ui->tableView->setItemData(row, column, false, ItemCheckedRole);
                } else {
                    ui->tableView->setItemData(row, column, true, ItemCheckedRole);
                }
                mask_area_ex &area = m_privacy_mask.area[id];
                area.enable = ui->tableView->itemData(row, ColumnEnable, ItemCheckedRole).toInt();
            }
            break;
        case ColumnDelete: {
            enable = ui->checkBox_enable->isChecked();
            if (!enable) {
                break;
            }
            mask_area_ex &area = m_privacy_mask.area[id];
            area.start_x = 0;
            area.start_y = 0;
            area.area_width = 0;
            area.area_height = 0;
            area.enable = false;

            ui->tableView->removeRow(row);
            m_drawItemMask->clear(id);

            m_privacy_mask.id = m_currentChannel;
            m_privacy_mask.enable = ui->checkBox_enable->isChecked();
            ui->pushButtonAdd->setEnabled(ui->checkBox_enable->isChecked()
                                          && ui->tableView->rowCount() < m_maskNUM);
            break;
        }
        case ColumnEdit: {
            mask_area_ex &area = m_privacy_mask.area[id];
            if (area.fill_color != 8) {
                EditImageRegionType editImageRegionType(this);
                editImageRegionType.setColor(area.fill_color);
                m_drawItemMask->setSelectedItem(ui->tableView->itemIntValue(row, ColumnID) - 1);
                int result = editImageRegionType.exec();
                if (result == QDialog::Accepted) {
                    area.fill_color = editImageRegionType.getColor();
                    if (m_privacy_mask.colorSupport && area.fill_color != 8) {
                        m_drawItemMask->updateSingleColor(m_privacy_mask.area, area.fill_color);
                    }
                }
                m_drawItemMask->clearSelected();
                refreshTable();
            }
            break;
        }
        default:
            break;
        }
    }
}

void ImagePageMask::setMaskNewVersion(bool enable)
{
    if (enable) {
        ui->label_note->show();
        m_drawItemMask->setMaxItemCount(m_maskNUM);
        ui->comboBox_type->clear();
        if (QString(m_privacy_mask.colorList).isEmpty()) {
            ui->comboBox_type->addItem(GET_TEXT("IMAGE/37322", "White"), 0);
            ui->comboBox_type->addItem(GET_TEXT("IMAGE/37323", "Black"), 1);
            ui->comboBox_type->addItem(GET_TEXT("IMAGE/37324", "Blue"), 2);
            ui->comboBox_type->addItem(GET_TEXT("IMAGE/37403", "Yellow"), 3);
            ui->comboBox_type->addItem(GET_TEXT("IMAGE/37404", "Green"), 4);
            ui->comboBox_type->addItem(GET_TEXT("IMAGE/37405", "Brown"), 5);
            ui->comboBox_type->addItem(GET_TEXT("IMAGE/37406", "Red"), 6);
            ui->comboBox_type->addItem(GET_TEXT("IMAGE/37407", "Purple"), 7);
        } else {
            QStringList colorList = QString(m_privacy_mask.colorList).split(",");
            for (int i = 0; i < colorList.size(); ++i) {
                int index = colorList.at(i).toInt();
                switch (index) {
                case 0:
                    ui->comboBox_type->addItem(GET_TEXT("IMAGE/37322", "White"), 0);
                    break;
                case 1:
                    ui->comboBox_type->addItem(GET_TEXT("IMAGE/37323", "Black"), 1);
                    break;
                case 2:
                    ui->comboBox_type->addItem(GET_TEXT("IMAGE/37324", "Blue"), 2);
                    break;
                case 3:
                    ui->comboBox_type->addItem(GET_TEXT("IMAGE/37403", "Yellow"), 3);
                    break;
                case 4:
                    ui->comboBox_type->addItem(GET_TEXT("IMAGE/37404", "Green"), 4);
                    break;
                case 5:
                    ui->comboBox_type->addItem(GET_TEXT("IMAGE/37405", "Brown"), 5);
                    break;
                case 6:
                    ui->comboBox_type->addItem(GET_TEXT("IMAGE/37406", "Red"), 6);
                    break;
                case 7:
                    ui->comboBox_type->addItem(GET_TEXT("IMAGE/37407", "Purple"), 7);
                    break;
                default:
                    break;
                }
            }
        }
    } else {
        ui->label_note->hide();
        m_drawItemMask->setMaxItemCount(4);
        ui->comboBox_type->clear();
        ui->comboBox_type->addItem(GET_TEXT("IMAGE/37322", "White"), 0);
        ui->comboBox_type->addItem(GET_TEXT("IMAGE/37323", "Black"), 1);
        ui->comboBox_type->addItem(GET_TEXT("IMAGE/37324", "Blue"), 2);
    }
}

void ImagePageMask::refreshTable()
{
    int row = 0;
    ui->tableView->clearContent();
    for (int i = 0; i < m_maskNUM; i++) {
        const mask_area_ex &area = m_privacy_mask.area[i];
        if (area.area_height == 0 && area.area_width == 0) {
            continue;
        }

        ui->tableView->setItemIntValue(row, ColumnID, area.area_id + 1);
        QString maskName = QString(GET_TEXT("IMAGE/37402", "Mask%1").arg(area.area_id + 1));
        ui->tableView->setItemText(row, ColumnName, maskName);
        if (m_privacy_mask.system_mosaic_support) {
            ui->tableView->setItemText(row, ColumnType, getType(area.fill_color));
            if (area.fill_color == 8) {
                ui->tableView->setItemText(row, ColumnEdit, "-");
            }
        }
        ui->tableView->setItemData(row, ColumnEnable, area.enable, ItemCheckedRole);

        row++;
    }
    ui->pushButtonAdd->setEnabled(ui->checkBox_enable->isChecked()
                                  && ui->tableView->rowCount() < m_maskNUM);

    setTableEnabledStatus();
}

void ImagePageMask::ON_RESPONSE_FLAG_DELETE_PRIVACY_MASK(MessageReceive *message)
{
    //MsWaitting::closeGlobalWait();
    if (!message->data) {
        return;
    }

    int id = 0;
    int resp = *(int *)message->data;
    if (resp == 0) {
        id = ui->tableView->itemText(m_currentRow, ColumnID).toInt() - 1;
        mask_area_ex &area = m_privacy_mask.area[id];
        area.area_height = 0;
        area.area_width = 0;
        area.enable = 0;
        ui->tableView->removeRow(m_currentRow);

        m_drawItemMask->clearAll();
        for (int i = 0; i < m_maskNUM; ++i) {
            const mask_area_ex &area = m_privacy_mask.area[i];
            if (area.area_height == 0 && area.area_width == 0) {
                continue;
            }

            m_drawItemMask->addMask(area);
        }
        m_drawItemMask->refreshstack();
    }
}

void ImagePageMask::setTableEnabledStatus()
{
    int rowCount = ui->tableView->rowCount();
    bool enabled = ui->checkBox_enable->isChecked();

    for (int i = 0; i < rowCount; i++) {
        ui->tableView->setRowEnable(i, enabled);
    }
}

void ImagePageMask::isSupportMosaic()
{
    bool support = m_privacy_mask.system_mosaic_support;
    ui->tableView->setColumnHidden(ColumnType, !support);
    ui->tableView->setColumnHidden(ColumnEdit, !support);
    ui->labelRegionType->setVisible(support);
    ui->comboBoxRegionType->setVisible(support);
    ui->comboBoxRegionType->setCurrentIndex(0);
    on_comboBoxRegionType_activated(0);

    m_drawItemMask->setIsSupportMosaic(support);
    m_drawItemMask->setMaxItemCount(m_maskNUM);
    if (support) {
        ui->tableView->setColumnWidth(ColumnID, 100);
        ui->tableView->setColumnWidth(ColumnName, 160);
        ui->tableView->setColumnWidth(ColumnType, 200);
        ui->tableView->setColumnWidth(ColumnEnable, 160);
        ui->tableView->setColumnWidth(ColumnEdit, 160);
        ui->tableView->setColumnWidth(ColumnDelete, 160);
    } else {
        ui->tableView->setColumnWidth(ColumnID, 100);
        ui->tableView->setColumnWidth(ColumnName, 250);
        ui->tableView->setColumnWidth(ColumnEnable, 250);
        ui->tableView->setColumnWidth(ColumnDelete, 250);
    }
}

int ImagePageMask::waitForSaveMask(int channel)
{
    m_drawItemMask->updateMask(m_privacy_mask.area);
    m_privacy_mask.enable = ui->checkBox_enable->isChecked();
    resp_privacy_mask set_mask;
    memcpy(&set_mask, &m_privacy_mask, sizeof(resp_privacy_mask));
    set_mask.id = channel;
    set_mask.areaMaskEnable = 0xfffffff;
    set_mask.delMask = 0xfffffff;
    set_mask.cgiSdkFlag = 1;
    qDebug() << QString("PtzPrivacyMaskPage::ON_RESPONSE_FLAG_SET_PRIVACY_MASK, channel: %1, enable: %2, num: %3, cgiSdkFlag: %4, sdkversion: %5")
                    .arg(set_mask.id)
                    .arg(set_mask.enable)
                    .arg(set_mask.num)
                    .arg(set_mask.cgiSdkFlag)
                    .arg(set_mask.sdkversion);
    for (int i = 0; i < m_maskNUM; ++i) {
        const mask_area_ex &area = set_mask.area[i];
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

    return 1;
}

QString ImagePageMask::getType(int type)
{
    //"0,1,2,3,4,5,6,7"  0=white,1=black,2=blue,3=yellow,4=green,5=brown,6=red,7=violet,8=mosaic
    QString typeName;
    switch (type) {
    case 0:
        typeName = GET_TEXT("IMAGE/37322", "White");
        break;
    case 1:
        typeName = GET_TEXT("IMAGE/37323", "Black");
        break;
    case 2:
        typeName = GET_TEXT("IMAGE/37324", "Blue");
        break;
    case 3:
        typeName = GET_TEXT("IMAGE/37403", "Yellow");
        break;
    case 4:
        typeName = GET_TEXT("IMAGE/37404", "Green");
        break;
    case 5:
        typeName = GET_TEXT("IMAGE/37405", "Brown");
        break;
    case 6:
        typeName = GET_TEXT("IMAGE/37406", "Red");
        break;
    case 7:
        typeName = GET_TEXT("IMAGE/37407", "Purple");
        break;
    case 8:
        typeName = GET_TEXT("IMAGE/120004", "Mosaic");
    }
    return typeName;
}

void ImagePageMask::ON_RESPONSE_FLAG_SET_PRIVACY_MASK(MessageReceive *message)
{
    int res = 0;
    if (message->data) {
        res = *(int *)message->data;
    }
    m_eventLoop.exit(res);
}

void ImagePageMask::clearSetting()
{
    ui->checkBox_enable->setChecked(false);
    on_checkBox_enable_clicked(false);
    m_privacy_mask.system_mosaic_support = true;
    isSupportMosaic();
    ui->label_type->setText(GET_TEXT("IMAGE/120005", "Mask Color"));
    ui->comboBox_type->setEnabled(false);
}

void ImagePageMask::setSettingEnable(bool enable)
{
    ui->checkBox_enable->setEnabled(enable);
    ui->pushButton_copy->setEnabled(enable);
    ui->pushButton_apply->setEnabled(enable);
}

void ImagePageMask::on_pushButtonAdd_clicked()
{
    if (!qMsNvr->isNT(m_currentChannel) && !m_drawItemMask->checkRegionNum(m_privacy_mask.area)) {
        ShowMessageBox(GET_TEXT("IMAGE/120007", "Only support up to 24 mask and 4 mosaic areas."));
        return;
    }
    m_drawItemMask->getMask(m_privacy_mask.area);
    if (m_privacy_mask.colorSupport && ui->comboBoxRegionType->currentIntData() != 8) {
        m_drawItemMask->updateSingleColor(m_privacy_mask.area, ui->comboBoxRegionType->currentIntData());
    }
    m_privacy_mask.id = m_currentChannel;
    m_privacy_mask.enable = ui->checkBox_enable->isChecked();
    refreshTable();
}

void ImagePageMask::on_pushButtonClear_clicked()
{
    int index = m_drawItemMask->getCurrentItemID();
    if (index != -1) {
        mask_area_ex &area = m_privacy_mask.area[m_drawItemMask->getCurrentItemID()];
        area.start_x = 0;
        area.start_y = 0;
        area.area_width = 0;
        area.area_height = 0;
        area.enable = false;
        int deleteValue = area.area_id + 1;
        for (int i = 0; i < ui->tableView->rowCount(); ++i) {
            if (ui->tableView->itemIntValue(i, ColumnID) == deleteValue) {
                ui->tableView->removeRow(i);
                break;
            }
        }
        m_drawItemMask->clear();
        m_privacy_mask.id = m_currentChannel;
        m_privacy_mask.enable = ui->checkBox_enable->isChecked();
    }
    ui->pushButtonAdd->setEnabled(ui->checkBox_enable->isChecked()
                                  && ui->tableView->rowCount() < m_maskNUM);
}

void ImagePageMask::on_comboBoxRegionType_activated(int index)
{
    switch (index) {
    case 0:
        ui->comboBox_type->setEnabled(true);
        m_drawItemMask->setColorType(ui->comboBox_type->currentIntData());
        break;
    case 1:
        ui->comboBox_type->setEnabled(false);
        m_drawItemMask->setColorType(8);
        break;
    }
}
