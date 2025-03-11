#include "imagepageroi.h"
#include "ui_imagepageroi.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "channelcopydialog.h"
#include <QScopedValueRollback>

#define MAX_ROI_AREA_NUM_OLD 3
#define MAX_ROI_AREA_NUM_NEW 8
#define ROI_VERSION_SUPPORT_8AREAS 2402
#define ROI_VERSION_SUPPORT_8AREAS_CHAR "2.4.02"

ImagePageRoi::ImagePageRoi(QWidget *parent)
    : AbstractImagePage(parent)
    , ui(new Ui::ImagePageRoi)
{
    ui->setupUi(this);

    //tableview
    m_uncheckedPixmap = QPixmap(":/common/common/checkbox-unchecked.png");
    m_checkedPixmap = QPixmap(":/common/common/checkbox-checked.png");

    QStringList headerList;
    headerList << GET_TEXT("DISKMANAGE/72022", "ID");
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

    ui->tableView->setColumnWidth(ColumnID, 100);
    ui->tableView->setColumnWidth(ColumnName, 250);
    ui->tableView->setColumnWidth(ColumnEnable, 250);
    ui->tableView->setColumnWidth(ColumnDelete, 250);

    ui->comboBox_VideoStream->clear();
    ui->comboBox_VideoStream->addItem(GET_TEXT("SYSTEMNETWORK/71208", "Primary Stream"), 0);
    ui->comboBox_VideoStream->addItem(GET_TEXT("SYSTEMNETWORK/71209", "Secondary Stream"), 1);

    ui->checkBox_enable->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->label_mask->setText(GET_TEXT("IMAGE/37006", "ROI"));
    ui->label_clear->setText(GET_TEXT("ANPR/103021", "Region"));
    ui->label_VideoStream->setText(GET_TEXT("IMAGE/37326", "Video Stream"));

    ui->pushButtonDeleteAll->setText(GET_TEXT("PLAYBACK/80119", "Delete All"));
    ui->pushButtonAdd->setText(GET_TEXT("COMMON/1000", "Add"));
    ui->pushButtonClear->setText(GET_TEXT("PTZCONFIG/36039", "Clear"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));

    ui->label_note->setText(GET_TEXT("IMAGE/37409", "ote: Support up to 8 ROI areas."));

    m_drawView = new DrawView(this);
    m_drawScene = new DrawSceneRoi(this);
    m_drawView->setScene(m_drawScene);
    m_drawScene->setMaxItemCount(MAX_ROI_AREA_NUM_NEW);

    setMaskNewVersion(false);
}

ImagePageRoi::~ImagePageRoi()
{
    m_drawScene->clearAll();
    delete ui;
}

void ImagePageRoi::initializeData(int channel)
{
    m_currentRow = -1;
    m_sdkVersion = 0;
    m_channel = channel;

    setMaskNewVersion(false);
    m_drawScene->clearAll();
    ui->tableView->clearContent();

    setDrawWidget(m_drawView);
    ui->checkBox_enable->setChecked(false);
    setSettingEnable(false);

    ui->widgetMessage->hideMessage();
    if (!isChannelConnected()) {
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
        return;
    }
    if (!qMsNvr->isMsCamera(m_channel)) {
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        return;
    }
    //MsWaitting::showGlobalWait();
    sendMessage(REQUEST_FLAG_GET_ROI, &m_channel, sizeof(int));
}

void ImagePageRoi::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_ROI:
        ON_RESPONSE_FLAG_GET_ROI(message);
        break;
    case RESPONSE_FLAG_DELETE_ROI_AREA:
        ON_RESPONSE_FLAG_DELETE_ROI_AREA(message);
        break;
    case RESPONSE_FLAG_GET_IPC_DIGITPOS_ZOOM:
        ON_RESPONSE_FLAG_GET_IPC_DIGITPOS_ZOOM(message);
        break;
    default:
        break;
    }
}

void ImagePageRoi::hideEvent(QHideEvent *event)
{
    m_drawScene->clearAll();
    QWidget::hideEvent(event);
}

void ImagePageRoi::closeWait()
{
    //MsWaitting::closeGlobalWait();
}

void ImagePageRoi::ON_RESPONSE_FLAG_GET_IPC_DIGITPOS_ZOOM(MessageReceive *message)
{
    ms_digitpos_zoom_state *data = (ms_digitpos_zoom_state *)message->data;
    if (!data) {
        qWarning() << "ImagePageRoi::ON_RESPONSE_FLAG_GET_IPC_DIGITPOS_ZOOM, data is null.";
        //MsWaitting::closeGlobalWait();
        return;
    }

    if (data->state) {
        //MsWaitting::closeGlobalWait();
        ShowMessageBox(GET_TEXT("IMAGE/37416", "ROI settings are not supported at this magnification."));
        m_drawScene->clearAll();
        sendMessage(REQUEST_FLAG_GET_ROI, &m_channel, sizeof(int));
        return;
    }
    apply();
}

void ImagePageRoi::ON_RESPONSE_FLAG_GET_ROI(MessageReceive *message)
{
    qDebug() << "====ImagePageRoi::ON_RESPONSE_FLAG_GET_ROI====";
    struct set_image_roi *roiMask = (struct set_image_roi *)message->data;
    if (!roiMask) {
        qWarning() << "ImagePageRoi::ON_RESPONSE_FLAG_GET_ROI";
        return;
    }

    memcpy(&m_roiMask, roiMask, sizeof(set_image_roi));
    if (compareSdkVersions(roiMask->sdkversion, ROI_VERSION_SUPPORT_8AREAS_CHAR) >= 0) {
        //sdk version >= 2.4.02 support 8 areas
        setMaskNewVersion(true);
        m_maxArea = MAX_ROI_AREA_NUM_NEW;
    } else {
        m_maxArea = MAX_ROI_AREA_NUM_OLD;
    }

    QString text;
    for (int i = 0; i < m_maxArea; ++i) {
        const image_roi_area &area = m_roiMask.area[i];
        text.append(QString("\nindex: %1, enable: %2, left: %3, top: %4, right: %5, bottom: %6")
                        .arg(i)
                        .arg(area.enable)
                        .arg(area.left)
                        .arg(area.top)
                        .arg(area.right)
                        .arg(area.bottom));
    }
    qMsDebug() << text;

    on_comboBox_VideoStream_activated();
    ui->checkBox_enable->setChecked(roiMask->enable);
    setSettingEnable(true);
    on_checkBox_enable_clicked(roiMask->enable);
    //MsWaitting::closeGlobalWait();
}

void ImagePageRoi::saveRoiInfo()
{
    int offset = 0;
    if (ui->comboBox_VideoStream->currentIndex()) {
        offset = MAX_ROI_AREA_NUM_NEW;
    }
    m_drawScene->updateRotArea(m_roiMask.area + offset, MAX_ROI_AREA_NUM_NEW);
    struct set_image_roi info;

    memcpy(&info, &m_roiMask, sizeof(struct set_image_roi));
    info.chanid = m_copyChannelList.takeFirst();
    info.enable = ui->checkBox_enable->isChecked();
    QString text;
    for (int i = 0; i < m_maxArea; ++i) {
        const image_roi_area &area = m_roiMask.area[i];
        text.append(QString("\nindex: %1, enable: %2, left: %3, top: %4, right: %5, bottom: %6")
                        .arg(i)
                        .arg(area.enable)
                        .arg(area.left)
                        .arg(area.top)
                        .arg(area.right)
                        .arg(area.bottom));
    }
    qMsDebug() << text;
    sendMessage(REQUEST_FLAG_SET_ROI, &info, sizeof(struct set_image_roi));
}

void ImagePageRoi::on_pushButton_copy_clicked()
{
    m_copyChannelList.clear();

    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(m_channel);
    int result = copy.exec();
    if (result == QDialog::Accepted) {
        m_copyChannelList = copy.checkedList();
        QTimer::singleShot(0, this, SLOT(on_pushButton_apply_clicked()));
    }
}

void ImagePageRoi::on_pushButton_apply_clicked()
{
    sendMessage(REQUEST_FLAG_GET_IPC_DIGITPOS_ZOOM, &m_channel, sizeof(int));
    //MsWaitting::showGlobalWait();
}

void ImagePageRoi::apply()
{
    if (m_copyChannelList.isEmpty()) {
        m_copyChannelList.append(m_channel);
    }
    do {
        saveRoiInfo();
        qApp->processEvents();
    } while (!m_copyChannelList.isEmpty());
    initializeData(m_channel);
    QTimer::singleShot(1000, this, SLOT(closeWait()));
}

void ImagePageRoi::on_pushButton_back_clicked()
{
    back();
}

void ImagePageRoi::on_pushButtonDeleteAll_clicked()
{
    int type = ui->comboBox_VideoStream->currentIndex();
    int offset = 0;
    if (type == 1) {
        offset = MAX_ROI_AREA_NUM_NEW;
    }
    for (int i = 0; i < m_maxArea; ++i) {
        image_roi_area &area = m_roiMask.area[i + offset];
        memset(&area, 0, sizeof(image_roi_area));
        area.enable = false;
    }
    m_drawScene->clearAll();
    on_pushButtonAdd_clicked();
}

void ImagePageRoi::on_checkBox_enable_clicked(bool checked)
{
    ui->pushButtonDeleteAll->setDisabled(!checked);
    ui->pushButtonAdd->setEnabled(ui->checkBox_enable->isChecked() && ui->tableView->rowCount() < m_maxArea);
    ui->pushButtonClear->setDisabled(!checked);
    ui->comboBox_VideoStream->setDisabled(!checked);
    m_drawScene->setEnabled(checked);
    setTableEnabledStatus();
}

void ImagePageRoi::on_comboBox_VideoStream_activated()
{
    //if(!ui->checkBox_enable->isChecked())
    //return;

    setAreaData();
    refreshTable();
}

void ImagePageRoi::onTableItemClicked(int row, int column)
{
    int enable = 0;
    int offset = 0;
    if (ui->comboBox_VideoStream->currentIndex() == 1) {
        offset = MAX_ROI_AREA_NUM_NEW;
    }
    int id = ui->tableView->itemText(row, ColumnID).toInt() - 1;
    m_currentRow = row;
    switch (column) {
    case ColumnEnable: {
        if (ui->checkBox_enable->isChecked()) {
            enable = ui->tableView->itemData(row, column, ItemCheckedRole).toInt();
            if (enable) {
                ui->tableView->setItemData(row, column, false, ItemCheckedRole);
            } else {
                ui->tableView->setItemData(row, column, true, ItemCheckedRole);
            }
            image_roi_area &area = m_roiMask.area[id + offset];
            area.enable = ui->tableView->itemData(row, ColumnEnable, ItemCheckedRole).toInt();
        }
        break;
    }
    case ColumnDelete: {
        enable = ui->checkBox_enable->isChecked();
        if (!enable) {
            break;
        }
        image_roi_area &area = m_roiMask.area[id + offset];
        area.left = 0;
        area.top = 0;
        area.right = 0;
        area.bottom = 0;
        area.enable = false;
        ui->tableView->removeRow(row);
        m_drawScene->clear(id);
        ui->pushButtonAdd->setEnabled(ui->checkBox_enable->isChecked() && ui->tableView->rowCount() < m_maxArea);
        break;
    }
    default:
        break;
    }
}

void ImagePageRoi::refreshTable()
{
    int row = 0;
    int count = 0;
    int videoStream = ui->comboBox_VideoStream->currentIndex();

    ui->tableView->clearContent();
    int offset = 0;
    int width = 0;
    int height = 0;

    if (videoStream == 1) {
        offset = MAX_ROI_AREA_NUM_NEW;
    }

    for (int i = 0; i < m_maxArea; i++) {
        const image_roi_area &area = m_roiMask.area[i + offset];
        width = m_drawScene->getRoiWidth(area);
        if (!width)
            continue;
        height = m_drawScene->getRoiHeight(area);
        if (!height)
            continue;
        count++;
    }

    if (!count) {
        ui->pushButtonAdd->setEnabled(ui->checkBox_enable->isChecked() && ui->tableView->rowCount() < m_maxArea);
        return;
    }

    ui->tableView->setRowCount(count);
    for (int i = 0; i < m_maxArea; i++) {
        const image_roi_area &area = m_roiMask.area[i + offset];

        width = m_drawScene->getRoiWidth(area);
        height = m_drawScene->getRoiHeight(area);
        if (!height || !width)
            continue;

        ui->tableView->setItemIntValue(row, ColumnID, i + 1);
        QString maskName = QString(GET_TEXT("IMAGE/37408", "ROI%1").arg(i + 1));
        ui->tableView->setItemText(row, ColumnName, maskName);
        if (area.enable) {
            ui->tableView->setItemData(row, ColumnEnable, true, ItemCheckedRole);
        } else {
            ui->tableView->setItemData(row, ColumnEnable, false, ItemCheckedRole);
        }
        row++;
    }
    ui->pushButtonAdd->setEnabled(ui->checkBox_enable->isChecked() && ui->tableView->rowCount() < m_maxArea);
    ui->pushButtonClear->clearFocus();
    setTableEnabledStatus();
}

void ImagePageRoi::ON_RESPONSE_FLAG_DELETE_ROI_AREA(MessageReceive *message)
{
    //MsWaitting::closeGlobalWait();
    if (!message->data)
        return;

    int id = 0;
    int resp = *(int *)message->data;
    int type = ui->comboBox_VideoStream->currentIndex();
    int offset = 0;

    if (type == 1)
        offset = MAX_ROI_AREA_NUM_NEW;

    if (resp == 0) {
        id = ui->tableView->itemText(m_currentRow, ColumnID).toInt() - 1 + offset;
        m_roiMask.area[id].enable = 0;
        m_roiMask.area[id].left = 0;
        m_roiMask.area[id].right = 0;
        m_roiMask.area[id].top = 0;
        m_roiMask.area[id].bottom = 0;
        ui->tableView->removeRow(m_currentRow);

        setAreaData();
    }
}

void ImagePageRoi::setSettingEnable(bool enable)
{
    ui->checkBox_enable->setEnabled(enable);
    ui->pushButton_copy->setEnabled(enable);
    ui->pushButton_apply->setEnabled(enable);
    if (enable) {

    } else {
        on_checkBox_enable_clicked(enable);
    }
}

void ImagePageRoi::setMaskNewVersion(bool enable)
{
    if (enable) {
        ui->label_note->show();
        m_drawScene->setMaxItemCount(MAX_ROI_AREA_NUM_NEW);
    } else {
        ui->label_note->hide();
        m_drawScene->setMaxItemCount(MAX_ROI_AREA_NUM_OLD);
    }
}

void ImagePageRoi::setAreaData()
{
    int width = 0, height = 0;
    int offset = 0;
    int type = ui->comboBox_VideoStream->currentIndex();

    if (type == 1) {
        offset = MAX_ROI_AREA_NUM_NEW;
    }

    m_drawScene->clearAll();
    for (int i = 0; i < m_maxArea; ++i) {
        const image_roi_area &area = m_roiMask.area[i + offset];
        width = m_drawScene->getRoiWidth(area);
        if (!width) {
            continue;
        }
        height = m_drawScene->getRoiHeight(area);
        if (!height) {
            continue;
        }
        m_drawScene->addRoiArea(area, i);
    }
    m_drawScene->refreshstack();
}

void ImagePageRoi::setTableEnabledStatus()
{
    int rowCount = ui->tableView->rowCount();
    bool enabled = ui->checkBox_enable->isChecked();

    for (int i = 0; i < rowCount; i++) {
        ui->tableView->setRowEnable(i, enabled);
    }
}

void ImagePageRoi::on_pushButtonAdd_clicked()
{
    struct set_image_roi info;

    memcpy(&info, &m_roiMask, sizeof(struct set_image_roi));
    info.chanid = m_channel;
    info.enable = ui->checkBox_enable->isChecked();
    int offset = 0;

    if (ui->comboBox_VideoStream->currentIndex()) {
        offset = MAX_ROI_AREA_NUM_NEW;
    }
    m_drawScene->getRoiArea(info.area + offset, MAX_ROI_AREA_NUM_NEW);

    //check is enable
    int rowCount = ui->tableView->rowCount();
    int id = 0;
    for (int row = 0; row < rowCount; row++) {
        id = ui->tableView->itemText(row, ColumnID).toInt() - 1;
        info.area[id + offset].enable = ui->tableView->itemData(row, ColumnEnable, ItemCheckedRole).toInt();
    }

    memcpy(&m_roiMask, &info, sizeof(struct set_image_roi));
    refreshTable();
}

void ImagePageRoi::on_pushButtonClear_clicked()
{
    if (m_drawScene->getCurrentItemID() != -1) {
        int offset = 0;
        if (ui->comboBox_VideoStream->currentIndex() == 1) {
            offset = MAX_ROI_AREA_NUM_NEW;
        }
        image_roi_area &area = m_roiMask.area[m_drawScene->getCurrentItemID() + offset];
        memset(&area, 0, sizeof(image_roi_area));
        int id = m_drawScene->getCurrentItemID() + 1 ;
        for (int i = 0; i < ui->tableView->rowCount(); ++i) {
            if (ui->tableView->itemIntValue(i, ColumnID) == id) {
                ui->tableView->removeRow(i);
                break;
            }
        }
        m_drawScene->clear();
    }
    ui->pushButtonAdd->setEnabled(ui->checkBox_enable->isChecked() && ui->tableView->rowCount() < m_maxArea);
}
